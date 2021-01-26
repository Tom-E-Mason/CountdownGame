
#define OLC_PGE_APPLICATION
#define _USE_MATH_DEFINES

#include "CountdownCalculator.h"
#include "Tile.h"
#include "WavPlayer.h"
#include "Timer.h"
#include "KeyboardInput.h"
#include "ScreenText.h"

#include <random>

#define NINE_LETTERS 9
#define SIX_NUMBERS 6
#define COUNTDOWN_LENGTH 30.0f

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define PIXEL_SIZE 1

#define BIT_DEPTH 24
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2
#define BUFFER_SIZE 512

// -----------------------------------------------------------------------------
// Version History
// -----------------------------------------------------------------------------
//
// 1.7
//     - Additions:
//         - Music via WavPlayer class.
//	       - Improved countdown calculator (CountdownCalculator 1.2b).
// 1.8
//     - Additions:
//         - Faster countdown calculator (CountdownCalculator 1.3).
//         - Tile picking method makes more sense (not much faster though).
//         - Tiles no longer pointers (never needed to be).
//         - Reset() is now targeted by round type.
//
//     - Issues:
//         - If Reset() and restarted too quickly, music and clock don't work...
// -----------------------------------------------------------------------------

class Tile;
class ScreenText;

// -----------------------------------------------------------------------------
// Countdown class derived from PixelGameEngine.
// -----------------------------------------------------------------------------
class Countdown : public olc::PixelGameEngine
{
public:
	Countdown();
	~Countdown() {};

	bool OnUserCreate() override;
	bool OnUserUpdate(float _fElapsedTime) override;

private:
	// game state variables
	bool m_bMainGame;
	bool m_bStartScreen;
	int m_nLettersRoundCount;
	int m_nNumbersRoundCount;
	std::vector<std::string> m_listPlayers;


	// randomising features
	std::default_random_engine m_RNG;

	// tiles
	std::vector<Tile> m_vTiles;
	const olc::vf2d m_CARD_INCREMENT = { 110, 0 };
	float fTilesY;

	// letters game
	bool m_bLettersGame;
	const std::string m_strCONSONANT_DIST = 
		"BBCCCDDDDDDFFGGGHHJKLLLLLMMMMNNNNNNNNPPPPQRRRRRRRRRSSSSSSSSSTTTTTTTTTVWXYZ";
	std::string m_strConsonants;
	const std::string m_strVOWEL_DIST = 
		"AAAAAAAAAAAAAAAEEEEEEEEEEEEEEEEEEEEEIIIIIIIIIIIIIOOOOOOOOOOOOOUUUUU";
	std::string m_strVowels;	

	// numbers game
	bool m_bNumbersGame;
	CountdownCalculator* m_calc;
	std::string m_strNumbersGameSolution;
	std::thread* m_calcThread;
	const std::string m_strBIG_NUMBER_DIST = "1234";
	std::string m_strBigNumbers;
	const std::string m_strSMALL_NUMBER_DIST = "112233445566778899";
	std::string m_strSmallNumbers;

	// anagram game
	bool m_bAnagramGame;
	std::vector<int> m_vAnagramLetterPicks;
	const std::string m_strANAGRAM_DIST = "COUNTDOWN";
	std::string m_strAnagram;

	// countdown clock
	std::chrono::system_clock::time_point m_tpTimeNow;
	std::chrono::system_clock::duration m_durElapsedTime;
	const float m_fClockRadius = 140;
	const olc::vf2d m_vf2dClockCentre = { (float)SCREEN_WIDTH / 2.0f, 
		                                  (float)SCREEN_HEIGHT / 3.0f };
	const olc::vf2d m_vf2dCLOCK_HAND_START = m_vf2dClockCentre - 
		                                     olc::vf2d(0, m_fClockRadius);
	olc::vf2d m_vf2dClockHand;
	std::chrono::system_clock::time_point m_tpCountdownStart;
	std::chrono::system_clock::time_point m_tpLastFrame;
	std::chrono::duration<double> m_durCountdownTimeSoFar;
	double m_dCountdownProgress;
	bool m_bRoundOver;
	const char* m_strMUSIC_FILE_NAME = ".\\music\\countdownTheme.wav";

	// music
	WavPlayer* m_music;

	// start screen
	ScreenText m_txtTitle;
	ScreenText m_txtEnterName;
	ScreenText m_txtUserInput;
	ScreenText m_txtStart;

private:
	olc::vf2d CalcFirstCardCoordinates(int _numCards);
	void GenerateNewTile(std::string& _strTileSet);

	// main game
	void Reset();
	void LettersGame();
	void NumbersGame();
	void AnagramGame();
	void StartCountdown();
	void CountdownClock();

	void StartScreen();
	olc::vf2d StringPixelDimensions(const ScreenText& _text) const;
	olc::vf2d CentreAdjustString(const ScreenText& _text,
		                         olc::vf2d _middle) const;
	olc::vf2d AppendString(const ScreenText& _first,
		                   const ScreenText& _second);
};


