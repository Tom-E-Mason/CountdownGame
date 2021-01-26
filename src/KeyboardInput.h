
#pragma once

#include <string>

#include "olcPixelGameEngine.h"

class KeyboardInput
{
public:
    ~KeyboardInput() {};
    static olc::Key ReceiveInput(std::string& _buffer,
                                 olc::PixelGameEngine* _pge)
    {
        return m_s_instance.ReceiveInputImpl(_buffer, _pge);
    }

private:
    KeyboardInput() {};
    KeyboardInput(const KeyboardInput& _other) = delete;
    void operator=(const KeyboardInput & _other) = delete;
    static KeyboardInput m_s_instance;


    olc::Key ReceiveInputImpl(std::string& _buffer,
                              olc::PixelGameEngine* _pge) const;
};
