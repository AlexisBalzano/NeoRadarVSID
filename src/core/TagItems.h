#pragma once
#include <chrono>
#include <format>
#include <string>

#include "NeoVSID.h"
#include "../utils/Format.h"

#ifdef DEV
#define LOG_DEBUG(loglevel, message) logger_->log(loglevel, message)
#else
#define LOG_DEBUG(loglevel, message) void(0)
#endif

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
    int vsidCfl = param.pilot.cfl;

    int cfl = 0;
    std::optional<ControllerData::ControllerDataModel> controllerData = param.controllerDataAPI_->getByCallsign(param.callsign);
    if (controllerData.has_value()) {
        cfl = controllerData->clearedFlightLevel;
    }
    std::string cfl_string = (cfl == 0) ? std::to_string(vsidCfl) : std::to_string(cfl);
    cfl_string = formatCFL(cfl_string, dataManager_->getTransAltitude(param.pilot.oaci));
    tagContext.colour = colorizeCfl(cfl, vsidCfl);

    updateTagValueIfChanged(param.callsign, param.tagId_, cfl_string, tagContext);
}

void NeoVSID::updateRWY(tagUpdateParam param) {
    Tag::TagContext tagContext;
    tagContext.callsign = param.callsign;
    std::string vsidRwy = param.pilot.rwy;
    std::optional<Flightplan::Flightplan> fp = flightplanAPI_->getByCallsign(param.callsign);
    std::string rwy;
    bool isDepRwy = false;

    if (fp.has_value()) {
        rwy = fp->route.depRunway;
        auto airportConfig = airportAPI_->getConfigurationByIcao(fp->origin);
        if (!airportConfig.has_value()) {
            return;
        }
        for (const auto& rwy_ : airportConfig->depRunways) {
            if (vsidRwy == rwy_) {
                isDepRwy = true;
            }
        }
    }

    tagContext.colour = colorizeRwy(rwy, vsidRwy, isDepRwy);
    if (rwy.empty()) rwy = vsidRwy;
    updateTagValueIfChanged(param.callsign, param.tagId_, rwy, tagContext);
}

void NeoVSID::updateSID(tagUpdateParam param) {
    Tag::TagContext tagContext;
    tagContext.callsign = param.callsign;
    std::string vsidSid = param.pilot.sid;
    std::optional<Flightplan::Flightplan> fp = flightplanAPI_->getByCallsign(param.callsign);
    std::string sid;
    if (fp.has_value()) {
        sid = fp->route.sid;
    }
    tagContext.colour = colorizeSid(sid, vsidSid);
    if (sid.empty()) sid = vsidSid;
    updateTagValueIfChanged(param.callsign, param.tagId_, sid, tagContext);
}

inline void NeoVSID::updateAlert(const std::string& callsign)
{
    Tag::TagContext tagContext;
    tagContext.callsign = callsign;
    tagContext.colour = colorizeAlert();

    std::string alert;

    std::optional<Aircraft::Aircraft> aircraft = aircraftAPI_->getByCallsign(callsign);
    if (!aircraft.has_value()) {
        updateTagValueIfChanged(callsign, alertsId_, alert, tagContext);
        return;
    }

    int aircraftSpeed = aircraft->position.groundSpeed;
    int aircraftHeading = aircraft->position.reportedHeading;
    int aircraftTrackHeading = aircraft->position.trackHeading;
    int aircraftAltitude = aircraft->position.altitude;

    if (aircraftAltitude > dataManager_->getAlertMaxAltitude()) {
        updateTagValueIfChanged(callsign, alertsId_, alert, tagContext);
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
    if (headingDiff >= 120 && headingDiff <= 240) {
        isReversing = true;
    }

    bool isStopped = aircraft->position.stopped;
    bool onGround = aircraft->position.onGround;

    if (groundStatus == ControllerData::GroundStatus::Dep && aircraftTransponder == Aircraft::TransponderMode::Standby) {
        alert = "XPDR STDBY";
        tagContext.backgroundColour = dataManager_->getColor(vsid::ColorName::XPDRSTDBY);
    }
    else if (isStopped && onGround && groundStatus == ControllerData::GroundStatus::Dep) {
        alert = "STAT RPA";
        tagContext.backgroundColour = dataManager_->getColor(vsid::ColorName::STATRPA);
    }
    else if (aircraftSpeed > 0 && isReversing && groundStatus != ControllerData::GroundStatus::Push) {
        alert = "NO PUSH CLR";
        tagContext.backgroundColour = dataManager_->getColor(vsid::ColorName::NOPUSH);
    }
    else if (aircraftSpeed > 35 && groundStatus < ControllerData::GroundStatus::Dep) {
        alert = "NO TKOF CLR";
        tagContext.backgroundColour = dataManager_->getColor(vsid::ColorName::NOTKOFF);
    }
    else if (aircraftSpeed > 5 && !isReversing && groundStatus < ControllerData::GroundStatus::Taxi) {
        alert = "NO TAXI CLR";
        tagContext.backgroundColour = dataManager_->getColor(vsid::ColorName::NOTAXI);
    }

    updateTagValueIfChanged(callsign, alertsId_, alert, tagContext);
}

inline void NeoVSID::updateRequest(const std::string& callsign, const std::string& request)
{
    Tag::TagContext tagContext;
    tagContext.callsign = callsign;
    tagContext.colour = colorizeRequest();

    std::string text;

    {
        std::lock_guard<std::mutex> lock(requestsMutex);
        std::pair<std::string, size_t> data = getRequestAndIndex(callsign);
        std::string previousRequest = data.first;
        size_t index = data.second;

        if (request == "ReqNoReq") {
            if (previousRequest == "clearance") requestingClearance.erase(requestingClearance.begin() + index);
            else if (previousRequest == "push") requestingPush.erase(requestingPush.begin() + index);
            else if (previousRequest == "taxi") requestingTaxi.erase(requestingTaxi.begin() + index);
        }
        else if (request == "ReqClearance") {
            if (previousRequest == "push") requestingPush.erase(requestingPush.begin() + index);
            else if (previousRequest == "taxi") requestingTaxi.erase(requestingTaxi.begin() + index);
            else if (previousRequest == "clearance") requestingClearance.erase(requestingClearance.begin() + index);
            requestingClearance.push_back(callsign);
            text = "R" + std::to_string(requestingClearance.size()) + "C";
        }
        else if (request == "ReqPush") {
            if (previousRequest == "push") requestingPush.erase(requestingPush.begin() + index);
            else if (previousRequest == "taxi") requestingTaxi.erase(requestingTaxi.begin() + index);
            else if (previousRequest == "clearance") requestingClearance.erase(requestingClearance.begin() + index);
            requestingPush.push_back(callsign);
            text = "R" + std::to_string(requestingPush.size()) + "P";
        }
        else if (request == "ReqTaxi") {
            if (previousRequest == "push") requestingPush.erase(requestingPush.begin() + index);
            else if (previousRequest == "taxi") requestingTaxi.erase(requestingTaxi.begin() + index);
            else if (previousRequest == "clearance") requestingClearance.erase(requestingClearance.begin() + index);
            requestingTaxi.push_back(callsign);
            text = "R" + std::to_string(requestingTaxi.size()) + "T";
        }
        else {
            if (previousRequest == "push") text = "R" + std::to_string(index + 1) + "P";
            else if (previousRequest == "taxi") text = "R" + std::to_string(index + 1) + "T";
            else if (previousRequest == "clearance") text = "R" + std::to_string(index + 1) + "C";
        }
    }

    updateAllRequests();
    updateTagValueIfChanged(callsign, requestId_, text, tagContext);
}

inline void NeoVSID::updateAllRequests()
{
	std::lock_guard<std::mutex> lock(requestsMutex);
    std::vector<std::string> callsigns = requestingClearance;
    callsigns.insert(callsigns.end(), requestingPush.begin(), requestingPush.end());
    callsigns.insert(callsigns.end(), requestingTaxi.begin(), requestingTaxi.end());
    for (const auto& callsign : callsigns) {
        Tag::TagContext tagContext;
        tagContext.callsign = callsign;
        tagContext.colour = colorizeRequest();
        std::pair<std::string, size_t> data = getRequestAndIndex(callsign);
        std::string previousRequest = data.first;
        size_t index = data.second;
        std::string text;
        if (previousRequest == "push") text = "R" + std::to_string(index + 1) + "P";
        else if (previousRequest == "taxi") text = "R" + std::to_string(index + 1) + "T";
        else if (previousRequest == "clearance") text = "R" + std::to_string(index + 1) + "C";
        updateTagValueIfChanged(callsign, requestId_, text, tagContext);
    }
}

void NeoVSID::UpdateTagItems() {
    callsignsScope = dataManager_->getAllDepartureCallsigns();
    for (auto &callsign : callsignsScope)
    {
        UpdateTagItems(callsign);
    }
}

void NeoVSID::UpdateTagItems(std::string callsign) {
    if (dataManager_->pilotExists(callsign) == false) {
        dataManager_->addPilot(callsign);
    }
    Pilot pilot = dataManager_->getPilotByCallsign(callsign);
    if (pilot.empty()) return;

    updateCFL({ callsign, pilot, controllerDataAPI_, tagInterface_, cflId_ });
    updateRWY({ callsign, pilot, controllerDataAPI_, tagInterface_, rwyId_ });
    updateSID({ callsign, pilot, controllerDataAPI_, tagInterface_, sidId_ });
    updateAlert(callsign);
}

// COLOR FUNCTIONS
Color NeoVSID::colorizeCfl(const int& cfl, const int& vsidCfl) {
    if (cfl == 0) return dataManager_->getColor(ColorName::UNCONFIRMED);
    if (cfl == vsidCfl) return dataManager_->getColor(ColorName::CONFIRMED);
    else return dataManager_->getColor(ColorName::DEVIATION);
}

Color NeoVSID::colorizeRwy(const std::string& rwy, const std::string& vsidRwy, const bool& isDepRwy) {
    if (rwy == "") return dataManager_->getColor(ColorName::UNCONFIRMED);
    if (rwy == vsidRwy && isDepRwy) return dataManager_->getColor(ColorName::CONFIRMED);
    else return dataManager_->getColor(ColorName::DEVIATION);
}

Color NeoVSID::colorizeSid(const std::string& sid, const std::string& vsidSid) {
    if (vsidSid == "CHECKFP" && sid == "") return dataManager_->getColor(ColorName::CHECKFP);
    if (sid == "") return dataManager_->getColor(ColorName::UNCONFIRMED);
    if (sid == vsidSid) return dataManager_->getColor(ColorName::CONFIRMED);
    else return dataManager_->getColor(ColorName::DEVIATION);
}

Color NeoVSID::colorizeAlert() {
    return dataManager_->getColor(ColorName::ALERTTEXT);
}

Color NeoVSID::colorizeRequest() {
    return dataManager_->getColor(ColorName::REQUESTTEXT);
}

}  // namespace vsid