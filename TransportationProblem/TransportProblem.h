// ---------------------------------------------------------------------------

#ifndef TransportProblemH
#define TransportProblemH

// ---------------------------------------------------------------------------

#include "TransportMatrix.h"

// ---------------------------------------------------------------------------

enum class TPDummyElement
{
    NoDummy,
    DummyStock,
    DummyNeed
};

class TransportProblem
{
    TransportMatrix m_matrix;

    std::vector<int> m_aT;
    std::vector<int> m_b;

    int m_nZMin;
    TPDummyElement m_dummy;

    bool m_bIsFile;

    void EqualizeGoodsAndNeeds();

    void DoNorthwestCornerMethod();

    bool FindSurroundingRouteForElement(const MatrixElement& elem,
                                        SurroundingRoute& route) const;

public:
    TransportProblem() = default;
    TransportProblem(int nM, int nN);

    void InitMatrixElement(int i, int j, int nTime);
    void AddAvaibleStock(int nAmount);
    void AddCustomerNeeds(int nAmount);

    void Solve();

    void SetFileFlag(bool bFlag) { m_bIsFile = bFlag; }

    friend std::ostream& operator<<(std::ostream& os,
                                    const TransportProblem& obj);

    friend std::istream& operator>>(std::istream& is, TransportProblem& obj);
};

// ---------------------------------------------------------------------------

inline bool __IsInPath(const SurroundingRoute& route, int i, int j)
{
    for (auto const& elem : route)
        if (elem.i == i && elem.j == j)
            return true;

    return false;
}

// ---------------------------------------------------------------------------

#endif
