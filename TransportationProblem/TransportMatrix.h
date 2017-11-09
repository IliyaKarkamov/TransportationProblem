// ---------------------------------------------------------------------------

#ifndef TransportMatrixH
#define TransportMatrixH

// ---------------------------------------------------------------------------

#include <vector>
#include <functional>

// ---------------------------------------------------------------------------

#define EMPTY_VALUE -1
#define MIN_SURROUND_ROUTE_LENGTH 3

// ---------------------------------------------------------------------------

struct MatrixElement
{
	int nValue;
	int nTime;

	bool bActive;

	int i;
	int j;

	MatrixElement()
		: nValue(0), nTime(0), bActive(true), i(0), j(0)
	{
	}

	MatrixElement(const MatrixElement&) = default;
};

// ---------------------------------------------------------------------------

typedef std::vector<MatrixElement> SurroundingRoute;

struct PathNode
{
	SurroundingRoute route;
	MatrixElement currElement;

	bool bIsHorizontalMove;
	bool bIsHorizontalFirst;

	PathNode() = default;
};

// ---------------------------------------------------------------------------

class TransportMatrix
{
	std::vector<std::vector<MatrixElement>> m_matrix;

	int m_nM;
	int m_nN;

public:
	TransportMatrix() = default;
	TransportMatrix(int nM, int nN);

	void InitElement(int i, int j, int nTime);

	void SetM(int nM) { m_nM = nM; }
	int GetM() const { return m_nM; }

	void SetN(int nN) { m_nN = nN; }
	int GetN() const { return m_nN; }

	const MatrixElement& GetElement(int i, int j) const { return m_matrix[i][j]; }

	void SetElementValue(int i, int j, int nValue);

	void AddRow();
	void RemoveLastRow();

	void AddColumn();
	void RemoveLastColumns();

	const MatrixElement& FindUsedElementMaxTime();
	void DeactivateGreaterTimeElements(const MatrixElement& element);
	void DoSurroundingRouteValueChanges(SurroundingRoute& route, const MatrixElement& startElem);

	friend std::istream& operator>>(std::istream& is, TransportMatrix& obj);
};

// ---------------------------------------------------------------------------

#endif
