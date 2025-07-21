#pragma once

#include <chrono>
#include <format>
#include <string>

#include "TagItemsColor.h"
#include "NeoVSID.h"
#include "../utils/Format.h"

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
}


// TAG ITEM UPDATE FUNCTIONS
void NeoVSID::updateCFL(tagUpdateParam param) {
    Tag::TagContext tagContext;
	tagContext.callsign = param.callsign;
    int vsidCfl = param.pilot.cfl; // Get vsid assigned CFL
    int cfl = param.controllerDataAPI_->getByCallsign(param.callsign)->clearedFlightLevel;
    std::string cfl_string;

    if (cfl == 0) {
        cfl_string = std::to_string(vsidCfl);
        //TODO: Set CFL value to vsidCFL
    }
    else {
        cfl_string = std::to_string(cfl);
    }

    cfl_string = formatCFL(cfl_string);
    tagContext.colour = Color::colorizeCfl(cfl, vsidCfl);

    param.tagInterface_->UpdateTagValue(param.tagId_, cfl_string, tagContext);
}

void NeoVSID::updateRWY(tagUpdateParam param) {
    Tag::TagContext tagContext;
    tagContext.callsign = param.callsign;
    std::string vsidRwy = param.pilot.rwy;
    std::optional<Flightplan::Flightplan> fp = flightplanAPI_->getByCallsign(param.callsign);
    std::string rwy = fp->route.depRunway;


    std::vector<std::string> depRwys = airportAPI_->getConfigurationByIcao(fp->origin)->depRunways;
    bool isDepRwy = false;
    for (const auto& rwy_ : depRwys)
    {
        if (vsidRwy == rwy_)
            isDepRwy = true;
    }
    tagContext.colour = Color::colorizeRwy(rwy, vsidRwy, isDepRwy);
    if (rwy == "") {
        //TODO: Set RWY value to vsidRwy
        rwy = vsidRwy;
    }
    tagInterface_->UpdateTagValue(param.tagId_, rwy, tagContext);
}

void NeoVSID::updateSID(tagUpdateParam param) {
    Tag::TagContext tagContext;
    tagContext.callsign = param.callsign;
    std::string vsidSid = param.pilot.sid;
    std::optional<Flightplan::Flightplan> fp = flightplanAPI_->getByCallsign(param.callsign);
    std::string sid = fp->route.sid;

    tagContext.colour = Color::colorizeSid(sid, vsidSid);
    if (sid == "")
    {
        //TODO: Set SID value to vsidSid
        sid = vsidSid;
    }
    tagInterface_->UpdateTagValue(param.tagId_, sid, tagContext);
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

	updateCFL({ callsign, pilot, controllerDataAPI_, tagInterface_, cflId_ });
	updateRWY({ callsign, pilot, controllerDataAPI_, tagInterface_, rwyId_ });
	updateSID({ callsign, pilot, controllerDataAPI_, tagInterface_, sidId_ });

}


}  // namespace vsid