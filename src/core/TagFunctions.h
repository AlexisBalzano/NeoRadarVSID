#pragma once

#include "NeoVSID.h"

using namespace vsid;
//using namespace vsid::core;
//using namespace vsid::com;

namespace vsid {

void NeoVSID::RegisterTagActions()
{
    PluginSDK::Tag::TagActionDefinition tagDef;
    tagDef.name = "confirmCFL";
	tagDef.description = "confirm the CFL based on departure.";
	tagDef.requiresInput = false; // pas de valeur à entrer (just cliquer sur le tag)
    confirmCFLId_ = tagInterface_->RegisterTagAction(tagDef);
}

void NeoVSID::OnTagAction(const PluginSDK::Tag::TagActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

    TagProcessing(event->callsign, event->actionId, event->userInput);
}

void NeoVSID::OnTagDropdownAction(const PluginSDK::Tag::DropdownActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

}

void NeoVSID::TagProcessing(const std::string &callsign, const std::string &actionId, const std::string &userInput)
{
    if (!dataManager_->pilotExists(callsign)) return;

    if (actionId == confirmCFLId_)
    {
        UpdateTagItems(callsign);
	}
}

}  // namespace vacdm