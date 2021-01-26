
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>

// -----------------------------------------------------------------------------
// CountdownCalculator class
// -----------------------------------------------------------------------------
class CountdownCalculator
{
public:
    CountdownCalculator();
    ~CountdownCalculator() {};

public:
    void Calculate(std::vector<int> numberTiles,
                   int _target);
    std::string GetBestSolution() const { return m_strBestSolution; };
    void CancelCalc() { m_bCancelCalc = true; };
    
private:
    std::unordered_map<std::string, int> m_mapNumbers;
    std::unordered_map<std::string, int> m_mapSolutions;
    int m_nNumSolutions;
    int m_nLatestSolution;
    int m_nLatestDiffFromTarget;
    int m_nTarget;
    std::string m_strBestSolution;

    std::atomic<bool> m_bCancelCalc;
    bool m_bSolved;

    std::chrono::system_clock::time_point start;
    std::chrono::duration<float> dur;

    // constants
    const std::string m_strNUMBER_TILES = "abcdef";
    const std::string m_strOPERATORS = "+-*/";

private:
    void CalcAllOperators(const std::pair<std::string, int>& firstOperand,
                          const std::pair<std::string, int>& secondOperand);
    bool IsCompatible(const std::string& firstOperand,
                      const std::string& secondOperand) const;
    void DeParenthesise(std::string& solution) const;
};

