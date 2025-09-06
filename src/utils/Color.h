#pragma once
#include <array>
#include <optional>

namespace vsid {
    using Color = std::optional<std::array<unsigned int, 3>>;
    enum class ColorName { 
        CONFIRMED = 0,
        UNCONFIRMED,
        CHECKFP,
        DEVIATION,
        ALERTTEXT,
        ALERTBACKGROUND,
        REQUESTTEXT
    };
}