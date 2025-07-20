#pragma once
#include "config/Colors.h"

using namespace vsid;

namespace vsid::tagitems {
    class Color {
    public:
        Color() = delete;
        Color(const Color&) = delete;
        Color(Color&&) = delete;
        Color& operator=(const Color&) = delete;
        Color& operator=(Color&&) = delete;

		// Example color functions for different tag items
		static std::optional<std::array<unsigned int, 3>> colorizeTag() { return Colors::lightgreen; };
    };
 
}  // namespace vacdm::tagitems