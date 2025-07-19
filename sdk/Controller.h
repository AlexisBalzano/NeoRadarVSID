#pragma once



namespace PluginSDK::Controller {

/**
 * @enum Rating
 * @brief Controller rating
 */
enum class Rating {
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
 * @enum FacilityType
 * @brief Controller facility type
 */
enum class FacilityType {
    OBS = 0,
    FSS = 1,
    DEL = 2,
    GND = 3,
    TWR = 4,
    APP = 5,
    DEP = 6,
    CTR = 7
};

/**
 * @struct Controller
 * @brief Represents a controller in the system
 */
struct Controller {
    std::string callsign;
    std::string name;
    std::string cid;
    Rating rating;
    FacilityType facility;
    double latitude;
    double longitude;
    std::vector<int> frequencies;
    std::vector<std::string> atisLines;
    std::string atisLetter;
    bool isATC;
    bool isATIS;
    std::string sectorFileName;
};

/**
 * @struct AtcPositionUpdateEvent
 * @brief Event fired when an ATC position is updated
 */
struct AtcPositionUpdateEvent {
    std::string callsign;
    double latitude;
    double longitude;
};

/**
 * @struct AtisLinesUpdateEvent
 * @brief Event fired when ATIS lines are updated
 */
struct AtisLinesUpdateEvent {
    std::string callsign;
    int lineCount;
};

/**
 * @struct CapabilitiesUpdateEvent
 * @brief Event fired when controller capabilities are updated
 */
struct CapabilitiesUpdateEvent {
    std::string callsign;
};

/**
 * @struct ControllerDisconnectedEvent
 * @brief Event fired when a controller disconnects
 */
struct ControllerDisconnectedEvent {
    std::string callsign;
};

/**
 * @struct ControllerConnectedEvent
 * @brief Event fired when a controller connects
 */
struct ControllerConnectedEvent {
    std::string callsign;
    std::string name;
    std::string cid;
    Rating rating;
    FacilityType facility;
};

/**
 * @struct IsControllerATCEvent
 * @brief Event fired when a controller's status changes
 */
struct IsControllerATCEvent {
    std::string callsign;
    bool isATC;
};

class ControllerAPI {
public:
    virtual ~ControllerAPI() = default;

    /**
     * @brief Get all controllers in the system
     * @return Vector of controller data
     */
    virtual std::vector<Controller> getAll() = 0;

    /**
     * @brief Get a controller by callsign
     * @param callsign The callsign to look up
     * @return Controller data or nullptr if not found
     */
    virtual std::optional<Controller> getByCallsign(const std::string& callsign) = 0;
};

} // namespace PluginSDK::Controller
