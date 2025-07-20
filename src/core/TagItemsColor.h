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

    
    
    
    
        static std::optional<std::array<unsigned int, 3>> colorizeCfl(const int& cfl, const int& vsidCfl) {
			if (cfl == 0) return Colors::white;
            if (cfl == vsidCfl) return Colors::green;
            else return Colors::orange;
        }
    
		static std::optional<std::array<unsigned int, 3>> colorizeRwy(const std::string& rwy, const std::string& vsidRwy) {
            if (rwy == "") return Colors::white;
            if (rwy == vsidRwy) return Colors::green;
            else return Colors::orange;
        }
		static std::optional<std::array<unsigned int, 3>> colorizeSid(const std::string& sid, const std::string& vsidSid) {
            if (sid == "") return Colors::white;
            if (sid == vsidSid) return Colors::green;
            else return Colors::orange;
        }
    };
 
}  // namespace vacdm::tagitems