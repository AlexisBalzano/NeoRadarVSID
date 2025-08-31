#pragma once

#define PLUGIN_SDK_VERSION_MAJOR 1
#define PLUGIN_SDK_VERSION_MINOR 0
#define PLUGIN_SDK_VERSION_PATCH 0

#include "Aircraft.h"
#include "Flightplan.h"
#include "Controller.h"
#include "ControllerData.h"
#include "Airport.h"
#include "Squawk.h"
#include "Tag.h"
#include "Logger.h"
#include "Fsd.h"
#include "EuroScope.h"
#include "Sectors.h"
#include "Chat.h"
#include <filesystem>
#include <string>

namespace PluginSDK {

/**
 * @struct PluginMetadata
 * @brief Metadata about the plugin
 */
struct PluginMetadata {
    std::string name;
    std::string version;
    std::string author;
};

/**
 * @struct ClientInformation
 * @brief Information about the client application
 */
struct ClientInformation {
    std::string clientName;
    std::string clientVersion;
    std::string fdpsVersion;
    std::string combinedVersion;
    std::filesystem::path documentsPath;
};

/**
 * @interface CoreAPI
 * @brief Modern modular Core API interface for plugins
 */
class CoreAPI {
public:
    virtual ~CoreAPI() = default;

    /**
     * @brief Get the aircraft API
     * @return Reference to the aircraft API
     */
    virtual Aircraft::AircraftAPI& aircraft() = 0;

    /**
     * @brief Get the flightplan API
     * @return Reference to the flightplan API
     */
    virtual Flightplan::FlightplanAPI& flightplan() = 0;

    /**
     * @brief Get the controller API
     * @return Reference to the controller API
     */
    virtual Controller::ControllerAPI& controller() = 0;

    /**
     * @brief Get the controller data API
     * @return Reference to the controller data API
     */
    virtual ControllerData::ControllerDataAPI& controllerData() = 0;

    /**
     * @brief Get the airport API
     * @return Reference to the airport API
     */
    virtual Airport::AirportAPI& airport() = 0;

    /**
     * @brief Get the squawk API
     * @return Reference to the squawk API
     */
    virtual Squawk::SquawkAPI& squawk() = 0;

    /**
     * @brief Get the tag API
     * @return Reference to the tag API
     */
    virtual Tag::TagAPI& tag() = 0;

    /**
     * @brief Get the logger API
     * @return Reference to the logger API
     */
    virtual Logger::LoggerAPI& logger() = 0;

    /**
     * @brief Get the FSD API
     * @return Reference to the FSD API
     */
    virtual Fsd::FsdAPI& fsd() = 0;

    /**
     * @brief Get the Chat API
     * @return Reference to the Chat API
     */
    virtual Chat::ChatAPI& chat() = 0;
};

/**
 * @interface BasePlugin
 * @brief Base interface for all plugins
 */
class BasePlugin {
public:
    virtual ~BasePlugin() = default;

    /**
     * @brief Initialize the plugin
     * @param metadata The plugin's metadata
     * @param coreAPI Pointer to the core API interface
     * @param info Client information
     */
    virtual void Initialize(const PluginMetadata& metadata, CoreAPI* coreAPI, ClientInformation info) = 0;

    /**
     * @brief Shut down the plugin
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Get the plugin's metadata
     * @return Plugin metadata
     */
    virtual PluginMetadata GetMetadata() const = 0;

    // Aircraft events
    virtual void OnAircraftConnected(const Aircraft::AircraftConnectedEvent* event) {}
    virtual void OnAircraftDisconnected(const Aircraft::AircraftDisconnectedEvent* event) {}
    virtual void OnPositionUpdate(const Aircraft::PositionUpdateEvent* event) {}

    // Airport events
    virtual void OnAirportAdded(const Airport::AirportAddedEvent* event) {}
    virtual void OnAirportRemoved(const Airport::AirportRemovedEvent* event) {}
    virtual void OnAirportStatusChanged(const Airport::AirportStatusChangedEvent* event) {}
    virtual void OnRunwayStatusChanged(const Airport::RunwayStatusChangedEvent* event) {}
    virtual void OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) {}

    // Controller events
    virtual void OnAtcPositionUpdate(const Controller::AtcPositionUpdateEvent* event) {}
    virtual void OnAtisLinesUpdate(const Controller::AtisLinesUpdateEvent* event) {}
    virtual void OnCapabilitiesUpdate(const Controller::CapabilitiesUpdateEvent* event) {}
    virtual void OnControllerDisconnected(const Controller::ControllerDisconnectedEvent* event) {}
    virtual void OnControllerConnected(const Controller::ControllerConnectedEvent* event) {}
    virtual void OnIsControllerATC(const Controller::IsControllerATCEvent* event) {}

    // Controller Data events
    virtual void OnControllerDataUpdated(const ControllerData::ControllerDataUpdatedEvent* event) {}
    virtual void OnAircraftBeaconCodeChanged(const ControllerData::AircraftBeaconCodeChangedEvent* event) {}
    virtual void OnAircraftHandoffCancelled(const ControllerData::AircraftHandoffCancelledEvent* event) {}
    virtual void OnAircraftOwnedByChanged(const ControllerData::AircraftOwnedByChangedEvent* event) {}
    virtual void OnAircraftHandoffRejected(const ControllerData::AircraftHandoffRejectedEvent* event) {}
    virtual void OnAircraftTerminatedTracking(const ControllerData::AircraftTerminatedTrackingEvent* event) {}
    virtual void OnAircraftInitiatedTracking(const ControllerData::AircraftInitiatedTrackingEvent* event) {}
    virtual void OnAircraftTemporaryAltitudeChanged(const ControllerData::AircraftTemporaryAltitudeChangedEvent* event) {}
    virtual void OnAircraftCDMStatusChanged(const ControllerData::AircraftCDMStatusChangedEvent* event) {}
    virtual void OnAircraftScratchpadUpdated(const ControllerData::AircraftScratchpadUpdatedEvent* event) {}
    virtual void OnAircraftHeadingChanged(const ControllerData::AircraftHeadingChangedEvent* event) {}
    virtual void OnAircraftAssignedSpeedChanged(const ControllerData::AircraftAssignedSpeedChangedEvent* event) {}

    // EuroScope events
    virtual void OnEuroScopeProxyServerStart(const EuroScope::EuroScopeProxyServerStartEvent* event) {}
    virtual void OnEuroScopeProxyServerStop(const EuroScope::EuroScopeProxyServerStopEvent* event) {}
    virtual void OnEuroScopeProxyServerClientConnected(const EuroScope::EuroScopeProxyServerClientConnectedEvent* event) {}
    virtual void OnEuroScopeProxyServerClientDisconnected(const EuroScope::EuroScopeProxyServerClientDisconnectedEvent* event) {}
    virtual void OnEuroScopeProxyServerError(const EuroScope::EuroScopeProxyServerErrorEvent* event) {}

    // Flightplan events
    virtual void OnFlightplanUpdated(const Flightplan::FlightplanUpdatedEvent* event) {}
    virtual void OnFlightplanRemoved(const Flightplan::FlightplanRemovedEvent* event) {}
    virtual void OnFlightplanVoiceTypeChanged(const Flightplan::FlightplanVoiceTypeChangedEvent* event) {}
    virtual void OnFlightplanRouteChanged(const Flightplan::FlightplanRouteChangedEvent* event) {}

    // FSD events
    virtual void OnFsdError(const Fsd::FsdErrorEvent* event) {}
    virtual void OnFsdConnectionStateChange(const Fsd::FsdConnectionStateChangeEvent* event) {}
    virtual void OnFsdConnected(const Fsd::FsdConnectedEvent* event) {}
    virtual void OnFsdDisconnected(const Fsd::FsdDisconnectedEvent* event) {}
    virtual void OnFsdConnectionModelUpdated(const Fsd::FsdConnectionModelUpdatedEvent* event) {}

    // Sectors events
    virtual void OnOwnedSectorUpdated(const Sectors::OwnedSectorUpdatedEvent* event) {}

    // Text Message events
    virtual void OnFrequencyMessageReceived(const Chat::FrequencyMessageReceivedEvent* event) {}
    virtual void OnFlightplanMessageReceived(const Chat::FlightplanMessageReceivedEvent* event) {}
    virtual void OnATISInfoMessageReceived(const Chat::ATISInfoMessageReceivedEvent* event) {}
    virtual void OnPrivateMessageReceived(const Chat::PrivateMessageReceivedEvent* event) {}
    virtual void OnBroadcastMessageReceived(const Chat::BroadcastMessageReceivedEvent* event) {}
    virtual void OnSupervisorMessageReceived(const Chat::SupervisorMessageReceivedEvent* event) {}
    virtual void OnServerMessageReceived(const Chat::ServerMessageReceivedEvent* event) {}
    virtual void OnAtcMessageReceived(const Chat::AtcMessageReceivedEvent* event) {}

    // Squawk events
    virtual void OnSquawkAssigned(const Squawk::SquawkAssignedEvent* event) {}

    // Tag events
    virtual void OnTagAction(const Tag::TagActionEvent* event) {}
    virtual void OnTagDropdownAction(const Tag::DropdownActionEvent* event) {}
};

} // namespace PluginSDK

#if defined(_MSC_VER)
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __attribute__((visibility("default")))
#endif

extern "C" {
PLUGIN_API inline int GetPluginSDKVersionMajor() { return PLUGIN_SDK_VERSION_MAJOR; }
PLUGIN_API inline int GetPluginSDKVersionMinor() { return PLUGIN_SDK_VERSION_MINOR; }
PLUGIN_API inline int GetPluginSDKVersionPatch() { return PLUGIN_SDK_VERSION_PATCH; }
PLUGIN_API PluginSDK::BasePlugin* CreatePluginInstance();
}