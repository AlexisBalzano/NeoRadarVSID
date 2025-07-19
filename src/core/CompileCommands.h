#include <algorithm>
#include <string>

#include "log/Logger.h"
#include "utils/String.h"
#include "NeoVSID.h"

using namespace vsid;

namespace vsid {
void NeoVSID::RegisterCommand() {

    logger_->info("RegisterCommand");

}

Chat::CommandResult NeoVSIDCommandProvider::Execute(
    const std::string &commandId,
    const std::vector<std::string> &args)
{
    size_t size = args.size();
    logger_->info("Executing command with " + std::to_string(size) + " arguments");

    if (args.empty())
    {
        std::string error = "Argument is required";
        neoVSID_->DisplayMessage(error);
        return {false, error};
    }

    
}
}  // namespace vsid