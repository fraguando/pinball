#pragma once
#include <array>
#include <cstdint>

namespace pinball::gl {

// A minimal 5x7 uppercase bitmap font for HUD/editor text. Each glyph is 7 rows;
// in each row the low 5 bits are columns (bit 4 = leftmost pixel).
class Font5x7 {
public:
    static constexpr int kCols = 5;
    static constexpr int kRows = 7;

    // Returns the 7 row-bitmasks for a character (unknown chars render blank).
    static const std::array<uint8_t, kRows>& glyph(char c);
};

} // namespace pinball::gl
