
#pragma once

#include <iostream>

// -----------------------------------------------------------------------------
// WAVE Header class for storing WAVE file metadata.
// -----------------------------------------------------------------------------
class WavHeader
{
public:
    WavHeader() {};
    ~WavHeader() {};

    // -------------------------------------------------------------------------
    // Reads in each section of a standard RIFF wave file header and stores it
    // in its respective variable in a WavHeader object.
    //
    // Arguments:
    //     _fileWavId - file handle to wave file to be read
    //
    // Returns:
    //     void
    // -------------------------------------------------------------------------
    void FillHeader(FILE* _fileWavId)
    {
        fread(m_nRIFF, 1, 4, _fileWavId);
        fread(&m_nChunkSize, 4, 1, _fileWavId);
        fread(m_nWAVE, 1, 4, _fileWavId);
        fread(m_nFMT, 1, 4, _fileWavId);
        fread(&m_nSubchunk1Size, 4, 1, _fileWavId);
        fread(&m_nAudioFormat, 2, 1, _fileWavId);
        fread(&m_nNumOfChan, 2, 1, _fileWavId);
        fread(&m_nSamplesPerSec, 4, 1, _fileWavId);
        fread(&m_nBytesPerSec, 4, 1, _fileWavId);
        fread(&m_nBlockAlign, 2, 1, _fileWavId);
        fread(&m_nBitsPerSample, 2, 1, _fileWavId);
        fread(m_nSubchunk2ID, 1, 4, _fileWavId);
        fread(&m_nSubchunk2Size, 4, 1, _fileWavId);
    }

    // accessors
    uint16_t GetNumOfChan() const { return m_nNumOfChan; };
    uint32_t GetSamplesPerSec() const { return m_nSamplesPerSec; };
    uint32_t GetBytesPerSec() const { return m_nBytesPerSec; };
    uint16_t GetBlockAlign() const { return m_nBlockAlign; };
    uint16_t GetBitsPerSample() const { return m_nBitsPerSample; };

private:
    /* RIFF Chunk Descriptor */
    uint8_t         m_nRIFF[4] = { 0 };        // RIFF Header Magic header
    uint32_t        m_nChunkSize = 0;          // RIFF Chunk Size
    uint8_t         m_nWAVE[4] = { 0 };        // WAVE Header
    /* "fmt" sub-chunk */                      
    uint8_t         m_nFMT[4] = { 0 };         // FMT header
    uint32_t        m_nSubchunk1Size = 0;      // Size of the fmt chunk
    uint16_t        m_nAudioFormat = 0;        // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        m_nNumOfChan = 0;          // Number of channels 1=Mono 2=Stereo
    uint32_t        m_nSamplesPerSec = 0;      // Sampling Frequency in Hz
    uint32_t        m_nBytesPerSec = 0;        // bytes per second
    uint16_t        m_nBlockAlign = 0;         // 2=16-bit mono, 4=16-bit stereo
    uint16_t        m_nBitsPerSample = 0;      // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         m_nSubchunk2ID[4] = { 0 }; // "data"  string
    uint32_t        m_nSubchunk2Size = 0;      // Sampled data length
};
