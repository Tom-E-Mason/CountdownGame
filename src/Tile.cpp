
#include "Tile.h"

// -----------------------------------------------------------------------------
// Constructor.
//
// Arguments:
//     _tile   - m_strTile text string
//	   _middle - coordinates of m_strTile centre
// -----------------------------------------------------------------------------
Tile::Tile(const std::string& _tile, const olc::vf2d& _middle)
{
	m_vf2dMiddle = m_vf2dStringMiddle = _middle;
	SetTile(_tile);
}

// -----------------------------------------------------------------------------
// Converts a string to a number if it is made up of only numbers.
//
// Returns:
//     converted number
// -----------------------------------------------------------------------------
int Tile::Number() const
{
	for (const auto& t : m_strTile)
	{
		if (!isdigit(t))
		{
			return 0;
		}
	}

	return std::stoi(m_strTile);
}

// -----------------------------------------------------------------------------
// Sets string text and font size based on string length.
//
// Arguments:
//     str - m_strTile text
// Returns:
//     void
// -----------------------------------------------------------------------------
void Tile::SetTile(const std::string& _str)
{
	m_strTile = _str;
	if (m_strTile.length() == 1)
	{
		m_nTextSize = ONE_CHAR_TEXT_SIZE;
	}
	else if (m_strTile.length() == 2)
	{
		m_nTextSize = TWO_CHAR_TEXT_SIZE;
	}
	else if (m_strTile.length() == 3)
	{
		m_nTextSize = THREE_CHAR_TEXT_SIZE;
	}
	else
		return;

	AlignTextAndBackground();
}

// -----------------------------------------------------------------------------
// Sets m_strTile background based on string m_vf2dDimensions;
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void Tile::SetBackground()
{
	float textPixels = (float)(m_nTextSize * TEXT_ONLY_PIXEL_MULT);
	int offset = m_nTextSize * (m_strTile.length() - 1);
	olc::vf2d padding = 2 * olc::vf2d((float)m_nTextSize, (float)m_nTextSize);
	m_vf2dDimensions = m_vf2dStringPixelDimensions + padding;
}

// -----------------------------------------------------------------------------
// Sets text size and realigns the text with the background.
//
// Arguments:
//     size - text size (multiplies by 7 pixels in PixelGameEngine)
// Returns:
//     void
// -----------------------------------------------------------------------------
void Tile::SetTextSize(int _size)
{
	m_nTextSize = _size;
	AlignTextAndBackground();
};

// -----------------------------------------------------------------------------
// Makes a m_strTile draw itself to the screen.
//
// Arguments:
//     engine - PGE pointer allowing use of drawing tools
// Returns:
//     void
// -----------------------------------------------------------------------------
void Tile::DrawSelf(olc::PixelGameEngine* _engine) const
{
	_engine->FillRect(m_vf2dMiddle - m_vf2dDimensions / 2,
		              m_vf2dDimensions,
		              olc::Pixel(27, 54, 150));
	_engine->DrawString(m_vf2dStringMiddle - m_vf2dStringPixelDimensions / 2,
		                m_strTile,
		                olc::WHITE,
		                m_nTextSize);
}

// -----------------------------------------------------------------------------
// Sets string m_vf2dDimensions.
//
// Arguments:
//     engine - PGE pointer allowing use of drawing tools
// Returns:
//     void
// -----------------------------------------------------------------------------
void Tile::AlignTextAndBackground()
{
	int offset = m_nTextSize * (m_strTile.length() - 1);
	float textPixels = (float)(m_nTextSize * TEXT_ONLY_PIXEL_MULT);
	m_vf2dStringPixelDimensions = { textPixels * m_strTile.length() + offset,
							        textPixels };

	// characters "1", "T" and "I" don't appear central without this adjustment
	m_vf2dStringMiddle = m_vf2dMiddle;
	if (m_strTile.front() - '0' == 1
		|| m_strTile.front() == 'T'
		|| m_strTile.front() == 'I')
		m_vf2dStringMiddle.x -= (m_nTextSize / 2);
}