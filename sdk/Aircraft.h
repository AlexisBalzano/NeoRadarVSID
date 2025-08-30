#pragma once
#include <string>
#include <optional>
#include <vector>
namespace PluginSDK::Aircraft {

/**
 * @enum TransponderMode
 * @brief Represents the status of an aircraft's transponder
 */
enum class TransponderMode { Standby = 0, Normal = 1, Ident = 2 };

/**
 * @struct Position
 * @brief Represents an aircraft's position in 3D space
 */
struct Position {
    // Location
    double latitude = 0.0; // Latitude in degrees
    double longitude = 0.0; // Longitude in degrees

    // Altitude
    int altitude = 0; // Pressure altitude rounded to nearest 25ft
    double agl = 0.0; // Height above ground level
    double trueAltitude = 0.0; // True altitude
    double pressureAltitude = 0.0; // Raw pressure altitude

    // Speed & heading
    int groundSpeed = 0; // Ground speed in knots
    int reportedHeading = 0; // Reported heading in degrees
    int trackHeading = 0; // Track (derived) heading in degrees
    int verticalSpeed = 0; // Vertical speed in feet per minute
    int verticalTrend = 0; // Up (1), level (0), or down (-1)

    // Attitude
    double pitch = 0.0; // Aircraft pitch in degrees
    double bank = 0.0; // Aircraft bank in degrees

    // Velocity components
    double velX = 0.0; // Longitude velocity
    double velY = 0.0; // Altitude velocity
    double velZ = 0.0; // Latitude velocity
    double velH = 0.0; // Heading velocity

    // Status
    bool onGround = false; // Whether aircraft is on ground
    bool stopped = true; // Whether aircraft is stopped
    TransponderMode transponderMode = TransponderMode::Standby;

    // Timestamp of position update (ISO string format)
    std::string timestamp;

    bool operator==(const Position& other) const
    {
        return latitude == other.latitude && longitude == other.longitude
            && altitude == other.altitude && agl == other.agl
            && trueAltitude == other.trueAltitude
            && pressureAltitude == other.pressureAltitude
            && groundSpeed == other.groundSpeed
            && reportedHeading == other.reportedHeading
            && trackHeading == other.trackHeading && verticalSpeed == other.verticalSpeed
            && verticalTrend == other.verticalTrend && pitch == other.pitch
            && bank == other.bank && velX == other.velX && velY == other.velY
            && velZ == other.velZ && velH == other.velH && onGround == other.onGround
            && stopped == other.stopped && transponderMode == other.transponderMode
            && timestamp == other.timestamp;
    }

    bool operator!=(const Position& other) const { return !(*this == other); }
};

/**
 * @struct Aircraft
 * @brief Represents an aircraft in the system
 */
struct Aircraft {
    std::string callsign;
    int cid;
    std::string name;

    Position position;
    std::vector<Position> previousPositions;

    bool hasPilotDetails = false;
    bool isTimingOut = false;
    bool isReady = false;

    // (ISO string format)
    std::string lastPositionUpdateTime;

    std::string squawk;
    TransponderMode transponderMode = TransponderMode::Standby;

    int pressureDifference = 0;
};


/**
 * @struct AircraftConnectedEvent
 * @brief Event fired when an aircraft connects to the network
 */
struct AircraftConnectedEvent {
    std::string callsign;
    int cid;
};

/**
 * @struct AircraftDisconnectedEvent
 * @brief Event fired when an aircraft disconnects from the network
 */
struct AircraftDisconnectedEvent {
    std::string callsign;
};

/**
 * @struct PositionUpdateEvent
 * @brief Event fired when aircraft positions are updated
 */
struct PositionUpdateEvent {
    std::vector<Aircraft> aircrafts;
};

class AircraftAPI {
public:
    virtual ~AircraftAPI() = default;

    /**
     * @brief Get all aircraft in the system
     * @return Vector of aircraft data
     */
    virtual std::vector<Aircraft> getAll() = 0;

    /**
     * @brief Get an aircraft by callsign
     * @param callsign The callsign to look up
     * @return Aircraft data or nullptr if not found
     */
    virtual std::optional<Aircraft> getByCallsign(const std::string& callsign) = 0;

    /**
     * @brief Get the distance from aircraft to its origin airport
     * @param callsign The aircraft callsign
     * @return Distance in nautical miles if available, std::nullopt if aircraft or origin not
     * found
     */
    virtual std::optional<double> getDistanceFromOrigin(const std::string& callsign) = 0;

    /**
     * @brief Get the distance from aircraft to its destination airport
     * @param callsign The aircraft callsign
     * @return Distance in nautical miles if available, std::nullopt if aircraft or destination
     * not found
     */
    virtual std::optional<double> getDistanceToDestination(const std::string& callsign)
        = 0;
};

} // namespace PluginSDK::Aircraft
