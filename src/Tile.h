
#pragma once

#include "olcPixelGameEngine.h"

#define ONE_CHAR_TEXT_SIZE 10
#define TWO_CHAR_TEXT_SIZE 6
#define THREE_CHAR_TEXT_SIZE 4
#define SOLUTION_TEXT_SIZE 5

#define TEXT_ONLY_PIXEL_MULT 7
#define TEXT_PLUS_SPACE_MULT 8

// -----------------------------------------------------------------------------
// Tile class for displaying letter and number tiles.
// -----------------------------------------------------------------------------
class Tile
{
public:
	Tile(const std::string& _tile, 
		 const olc::vf2d& _middle);
	
	int Number() const;

	void SetTile(const std::string& _str);
	void SetBackground();
	void SetTextSize(int _size);

	void DrawSelf(olc::PixelGameEngine* _engine) const;

private:
	std::string m_strTile;
	int m_nTextSize;
	olc::vf2d m_vf2dMiddle;
	olc::vf2d m_vf2dStringMiddle;
	olc::vf2d m_vf2dDimensions = { 100, 100 };
	olc::vf2d m_vf2dStringPixelDimensions;

private:
	void AlignTextAndBackground();
};
