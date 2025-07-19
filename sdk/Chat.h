#pragma once
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace PluginSDK::Chat {

enum class ParameterType { String, Number, Boolean };

struct CommandParameter {
    std::string name;
    ParameterType type;
    bool typeExplicit = false;
    bool required = false;
    int length = 0;
    int minLength = 0;
    int maxLength = 0;
};

// Command definition
struct CommandDefinition {
    std::string name;
    std::string description;
    std::vector<CommandParameter> parameters;
    bool lastParameterHasSpaces = false;
};

struct ClientTextMessageEvent {
    std::string sentFrom;
    std::string message;
    std::optional<std::array<int, 3>> colour;
    bool isUrgent = false;
    bool useDedicatedChannel = false;
};

/**
 * @enum RequestType
 * @brief Type of flightplan request
 */
enum class RequestType {
    Unknown = 0,
    Clearance = 1,
    Pushback = 2,
    TaxiOut = 3,
    TaxiIn = 4,
    Startup = 5
};

/**
 * @struct FrequencyMessageReceivedEvent
 * @brief Event fired when a frequency message is received
 */
struct FrequencyMessageReceivedEvent {
    std::string sentFrom;
    std::string message;
    std::vector<int> frequencies;
    bool fromMe;
};

/**
 * @struct FlightplanMessageReceivedEvent
 * @brief Event fired when a flightplan message is received
 */
struct FlightplanMessageReceivedEvent {
    RequestType requestType;
    std::string callsign;
    std::string request;
};

/**
 * @struct ATISInfoMessageReceivedEvent
 * @brief Event fired when an ATIS info message is received
 */
struct ATISInfoMessageReceivedEvent {
    std::string callsign;
    std::string icao;
    std::string atisLetter;
};

/**
 * @struct PrivateMessageReceivedEvent
 * @brief Event fired when a private message is received
 */
struct PrivateMessageReceivedEvent {
    std::string sentFrom;
    std::string sentTo;
    std::string message;
    bool fromMe;
};

/**
 * @struct BroadcastMessageReceivedEvent
 * @brief Event fired when a broadcast message is received
 */
struct BroadcastMessageReceivedEvent {
    std::string sentFrom;
    std::string message;
    bool fromMe;
};

/**
 * @struct SupervisorMessageReceivedEvent
 * @brief Event fired when a supervisor message is received
 */
struct SupervisorMessageReceivedEvent {
    std::string sentFrom;
    std::string message;
    bool fromMe;
};

/**
 * @struct ServerMessageReceivedEvent
 * @brief Event fired when a server message is received
 */
struct ServerMessageReceivedEvent {
    std::string sentFrom;
    std::string message;
};

/**
 * @struct AtcMessageReceivedEvent
 * @brief Event fired when an ATC message is received
 */
struct AtcMessageReceivedEvent {
    std::string sentFrom;
    std::string message;
};

// Command validation result
struct ValidationResult {
    bool isValid;
    std::optional<std::string> message;
};

struct CommandResult {
    bool success;
    std::optional<std::string> message;
};

// Command Provider interface
class CommandProvider {
public:
    virtual ~CommandProvider() = default;

    // Execute a command with arguments
    virtual CommandResult Execute(
        const std::string& commandId, const std::vector<std::string>& args)
        = 0;

};

// Registration token
class RegistrationToken {
public:
    virtual ~RegistrationToken() = default;

    // Non-copyable but movable
    RegistrationToken(const RegistrationToken&) = delete;
    RegistrationToken& operator=(const RegistrationToken&) = delete;
    RegistrationToken(RegistrationToken&&) = default;
    RegistrationToken& operator=(RegistrationToken&&) = default;

protected:
    RegistrationToken() = default;
};

// Chat API
class ChatAPI {
public:
    virtual ~ChatAPI() = default;

   /**
     * @brief Registers a command with the system
     *
     * @param name Command name without dot prefix (e.g., "speed" for ".speed")
     * @param definition Command definition
     * @param provider Provider implementation handling execution and validation
     * @return Command ID if successful, empty string if failed (e.g., command name
     * collision or reserved name)
     *
     * @note The following command names are reserved and cannot be registered:
     *       "hello", "wx", "clear", "dev", "rings", "vis", "wallop", "chat", "center"
     */
    virtual std::string registerCommand(const std::string& name,
        const CommandDefinition& definition, std::shared_ptr<CommandProvider> provider)
        = 0;

    virtual bool unregisterCommand(const std::string& commandId) = 0;

    virtual void sendClientMessage(const ClientTextMessageEvent message) = 0;
};

} // namespace PluginSDK::Chat