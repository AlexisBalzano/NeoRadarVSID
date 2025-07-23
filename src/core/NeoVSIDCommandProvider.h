#pragma once
#include "SDK.h"
#include "NeoVSID.h"

using namespace PluginSDK;

namespace vsid {

class NeoVSID;

class NeoVSIDCommandProvider : public PluginSDK::Chat::CommandProvider
{
public:
    NeoVSIDCommandProvider(vsid::NeoVSID *neoVSID, PluginSDK::Logger::LoggerAPI *logger, Chat::ChatAPI *chatAPI, Fsd::FsdAPI *fsdAPI)
            : neoVSID_(neoVSID), logger_(logger), chatAPI_(chatAPI), fsdAPI_(fsdAPI) {}
		
	Chat::CommandResult Execute(const std::string &commandId, const std::vector<std::string> &args) override;

private:
    Logger::LoggerAPI *logger_ = nullptr;
    Chat::ChatAPI *chatAPI_ = nullptr;
    Fsd::FsdAPI *fsdAPI_ = nullptr;
    NeoVSID *neoVSID_ = nullptr;
};
}  // namespace vsid