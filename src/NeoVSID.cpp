#include "NeoVSID.h"
#include <numeric>
#include <chrono>

#include "Version.h"
#include "core/CompileCommands.h"
#include "core/TagFunctions.h"
#include "core/TagItems.h"

#ifdef DEV
#define LOG_DEBUG(loglevel, message) logger_->log(loglevel, message)
#else
#define LOG_DEBUG(loglevel, message) void(0)
#endif

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
	dataManager_ = std::make_unique<DataManager>(this);

    DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded.", "Initialisation");
    
    callsignsScope.clear();
	dataManager_->removeAllPilots();
	dataManager_->populateActiveAirports();

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
        LOG_DEBUG(Logger::LogLevel::Info, "NeoVSID shutdown complete");
    }

	if (dataManager_) dataManager_.reset();

    this->m_stop = true;
    this->m_worker.join();

    this->unegisterCommand();
}

void vsid::NeoVSID::Reset()
{
    autoModeState = false;
	requestingClearance.clear();
	requestingPush.clear();
	requestingTaxi.clear();
	callsignsScope.clear();
}

void NeoVSID::DisplayMessage(const std::string &message, const std::string &sender) {
    Chat::ClientTextMessageEvent textMessage;
    textMessage.sentFrom = "NeoVSID";
    (sender.empty()) ? textMessage.message = ": " + message : textMessage.message = sender + ": " + message;
    textMessage.useDedicatedChannel = true;

    chatAPI_->sendClientMessage(textMessage);
}

void NeoVSID::runScopeUpdate() {
	std::lock_guard<std::mutex> lock(callsignsMutex);
    UpdateTagItems();
}

void NeoVSID::OnTimer(int Counter) {
    if (Counter % 5 == 0 && autoModeState) this->runScopeUpdate();
}

void vsid::NeoVSID::OnControllerDataUpdated(const ControllerData::ControllerDataUpdatedEvent* event)
{
    if (!event || event->callsign.empty())
        return;
    std::optional<ControllerData::ControllerDataModel> controllerDataBlock = controllerDataAPI_->getByCallsign(event->callsign);
    if (!controllerDataBlock.has_value()) return;
    if (controllerDataBlock->groundStatus == ControllerData::GroundStatus::Dep) {
        dataManager_->removePilot(event->callsign);
        return;
    }
    else {
        std::string request = getRequestAndIndex(event->callsign).first;
        if ((request == "clearance" && controllerDataBlock->clearanceIssued)
            || (request == "push" && controllerDataBlock->groundStatus == ControllerData::GroundStatus::Push)
            || (request == "taxi" && controllerDataBlock->groundStatus == ControllerData::GroundStatus::Taxi)) {
            updateRequest(event->callsign, "ReqNoReq");
        }

    }
}

void NeoVSID::OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) {
	std::lock_guard<std::mutex> lock(callsignsMutex);
    //Force recomputation of all RWY, SID & CFL
    dataManager_->removeAllPilots();
    dataManager_->populateActiveAirports();
	LOG_DEBUG(Logger::LogLevel::Info, "Airport configurations updated.");
}

void vsid::NeoVSID::OnAircraftTemporaryAltitudeChanged(const ControllerData::AircraftTemporaryAltitudeChangedEvent* event)
{
    if (!event || event->callsign.empty())
        return;

    {
        std::lock_guard<std::mutex> lock(callsignsMutex);
        // Do not update tags if the callsign is not in the scope
        if (std::find(callsignsScope.begin(), callsignsScope.end(), event->callsign) == callsignsScope.end())
            return;
    }

	std::optional<double> distanceFromOrigin = aircraftAPI_->getDistanceFromOrigin(event->callsign);
	if (!distanceFromOrigin.has_value()) {
		logger_->error("Failed to retrieve distance from origin for callsign: " + event->callsign);
		return;
	}
    if (distanceFromOrigin > MAX_DISTANCE) {
		dataManager_->removePilot(event->callsign);
        return;
    }

    UpdateTagItems(event->callsign);
}

void vsid::NeoVSID::OnPositionUpdate(const Aircraft::PositionUpdateEvent* event)
{
    for (const auto& aircraft : event->aircrafts) {
        if (aircraft.callsign.empty())
            continue;
        {
			std::lock_guard<std::mutex> lock(callsignsMutex);
            // Do not update tags if the callsign is not in the scope
            if (std::find(callsignsScope.begin(), callsignsScope.end(), aircraft.callsign) == callsignsScope.end())
                continue;
        }

        std::optional<double> distanceFromOrigin = aircraftAPI_->getDistanceFromOrigin(aircraft.callsign);
        if (!distanceFromOrigin.has_value()) {
            logger_->error("Failed to retrieve distance from origin for callsign: " + aircraft.callsign);
            return;
        }
        if (distanceFromOrigin > MAX_DISTANCE) {
            dataManager_->removePilot(aircraft.callsign);
            return;
        }
        
        updateAlert(aircraft.callsign);
	}
}

void vsid::NeoVSID::OnFlightplanUpdated(const Flightplan::FlightplanUpdatedEvent* event)
{
    if (!event || event->callsign.empty())
        return;

    {
        std::lock_guard<std::mutex> lock(callsignsMutex);
        // Do not update tags if the callsign is not in the scope
        if (std::find(callsignsScope.begin(), callsignsScope.end(), event->callsign) == callsignsScope.end())
            return;
	}


    // Force recomputation of RWY, SID and CFL
    dataManager_->removePilot(event->callsign);
	
	std::optional<double> distanceFromOrigin = aircraftAPI_->getDistanceFromOrigin(event->callsign);
	if (!distanceFromOrigin.has_value()) {
		logger_->error("Failed to retrieve distance from origin for callsign: " + event->callsign);
		return;
	}
    if (distanceFromOrigin > MAX_DISTANCE)
		return;

	UpdateTagItems(event->callsign); //Might be the cause of the crash when changing runway config
}


void vsid::NeoVSID::OnFlightplanRemoved(const Flightplan::FlightplanRemovedEvent* event)
{
    if (!event || event->callsign.empty())
        return;
    dataManager_->removePilot(event->callsign);
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

std::pair<std::string, size_t> vsid::NeoVSID::getRequestAndIndex(const std::string& callsign)
{
    for (const auto& request : requestingClearance) {
        if (request == callsign) {
            return {"clearance", &request - &requestingClearance[0]};
        }
	}
    for (const auto& request : requestingPush) {
        if (request == callsign) {
            return {"push", &request - &requestingPush[0]};
        }
    }
    for (const auto& request : requestingTaxi) {
        if (request == callsign) {
            return {"taxi", &request - &requestingTaxi[0]};
        }
    }
	return { "", 0 };
}

PluginSDK::PluginMetadata NeoVSID::GetMetadata() const
{
    return {"NeoVSID", PLUGIN_VERSION, "French VACC"};
}