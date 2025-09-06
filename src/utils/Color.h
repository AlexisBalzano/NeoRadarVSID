#pragma once
#include <array>
#include <optional>

namespace vsid {
    using Color = std::optional<std::array<unsigned int, 3>>;
    enum class ColorName { GREEN = 0, WHITE, RED, ORANGE };
}