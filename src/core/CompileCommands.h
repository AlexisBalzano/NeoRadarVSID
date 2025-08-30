#include <algorithm>
#include <string>

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
        
        definition.name = "vsid areas";
        definition.description = "return area list";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        areasCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid rules";
        definition.description = "return rule list";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        rulesCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid reset";
        definition.description = "reset active airport & pilot lists";
        definition.lastParameterHasSpaces = false;
        definition.parameters.clear();

        resetCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid remove";
        definition.description = "remove pilot from pilot list";
        definition.lastParameterHasSpaces = false;

        Chat::CommandParameter parameter;
        parameter.name = "CALLSIGN";
        parameter.type = Chat::ParameterType::String;
        parameter.required = true;
        definition.parameters.push_back(parameter);

        removeCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid position";
        definition.description = "print aircraft coordinates inside NeoRadar";
        definition.lastParameterHasSpaces = false;

		parameter.name = "AREANAME";
        parameter.required = true;
        definition.parameters.push_back(parameter);

        positionCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid rule";
        definition.description = "toggle rule state";
        definition.lastParameterHasSpaces = false;

        definition.parameters.clear();
        parameter.name = "OACI";
        parameter.type = Chat::ParameterType::String;
		parameter.minLength = 4;
		parameter.maxLength = 4;
        parameter.required = true;
        definition.parameters.push_back(parameter);
        parameter.name = "RULENAME";
		parameter.minLength = 0;
		parameter.maxLength = 0;
        parameter.required = true;
        definition.parameters.push_back(parameter);

        ruleCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vsid area";
        definition.description = "toggle area state";
        definition.lastParameterHasSpaces = false;

        areaCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);
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
        chatAPI_->unregisterCommand(areasCommandId_);
        chatAPI_->unregisterCommand(rulesCommandId_);
        chatAPI_->unregisterCommand(resetCommandId_);
        chatAPI_->unregisterCommand(removeCommandId_);
        chatAPI_->unregisterCommand(positionCommandId_);
        chatAPI_->unregisterCommand(ruleCommandId_);
        chatAPI_->unregisterCommand(areaCommandId_);
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
        neoVSID_->DisplayMessage(".vsid rules");
        neoVSID_->DisplayMessage(".vsid areas");
        neoVSID_->DisplayMessage(".vsid reset");
        neoVSID_->DisplayMessage(".vsid remove <CALLSIGN>");
        neoVSID_->DisplayMessage(".vsid position <CALLSIGN> <AREANAME>");
        neoVSID_->DisplayMessage(".vsid rule <OACI> <RULENAME>");
        neoVSID_->DisplayMessage(".vsid area <OACI> <AREANAME>");
    }
    else if (commandId == neoVSID_->autoModeCommandId_)
    {
        neoVSID_->switchAutoModeState();
        std::string message = "Automode set to " + (neoVSID_->getAutoModeState() ? std::string("True") : std::string("False"));
        neoVSID_->DisplayMessage(message);
		neoVSID_->OnTimer(0); // Trigger an immediate update
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
    else if (commandId == neoVSID_->areasCommandId_)
    {
        std::vector<areaData> areas = neoVSID_->GetDataManager()->getAreas();
        if (areas.empty()) {
            neoVSID_->DisplayMessage("No Area found.");
        }
        else {
			int count = 0;
            std::string message = "Areas: ";
            for (const auto& area : areas) {
				++count;
                message += area.oaci + " " + area.name + " " + std::string(area.active ? "Active" : "Inactive") + ", ";
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
    else if (commandId == neoVSID_->rulesCommandId_)
    {
        std::vector<ruleData> rules = neoVSID_->GetDataManager()->getRules();
        if (rules.empty()) {
            neoVSID_->DisplayMessage("No rule found.");
        }
        else {
			int count = 0;
            std::string message = "Rules: ";
            for (const auto& rule : rules) {
				++count;
                message += rule.oaci + " " + rule.name + " " + std::string(rule.active ? "Active": "Inactive") + ", ";
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
		std::string callsign = args[0];
        std::transform(callsign.begin(), callsign.end(), callsign.begin(), ::toupper);

        bool successful = neoVSID_->GetDataManager()->removePilot(callsign);
		if (!successful) {
            std::string error = "Pilot " + callsign + " not found.";
            neoVSID_->DisplayMessage(error);
            return { true, std::nullopt };
		}
        neoVSID_->DisplayMessage("Pilot " + callsign + " removed.");
        return { true, std::nullopt };
	}
    else if (commandId == neoVSID_->positionCommandId_)
    {
        if (args.empty() || args[0].empty() || args[1].empty()) {
            std::string error = "CALLSIGN and AREANAME are required. Use .vsid rule <oaci> <areaname>";
            neoVSID_->DisplayMessage(error);
            return { true, std::nullopt };
        }
		std::string callsign = args[0];
        std::string areaName = args[1];
		std::transform(callsign.begin(), callsign.end(), callsign.begin(), ::toupper);
		std::transform(areaName.begin(), areaName.end(), areaName.begin(), ::tolower);
        auto aircraftOpt = neoVSID_->GetAircraftAPI()->getByCallsign(callsign);
        if (!aircraftOpt) {
            neoVSID_->DisplayMessage("Aircraft " + callsign + " not found.");
            return { true, std::nullopt };
        }
        const auto& aircraft = *aircraftOpt;
        neoVSID_->DisplayMessage(callsign + "'s position is N" + std::to_string(aircraft.position.latitude) + " E" + std::to_string(aircraft.position.longitude));
        std::string oaci = neoVSID_->GetFlightplanAPI()->getByCallsign(callsign)->origin;
		bool isInArea = neoVSID_->GetDataManager()->isInArea(aircraft.position.latitude, aircraft.position.longitude, oaci, areaName);
        neoVSID_->DisplayMessage("Aircraft " + std::string(isInArea ? "is in Area" : "isn't in Area"));
        return { true, std::nullopt };
	}
    else if (commandId == neoVSID_->ruleCommandId_)
    {
        if (args.empty() || args[0].empty() || args[1].empty()) {
            std::string error = "OACI and RULENAME are required. Use .vsid rule <oaci> <rulename>";
            neoVSID_->DisplayMessage(error);
            return { true, std::nullopt };
        }
		std::string oaci = args[0];
		std::transform(oaci.begin(), oaci.end(), oaci.begin(), ::toupper);
		std::string ruleName = args[1];
		std::transform(ruleName.begin(), ruleName.end(), ruleName.begin(), ::tolower);

		//check if rule exists
		auto rules = neoVSID_->GetDataManager()->getRules();
		auto it = std::find_if(rules.begin(), rules.end(), [&](const ruleData& rule) {
			return rule.oaci == oaci && rule.name == ruleName;
		});
        if (it == rules.end()) {
            std::string error = "Rule " + ruleName + " for OACI " + oaci + " not found.";
            neoVSID_->DisplayMessage(error);
            return { true, std::nullopt };
        }
		neoVSID_->GetDataManager()->switchRuleState(oaci, ruleName);
		std::string message = "Rule " + ruleName + " for OACI " + oaci + " is now " + (it->active ? "inactive" : "active") + ".";
		neoVSID_->DisplayMessage(message);

        return { true, std::nullopt };
	}
    else if (commandId == neoVSID_->areaCommandId_)
    {
        if (args.empty() || args[0].empty() || args[1].empty()) {
            std::string error = "OACI and AREANAME are required. Use .vsid rule <oaci> <areaname>";
            neoVSID_->DisplayMessage(error);
            return { true, std::nullopt };
        }
		std::string oaci = args[0];
		std::transform(oaci.begin(), oaci.end(), oaci.begin(), ::toupper);
		std::string areaName = args[1];
		std::transform(areaName.begin(), areaName.end(), areaName.begin(), ::tolower);

		//check if area exists
		auto areas = neoVSID_->GetDataManager()->getAreas();
		auto it = std::find_if(areas.begin(), areas.end(), [&](const areaData& area) {
			return area.oaci == oaci && area.name == areaName;
		});
        if (it == areas.end()) {
            std::string error = "Area " + areaName + " for OACI " + oaci + " not found.";
            neoVSID_->DisplayMessage(error);
            return { true, std::nullopt };
        }
		neoVSID_->GetDataManager()->switchAreaState(oaci, areaName);
		std::string message = "Area " + areaName + " for OACI " + oaci + " is now " + (it->active ? "inactive" : "active") + ".";
		neoVSID_->DisplayMessage(message);

        return { true, std::nullopt };
	}
    else if (commandId == neoVSID_->resetCommandId_)
    {
        neoVSID_->DisplayMessage("NeoVSID resetted.");
        neoVSID_->GetDataManager()->removeAllPilots();
		neoVSID_->GetDataManager()->clearJson();
		neoVSID_->GetDataManager()->loadAircraftDataJson();
        neoVSID_->GetDataManager()->populateActiveAirports();
        neoVSID_->Reset();
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