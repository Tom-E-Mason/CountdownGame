
#include "KeyboardInput.h"

KeyboardInput KeyboardInput::m_s_instance;

olc::Key KeyboardInput::ReceiveInputImpl(std::string& _buffer,
                                         olc::PixelGameEngine* _pge) const
{
    auto TestKey = [&](olc::Key _key)
    {
        if (_pge->GetKey(_key).bReleased)
        {
            if (_pge->GetKey(olc::Key::SHIFT).bHeld)
                _buffer += 'A' - 1 + (int)_key;
            else
                _buffer += 'a' - 1 + (int)_key;

            return true;
        }

        return false;
    };

    if (TestKey(olc::Key::A)) return olc::Key::A;
    if (TestKey(olc::Key::B)) return olc::Key::B;
    if (TestKey(olc::Key::C)) return olc::Key::C;
    if (TestKey(olc::Key::D)) return olc::Key::D;
    if (TestKey(olc::Key::E)) return olc::Key::E;
    if (TestKey(olc::Key::F)) return olc::Key::F;
    if (TestKey(olc::Key::G)) return olc::Key::G;
    if (TestKey(olc::Key::H)) return olc::Key::H;
    if (TestKey(olc::Key::I)) return olc::Key::I;
    if (TestKey(olc::Key::J)) return olc::Key::J;
    if (TestKey(olc::Key::K)) return olc::Key::K;
    if (TestKey(olc::Key::L)) return olc::Key::L;
    if (TestKey(olc::Key::M)) return olc::Key::M;
    if (TestKey(olc::Key::N)) return olc::Key::N;
    if (TestKey(olc::Key::O)) return olc::Key::O;
    if (TestKey(olc::Key::P)) return olc::Key::P;
    if (TestKey(olc::Key::Q)) return olc::Key::Q;
    if (TestKey(olc::Key::R)) return olc::Key::R;
    if (TestKey(olc::Key::S)) return olc::Key::S;
    if (TestKey(olc::Key::T)) return olc::Key::T;
    if (TestKey(olc::Key::U)) return olc::Key::U;
    if (TestKey(olc::Key::V)) return olc::Key::V;
    if (TestKey(olc::Key::W)) return olc::Key::W;
    if (TestKey(olc::Key::X)) return olc::Key::X;
    if (TestKey(olc::Key::Y)) return olc::Key::Y;
    if (TestKey(olc::Key::Z)) return olc::Key::Z;
    if (_pge->GetKey(olc::Key::ENTER).bReleased) return olc::Key::ENTER;
    
    if (_pge->GetKey(olc::Key::SPACE).bReleased)
    {
        _buffer += " ";
        return olc::Key::SPACE;
    }
    if (_pge->GetKey(olc::Key::BACK).bReleased && _buffer.length())
    {
        _buffer.pop_back();
        return olc::Key::BACK;
    }

    return olc::Key::NONE;
}
