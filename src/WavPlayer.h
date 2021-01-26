
#pragma once

#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <bitset>

#include <Windows.h>

#include "WavHeader.h"

// -----------------------------------------------------------------------------
// Class to open a WAVE file and play and pause it.
// -----------------------------------------------------------------------------
class WavPlayer
{
public:
    WavPlayer(std::wstring sOutputDevice, 
			  const char* strWavFile, 
			  unsigned int nBlocks = 8, 
			  unsigned int nBlockSamples = 512);
    ~WavPlayer();

	// produces vector of audio device names
	static std::vector<std::wstring> Enumerate();

	// getters
	bool IsCancelled() const { return m_bCancel; };
	bool HasStarted() const { return m_bStarted; };
	bool IsPaused() const { return m_bPaused; };
	HWAVEOUT GetHwDevice() { return m_hwDevice; };

	// playback control
	void Start();
	void TogglePause();

private:
	// wav header 
    FILE* fileWavHandle;
	unsigned int m_nBitsPerSample;
	unsigned int m_nSampleRate;
	unsigned int m_nChannels;
	unsigned int m_nBlockCount;
	unsigned int m_nBlockSamples;
	unsigned int m_nBlockCurrent;

	// blocks, headers and audio device handle
	short* m_pBlockMemory;
	WAVEHDR* m_pWaveHeaders;
	HWAVEOUT m_hwDevice;

	// threads for assigning samples to blocks and constructing the WavPlayer
	std::thread m_blockThread;
	std::thread m_constructThread;

	// utility variables
	std::atomic<bool> m_bReady;
	std::atomic<bool> m_bPaused;
	std::atomic<bool> m_bStarted;
	std::atomic<bool> m_bCancel;
	std::atomic<unsigned int> m_nBlockFree;
	std::condition_variable m_cvBlockNotZero;
	std::mutex m_muxBlockNotZero;

	// method for constructor thread
	void CreateNewPlayer(std::wstring _sOutputDevice,
						 const char* _strWavFile,
						 unsigned int _nBlocks,
						 unsigned int _nBlockSamples);
	
	// reads samples from file and stores them in allocated memory blocks
	void MainThread();

	short SampleCoder(short _sampleValue);

	// sound card handlers
	void waveOutProc(HWAVEOUT _hWaveOut,
		             UINT _uMsg,
		             DWORD _dwParam1,
		             DWORD _dwParam2);
	static void CALLBACK waveOutProcWrap(HWAVEOUT _hWaveOut,
		                                 UINT _uMsg,
		                                 DWORD _dwInstance,
		                                 DWORD _dwParam1,
		                                 DWORD _dwParam2);
};

