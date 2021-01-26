
#include "Countdown.h"

// -----------------------------------------------------------------------------
// Constructor.
// -----------------------------------------------------------------------------
Countdown::Countdown()
{
	sAppName = "Countdown";

	m_bMainGame = false;
	m_bStartScreen = true;

	// tile piles set to fixed distributions
	m_strConsonants = m_strCONSONANT_DIST;
	m_strVowels = m_strVOWEL_DIST;
	m_strBigNumbers = m_strBIG_NUMBER_DIST;
	m_strSmallNumbers = m_strSMALL_NUMBER_DIST;
	m_strAnagram = m_strANAGRAM_DIST;

	m_vTiles.reserve(NINE_LETTERS);

	m_RNG.seed(
		(unsigned int)std::chrono::system_clock::now().time_since_epoch().count()
	);

	// countdown clock
	m_vf2dClockHand = m_vf2dCLOCK_HAND_START;
	m_tpCountdownStart = std::chrono::system_clock::now();
	m_tpLastFrame = std::chrono::system_clock::now();
	m_durCountdownTimeSoFar = m_durCountdownTimeSoFar.zero();
	m_dCountdownProgress = 0;
	m_bRoundOver = false;
	m_music = new WavPlayer(WavPlayer::Enumerate().at(0), m_strMUSIC_FILE_NAME);

	m_bLettersGame = m_bNumbersGame = m_bAnagramGame = false;

	m_calc = nullptr;
	m_calcThread = nullptr;
}

// -----------------------------------------------------------------------------
// Calculates the location of the first number or letter card based on the 
// number of cards to be displayed.
//
// Arguments:
//     numCards - number of cards to be displayed
// Returns:
//     olc::vf2d - coordinates of first card
// -----------------------------------------------------------------------------
olc::vf2d Countdown::CalcFirstCardCoordinates(int numCards)
{
	float offset = (float)((int)(numCards / 2));

	if (numCards % 2 == 0)
		offset -= 0.5;

	olc::vf2d coords((float)(ScreenWidth() / 2.0f - 
		                     m_CARD_INCREMENT.x * offset),
		             (float)(2.0f * ScreenHeight() / 3.0f));
		
	return olc::vf2d((float)(ScreenWidth() / 2.0f - m_CARD_INCREMENT.x * offset),
		             (float)(2.0f * ScreenHeight() / 3.0f));
}

// -----------------------------------------------------------------------------
// Creates a new Tile object and stores it in a vector. Randomly selects the
// tile from the remaining tiles of a given set (consonants, vowels, etc.). The
// selected tile is removed from the set.
//
// Arguments:
//     _strTileSet - remaining tiles to choose from
//     
// Returns:
//     void
// -----------------------------------------------------------------------------
void Countdown::GenerateNewTile(std::string& _strTileSet)
{
	// random tile from pile
	std::uniform_int_distribution<int> distUniform(0, _strTileSet.length() - 1);
	int idx = distUniform(m_RNG);

	std::string strTileText = std::string(1, _strTileSet.at(idx));
	_strTileSet.erase(idx, 1);

	// positions tile
	olc::vf2d vf2dTileCoordinates;
	if (isdigit(strTileText.front()))
		vf2dTileCoordinates = CalcFirstCardCoordinates(SIX_NUMBERS);
	else
		vf2dTileCoordinates = CalcFirstCardCoordinates(NINE_LETTERS);

	vf2dTileCoordinates = vf2dTileCoordinates +
		                  m_CARD_INCREMENT *
		                  (float)m_vTiles.size();

	m_vTiles.emplace_back(strTileText,
		                  vf2dTileCoordinates);
}



// -----------------------------------------------------------------------------
// Handles resets after each round.
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void Countdown::Reset()
{
	if (GetKey(olc::Key::R).bReleased)
	{
		if (m_music->HasStarted())
		{
			delete m_music;
			m_music = new WavPlayer(WavPlayer::Enumerate().at(0), m_strMUSIC_FILE_NAME);
		}

		// round specific resets
		if (m_bLettersGame)
		{
			m_strConsonants = m_strCONSONANT_DIST;
			m_strVowels = m_strVOWEL_DIST;

			m_bLettersGame = false;
		}
		else if (m_bNumbersGame)
		{
			m_strBigNumbers = m_strBIG_NUMBER_DIST;
			m_strSmallNumbers = m_strSMALL_NUMBER_DIST;

			if (m_calc != nullptr)
			{
				m_calc->CancelCalc();
				m_calcThread->join();
				delete m_calc;
				m_calc = nullptr;
				delete m_calcThread;
			}

			m_strNumbersGameSolution.clear();

			m_bNumbersGame = false;
		}
		else if (m_bAnagramGame)
		{
			m_strAnagram = m_strANAGRAM_DIST;
			m_bAnagramGame = false;
		}

		m_vTiles.clear();
		
		m_bRoundOver = false;
		m_vf2dClockHand = m_vf2dCLOCK_HAND_START;
	}
}

// -----------------------------------------------------------------------------
// Starts Letters game and displays selected letter tiles
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void Countdown::LettersGame()
{
	// adds consonants and vowels to Letter vector
	if (!m_bNumbersGame && !m_bAnagramGame)
	{
		if (GetKey(olc::Key::C).bReleased && m_vTiles.size() < 9)
			GenerateNewTile(m_strConsonants);
		if (GetKey(olc::Key::V).bReleased && m_vTiles.size() < 9)
			GenerateNewTile(m_strVowels);
		if (m_vTiles.size() > 0)
			m_bLettersGame = true;
	}

	// starts countdown at nine letters and draws letter tiles
	if (m_bLettersGame)
	{
		if (m_vTiles.size() == NINE_LETTERS && !m_music->HasStarted() && !m_bRoundOver)
			StartCountdown();
		for (const auto& letter : m_vTiles)
			letter.DrawSelf(this);
	}
}

// -----------------------------------------------------------------------------
// Starts Numbers game and displays selected number tiles, and calculates a
// solution.
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void Countdown::NumbersGame()
{
	// adds big and small numbers to Number vector
	if (!m_bLettersGame && !m_bAnagramGame)
	{
		if (GetKey(olc::Key::B).bReleased && m_vTiles.size() < 6 && m_strBigNumbers.size())
		{
			GenerateNewTile(m_strBigNumbers);

			int correctedValue = m_vTiles.at(m_vTiles.size() - 1).Number() * 25;
			m_vTiles.back().SetTile(std::to_string(correctedValue));
		}
		if (GetKey(olc::Key::S).bReleased && m_vTiles.size() < 6)
			GenerateNewTile(m_strSmallNumbers);
		if (m_vTiles.size() > 0)
			m_bNumbersGame = true;
	}

	if (m_bNumbersGame)
	{
		// starts clock when six numbers chosen
		if (m_vTiles.size() == SIX_NUMBERS && !m_music->HasStarted() && !m_bRoundOver)
		{
			olc::vf2d targetPosition = { SCREEN_WIDTH / 2, SCREEN_HEIGHT * 5 / 6 };
			std::uniform_int_distribution<int> distTargetNumber(100, 1000);
			m_vTiles.emplace_back(std::to_string(distTargetNumber(m_RNG)),
				                                 targetPosition);
			m_vTiles.back().SetTextSize(ONE_CHAR_TEXT_SIZE);
			m_vTiles.back().SetBackground();

			StartCountdown();

			// new thread for calculation
			std::vector<int> numbers;
			for (int i = 0; i < SIX_NUMBERS; i++)
				numbers.push_back(m_vTiles.at(i).Number());

			m_calc = new CountdownCalculator;
			m_calcThread = new std::thread(&CountdownCalculator::Calculate,
				                           m_calc,
				                           numbers,
				                           m_vTiles.back().Number());
		}

		// retrieves solution and draws it
		if (GetKey(olc::Key::ENTER).bReleased && m_bRoundOver)
		{
			m_strNumbersGameSolution = m_calc->GetBestSolution();
			m_vTiles.emplace_back(m_strNumbersGameSolution, 
				                  olc::vf2d(SCREEN_WIDTH / 2, 50));
			m_vTiles.back().SetTextSize(SOLUTION_TEXT_SIZE);
			m_vTiles.back().SetBackground();
		}
	}

	// draws number tiles
	for (const auto& number : m_vTiles)
		number.DrawSelf(this);
}

// -----------------------------------------------------------------------------
// Starts the Anagram game and displays the anagram.
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void Countdown::AnagramGame()
{
	// starts when user presses A key and creates the anagram
	if (GetKey(olc::Key::A).bReleased && !m_bLettersGame && !m_bNumbersGame && !m_bAnagramGame)
	{
		m_bAnagramGame = true;
		StartCountdown();

		// assigns letters
		for (size_t i = 0; i < m_strANAGRAM_DIST.size(); ++i)
			GenerateNewTile(m_strAnagram);
	}

	if (m_bAnagramGame)
	{
		// displays letter tiles
		for (const auto& letter : m_vTiles)
			letter.DrawSelf(this);
		
		// displays anagram solution
		if (GetKey(olc::Key::ENTER).bReleased && (m_bRoundOver || m_music->IsPaused()))
		{
			for (size_t i = 0; i < m_vTiles.size(); ++i)
				m_vTiles.at(i).SetTile(std::string(1, m_strANAGRAM_DIST.at(i)));
			
			m_bRoundOver = true;
		}
	}
}

// -----------------------------------------------------------------------------
// Starts music and clock.
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void Countdown::StartCountdown()
{
	m_music->Start();
	m_tpCountdownStart = m_tpTimeNow;
}

// -----------------------------------------------------------------------------
// Handles the countdown clock
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void Countdown::CountdownClock()
{
	if (m_music != nullptr && m_music->HasStarted())
	{
		if (m_music->IsPaused())
		{
			m_tpCountdownStart += m_durElapsedTime;
		}

		m_durCountdownTimeSoFar = m_tpTimeNow -
			                 m_tpCountdownStart;
		m_dCountdownProgress = m_durCountdownTimeSoFar.count() / COUNTDOWN_LENGTH;

		// pauses clock
		if (GetKey(olc::Key::SPACE).bReleased && m_bAnagramGame && !m_bRoundOver)
			m_music->TogglePause();

		// ends countdown
		if (m_dCountdownProgress >= 1)
		{
			m_dCountdownProgress = 1;
			m_bRoundOver = true;
			if (m_calc != nullptr)
				m_calc->CancelCalc();
		}

		m_vf2dClockHand.x = m_vf2dClockCentre.x + (float)(140 * sin(m_dCountdownProgress * M_PI));
		m_vf2dClockHand.y = m_vf2dClockCentre.y - (float)(140 * cos(m_dCountdownProgress * M_PI));
	}
	FillCircle(m_vf2dClockCentre, 140);
	DrawLine(m_vf2dClockCentre, m_vf2dClockHand, olc::BLUE);
}

void Countdown::StartScreen()
{
	olc::Key key = KeyboardInput::ReceiveInput(m_txtUserInput.string, this);
	if (key == olc::Key::ENTER)
		m_listPlayers.insert(m_listPlayers.begin(),
			                 std::move(m_txtUserInput.string));

	DrawString(m_txtTitle.pos,
		       m_txtTitle.string,
		       m_txtTitle.col,
		       m_txtTitle.size);
	DrawString(m_txtEnterName.pos,
		       m_txtEnterName.string,
		       m_txtEnterName.col,
		       m_txtEnterName.size);
	DrawString(m_txtUserInput.pos,
		       m_txtUserInput.string,
		       m_txtUserInput.col,
		       m_txtUserInput.size);
	DrawString(m_txtStart.pos,
		       m_txtStart.string,
		       m_txtStart.col,
		       m_txtStart.size);

	olc::vf2d vf2dPlayerList = m_txtUserInput.pos;
	for (const auto& p : m_listPlayers)
	{
		vf2dPlayerList.y += (TEXT_PLUS_SPACE_MULT + 1) * m_txtUserInput.size;
		DrawString(vf2dPlayerList, p, olc::WHITE, m_txtUserInput.size);
	}

	if (GetMouse(0).bReleased)
	{
		olc::vf2d mouse = { (float)GetMouseX(), (float)GetMouseY() };
		olc::vf2d bottomRight = m_txtStart.pos +
			                    StringPixelDimensions(m_txtStart);

		if (mouse.x > m_txtStart.pos.x
			&& mouse.y > m_txtStart.pos.y
			&& mouse.x < bottomRight.x
			&& mouse.y < bottomRight.y)
		{
			m_bStartScreen = false;
			m_bMainGame = true;
		}

	}
}

olc::vf2d Countdown::CentreAdjustString(const ScreenText& _text,
	                                    olc::vf2d _middle) const
{
	olc::vf2d vf2dStringDimensions = StringPixelDimensions(_text);
	return _middle - vf2dStringDimensions / 2;
}

olc::vf2d Countdown::StringPixelDimensions(const ScreenText& _text) const
{
	// each character is 7 * _size pixels wide and high and the space between
	// is just _size
	float fPixelWidth = TEXT_ONLY_PIXEL_MULT * _text.size * _text.string.length() +
		                _text.size * (_text.string.length() - 1);
	float fPixelHeight = TEXT_ONLY_PIXEL_MULT * _text.size;

	return { fPixelWidth, fPixelHeight };
}

olc::vf2d Countdown::AppendString(const ScreenText& _first,
	                              const ScreenText& _second)
{
	olc::vf2d vf2dStringDimensions = StringPixelDimensions(_first);

	if (_first.size == _second.size)
		return { _first.pos.x + vf2dStringDimensions.x, _first.pos.y };

	float fDiff = (_first.size - _second.size) / 2;
	float fNewY = _first.pos.y + fDiff * TEXT_ONLY_PIXEL_MULT;

	return { _first.pos.x + vf2dStringDimensions.x, fNewY };
}

// -----------------------------------------------------------------------------
// Called when program is run. Allows initialisation of variables dependent on
// screen dimensions.
//
// Returns:
//     true
// -----------------------------------------------------------------------------
bool Countdown::OnUserCreate()
{
	fTilesY = (float)(2.0f * ScreenHeight() / 3.0f);

	m_txtTitle.string = "COUNTDOWN";
	m_txtTitle.size = 16,
		m_txtTitle.pos = CentreAdjustString(m_txtTitle,
			{ (float)ScreenWidth() / 2,
			  (float)ScreenHeight() / 6 });

	m_txtEnterName.string = "Enter Name: ";
	m_txtEnterName.size = 4,
		m_txtEnterName.pos = { m_txtTitle.pos.x, 200 };

	m_txtUserInput.size = 4,
		m_txtUserInput.pos = AppendString(m_txtEnterName, m_txtUserInput);

	m_txtStart.string = "START";
	m_txtStart.size = 10;
	m_txtStart.pos = CentreAdjustString(m_txtStart,
		{ (float)ScreenWidth() / 2,
		   5 * (float)ScreenHeight() / 6 });

	return true;
}

// -----------------------------------------------------------------------------
// Called each frame.
// 
// Arguments:
//     fElapsedTime - time between frames in ms
// Returns:
//     true
// -----------------------------------------------------------------------------
bool Countdown::OnUserUpdate(float fElapsedTime)
{
	Clear(olc::Pixel(107, 161, 230));
	m_tpTimeNow = std::chrono::system_clock::now();
	m_durElapsedTime = m_tpTimeNow - m_tpLastFrame;

	if (m_bMainGame)
	{
		Reset();
		LettersGame();
		NumbersGame();
		AnagramGame();
		CountdownClock();
	}
	else if (m_bStartScreen)
		StartScreen();

	m_tpLastFrame = m_tpTimeNow;
	return true;
}

// -----------------------------------------------------------------------------
// Starts the program.
// -----------------------------------------------------------------------------
int main()
{
	Countdown demo;
	if (demo.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_SIZE, PIXEL_SIZE))
		demo.Start();
	return 0;
}

