#pragma once
#include "Aircraft.h"
#include "Flightplan.h"
#include <memory>
#include <string>
#include <vector>

namespace PluginSDK::Squawk {

class RegistrationToken;
class SquawkProviderInterface;

/**
 * @interface SquawkAPI
 * @brief Interface for squawk operations
 */
class SquawkAPI {
public:
    virtual ~SquawkAPI() = default;

    /**
     * @brief Register a squawk provider
     * @param provider The provider to register
     * @return True if registration was successful
     */
    virtual bool registerProvider(std::shared_ptr<SquawkProviderInterface> provider) = 0;

    /**
     * @brief Register a squawk provider with automatic lifetime management
     * @param provider The provider to register
     * @return Token that will automatically unregister the provider when destroyed
     */
    virtual std::unique_ptr<RegistrationToken> registerProviderWithToken(
        std::shared_ptr<SquawkProviderInterface> provider)
        = 0;

    /**
     * @brief Set the active squawk provider
     * @param providerName Name of the provider to activate
     * @return True if successful
     */
    virtual bool setActiveProvider(const char* providerName) = 0;

    /**
     * @brief Get available squawk providers
     * @return List of provider names
     */
    virtual std::vector<std::string> getAvailableProviders() = 0;
};

/**
 * @class RegistrationToken
 * @brief Token representing a registered provider
 */
class RegistrationToken {
public:
    virtual ~RegistrationToken() = default;

    // Non-copyable but movable
    RegistrationToken(const RegistrationToken&) = delete;
    RegistrationToken& operator=(const RegistrationToken&) = delete;
    RegistrationToken(RegistrationToken&&) = default;
    RegistrationToken& operator=(RegistrationToken&&) = default;

protected:
    // Only derived classes can construct this
    RegistrationToken() = default;
};

/**
 * @interface SquawkProviderInterface
 * @brief Interface for custom squawk providers
 */
class SquawkProviderInterface {
public:
    virtual ~SquawkProviderInterface() = default;

    /**
     * @brief Generate a squawk code
     * @param callsign
     * @param flightplan
     * @return Generated squawk code (must be 4 octal digits: 0-7)
     */
    virtual std::string GenerateSquawk(const std::string& callsign,
        const Aircraft::Aircraft& aircraft, const Flightplan::Flightplan& flightplan)
        = 0;

    /**
     * @brief Get the provider's name
     * @return Provider name (must be unique within the system)
     */
    virtual std::string GetProviderName() const = 0;

    /**
     * @brief Get the provider's priority
     * @return Priority value (higher values have higher priority)
     */
    virtual int GetPriority() const = 0;
};

/**
 * @struct SquawkAssignedEvent
 * @brief Event fired when a squawk code is assigned
 */
struct SquawkAssignedEvent {
    const char* callsign;
    const char* squawk;
    const char* providerName;
};

} // namespace PluginSDK
