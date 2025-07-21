#include <algorithm>
#include <string>

#include "utils/String.h"
#include "NeoVSID.h"

using namespace vsid;

namespace vsid {
void NeoVSID::RegisterCommand() {

    logger_->info("RegisterCommand");
    try
    {
        PluginSDK::Chat::CommandDefinition definition;
        definition.name = "vsid";
        definition.description = "NeoVSID command";
        definition.lastParameterHasSpaces = false;

        Chat::CommandParameter parameter;
        parameter.name = "command";
        parameter.type = Chat::ParameterType::String;
        parameter.required = true;
        definition.parameters.push_back(parameter);

        CommandProvider_ = std::make_shared<NeoVSIDCommandProvider>(this, logger_, chatAPI_, fsdAPI_);

        std::string commandId = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        if (!commandId.empty())
        {
            logger_->info("Successfully registered .vsid command with ID: " + commandId);
            commandId_ = commandId;
        }
        else
        {
            logger_->error("Failed to register .vsid command");
        }
    }
    catch (const std::exception& ex)
    {
        logger_->error("Error registering command: " + std::string(ex.what()));
    }
}

Chat::CommandResult NeoVSIDCommandProvider::Execute(
    const std::string &commandId,
    const std::vector<std::string> &args)
{
    size_t size = args.size();
    logger_->info("Executing command with " + std::to_string(size) + " arguments");

    if (args.empty())
    {
        std::string error = "Argument is required. Use .vsid <command> <param>";
        neoVSID_->DisplayMessage(error);
        return {false, error};
    }

	std::string command = args[0];
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);
	logger_->info("Command: " + command);

    if (command == "VERSION")
    {
        std::string message = "Version " + std::string(PLUGIN_VERSION);
        neoVSID_->DisplayMessage(message);
        return { true, std::nullopt };
	}
    else if (command == "HELP")
    {
        neoVSID_->DisplayMessage("To Check current NeoVSID version, use.vsid version");
        neoVSID_->DisplayMessage("To set auto mode, use.vsid auto");
        neoVSID_->DisplayMessage("Other commands are comming soon.");
    }
    else if (command == "AUTO")
    {
        neoVSID_->switchAutoModeState();
        std::string message = "Automode set to " + (neoVSID_->getAutoModeState() ? std::string("True") : std::string("False"));
        neoVSID_->DisplayMessage(message);
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