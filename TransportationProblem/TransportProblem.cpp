// ---------------------------------------------------------------------------

#include "stdafx.h"

#include "TransportProblem.h"

#include <ostream>
#include <istream>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <stack>

// ---------------------------------------------------------------------------

TransportProblem::TransportProblem(int nM, int nN)
    : m_matrix(nM, nN), m_nZMin(0), m_dummy(TPDummyElement::NoDummy), m_bIsFile(false)
{
}

// ---------------------------------------------------------------------------

void TransportProblem::InitMatrixElement(int i, int j, int nTime)
{
    m_matrix.InitElement(i, j, nTime);
}

// ---------------------------------------------------------------------------

void TransportProblem::AddAvaibleStock(int nAmount)
{
    m_aT.push_back(nAmount);
}

// ---------------------------------------------------------------------------

void TransportProblem::AddCustomerNeeds(int nAmount)
{
    m_b.push_back(nAmount);
}

// ---------------------------------------------------------------------------

void TransportProblem::Solve()
{
    // Проверява се Сума ai = Сума bj, Ако не се добавя фиктивен A или B.
    EqualizeGoodsAndNeeds();

    // Чертаем начален опорен план (таблицата) и използваме един от методите и попълваме Xij
    DoNorthwestCornerMethod();

    // Намираме Z(Xi) = max от t на пълните клетки (tke)
    // Намираме Xij с t >= tke и зачеркваме.
    // За клетка Xke построяваме обходна линия Kke - Ако съществува продължаваме иначе край
    // Определяме min от отрицателните клетки
    // Правим нов опорен план -> и пак..
    while (true)
    {
        const auto& maxElem = m_matrix.FindUsedElementMaxTime();

        m_matrix.DeactivateGreaterTimeElements(maxElem);

        SurroundingRoute route;

        if (!FindSurroundingRouteForElement(maxElem, route))
        {
            m_nZMin = maxElem.nTime;
            break;
        }

        m_matrix.DoSurroundingRouteValueChanges(route, maxElem);

        std::cout << std::endl << std::endl;
        for (auto i = 0; i < route.size(); i++)
        {
            std::cout << "(" << route[i].i << ", " << route[i].j << ") ";
        }
        std::cout << std::endl << std::endl;
    }

    if (m_dummy == TPDummyElement::DummyStock)
        m_matrix.RemoveLastRow();
    else if (m_dummy == TPDummyElement::DummyNeed)
        m_matrix.RemoveLastColumns();
}

// ---------------------------------------------------------------------------

void TransportProblem::EqualizeGoodsAndNeeds()
{
    // Изчисляваме сумите
    const auto aTsum = accumulate(m_aT.begin(), m_aT.end(), 0);
    const auto bsum = accumulate(m_b.begin(), m_b.end(), 0);

    // Подравняваме двата вектора
    if (aTsum > bsum)
    {
        const auto nDiff = aTsum - bsum;

        AddCustomerNeeds(nDiff);
        m_matrix.AddColumn();
        m_dummy = TPDummyElement::DummyNeed;
    }
    else if (aTsum < bsum)
    {
        const auto nDiff = bsum - aTsum;

        AddAvaibleStock(nDiff);
        m_matrix.AddRow();
        m_dummy = TPDummyElement::DummyStock;
    }
}

// ---------------------------------------------------------------------------

void TransportProblem::DoNorthwestCornerMethod()
{
    const auto nM = m_matrix.GetM();
    const auto nN = m_matrix.GetN();

    auto i = 0, j = 0;

    while (true)
    {
        if (i >= nM)
        {
            i = 0;
            j++;
            continue;
        }

        if (m_matrix.GetElement(i, j).nValue == EMPTY_VALUE)
        {
            if (m_matrix.GetElement(nM - 1, j).nValue == EMPTY_VALUE)
            {
                i = 0;
                j++;

                continue;
            }

            i++;

            continue;
        }

        const auto nValue = std::min(m_aT[i], m_b[j]);

        m_matrix.SetElementValue(i, j, nValue);
        m_aT[i] -= nValue;
        m_b[j] -= nValue;

        if (m_aT[i] == 0)
            for (auto jj = j + 1; jj < nN; jj++)
                m_matrix.SetElementValue(i, jj, EMPTY_VALUE);

        if (m_b[j] == 0)
            for (auto ii = i + 1; ii < nM; ii++)
                m_matrix.SetElementValue(ii, j, EMPTY_VALUE);

        if (j + 1 >= nN && i + 1 >= nM)
            break;

        i++;
    }
}

// ---------------------------------------------------------------------------

bool TransportProblem::FindSurroundingRouteForElement(const MatrixElement& elem,
                                                      SurroundingRoute& route) const
{
    const auto nM = m_matrix.GetM();
    const auto nN = m_matrix.GetN();

    std::stack<PathNode> stack;

    // Ако няма свободна точка по хоризонтала и по вертикала -> край
    auto bHasHorizontal = false;
    auto bHasVertical = false;

    for (auto i = 0; i < nM; i++)
        if (m_matrix.GetElement(i, elem.j).bActive && i != elem.i)
            bHasVertical = true;

    for (auto j = 0; j < nN; j++)
        if (m_matrix.GetElement(elem.i, j).bActive && elem.j != j)
            bHasHorizontal = true;

    if (!bHasVertical || !bHasHorizontal)
        return false;

    // Намираме началните точки и посоки
    for (auto iPos = 0; iPos < nM; iPos++)
    {
        if (iPos == elem.i)
            continue;

        const auto& currentElement = m_matrix.GetElement(iPos, elem.j);

        if (!currentElement.bActive)
            continue;

        stack.push(PathNode());

        stack.top().route.push_back(currentElement);
        stack.top().currElement = currentElement;
        stack.top().bIsHorizontalMove = true;
        stack.top().bIsHorizontalFirst = false;
    }

    // За всяка начална точка търсим съседите, съседите на съседите и т.н.
    // докато не стигнем до крайната точка
    while (!stack.empty())
    {
        auto curr = stack.top();

        if (curr.route.size() > MIN_SURROUND_ROUTE_LENGTH &&
            curr.currElement.i == elem.i &&
            curr.currElement.j == elem.j)
        {
            // Имаме победител		
            route.swap(curr.route);
            return true;
        }

        stack.pop();

        for (auto iPos = 0; iPos < (curr.bIsHorizontalMove ? nN : nM); iPos++)
        {
            const auto i = curr.bIsHorizontalMove ? curr.currElement.i : iPos;
            const auto j = curr.bIsHorizontalMove ? iPos : curr.currElement.j;

            const auto bInPath = __IsInPath(curr.route, i, j);
            const auto& currentElement = m_matrix.GetElement(i, j);

            if (!currentElement.bActive)
                continue;

            if (
                !(!bInPath || (bInPath &&
                        curr.route.size() >= MIN_SURROUND_ROUTE_LENGTH &&
                        i == elem.i &&
                        j == elem.j &&
                        curr.bIsHorizontalMove != curr.bIsHorizontalFirst)
                ) ||
                !(curr.route.size() % 2 == 0 || curr.route.size()
                    % 2 != 0 && currentElement.nValue != EMPTY_VALUE)
            )
            {
                continue;
            }

            if (currentElement.nValue != EMPTY_VALUE &&
                currentElement.nValue < elem.nValue)
            {
                continue;
            }

            stack.push(PathNode());

            stack.top().route = curr.route;
            stack.top().currElement = currentElement;
            stack.top().bIsHorizontalMove = !curr.bIsHorizontalMove;

            stack.top().route.push_back(currentElement);
        }
    }

    // Няма обходен маршрут
    return false;
}

// ---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const TransportProblem& obj)
{
    os << "Solution:" << std::endl;
    os << "-------------------------------------------" << std::endl;

    for (auto i = 0; i < obj.m_matrix.GetM(); i++)
    {
        for (auto j = 0; j < obj.m_matrix.GetN(); j++)
        {
            if (obj.m_matrix.GetElement(i, j).nValue > 0)
                os << obj.m_matrix.GetElement(i, j).nValue << " ";
            else
                os << "0 ";
        }

        os << std::endl;
    }

    os << "Zmin = " << obj.m_nZMin << std::endl;
    os << "\n-------------------------------------------" << std::endl;

    return os;
}

// ---------------------------------------------------------------------------

std::istream& operator>>(std::istream& is, TransportProblem& obj)
{
    if (!g_bLoadedFromFile)
    {
        std::cout << "Vhodni danni:" << std::endl;
        std::cout << " - Vhodna matrica: vyvedete MxN v vid \"M N\":";
    }

    int nM, nN;
    is >> nM >> nN;

    obj.m_matrix.SetM(nM);
    obj.m_matrix.SetN(nN);

    is >> obj.m_matrix;

    if (!g_bLoadedFromFile)
    {
        std::cout << " - Vhodni danni za aT:" << std::endl;
    }

    for (auto i = 0; i < obj.m_matrix.GetM(); i++)
    {
        if (!g_bLoadedFromFile)
        {
            std::cout << "\t- Vavedete element aT" << i << ": ";
        }

        int nAmount;
        is >> nAmount;

        obj.AddAvaibleStock(nAmount);
    }

    if (!g_bLoadedFromFile)
    {
        std::cout << " - Vhodni danni za b:" << std::endl;
    }

    for (auto i = 0; i < obj.m_matrix.GetN(); i++)
    {
        if (!g_bLoadedFromFile)
        {
            std::cout << "\t- Vavedete element b" << i << ": ";
        }

        int nAmount;
        is >> nAmount;

        obj.AddCustomerNeeds(nAmount);
    }

    return is;
}

// ---------------------------------------------------------------------------
