#pragma once
#include <cmath>
#include <optional>
#include <string>
#include <vector>

namespace PluginSDK::Flightplan {

enum class VoiceType { Unknown = 0, Full = 1, Receive = 2, Text = 3 };

enum class FlightRule { IFR, VFR, DVFR, SVFR };

enum class WaypointType {
    FIX,
    VOR,
    DME,
    VORDME,
    NDBDME,
    VORTAC,
    TACAN,
    NDB,
    AIRPORT,
    LATLON,
    UNKNOWN
};

enum class ParsingErrorLevel { INFO, PARSE_ERROR };

enum class ParsingErrorType {
    ROUTE_EMPTY,
    PROCEDURE_RUNWAY_MISMATCH,
    PROCEDURE_AIRPORT_MISMATCH,
    UNKNOWN_PROCEDURE,
    UNKNOWN_WAYPOINT,
    NO_PROCEDURE_FOUND,
    INVALID_RUNWAY,
    INVALID_DATA,
    UNKNOWN_AIRPORT,
    UNKNOWN_NAVAID,
    UNKNOWN_AIRWAY,
    INVALID_TOKEN_FORMAT,
    INVALID_AIRWAY_FORMAT,
    INVALID_AIRWAY_DIRECTION,
    AIRWAY_FIX_NOT_FOUND,
    INSUFFICIENT_FLIGHT_LEVEL,
    MULTIPLE_AIRWAYS_FOUND
};

enum class SpeedUnit { KNOTS, MACH, KMH };

enum class DistanceUnit { NAUTICAL_MILES, KILOMETERS, FEET, METERS };

struct Position {
    double latitude = 0.0;
    double longitude = 0.0;
};

struct Waypoint {
    WaypointType type = WaypointType::UNKNOWN;
    std::string identifier;
    Position position;
    int frequencyHz = 0;
};

struct PlannedAltitudeAndSpeed {
    std::optional<int> plannedAltitude;
    std::optional<int> plannedSpeed;
    DistanceUnit altitudeUnit = DistanceUnit::FEET;
    SpeedUnit speedUnit = SpeedUnit::KNOTS;
};

struct RouteWaypoint : public Waypoint {
    std::optional<PlannedAltitudeAndSpeed> plannedPosition;
    FlightRule flightRule = FlightRule::IFR;
};

struct ParsingError {
    ParsingErrorType type;
    std::string message;
    int tokenIndex = 0;
    std::string token;
    ParsingErrorLevel level;
};

struct ParsedRouteSegment {
    RouteWaypoint from;
    RouteWaypoint to;
    std::string airway; // "DCT" for direct connections
    int heading = 0;
    int minimumLevel = -1;
};

struct Route {
    std::string rawRoute;
    std::vector<RouteWaypoint> waypoints;
    std::vector<ParsedRouteSegment> segments;
    std::vector<ParsedRouteSegment> explicitSegments;
    std::vector<ParsingError> errors;
    std::vector<ParsedRouteSegment> originalSegments;

    bool hasDirectApplied = false;
    Waypoint currentDirectWaypoint;

    std::string sid;
    std::string star;
    std::string suggestedSid;
    std::string suggestedStar;
    std::string depRunway;
    std::string arrRunway;
    std::string suggestedDepRunway;
    std::string suggestedArrRunway;

    bool isAmended = false;
};

struct Flightplan {
    std::string callsign;
    std::string flightRule;
    std::string rawType;
    std::string acType; // Short aircraft type (e.g., "B738")
    std::string equipment; // ICAO equipment
    std::string wakeCategory; // Wake turbulence category
    std::string transponderEquipment; // ICAO transponder equipment
    std::string origin;
    std::string destination;
    std::string alternate;
    std::optional<Waypoint> originWaypoint;
    std::optional<Waypoint> destinationWaypoint;
    std::optional<Waypoint> alternateWaypoint;
    Route route;
    int plannedAltitude = 0;
    int plannedTas = 0;
    std::string flightTimeHours;
    std::string flightTimeMinutes;
    std::string fuelTimeHours;
    std::string fuelTimeMinutes;
    std::string eobt;
    std::string aobt;
    std::string remarks;
    bool isValid = true;
    VoiceType voiceType = VoiceType::Unknown;
};

struct FlightplanUpdatedEvent {
    std::string callsign;
    std::string origin;
    std::string destination;
    Route route;
    std::string acType;
    std::string altitude;
    std::string rules;
};

struct FlightplanRemovedEvent {
    std::string callsign;
};

struct FlightplanVoiceTypeChangedEvent {
    std::string callsign;
    VoiceType oldVoiceType;
    VoiceType newVoiceType;
};

struct FlightplanRouteChangedEvent {
    std::string callsign;
    std::string newRoute;
};

class FlightplanAPI {
public:
    virtual ~FlightplanAPI() = default;

    virtual std::vector<Flightplan> getAll() = 0;
    virtual std::optional<Flightplan> getByCallsign(const std::string& callsign) = 0;
};

} // namespace PluginSDK::Flightplan