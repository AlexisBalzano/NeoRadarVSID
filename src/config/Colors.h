#pragma once
#include <array>
#include <optional>

namespace Colors{
	// Example color definitions using RGB values
	std::optional<std::array<unsigned int, 3>> lightgreen = std::make_optional<std::array<unsigned int, 3>>({ 127, 252, 73 });
}