#pragma once

// #include <wtypes.h>

#include <chrono>
#include <format>
#include <string>

#include "TagItemsColor.h"
#include "NeoVSID.h"

using namespace vsid::tagitems;

namespace vsid {
enum itemType {

};

void NeoVSID::RegisterTagItems()
{
    PluginSDK::Tag::TagItemDefinition tagDef;

    tagDef.name = "CFL";
    tagDef.defaultValue = "130";

	std::string tagID = tagInterface_->RegisterTagItem(tagDef);
    cflId_ = tagID;
}

void NeoVSID::UpdateTagItems() {
	// Update the CFL tag item with the current value
    // How to get a vector of all callsigns inside departure list ?
    std::vector<std::string> callsigns;

    for (auto callsign : callsigns)
    {
		// Update CFL value in tag item
        int cfl = controllerDataAPI_->getByCallsign(callsign)->clearedFlightLevel;
		std::string cfl_string = std::to_string(cfl);
		Tag::TagContext tagContext;
		tagContext.callsign = callsign;

        tagInterface_->UpdateTagValue(cflId_, cfl_string, tagContext);
    }
}
}  // namespace vsid