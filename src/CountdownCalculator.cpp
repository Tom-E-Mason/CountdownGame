
#include "CountdownCalculator.h"

// -----------------------------------------------------------------------------
// Constructor.
// -----------------------------------------------------------------------------
CountdownCalculator::CountdownCalculator()
{
    //m_mapSolutions = std::unordered_map<std::string, int>(110000000);
    m_mapSolutions = std::unordered_map<std::string, int>(2048);
    m_nNumSolutions = 0;
    m_nLatestSolution = 0;
    m_nLatestDiffFromTarget = 0;
    m_nTarget = 0;
    m_bCancelCalc = false;
    m_bSolved = false;
}

// -----------------------------------------------------------------------------
// Attempts to find a solution for the m_nTarget value with the provided operands.
// Uses a 'quick' method ignoring operations between existing solutions but will
// solve almost everything given to it. The 'long' method is commented out and
// takes far too long on a standard machine to run. 
//
// Arguments:
//     numberTiles - vector of operands to make up the solution
//     _target     - m_nTarget value
// Returns:
//     void
// -----------------------------------------------------------------------------
void CountdownCalculator::Calculate(std::vector<int> numberTiles, 
                                    int _target)
{
    start = std::chrono::system_clock::now();

    // assigns letter keys to number tiles
    for (size_t i = 0; i < numberTiles.size(); i++)
    {
        m_mapNumbers.emplace((m_strNUMBER_TILES.substr(i, 1)), numberTiles.at(i));
        m_mapSolutions.emplace((m_strNUMBER_TILES.substr(i, 1)), numberTiles.at(i));
    }

    m_nTarget = _target;
    m_nLatestDiffFromTarget = m_nTarget;

    bool bFinished = false;
    while (!bFinished)
    {
        // edge case where solution is one of the tiles
        for (const auto& num : m_mapNumbers)
        {
            if (num.second == m_nTarget)
            {
                m_strBestSolution = std::to_string(m_nTarget) +
                                  " = " +
                                  std::to_string(m_nTarget);
                return;
            }
        }

        int nNewKeysAdded = 0;
        int nCurrentNumSolutions = m_nNumSolutions;
        for (const auto& sol : m_mapSolutions)
        {
            for (const auto& num : m_mapNumbers)
            {
                CalcAllOperators(sol, 
                                 num);
                if (m_bCancelCalc)
                    break;
            }
            if (m_bCancelCalc)
                break;
        }
        //for (auto one : m_mapSolutions)
        //{
        //    for (auto two : m_mapSolutions)
        //    {
        //        int numBefore = m_nNumSolutions;
        //        //if (sol.second > num.second && !bFinished)
        //        //if (!bFinished)
        //        calcAllOperators(one,
        //                         two);
        //        if (m_bCancelCalc)
        //            return;
        //    }
        //}
        nNewKeysAdded = m_nNumSolutions - nCurrentNumSolutions;
        if (nNewKeysAdded == 0)
        {
            DeParenthesise(m_strBestSolution);
            m_mapNumbers.clear();
            m_mapSolutions.clear();
            return;
        }
    }
}

// -----------------------------------------------------------------------------
// Runs all viable operations between two operands.
//
// Arguments:
//     _firstOperand  - first operand to the left of the operator
//     _secondOperand - second operand to the right of the operator
// Returns:
//     bool - true if a solution is found, otherwise false
// -----------------------------------------------------------------------------
void CountdownCalculator::CalcAllOperators(const std::pair<std::string, int>& _firstOperand,
                                           const std::pair<std::string, int>& _secondOperand)
{
    // aborts if out of time
    dur = std::chrono::system_clock::now() - start;
    if (dur.count() > 29)
    {
        m_bCancelCalc = true;
        return;
    }

    if (!IsCompatible(_firstOperand.first, _secondOperand.first))
        return;
    if (_firstOperand.second == m_nTarget)
        return;

    // removes multiplications and divisions by 1
    size_t nNumOperations = m_strOPERATORS.size();
    if (_secondOperand.second == 1)
        nNumOperations /= 2;

    for (size_t i = 0; i < nNumOperations; i++)
    {
        std::string strNewKey;
        strNewKey.append(std::string("("));
        strNewKey.append(_firstOperand.first);
        strNewKey.append(std::string(1, m_strOPERATORS.at(i)));
        strNewKey.append(_secondOperand.first);
        strNewKey.append(std::string(")"));

        if (m_mapSolutions.find(strNewKey) != m_mapSolutions.end())
            return;

        bool bNewKeyAdded = false;
        typedef std::unordered_map<std::string, int>::iterator mapIterator;
        std::pair<mapIterator, bool> retCode;
        if (m_strOPERATORS.at(i) == '+')
        {
            m_nLatestSolution = _firstOperand.second + _secondOperand.second;
        }
        else if (m_strOPERATORS.at(i) == '-'
            && _firstOperand.second > _secondOperand.second)
        {
            m_nLatestSolution = _firstOperand.second - _secondOperand.second;
        }
        else if (m_strOPERATORS.at(i) == '*')
        {
            m_nLatestSolution = _firstOperand.second * _secondOperand.second;
        }
        else if (m_strOPERATORS.at(i) == '/'
            && _firstOperand.second > _secondOperand.second)
        {
            float fDivisionResult = (float)_firstOperand.second /
                (float)_secondOperand.second;

            if (fDivisionResult != (int)fDivisionResult || _secondOperand.second == 0)
                return;

            m_nLatestSolution = (int)fDivisionResult;
        }
        else
        {
            m_nLatestSolution = NULL;
        }

        // ends search when m_nTarget is found
        if (m_nLatestSolution == m_nTarget)
        {
            std::string newBestSolution = strNewKey;
            if (!m_bSolved)
            {
                m_bSolved = true;
                m_strBestSolution = newBestSolution;
            }
            else if (newBestSolution.length() < m_strBestSolution.length())
            {
                m_strBestSolution = newBestSolution;
            }
        }

        if (m_nLatestSolution != NULL)
        {
            retCode = m_mapSolutions.insert({ strNewKey, m_nLatestSolution });
            bNewKeyAdded = retCode.second;
        }

        // stores current closest solution to m_nTarget +/- 10
        if (bNewKeyAdded)
        {
            m_nNumSolutions++;
            int newDiff = abs(m_nTarget - m_nLatestSolution);
            if (newDiff < 10)
                if (newDiff < m_nLatestDiffFromTarget)
                {
                    m_nLatestDiffFromTarget = newDiff;
                    m_strBestSolution = strNewKey;
                }
        }
    }

    return;
}

// -----------------------------------------------------------------------------
// Checks if the two operands are compatible.
//
// Arguments:
//     _firstOperand  - first operand to the left of the operator
//     _secondOperand - second operand to the right of the operator
// Returns:
//     bool - true if compatible, false if the same letter is found in both
//            operands
// -----------------------------------------------------------------------------
bool CountdownCalculator::IsCompatible(const std::string& _firstOperand,
                                       const std::string& _secondOperand) const
{
    for (const auto& tile : m_strNUMBER_TILES)
    {
        if (_firstOperand.find(tile) != std::string::npos)
            if (_secondOperand.find(tile) != std::string::npos)
                return false;
    }
        
    return true;
}

// -----------------------------------------------------------------------------
// Takes in a given solution and removed unnecessary parentheses. Only works
// with solutions produced using the 'quick' calculation method. The last open
// bracket and the first closed bracket are found, and then each operation on
// this central operation is added to a vector of operations (and operator and
// and operand. The solution is reconstructed from this vector and the central
// pair, using BIDMAS to decide if brackets need to be added after each
// operation.
//
// Arguments:
//     solution - solution string to be deparenthesised
// Returns:
//     std::string - deparenthesised solution
// -----------------------------------------------------------------------------
void CountdownCalculator::DeParenthesise(std::string& solution) const
{
    int result = m_mapSolutions.at(solution);
    std::string strBracketedSegment;
    std::vector<std::string> vOperations;

    typedef std::string::const_iterator StringConstIterator;

    // finds first bracketed pair
    std::pair<StringConstIterator, StringConstIterator> itFirstPair;
    bool bFirstPairFound = false;
    StringConstIterator itFwd = solution.cbegin();
    while (!bFirstPairFound && itFwd != solution.cend())
    {
        if (*itFwd == ')')
        {
            if (itFwd != solution.cend() - 1)
            {
                //char nextOperand = *(itFwd + 1);
                StringConstIterator itRev = itFwd;
                while (itRev != solution.cbegin())
                {
                    itRev--;
                    if (*itRev == '(')
                    {
                        itFirstPair.first = itRev;
                        itFirstPair.second = itFwd;
                        bFirstPairFound = true;
                        break;
                    }
                }
            }
            else
                break;
        }
        itFwd++;
    }

    // edge case where there are only two operands
    if (itFwd == solution.cend() - 1)
    {
        solution.erase(solution.begin());
        solution.pop_back();

        char c_firstOperand = solution.front();
        char c_secondOperand = solution.back();

        solution.erase(solution.begin());
        solution.pop_back();

        std::string key = std::string(1, c_firstOperand);
        solution.insert(0,
                        std::to_string(m_mapSolutions.at(key)));

        key = std::string(1, c_secondOperand);
        solution.insert(solution.length(),
                        std::to_string(m_mapSolutions.at(key)));

        solution += (" = " + std::to_string(result));

        return;
    }
    
    // stores first bracketed pair
    vOperations.push_back(solution.substr(distance(solution.cbegin(), itFirstPair.first + 1),
                                         distance(itFirstPair.first, itFirstPair.second - 1)));

    // stores each further operation (operator then operand)
    std::pair<StringConstIterator, StringConstIterator> itNextOperation;
    itNextOperation.first = itNextOperation.second = itFirstPair.second;
    for (StringConstIterator it = itFirstPair.second; it != solution.cend(); it++)
    {
        for (const auto& op : m_strOPERATORS)
            if (op == *it || it == solution.cend() - 1)
            {
                if (itNextOperation.first == itNextOperation.second)
                    itNextOperation.first = it;
                else
                {
                    if (it == solution.cend() - 1)
                        it++;

                    itNextOperation.second = it;
                    it--;
                    vOperations.push_back(solution.substr(distance(solution.cbegin(),
                                                                   itNextOperation.first),
                                                          distance(itNextOperation.first,
                                                                   itNextOperation.second - 1)));
                    itNextOperation.second--;
                    itNextOperation.first = itNextOperation.second;
                }
                break;
            }
    }

    // gets operator in first bracket pair
    char cFirstOperator = ' ';
    bool bFirstOpFound = false;
    StringConstIterator itFirstOpSearch = vOperations.at(0).cbegin();
    while (!bFirstOpFound && itFirstOpSearch != vOperations.at(0).cend())
    {
        for (const auto& op : m_strOPERATORS)
            if (op == *itFirstOpSearch)
            {
                cFirstOperator = *itFirstOpSearch;
                bFirstOpFound = true;
                break;
            }
        itFirstOpSearch++;
    }

    // compares current and previous operators to determine if brackets needed
    solution = vOperations.at(0);
    char cPreviousOperator = cFirstOperator;
    for (size_t i = 1; i < vOperations.size(); i++)
    {
        // finds operator in vOperations[i]
        char cCurrentOperator = m_strOPERATORS.at(0);
        bool bOperatorFound = false;
        StringConstIterator itNextOpSearch = vOperations.at(i).cbegin();
        while (!bOperatorFound && itNextOpSearch != vOperations.at(i).cend())
        {
            for (const auto& op : m_strOPERATORS)
                if (op == *itNextOpSearch)
                {
                    cCurrentOperator = *itNextOpSearch;
                    bOperatorFound = true;
                    break;
                }
            itNextOpSearch++;
        }

        // adds brackets according to BIDMAS
        if (cPreviousOperator == '+' || cPreviousOperator == '-')
            if (cCurrentOperator == '*' || cCurrentOperator == '/')
                solution = '(' + solution + ')';
        
        cPreviousOperator = cCurrentOperator;
        solution += vOperations.at(i);
    }

    // switches tile letter (abcedf) with the tile number
    std::string strLetter = " ", strNumber;
    for (const auto& tile : m_strNUMBER_TILES)
    {
        size_t pos = solution.find(tile);
        if (pos != std::string::npos)
        {
            solution.erase(pos, 1);
            strLetter.front() = tile;
            strNumber = std::to_string(m_mapSolutions.at(strLetter));
            solution.insert(pos, strNumber);
        }
    }

    solution += (" = " + std::to_string(result));
}
