#pragma once

#include "NeoVSID.h"

using namespace vsid;
//using namespace vsid::core;
//using namespace vsid::com;

namespace vsid {

void NeoVSID::RegisterTagActions()
{
    
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
    
}

}  // namespace vacdm