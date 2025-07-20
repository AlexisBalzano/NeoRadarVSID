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
		static std::optional<std::array<unsigned int, 3>> colorizeRwy() { return Colors::white; }; //TODO: Implement actual color logic
		static std::optional<std::array<unsigned int, 3>> colorizeSid() { return Colors::white; }; //TODO: Implement actual color logic
    
    
    
    
        static std::optional<std::array<unsigned int, 3>> colorizeCfl(const int& cfl, const int& vsidCfl) {
			if (cfl == 0) return Colors::white;
            if (cfl == vsidCfl) return Colors::green;
            else return Colors::orange;
        }
    
    
    };
 
}  // namespace vacdm::tagitems