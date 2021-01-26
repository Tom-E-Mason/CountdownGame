
#include "WavPlayer.h"

// -----------------------------------------------------------------------------
// Constructor. Can take some time so implementation takes place in a new 
// thread.
//
// Arguments:
//     _sOutputDevice - output device name
//	   _strWavFile    - WAVE filename
//     _nBlocks       - number of audio blocks
//     _nBlockSamples - number of samples per audio block
// -----------------------------------------------------------------------------
WavPlayer::WavPlayer(std::wstring _sOutputDevice, 
	                 const char* _strWavFile, 
	                 unsigned int _nBlocks, 
	                 unsigned int _nBlockSamples)
{
	m_constructThread = std::thread(&WavPlayer::CreateNewPlayer, 
		                            this, 
		                            _sOutputDevice, 
		                            _strWavFile, 
		                            _nBlocks, 
		                            _nBlockSamples);
}

// -----------------------------------------------------------------------------
// Destructor. Joins with the constructor thread and cancels playback, waiting,
// for the block thread to join.
// -----------------------------------------------------------------------------
WavPlayer::~WavPlayer()
{
	m_constructThread.join();

	m_bCancel = true;
	if (m_bPaused)
	{
		std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
		m_cvBlockNotZero.notify_one();
	}	

	m_blockThread.join();
}

// -----------------------------------------------------------------------------
// Produces a vector of audio devices.
//
// Returns:
//     vector of audio devices.
// -----------------------------------------------------------------------------
std::vector<std::wstring> WavPlayer::Enumerate()
{
	int nDeviceCount = waveOutGetNumDevs();
	std::vector<std::wstring> sDevices;
	WAVEOUTCAPS woc;
	for (int n = 0; n < nDeviceCount; n++)
		if (waveOutGetDevCaps(n, &woc, sizeof(WAVEOUTCAPS)) == S_OK)
			sDevices.push_back(woc.szPname);
	return sDevices;
}

void WavPlayer::Start()
{
	m_bStarted = true;
	m_bPaused = false;
	waveOutRestart(m_hwDevice);
}

// -----------------------------------------------------------------------------
// Plays and pauses audio using Windows Waveform functions.
//
// Returns:
//     void.
// -----------------------------------------------------------------------------
void WavPlayer::TogglePause()
{
	if (!m_bPaused)
	{
		m_bPaused = true;
		waveOutPause(m_hwDevice);
	}
	else
	{
		m_bPaused = false;
		waveOutRestart(m_hwDevice);
	}
}

// -----------------------------------------------------------------------------
// Called from constructor, sets up audio device and memory for audio buffers.
//
// Arguments:
//     _sOutputDevice - output device name
//	   _strWavFile    - WAVE filename
//     _nBlocks       - number of audio blocks
//     _nBlockSamples - number of samples per audio block
// -----------------------------------------------------------------------------
void WavPlayer::CreateNewPlayer(std::wstring _sOutputDevice,
	                            const char* _strWavFile,
	                            unsigned int _nBlocks,
	                            unsigned int _nBlockSamples)
{
	// opens file and stores header data
	fileWavHandle = nullptr;
	fopen_s(&fileWavHandle, _strWavFile, "rb");
	WavHeader headerData;
	headerData.FillHeader(fileWavHandle);

	// stores important header data locally
	m_bReady = false;
	m_nBitsPerSample = headerData.GetBitsPerSample();
	m_nSampleRate = headerData.GetSamplesPerSec();
	m_nChannels = headerData.GetNumOfChan();
	m_nBlockCount = _nBlocks;
	m_nBlockSamples = _nBlockSamples;
	m_nBlockFree = m_nBlockCount;
	m_nBlockCurrent = 0;
	m_pBlockMemory = nullptr;
	m_pWaveHeaders = nullptr;

	// validates device
	std::vector<std::wstring> devices = Enumerate();
	auto d = std::find(devices.begin(), devices.end(), _sOutputDevice);
	if (d != devices.end())
	{
		// device found
		int nDeviceId = distance(devices.begin(), d);

		// stores header info to inform the audio device of the audio format
		WAVEFORMATEX waveFormat;
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nSamplesPerSec = m_nSampleRate;
		waveFormat.wBitsPerSample = m_nBitsPerSample;
		waveFormat.nChannels = m_nChannels;
		waveFormat.nBlockAlign = headerData.GetBlockAlign();
		waveFormat.nAvgBytesPerSec = headerData.GetBytesPerSec();
		waveFormat.cbSize = 0;

		// opens device if valid
		if (waveOutOpen(&m_hwDevice, nDeviceId, &waveFormat, (DWORD_PTR)waveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
			return;
		waveOutPause(m_hwDevice);

		// allocates block memory
		m_pBlockMemory = new short[m_nBlockCount * m_nBlockSamples];
		if (m_pBlockMemory == nullptr)
			return;
		ZeroMemory(m_pBlockMemory, sizeof(short) * m_nBlockCount * m_nBlockSamples);

		// allocates metadata memory
		m_pWaveHeaders = new WAVEHDR[m_nBlockCount];

		if (m_pWaveHeaders == nullptr)
			return;
		ZeroMemory(m_pWaveHeaders, sizeof(WAVEHDR) * m_nBlockCount);

		// links headers to block memory
		for (unsigned int n = 0; n < m_nBlockCount; n++)
		{
			m_pWaveHeaders[n].dwBufferLength = m_nBlockSamples * sizeof(short);
			m_pWaveHeaders[n].lpData = (LPSTR)(m_pBlockMemory + (n * m_nBlockSamples));
		}

		m_bReady = m_bPaused = true;
		m_bStarted = m_bCancel = false;
		m_blockThread = std::thread(&WavPlayer::MainThread, this);

		std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
		m_cvBlockNotZero.notify_one();
	}
}

// -----------------------------------------------------------------------------
// Fills blocks of samples for soundcard.
//
// Returns:
//     void.
// -----------------------------------------------------------------------------
void WavPlayer::MainThread()
{
	while (m_bReady)
	{
		// wait for a block to become available
		if (m_nBlockFree == 0)
		{
			std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
			m_cvBlockNotZero.wait(lm);
		}

		// uses block
		m_nBlockFree--;

		// prepares block for processing
		if (m_pWaveHeaders[m_nBlockCurrent].dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));

		int nCurrentBlockStartSample = m_nBlockCurrent * m_nBlockSamples;

		bool bNoMoreSamples = false;
		for (unsigned int n = 0; n < m_nBlockSamples; n++)
		{
			short nNewSample = 0;
			if (!bNoMoreSamples)
			{
				if (fread(&nNewSample, m_nBitsPerSample / 8, 1, fileWavHandle) <= 0)
					bNoMoreSamples = true;
				//nNewSample = sampleCoder(nNewSample);
			}
			if (m_bCancel)
				bNoMoreSamples = true;

			m_pBlockMemory[nCurrentBlockStartSample + n] = nNewSample;
		}

		// writes block and to sound device
		if (!m_bPaused || (m_bPaused && !m_bCancel))
		{
			waveOutPrepareHeader(m_hwDevice,
				                 &m_pWaveHeaders[m_nBlockCurrent],
				                 sizeof(WAVEHDR));
			waveOutWrite(m_hwDevice,
				         &m_pWaveHeaders[m_nBlockCurrent],
				         sizeof(WAVEHDR));
			m_nBlockCurrent = (m_nBlockCurrent + 1) % m_nBlockCount;
		}

		// cleans up at the end
		if (bNoMoreSamples)
		{
			fclose(fileWavHandle);
			waveOutReset(m_hwDevice);
			waveOutClose(m_hwDevice);
			return;
		}
	}
}

// -----------------------------------------------------------------------------
// Samples are read from the WAVE file in standard binary to decimal. This 
// method converts the decimal back to binary and then back to decimal but 
// interprets the binary value in two's complement, as this is how 
// waveOutWrite() is intended to work.
//
// Arguments:
//    _nSampleValue - decimal value of 16 bit sample
//
// Returns:
//     int representing the correct sample value.
// -----------------------------------------------------------------------------
short WavPlayer::SampleCoder(short _nSampleValue)
{
	// converts to binary
	std::bitset<16> bsSampleBits(_nSampleValue);

	// if MSB not set then it's a positive number and can be returned
	if (!bsSampleBits.test(bsSampleBits.size() - 1))
		return _nSampleValue;
	else
	{
		_nSampleValue = (short)-pow(2, bsSampleBits.size() - 1);
		for (size_t i = 0; i < bsSampleBits.size(); i++)
		{
			short nBitValue = 0;
			if (bsSampleBits.test(i))
				nBitValue = (short)pow(2, (int)i - 1);
			_nSampleValue += nBitValue;
		}
	}

	return _nSampleValue;
}

// -----------------------------------------------------------------------------
// Handler for soundcard request for more data.
//
// Arguments:
//     _hWaveOut - audio device handle
//     _uMsg     - user message
//     _dwParam1 - message parameter
//     _dwParam2 - message parameter
//
// Returns:
//     void.
// -----------------------------------------------------------------------------
void WavPlayer::waveOutProc(HWAVEOUT _hWaveOut,
	                        UINT _uMsg,
	                        DWORD _dwParam1,
	                        DWORD _dwParam2)
{
	// returns if block still being read
	if (_uMsg != WOM_DONE)
		return;

	m_nBlockFree++;
	std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
	m_cvBlockNotZero.notify_one();
}

// -----------------------------------------------------------------------------
// Static wrapper for sound card handler.
//
// Arguments:
//     _hWaveOut   - audio device handle
//     _uMsg       - user message
//     _dwInstance - user instance data specified with waveOutOpen
//     _dwParam1   - message parameter
//     _dwParam2   - message parameter
//
// Returns:
//     void
// -----------------------------------------------------------------------------
void CALLBACK WavPlayer::waveOutProcWrap(HWAVEOUT _hWaveOut,
	                                     UINT _uMsg,
	                                     DWORD _dwInstance,
	                                     DWORD _dwParam1,
	                                     DWORD _dwParam2)
{
		((WavPlayer*)_dwInstance)->waveOutProc(_hWaveOut,
			                                   _uMsg,
			                                   _dwParam1, 
		                                       _dwParam2);
}