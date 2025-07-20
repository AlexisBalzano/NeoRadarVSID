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
    tagDef.defaultValue = "-----";
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

void NeoVSID::UpdateTagItems(std::string Callsign) {
    // Update CFL value in tag item
	int vsidCfl = dataManager_->getPilotByCallsign(Callsign).cfl; // Get vsid assigned CFL
    int cfl = controllerDataAPI_->getByCallsign(Callsign)->clearedFlightLevel;
    std::string cfl_string;

    if (cfl == 0) {
        cfl_string = std::to_string(vsidCfl);
        //TODO: Set CFL value to vsidCFL
	}
    else {
        cfl_string = std::to_string(cfl);
	}
    
    cfl_string = formatCFL(cfl_string);
    Tag::TagContext tagContext;
    tagContext.callsign = Callsign;
    tagContext.colour = Color::colorizeCfl(cfl, vsidCfl);


    tagInterface_->UpdateTagValue(cflId_, cfl_string, tagContext);
}


}  // namespace vsid