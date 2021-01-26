
#pragma once

#include "olcPixelGameEngine.h"

struct ScreenText
{
    std::string string;
    uint32_t size;
    olc::vf2d pos;
    olc::Pixel col = olc::WHITE;
};

