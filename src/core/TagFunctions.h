#pragma once
#include "NeoVSID.h"

using namespace vsid;

namespace vsid {
void NeoVSID::RegisterTagActions()
{
    PluginSDK::Tag::TagActionDefinition tagDef;

    tagDef.name = "confirmCFL";
	tagDef.description = "confirm the CFL.";
	tagDef.requiresInput = false;
    confirmCFLId_ = tagInterface_->RegisterTagAction(tagDef);
    
    tagDef.name = "confirmRWY";
	tagDef.description = "confirm the RWY.";
    confirmRwyId_ = tagInterface_->RegisterTagAction(tagDef);
    
    tagDef.name = "confirmSID";
	tagDef.description = "confirm the SID.";
    confirmSidId_ = tagInterface_->RegisterTagAction(tagDef);
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
    
    if (dataManager_->pilotExists(callsign) == false) {
        dataManager_->addPilot(callsign);
    }

	bool printToTag = true;
    Pilot pilot = dataManager_->getPilotByCallsign(callsign);

    if (actionId == confirmCFLId_)
    {
        updateCFL({ callsign, pilot, controllerDataAPI_, tagInterface_, cflId_, printToTag });
	}

    if (actionId == confirmRwyId_)
    {
        updateRWY({ callsign, pilot, controllerDataAPI_, tagInterface_, rwyId_, printToTag });
	}

    if (actionId == confirmSidId_)
    {
        updateSID({ callsign, pilot, controllerDataAPI_, tagInterface_, sidId_, printToTag });
	}
}
}  // namespace vsid