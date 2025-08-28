#pragma once
#include <chrono>
#include <format>
#include <string>

#include "TagItemsColor.h"
#include "NeoVSID.h"
#include "../utils/Format.h"

#ifdef DEV
#define LOG_DEBUG(loglevel, message) logger_->log(loglevel, message)
#else
#define LOG_DEBUG(loglevel, message) void(0)
#endif

using namespace vsid::tagitems;

namespace vsid {
void NeoVSID::RegisterTagItems()
{
    PluginSDK::Tag::TagItemDefinition tagDef;

    tagDef.name = "CFL";
    tagDef.defaultValue = "---";
    std::string tagID = tagInterface_->RegisterTagItem(tagDef);
    cflId_ = tagID;
    
    tagDef.name = "RWY";
    tagDef.defaultValue = "---";
    tagID = tagInterface_->RegisterTagItem(tagDef);
    rwyId_ = tagID;

    tagDef.name = "SID";
    tagDef.defaultValue = "------";
    tagID = tagInterface_->RegisterTagItem(tagDef);
    sidId_ = tagID;
    
    tagDef.name = "ALERT";
    tagDef.defaultValue = "";
    tagID = tagInterface_->RegisterTagItem(tagDef);
    alertsId_ = tagID;

    tagDef.name = "REQUEST";
    tagDef.defaultValue = "";
    tagID = tagInterface_->RegisterTagItem(tagDef);
    requestId_ = tagID;
}


// TAG ITEM UPDATE FUNCTIONS
void NeoVSID::updateCFL(tagUpdateParam param) {
    Tag::TagContext tagContext;
	tagContext.callsign = param.callsign;
    int vsidCfl = param.pilot.cfl; // Get vsid assigned CFL

    int cfl = 0;
	std::optional<ControllerData::ControllerDataModel> controllerData = controllerDataAPI_->getByCallsign(param.callsign);
    if (controllerData.has_value()) {
        cfl = controllerData->clearedFlightLevel;
    }
    std::string cfl_string;

    if (cfl == 0) {
        cfl_string = std::to_string(vsidCfl);
    }
    else {
        cfl_string = std::to_string(cfl);
    }

	cfl_string = formatCFL(cfl_string, 5000); // Transition Altitude hardcoded to 5000 ft for now
    tagContext.colour = Color::colorizeCfl(cfl, vsidCfl);

    param.tagInterface_->UpdateTagValue(param.tagId_, cfl_string, tagContext);
}

void NeoVSID::updateRWY(tagUpdateParam param) {
    Tag::TagContext tagContext;
    tagContext.callsign = param.callsign;
    std::string vsidRwy = param.pilot.rwy;
    std::optional<Flightplan::Flightplan> fp = flightplanAPI_->getByCallsign(param.callsign);
    std::string rwy = "";
    bool isDepRwy = false;

    if (fp.has_value()) {
        rwy = fp->route.depRunway;
        auto airportConfig = airportAPI_->getConfigurationByIcao(fp->origin);
        if (!airportConfig.has_value()) {
            return; // No airport configuration found
		}

        for (const auto& rwy_ : airportConfig->depRunways)
        {
            if (vsidRwy == rwy_)
                isDepRwy = true;
        }
    }

    tagContext.colour = Color::colorizeRwy(rwy, vsidRwy, isDepRwy);
    if (rwy == "") rwy = vsidRwy;
    tagInterface_->UpdateTagValue(param.tagId_, rwy, tagContext);
}

void NeoVSID::updateSID(tagUpdateParam param) {
    Tag::TagContext tagContext;
    tagContext.callsign = param.callsign;
    std::string vsidSid = param.pilot.sid;
    std::optional<Flightplan::Flightplan> fp = flightplanAPI_->getByCallsign(param.callsign);
    std::string sid = "";
    
    if (fp.has_value()) {
        sid = fp->route.sid;
    }

    tagContext.colour = Color::colorizeSid(sid, vsidSid);
    if (sid == "") sid = vsidSid;
    tagInterface_->UpdateTagValue(param.tagId_, sid, tagContext);
}

inline void NeoVSID::updateAlert(const std::string& callsign)
{
    Tag::TagContext tagContext;
    tagContext.callsign = callsign;
    tagContext.colour = Color::colorizeAlert();

    std::string alert = "";

	std::optional<Aircraft::Aircraft> aircraft = aircraftAPI_->getByCallsign(callsign);
    if (!aircraft.has_value()) {
        tagInterface_->UpdateTagValue(alertsId_, alert, tagContext);
		return; // Aircraft not found
    }

    int aircraftSpeed = aircraft->position.groundSpeed;
    int aircraftHeading = aircraft->position.reportedHeading;
    int aircraftTrackHeading = aircraft->position.trackHeading;
    int aircraftAlitude = aircraft->position.altitude;

	if (aircraftAlitude > ALERT_MAX_ALTITUDE) {
        tagInterface_->UpdateTagValue(alertsId_, alert, tagContext);
		return;
	}

    Aircraft::TransponderMode aircraftTransponder = aircraft->transponderMode;

    ControllerData::GroundStatus groundStatus = ControllerData::GroundStatus::None;
	std::optional<ControllerData::ControllerDataModel> controllerData = controllerDataAPI_->getByCallsign(callsign);
    if (controllerData.has_value()) {
        groundStatus = controllerData->groundStatus;
    }

    bool isReversing = false;
    int headingDiff = std::abs(aircraftTrackHeading - aircraftHeading);
    if (headingDiff >= 120 && headingDiff <= 240) { // Heading threshold 60°
        isReversing = true;
    }

    bool isStopped = aircraft->position.stopped;

    if (groundStatus == ControllerData::GroundStatus::Dep && aircraftTransponder == Aircraft::TransponderMode::Standby) {
        alert = "XPDR STDBY";
    }
    else if (isStopped && groundStatus == ControllerData::GroundStatus::Dep) {
        alert = "STAT RPA";
    }
    else if (aircraftSpeed > 0 && isReversing && groundStatus != ControllerData::GroundStatus::Push) {
        alert = "NO PUSH CLR";
    }
    else if (aircraftSpeed > 35 && groundStatus < ControllerData::GroundStatus::Dep) {
        alert = "NO TKOF CLR";
    }
    else if (aircraftSpeed > 5 && !isReversing && groundStatus < ControllerData::GroundStatus::Taxi) {
        alert = "NO TAXI CLR";
    }

    tagInterface_->UpdateTagValue(alertsId_, alert, tagContext);
}

inline void NeoVSID::updateRequest(const std::string& callsign, const std::string& request)
{
    Tag::TagContext tagContext;
    tagContext.callsign = callsign;
    tagContext.colour = Color::colorizeRequest();

    std::string text = "";
    
    std::pair<std::string, size_t> data = getRequestAndIndex(callsign);
    std::string previousRequest = data.first;
	size_t index = data.second;

    if (request == "ReqNoReq") {
        if (previousRequest == "clearance") {
            requestingClearance.erase(requestingClearance.begin() + index);
        }
        else if (previousRequest == "push") {
            requestingPush.erase(requestingPush.begin() + index);
        }
        else if (previousRequest == "taxi") {
            requestingTaxi.erase(requestingTaxi.begin() + index);
        }
    }
    else if (request == "ReqClearance") {
        if (previousRequest == "push") {
            requestingPush.erase(requestingPush.begin() + index);
        }
        else if (previousRequest == "taxi") {
            requestingTaxi.erase(requestingTaxi.begin() + index);
        }
        else if (previousRequest == "clearance") {
            requestingClearance.erase(requestingClearance.begin() + index);
		}
        requestingClearance.push_back(callsign);
        text = "R" + std::to_string(requestingClearance.size()) + "C";
    }
    else if (request == "ReqPush") {
        if (previousRequest == "push") {
            requestingPush.erase(requestingPush.begin() + index);
        }
        else if (previousRequest == "taxi") {
            requestingTaxi.erase(requestingTaxi.begin() + index);
        }
        else if (previousRequest == "clearance") {
            requestingClearance.erase(requestingClearance.begin() + index);
        }
        requestingPush.push_back(callsign);
        text = "R" + std::to_string(requestingPush.size()) + "P";
    }
    else if (request == "ReqTaxi") {
        if (previousRequest == "push") {
            requestingPush.erase(requestingPush.begin() + index);
        }
        else if (previousRequest == "taxi") {
            requestingTaxi.erase(requestingTaxi.begin() + index);
        }
        else if (previousRequest == "clearance") {
            requestingClearance.erase(requestingClearance.begin() + index);
        }
        requestingTaxi.push_back(callsign);
        text = "R" + std::to_string(requestingTaxi.size()) + "T";
    }
    else {
        if (previousRequest == "push") {
            text = "R" + std::to_string(index + 1) + "P";
        }
        else if (previousRequest == "taxi") {
			text = "R" + std::to_string(index + 1) + "T";
        }
        else if (previousRequest == "clearance") {
			text = "R" + std::to_string(index + 1) + "C";
        }
	}
    updateAllRequests();
	tagInterface_->UpdateTagValue(requestId_, text, tagContext);
}

inline void NeoVSID::updateAllRequests()
{
    std::vector<std::string> callsigns = requestingClearance;
    callsigns.insert(callsigns.end(), requestingPush.begin(), requestingPush.end());
    callsigns.insert(callsigns.end(), requestingTaxi.begin(), requestingTaxi.end());
    for (const auto& callsign : callsigns) {
        Tag::TagContext tagContext;
        tagContext.callsign = callsign;
        tagContext.colour = Color::colorizeRequest();
        std::pair<std::string, size_t> data = getRequestAndIndex(callsign);
        std::string previousRequest = data.first;
        size_t index = data.second;
        std::string text = "";
        if (previousRequest == "push") {
            text = "R" + std::to_string(index + 1) + "P";
        }
        else if (previousRequest == "taxi") {
            text = "R" + std::to_string(index + 1) + "T";
        }
        else if (previousRequest == "clearance") {
            text = "R" + std::to_string(index + 1) + "C";
        }
        tagInterface_->UpdateTagValue(requestId_, text, tagContext);
    }
}

// Update all tag items for all pilots
void NeoVSID::UpdateTagItems() {
	callsignsScope = dataManager_->getAllDepartureCallsigns();
    
    for (auto &callsign : callsignsScope)
    {
		UpdateTagItems(callsign);
    }
}

// Update all tag items for a specific callsign
void NeoVSID::UpdateTagItems(std::string callsign) {
    if (dataManager_->pilotExists(callsign) == false) {
		dataManager_->addPilot(callsign);
	}
	Pilot pilot = dataManager_->getPilotByCallsign(callsign);
    if (pilot.empty()) return;


	updateCFL({ callsign, pilot, controllerDataAPI_, tagInterface_, cflId_});
	updateRWY({ callsign, pilot, controllerDataAPI_, tagInterface_, rwyId_});
	updateSID({ callsign, pilot, controllerDataAPI_, tagInterface_, sidId_});
    updateAlert(callsign);
}
}  // namespace vsid