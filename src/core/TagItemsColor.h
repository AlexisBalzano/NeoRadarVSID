#pragma once

using namespace vsid;

namespace vsid::tagitems {
    class Color {
    public:
        Color() = delete;
        Color(const Color&) = delete;
        Color(Color&&) = delete;
        Color& operator=(const Color&) = delete;
        Color& operator=(Color&&) = delete;

    };
 
}  // namespace vacdm::tagitems