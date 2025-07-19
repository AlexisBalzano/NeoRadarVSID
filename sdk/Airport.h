#pragma once


namespace PluginSDK::Airport {

/**
 * @enum AirportStatus
 * @brief Status of an airport (active/observed)
 */
enum class AirportStatus {
    Active = 0,
    Observed = 1
};

/**
 * @struct AirportConfig
 * @brief Configuration for an airport
 */
struct AirportConfig {
    std::string icao;
    std::vector<std::string> arrRunways;
    std::vector<std::string> depRunways;
    AirportStatus status;
};

/**
 * @struct AirportAddedEvent
 * @brief Event fired when an airport is added
 */
struct AirportAddedEvent {
    std::string icao;
};

/**
 * @struct AirportRemovedEvent
 * @brief Event fired when an airport is removed
 */
struct AirportRemovedEvent {
    std::string icao;
};

/**
 * @struct AirportStatusChangedEvent
 * @brief Event fired when an airport's status changes
 */
struct AirportStatusChangedEvent {
    std::string icao;
    AirportStatus status;
};

/**
 * @struct RunwayStatusChangedEvent
 * @brief Event fired when a runway status changes
 */
struct RunwayStatusChangedEvent {
    std::string icao;
    std::string runway;
    bool active;
    bool isArrival;
};

/**
 * @struct AirportConfigurationsUpdatedEvent
 * @brief Event fired when airport configurations are updated
 */
struct AirportConfigurationsUpdatedEvent {
    int configurationCount;
};

class AirportAPI {
public:
    virtual ~AirportAPI() = default;

    /**
     * @brief Get all airport configurations in the system
     * @return Vector of airport configurations
     */
    virtual std::vector<AirportConfig> getConfigurations() = 0;

    /**
     * @brief Get airport configuration by ICAO code
     * @param icao The ICAO code to look up
     * @return Airport configuration or nullptr if not found
     */
    virtual std::optional<AirportConfig> getConfigurationByIcao(const std::string& icao) = 0;
};

} // namespace PluginSDK::Airport
