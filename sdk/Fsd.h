#pragma once



namespace PluginSDK::Fsd {

/**
 * @enum ConnectionState
 * @brief States of FSD connection
 */
enum class ConnectionState {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Failed = 3
};

/**
 * @enum NetworkRating
 * @brief Controller ratings
 */
enum class NetworkRating {
    OBS = 1,
    S1 = 2,
    S2 = 3,
    S3 = 4,
    C1 = 5,
    C2 = 6,
    C3 = 7,
    I1 = 8,
    I2 = 9,
    I3 = 10,
    SUP = 11,
    ADM = 12
};

/**
 * @enum NetworkFacility
 * @brief Controller facility types
 */
enum class NetworkFacility {
    OBS = 0,
    FSS = 1,
    DEL = 2,
    GND = 3,
    TWR = 4,
    APP = 5,
    CTR = 6
};

/**
 * @enum ServerType
 * @brief Types of FSD servers
 */
enum class ServerType {
    Unknown = 0,
    Live = 1,
    Test = 2,
    Sweatbox = 3
};

/**
 * @enum ProtocolRevision
 * @brief FSD protocol revisions
 */
enum class ProtocolRevision {
    Unknown = 0,
    Classic = 9,
    VatsimNoAuth = 10,
    VatsimAuth = 100,
    Vatsim2022 = 101,
    VatsimVelocity = 102
};

/**
 * @struct ConnectionInfo
 * @brief Minimal connection information
 */
struct ConnectionInfo {
    std::string callsign;
    int cid;
    std::string name;
    NetworkRating rating;
    NetworkFacility facility;
    ServerType serverType;
    ProtocolRevision protocolRevision;
    bool isConnected;
    std::vector<std::string> atisLines;
    std::vector<int> frequencies;
};

/**
 * @struct FsdErrorEvent
 * @brief Event fired when an FSD error occurs
 */
struct FsdErrorEvent {
    std::string message;
};

/**
 * @struct FsdConnectionStateChangeEvent
 * @brief Event fired when FSD connection state changes
 */
struct FsdConnectionStateChangeEvent {
    ConnectionState state;
};

/**
 * @struct FsdConnectedEvent
 * @brief Event fired when connected to FSD
 */
struct FsdConnectedEvent {
    std::string callsign;
    std::string server;
    int facilityType;
};

/**
 * @struct FsdDisconnectedEvent
 * @brief Event fired when disconnected from FSD
 */
struct FsdDisconnectedEvent {
    std::string reason;
    bool userInitiated;
};

/**
 * @struct FsdConnectionModelUpdatedEvent
 * @brief Event fired when FSD connection model is updated
 */
struct FsdConnectionModelUpdatedEvent {
    std::string callsign;
};

/**
 * @interface FsdAPI
 * @brief Interface for FSD (Flight Server Data) operations
 */
class FsdAPI {
public:
    virtual ~FsdAPI() = default;

    /**
     * @brief Get the current connection state
     * @return Connection state
     */
    virtual const std::optional<ConnectionInfo> getConnection() = 0;

};

} // namespace PluginSDK::Fsd
