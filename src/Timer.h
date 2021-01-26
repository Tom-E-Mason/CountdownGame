
#pragma once

#include <iostream>
#include <chrono>
#include <string>

class Timer
{
public:
    enum class Unit
    {
        SECONDS = 1,
        MILLISECONDS = 1000,
        MICROSECONDS = 1000000,
    };

    Timer()
    {
        m_start = m_end = std::chrono::steady_clock::now();
        m_dur = m_dur.zero();
        m_unit = Unit::MILLISECONDS;
    }

    Timer(const std::string& _name, Unit _unit = Unit::MILLISECONDS)
    {
        m_name = _name;
        m_start = m_end = std::chrono::steady_clock::now();
        m_dur = m_dur.zero();
        m_unit = _unit;

    }

    Timer(std::string&& _name, Unit _unit = Unit::MILLISECONDS)
    {
        m_name = std::move(_name);
        m_start = m_end = std::chrono::steady_clock::now();
        m_dur = m_dur.zero();
        m_unit = _unit;
    }

    ~Timer()
    {
        m_end = std::chrono::steady_clock::now();

        if (m_name.length())
            std::cout << m_name << ": ";

        m_dur = m_end - m_start;

        std::string strUnit;
        switch (m_unit)
        {
        case Unit::SECONDS:
            strUnit = "s.";
            break;
        case Unit::MILLISECONDS:
            strUnit = "ms";
            break;
        case Unit::MICROSECONDS:
            strUnit = "us.";
            break;
        default:
            strUnit = " ---Unkown Unit!---";
        }

        std::cout << (int)(m_dur.count() * (int)m_unit) << strUnit << "\n";
    }

    

private:
    std::string m_name;
    std::chrono::steady_clock::time_point m_start;
    std::chrono::steady_clock::time_point m_end;
    std::chrono::duration<float> m_dur;
    Unit m_unit;
};
