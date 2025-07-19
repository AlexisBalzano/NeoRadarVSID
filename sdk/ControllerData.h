#pragma once


namespace PluginSDK::ControllerData {

/**
 * @enum AssignedSpeedVariance
 * @brief Variance for assigned speeds (plus/minus)
 */
enum class AssignedSpeedVariance {
    None = 0,
    Plus = 1,
    Minus = 2
};

/**
 * @enum AttentionState
 * @brief State of attention for a controlled aircraft
 */
enum class AttentionState {
    Unconcerned = 0,
    Notified = 1,
    IncomingTransfer = 2,
    Assumed = 3,
    OutgoingTransfer = 4,
    Intruder = 5
};

/**
 * @enum GroundStatus
 * @brief Ground status of an aircraft
 */
enum class GroundStatus {
    None = 0,
    Start = 1,
    Push = 2,
    Taxi = 3,
    Dep = 4
};

/**
 * @struct ControllerDataModel
 * @brief Represents controller data for an aircraft in the system
 */
struct ControllerDataModel {
    std::string callsign;
    int clearedFlightLevel;
    std::string assignedDirect;
    int assignedHeading;
    int assignedSpeed;
    int assignedMach;
    int assignedVerticalRate;
    std::string assignedSquawk;
    std::string scratchpad;
    std::string rawScratchpad;
    bool clearanceIssued;
    bool ownedByMe;
    std::string ownedByCallsign;
    std::string futureOwnerCallsign;
    AssignedSpeedVariance assignedSpeedVariance;
    AttentionState attentionState;
    GroundStatus groundStatus;
    int clearanceQueuePosition;
};

/**
 * @struct ControllerDataUpdatedEvent
 * @brief Event fired when controller data is updated
 */
struct ControllerDataUpdatedEvent {
    std::string callsign;
};

/**
 * @struct AircraftBeaconCodeChangedEvent
 * @brief Event fired when an aircraft's beacon code changes
 */
struct AircraftBeaconCodeChangedEvent {
    std::string callsign;
    std::string oldCode;
    std::string newCode;
};

/**
 * @struct AircraftHandoffCancelledEvent
 * @brief Event fired when a handoff is cancelled
 */
struct AircraftHandoffCancelledEvent {
    std::string callsign;
    std::string oldOwner;
    std::string newOwner;
};

/**
 * @struct AircraftOwnedByChangedEvent
 * @brief Event fired when aircraft ownership changes
 */
struct AircraftOwnedByChangedEvent {
    std::string callsign;
    std::string oldOwner;
    std::string newOwner;
};

/**
 * @struct AircraftHandoffRejectedEvent
 * @brief Event fired when a handoff is rejected
 */
struct AircraftHandoffRejectedEvent {
    std::string callsign;
    std::string from;
};

/**
 * @struct AircraftTerminatedTrackingEvent
 * @brief Event fired when aircraft tracking is terminated
 */
struct AircraftTerminatedTrackingEvent {
    std::string callsign;
};

/**
 * @struct AircraftInitiatedTrackingEvent
 * @brief Event fired when aircraft tracking is initiated
 */
struct AircraftInitiatedTrackingEvent {
    std::string callsign;
};

/**
 * @struct AircraftTemporaryAltitudeChangedEvent
 * @brief Event fired when an aircraft's temporary altitude changes
 */
struct AircraftTemporaryAltitudeChangedEvent {
    std::string callsign;
    int oldAltitude;
    int newAltitude;
};

/**
 * @struct AircraftCDMStatusChangedEvent
 * @brief Event fired when an aircraft's CDM status changes
 */
struct AircraftCDMStatusChangedEvent {
    std::string callsign;
    bool clearanceIssued;
    GroundStatus groundStatus;
};

/**
 * @struct AircraftScratchpadUpdatedEvent
 * @brief Event fired when an aircraft's scratchpad is updated
 */
struct AircraftScratchpadUpdatedEvent {
    std::string callsign;
    std::string scratchpad;
};

/**
 * @struct AircraftHeadingChangedEvent
 * @brief Event fired when an aircraft's assigned heading changes
 */
struct AircraftHeadingChangedEvent {
    std::string callsign;
    int oldHeading;
    int newHeading;
};

/**
 * @struct AircraftAssignedSpeedChangedEvent
 * @brief Event fired when an aircraft's assigned speed changes
 */
struct AircraftAssignedSpeedChangedEvent {
    std::string callsign;
    int oldSpeed;
    int newSpeed;
};

class ControllerDataAPI {
public:
    virtual ~ControllerDataAPI() = default;

    /**
     * @brief Get all controller data entries in the system
     * @return Vector of controller data
     */
    virtual std::vector<ControllerDataModel> getAll() = 0;

    /**
     * @brief Get controller data by callsign
     * @param callsign The callsign to look up
     * @return Controller data or nullptr if not found
     */
    virtual std::optional<ControllerDataModel> getByCallsign(const std::string& callsign) = 0;

    /**
     * @brief Set ground status for an aircraft (rate limited 1 call per second)
     * @param callsign The callsign to look up
     * @param groundStatus The new ground status to set
     * @return Boolean indicating success or failure
     */
    virtual bool setGroundStatus(const std::string& callsign, const GroundStatus groundStatus) = 0;
};

} // namespace PluginSDK::ControllerData
