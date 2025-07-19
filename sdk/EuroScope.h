// plugin/sdk/EuroScope.h
#pragma once

namespace PluginSDK::EuroScope {

/**
 * @enum ConnectionType
 * @brief Type of EuroScope connection
 */
enum class ConnectionType {
    Unknown = 0,
    EuroScope = 1,
    SimAware = 2,
    VATSIMExternalClient = 3
};

/**
 * @struct EuroScopeProxyServerStartEvent
 * @brief Event fired when the EuroScope proxy server starts
 */
struct EuroScopeProxyServerStartEvent {
    // Empty struct - event just indicates that the server started
};

/**
 * @struct EuroScopeProxyServerStopEvent
 * @brief Event fired when the EuroScope proxy server stops
 */
struct EuroScopeProxyServerStopEvent {
    // Empty struct - event just indicates that the server stopped
};

/**
 * @struct EuroScopeProxyServerClientConnectedEvent
 * @brief Event fired when a client connects to the EuroScope proxy server
 */
struct EuroScopeProxyServerClientConnectedEvent {
    std::string address;
    ConnectionType connectionType;
};

/**
 * @struct EuroScopeProxyServerClientDisconnectedEvent
 * @brief Event fired when a client disconnects from the EuroScope proxy server
 */
struct EuroScopeProxyServerClientDisconnectedEvent {
    std::string address;
    ConnectionType connectionType;
};

/**
 * @struct EuroScopeProxyServerErrorEvent
 * @brief Event fired when an error occurs in the EuroScope proxy server
 */
struct EuroScopeProxyServerErrorEvent {
    std::string error;
};

} // namespace PluginSDK::EuroScope
