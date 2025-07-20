#pragma once

// #include <wtypes.h>

#include <chrono>
#include <format>
#include <string>

#include "TagItemsColor.h"
#include "NeoVSID.h"
#include "../utils/Format.h"

using namespace vsid::tagitems;

namespace vsid {
enum itemType {

};

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

void NeoVSID::UpdateTagItems() {
	// Update the CFL tag item with the current value
    // 
	// Get a vector of all callsigns inside departure list for active airports
    std::vector<std::string> callsigns;
    callsigns = dataManager_->getAllDepartureCallsigns();

    for (auto &callsign : callsigns)
    {
		UpdateTagItems(callsign);
    }
}

void NeoVSID::UpdateTagItems(std::string callsign) {
    Tag::TagContext tagContext;
    tagContext.callsign = callsign;

	Pilot pilot = dataManager_->getPilotByCallsign(callsign);

    // Update CFL value in tag item
	int vsidCfl = pilot.cfl; // Get vsid assigned CFL
    int cfl = controllerDataAPI_->getByCallsign(callsign)->clearedFlightLevel;
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

    tagInterface_->UpdateTagValue(cflId_, cfl_string, tagContext);


    // Update RWY value in tag item
    std::string vsidRwy = pilot.rwy;
    std::optional<Flightplan::Flightplan> fp = flightplanAPI_->getByCallsign(callsign);
    std::string rwy = fp->route.depRunway;

    if (rwy == "") {
        //TODO: Set RWY value to vsidRwy
    }

    std::vector<std::string> depRwys = airportAPI_->getConfigurationByIcao(fp->origin)->depRunways;
    bool isDepRwy = false;
    for (auto rwy_ : depRwys)
    {
        if (vsidRwy == rwy_)
            isDepRwy = true;
    }

    tagContext.colour = Color::colorizeRwy(rwy, vsidRwy, isDepRwy);
    tagInterface_->UpdateTagValue(rwyId_, vsidRwy, tagContext);

    // Update SID value in tag item
    std::string vsidSid = pilot.sid;
    std::string sid = fp->route.sid;

    if (sid == "")
    {
        //TODO: Set SID value to vsidSid
    }

    tagContext.colour = Color::colorizeSid(sid, vsidSid);
    tagInterface_->UpdateTagValue(sidId_, vsidSid, tagContext);
}


}  // namespace vsid