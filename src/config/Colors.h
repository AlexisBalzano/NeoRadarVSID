#pragma once
#include <array>
#include <optional>

namespace Colors{
	std::optional<std::array<unsigned int, 3>> green = std::array<unsigned int, 3>{127, 252, 73};
	std::optional<std::array<unsigned int, 3>> white = std::array<unsigned int, 3>({ 255, 255, 255});
	std::optional<std::array<unsigned int, 3>> red = std::array<unsigned int, 3>({ 240, 0, 0 });
	std::optional<std::array<unsigned int, 3>> orange = std::array<unsigned int, 3>({ 255, 153, 51 });
}