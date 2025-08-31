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

    tagDef.name = "requestMenu";
    tagDef.description = "open the request menu";
    requestMenuId_ = tagInterface_->RegisterTagAction(tagDef);

    PluginSDK::Tag::DropdownDefinition dropdownDef;
    dropdownDef.title = "REQUEST";
    dropdownDef.width = 75;
    dropdownDef.maxHeight = 150;

    PluginSDK::Tag::DropdownComponent dropdownComponent;

    dropdownComponent.id = "ReqNoReq";
    dropdownComponent.type = PluginSDK::Tag::DropdownComponentType::Button;
    dropdownComponent.text = "NoReq";
    dropdownComponent.requiresInput = false;
    dropdownDef.components.push_back(dropdownComponent);

    dropdownComponent.id = "ReqClearance";
    dropdownComponent.type = PluginSDK::Tag::DropdownComponentType::Button;
    dropdownComponent.text = "ReqClr";
    dropdownDef.components.push_back(dropdownComponent);

    dropdownComponent.id = "ReqPush";
    dropdownComponent.type = PluginSDK::Tag::DropdownComponentType::Button;
    dropdownComponent.text = "ReqPush";
    dropdownDef.components.push_back(dropdownComponent);

    dropdownComponent.id = "ReqTaxi";
    dropdownComponent.type = PluginSDK::Tag::DropdownComponentType::Button;
    dropdownComponent.text = "ReqTaxi";
    dropdownDef.components.push_back(dropdownComponent);


    tagInterface_->SetActionDropdown(requestMenuId_, dropdownDef);
}

void NeoVSID::OnTagAction(const PluginSDK::Tag::TagActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

    std::string input;
	if (event->userInput) input = event->userInput.value();
    TagProcessing(event->callsign, event->actionId, input);
}

void NeoVSID::OnTagDropdownAction(const PluginSDK::Tag::DropdownActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

    updateRequest(event->callsign, event->componentId);
}

void NeoVSID::TagProcessing(const std::string &callsign, const std::string &actionId, const std::string &userInput)
{
    if (!dataManager_->pilotExists(callsign)) return;
    
    if (dataManager_->pilotExists(callsign) == false) {
        dataManager_->addPilot(callsign);
    }

    Pilot pilot = dataManager_->getPilotByCallsign(callsign);

    if (actionId == confirmCFLId_)
    {
        updateCFL({ callsign, pilot, controllerDataAPI_, tagInterface_, cflId_ });
	}

    if (actionId == confirmRwyId_)
    {
        updateRWY({ callsign, pilot, controllerDataAPI_, tagInterface_, rwyId_ });
	}

    if (actionId == confirmSidId_)
    {
        updateSID({ callsign, pilot, controllerDataAPI_, tagInterface_, sidId_ });
	}
}
}  // namespace vsid