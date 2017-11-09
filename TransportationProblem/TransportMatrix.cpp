// ---------------------------------------------------------------------------

#include "stdafx.h"

#include "TransportMatrix.h"

#include <iostream>
#include <algorithm>

// ---------------------------------------------------------------------------

TransportMatrix::TransportMatrix(int nM, int nN)
    : m_matrix(nM, std::vector<MatrixElement>(nN)), m_nM(nM), m_nN(nN)
{
}

// ---------------------------------------------------------------------------

void TransportMatrix::InitElement(int i, int j, int nTime)
{
    m_matrix[i][j].i = i;
    m_matrix[i][j].j = j;
    m_matrix[i][j].nTime = nTime;
}

// ---------------------------------------------------------------------------

void TransportMatrix::AddRow()
{
    m_matrix.resize(m_nM + 1, std::vector<MatrixElement>(m_nN, MatrixElement()));

    m_nM++;

    for (auto i = 0; i < m_nN; i++)
    {
        m_matrix.back()[i].i = m_nM - 1;
        m_matrix.back()[i].j = i;
    }
}

// ---------------------------------------------------------------------------

void TransportMatrix::AddColumn()
{
    for (auto& elem : m_matrix)
        elem.resize(m_nN + 1);

    m_nN++;

    for (auto i = 0; i < m_nM; i++)
    {
        m_matrix[i].back().i = i;
        m_matrix[i].back().j = m_nN - 1;
    }
}

// ---------------------------------------------------------------------------

void TransportMatrix::SetElementValue(int i, int j, int nValue)
{
    m_matrix[i][j].nValue = nValue;
}

// ---------------------------------------------------------------------------

const MatrixElement& TransportMatrix::FindUsedElementMaxTime()
{
    auto max = 0, e_i = 0, e_j = 0;

    for (auto i = 0; i < m_nM; i++)
    {
        for (auto j = 0; j < m_nN; j++)
        {
            if (m_matrix[i][j].nValue == EMPTY_VALUE)
                continue;

            if (m_matrix[i][j].nTime > max)
            {
                max = m_matrix[i][j].nTime;
                e_i = i;
                e_j = j;
            }
        }
    }

    return m_matrix[e_i][e_j];
}

// ---------------------------------------------------------------------------

void TransportMatrix::DeactivateGreaterTimeElements(const MatrixElement& element)
{
    for (auto i = 0; i < m_nM; i++)
    {
        for (auto j = 0; j < m_nN; j++)
        {
            if (m_matrix[i][j].nValue != EMPTY_VALUE)
                continue;

            if (i == element.i && j == element.j)
                continue;

            if (m_matrix[i][j].nTime >= element.nTime)
                m_matrix[i][j].bActive = false;
        }
    }
}

// ---------------------------------------------------------------------------

void TransportMatrix::DoSurroundingRouteValueChanges(SurroundingRoute& route, const MatrixElement& startElem)
{
    const auto nValue = std::minmax_element(route.begin(), route.end(),
                                            [](const MatrixElement& lhs, const MatrixElement& rhs)
                                            {
                                                if (lhs.nValue == EMPTY_VALUE)
                                                    return false;

                                                if (rhs.nValue == EMPTY_VALUE)
                                                    return true;

                                                return lhs.nValue < rhs.nValue;
                                            }).first->nValue;

    route.pop_back(); // това е началната точка тя не ни трябва

    if (m_matrix[startElem.i][startElem.j].nValue != EMPTY_VALUE)
        m_matrix[startElem.i][startElem.j].nValue -= nValue;

    if (m_matrix[startElem.i][startElem.j].nValue <= 0)
        m_matrix[startElem.i][startElem.j].nValue = EMPTY_VALUE;

    auto bPlus = true;

    for (auto const& elem : route)
    {
        if (bPlus)
            m_matrix[elem.i][elem.j].nValue != EMPTY_VALUE
                ? m_matrix[elem.i][elem.j].nValue += nValue
                : m_matrix[elem.i][elem.j].nValue = nValue;
        else
            m_matrix[elem.i][elem.j].nValue -= nValue;


        if (m_matrix[elem.i][elem.j].nValue <= 0)
            m_matrix[elem.i][elem.j].nValue = EMPTY_VALUE;

        bPlus = !bPlus;
    }
}

// ---------------------------------------------------------------------------

void TransportMatrix::RemoveLastRow()
{
    m_matrix.pop_back();
    m_nM--;
}

// ---------------------------------------------------------------------------

void TransportMatrix::RemoveLastColumns()
{
    for (auto i = 0; i < m_nN; i++)
        m_matrix[i].pop_back();

    m_nN--;
}

// ---------------------------------------------------------------------------

std::istream& operator>>(std::istream& is, TransportMatrix& obj)
{
    obj.m_matrix.resize(obj.m_nM, std::vector<MatrixElement>(obj.m_nN,
                                                             MatrixElement()));

    if (!g_bLoadedFromFile)
    {
        std::cout << " - Popylnete vsqka kletka na matricata " << obj.m_nM << "x" <<
            obj.m_nN << std::endl;
    }

    // Вход на матрицата
    for (auto i = 0; i < obj.m_nM; i++)
    {
        for (auto j = 0; j < obj.m_nN; j++)
        {
            if (!g_bLoadedFromFile)
            {
                std::cout << "\t- Vavedete element (" << i << ", " << j << "):";
            }

            int nTime;
            is >> nTime;

            obj.InitElement(i, j, nTime);
        }
    }

    return is;
}

// ---------------------------------------------------------------------------
