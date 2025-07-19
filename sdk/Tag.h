#pragma once
#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>
namespace PluginSDK {
namespace Tag {

struct TagItemDefinition {
    std::string name;
    std::string defaultValue;
    std::vector<std::string> allowedActions;
};

struct TagActionDefinition {
    std::string name;
    bool requiresInput;
    std::string description;
};

struct TagContext {
    std::string callsign;
    std::string listId;
    std::optional<std::array<unsigned int, 3>> colour;
    std::map<std::string, std::string> data;
};

struct TagActionEvent {
    std::string actionId;
    std::string tagId;
    std::string callsign;
    int button;
    std::string userInput;
    std::map<std::string, std::string> data;
};

// Dropdown types
enum class DropdownComponentType {
    Button,
    Divider,
    Label,
    Toggle,
    Radio,
    Group,
    ScrollArea
};

enum class DropdownAlignmentType { Center, Right, Left };

struct DropdownComponentStyle {
    std::optional<std::array<unsigned int, 3>> backgroundColor;
    std::optional<std::array<unsigned int, 3>> textColor;
    std::optional<int> fontSize;
    std::optional<std::string> fontWeight;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<bool> border;
    std::optional<std::string> className;
    std::optional<DropdownAlignmentType> textAlign;
};

struct DropdownComponent {
    std::string id;
    DropdownComponentType type;
    std::string text;
    std::map<std::string, std::string> data;
    DropdownComponentStyle style;
    std::vector<DropdownComponent> children;
};

struct DropdownDefinition {
    std::string title;
    int width;
    int maxHeight;
    std::vector<DropdownComponent> components;
};

struct DropdownActionEvent {
    std::string actionId;
    std::string componentId;
    std::string tagId;
    std::string callsign;
    std::map<std::string, std::string> data;
};

class TagInterface {
public:
    virtual ~TagInterface() = default;

    virtual std::string RegisterTagItem(const TagItemDefinition& definition) = 0;
    virtual std::string RegisterTagAction(const TagActionDefinition& definition) = 0;
    virtual bool UpdateTagValue(
        const std::string& tagId, const std::string& value, const TagContext& context)
        = 0;

    virtual bool SetActionDropdown(
        const std::string& actionId, const DropdownDefinition& dropdown)
        = 0;
    virtual bool UpdateActionDropdown(
        const std::string& actionId, const DropdownDefinition& dropdown)
        = 0;
    virtual bool RemoveActionDropdown(const std::string& actionId) = 0;

    virtual bool GetDropdownForAction(
        const std::string& actionId, DropdownDefinition& outDropdown) const
        = 0;
};

class TagAPI {
public:
    virtual ~TagAPI() = default;
    virtual TagInterface* getInterface() = 0;
};

} // namespace Tag
} // namespace PluginSDK