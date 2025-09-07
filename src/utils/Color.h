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
        REQUESTTEXT,
		XPDRSTDBY,
		STATRPA,
		NOPUSH,
		NOTKOFF,
		NOTAXI
    };
}