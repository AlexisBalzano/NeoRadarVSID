#include "NeoVSID.h"

extern "C" PLUGIN_API PluginSDK::BasePlugin *CreatePluginInstance()
{
    try
    {
        return new vsid::NeoVSID();
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}