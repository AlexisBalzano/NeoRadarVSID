#include <algorithm>
#include <string>

#include "utils/String.h"
#include "NeoVSID.h"

using namespace vsid;

namespace vsid {
void NeoVSID::RegisterCommand() {
    try
    {
        CommandProvider_ = std::make_shared<NeoVSIDCommandProvider>(this, logger_, chatAPI_, fsdAPI_);

        PluginSDK::Chat::CommandDefinition definition;
        definition.name = "vsid version";
        definition.description = "return NeoVSID version";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        versionCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);
        
        definition.name = "vsid help";
        definition.description = "display all the available vSID commands";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        helpCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid auto";
        definition.description = "toggle automode state";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        autoModeCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid airports";
        definition.description = "return active airport list";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        airportsCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid pilots";
        definition.description = "return pilot list";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        pilotsCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid reset";
        definition.description = "reset active airport & pilot lists";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        resetCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid remove";
        definition.description = "remove pilot from pilot list";
        definition.lastParameterHasSpaces = false;

        Chat::CommandParameter parameter;
        parameter.name = "callsign";
        parameter.type = Chat::ParameterType::String;
        parameter.required = true;
        definition.parameters.push_back(parameter);

        removeCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);
    }
    catch (const std::exception& ex)
    {
        logger_->error("Error registering command: " + std::string(ex.what()));
    }
}

inline void NeoVSID::unegisterCommand()
{
    if (CommandProvider_)
    {
        chatAPI_->unregisterCommand(versionCommandId_);
        chatAPI_->unregisterCommand(helpCommandId_);
        chatAPI_->unregisterCommand(autoModeCommandId_);
        chatAPI_->unregisterCommand(airportsCommandId_);
        chatAPI_->unregisterCommand(pilotsCommandId_);
        chatAPI_->unregisterCommand(resetCommandId_);
        chatAPI_->unregisterCommand(removeCommandId_);
        CommandProvider_.reset();
	}
}

Chat::CommandResult NeoVSIDCommandProvider::Execute( const std::string &commandId, const std::vector<std::string> &args)
{
    if (commandId == neoVSID_->versionCommandId_)
    {
        std::string message = "Version " + std::string(PLUGIN_VERSION);
        neoVSID_->DisplayMessage(message);
        return { true, std::nullopt };
	}
    else if (commandId == neoVSID_->helpCommandId_)
    {
        neoVSID_->DisplayMessage(".vsid version");
        neoVSID_->DisplayMessage(".vsid auto");
        neoVSID_->DisplayMessage(".vsid airports");
        neoVSID_->DisplayMessage(".vsid pilots");
        neoVSID_->DisplayMessage(".vsid reset");
        neoVSID_->DisplayMessage(".vsid remove <CALLSIGN>");
    }
    else if (commandId == neoVSID_->autoModeCommandId_)
    {
        neoVSID_->switchAutoModeState();
        std::string message = "Automode set to " + (neoVSID_->getAutoModeState() ? std::string("True") : std::string("False"));
        neoVSID_->DisplayMessage(message);
		return { true, std::nullopt };
    }
    else if (commandId == neoVSID_->airportsCommandId_)
    {
		std::vector<std::string> activeAirports = neoVSID_->GetDataManager()->getActiveAirports();
        if (activeAirports.empty()) {
            neoVSID_->DisplayMessage("No active airports found.");
        } else {
            std::string message = "Active Airports: ";
            for (const auto& airport : activeAirports) {
                message += airport + ", ";
            }
            // Remove the trailing comma and space
            message = message.substr(0, message.size() - 2);
            neoVSID_->DisplayMessage(message);
		}
		return { true, std::nullopt };
    }
    else if (commandId == neoVSID_->pilotsCommandId_)
    {
        std::vector<Pilot> activePilots = neoVSID_->GetDataManager()->getPilots();
        if (activePilots.empty()) {
            neoVSID_->DisplayMessage("No active pilots found.");
        }
        else {
			int count = 0;
            std::string message = "Active Pilots: ";
            for (const auto& pilot : activePilots) {
				++count;
                message += pilot.callsign + ", ";
                if (count >= 4) {
                    // Remove the trailing comma and space
                    message = message.substr(0, message.size() - 2);
                    neoVSID_->DisplayMessage(message);
					count = 0;
					message = "";
                }
            }
            if (!message.empty()) {
                // Remove the trailing comma and space
                message = message.substr(0, message.size() - 2);
                neoVSID_->DisplayMessage(message);
			}
        }
		return { true, std::nullopt };
    }
    else if (commandId == neoVSID_->removeCommandId_)
    {
        //if (args[0].empty()) {
        //    std::string error = "Callsign is required. Use .vsid remove <callsign>";
        //    neoVSID_->DisplayMessage(error);
        //    return { false, error };
        //}
		std::string callsign = args[0];
        std::transform(callsign.begin(), callsign.end(), callsign.begin(), ::toupper);

        bool successful = neoVSID_->GetDataManager()->removePilot(callsign);
		if (!successful) {
            std::string error = "Pilot " + callsign + " not found.";
            neoVSID_->DisplayMessage(error);
		}
        neoVSID_->DisplayMessage("Pilot " + callsign + " removed.");
        return { true, std::nullopt };
	}
    else if (commandId == neoVSID_->resetCommandId_)
    {
        neoVSID_->DisplayMessage("NeoVSID resetted.");
        neoVSID_->GetDataManager()->removeAllPilots();
        neoVSID_->GetDataManager()->populateActiveAirports();
        return { true, std::nullopt };
	}
    else {
        std::string error = "Invalid command. Use .vsid <command> <param>";
        neoVSID_->DisplayMessage(error);
        return { false, error };
    }

	return { true, std::nullopt };
}
}  // namespace vsid