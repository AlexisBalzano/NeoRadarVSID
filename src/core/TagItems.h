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


    }
}  // namespace vsid