// NeoVACDM.cpp
#include "NeoVSID.h"
#include <numeric>
#include <chrono>

#include "Version.h"
#include "core/CompileCommands.h"
#include "core/TagFunctions.h"
#include "core/TagItems.h"
#include "utils/String.h"

using namespace vsid;

NeoVSID::NeoVSID() : m_stop(false), controllerDataAPI_(nullptr) {};
NeoVSID::~NeoVSID() = default;

void NeoVSID::Initialize(const PluginMetadata &metadata, CoreAPI *coreAPI, ClientInformation info)
{
    metadata_ = metadata;
    clientInfo_ = info;
    CoreAPI *lcoreAPI = coreAPI;
    aircraftAPI_ = &lcoreAPI->aircraft();
    airportAPI_ = &lcoreAPI->airport();
    chatAPI_ = &lcoreAPI->chat();
    flightplanAPI_ = &lcoreAPI->flightplan();
    fsdAPI_ = &lcoreAPI->fsd();
    controllerDataAPI_ = &lcoreAPI->controllerData();
    logger_ = &lcoreAPI->logger();
    tagInterface_ = lcoreAPI->tag().getInterface();
	dataManager_ = std::make_unique<DataManager>(aircraftAPI_, flightplanAPI_, airportAPI_);

    logging::Logger::instance().setLogger(logger_);

    DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded", "Initialisation");
    
    logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Version " + std::string(PLUGIN_VERSION) + " loaded", logging::Logger::LogLevel::System);

    callsignsScope.clear();

    try
    {
        this->RegisterTagItems();
        this->RegisterTagActions();
        this->RegisterCommand();

        initialized_ = true;
    }
    catch (const std::exception &e)
    {
        logger_->error("Failed to initialize NeoVSID: " + std::string(e.what()));
    }

    this->m_stop = false;
    this->m_worker = std::thread(&NeoVSID::run, this);
}

void NeoVSID::Shutdown()
{
    if (initialized_)
    {
        initialized_ = false;
        logger_->info("NeoVSID shutdown complete");
    }

	if (dataManager_) dataManager_.reset();

    logging::Logger::instance().shutdown();

    this->m_stop = true;
    this->m_worker.join();

    //chatAPI_->unregisterCommand(commandId_);
}

void NeoVSID::DisplayMessage(const std::string &message, const std::string &sender) {
    Chat::ClientTextMessageEvent textMessage;
    textMessage.sentFrom = "NeoVSID";
    (sender.empty()) ? textMessage.message = message : textMessage.message = sender + ": " + message;
    textMessage.useDedicatedChannel = true;

    chatAPI_->sendClientMessage(textMessage);
}

void vsid::NeoVSID::SetGroundState(const std::string &callsign, const ControllerData::GroundStatus groundstate)
{
    controllerDataAPI_->setGroundStatus(callsign, groundstate);
}

void NeoVSID::runScopeUpdate() {
    UpdateTagItems();
}

void NeoVSID::OnTimer(int Counter) {
    if (Counter % 5 == 0) this->runScopeUpdate();
}

void NeoVSID::OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) {
    dataManager_->populateActiveAirports();
}

void vsid::NeoVSID::OnAircraftTemporaryAltitudeChanged(const ControllerData::AircraftTemporaryAltitudeChangedEvent* event)
{
    UpdateTagItems(event->callsign);
}

void vsid::NeoVSID::OnFlightplanUpdated(const Flightplan::FlightplanUpdatedEvent* event)
{
    if (!event || event->callsign.empty())
        return;

	// Do not update tags if the callsign is not in the scope
    if (std::find(callsignsScope.begin(), callsignsScope.end(), event->callsign) == callsignsScope.end())
        return;
    
    UpdateTagItems(event->callsign); 
}

void NeoVSID::run() {
    int counter = 1;
    while (true) {
        counter += 1;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (true == this->m_stop) return;
        
        this->OnTimer(counter);
    }
    return;
}

PluginSDK::PluginMetadata NeoVSID::GetMetadata() const
{
    return {"NeoVSID", PLUGIN_VERSION, "French VACC"};
}