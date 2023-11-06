#if !defined(LEVENSHTEIN_H)
#define LEVENSHTEIN_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stddef.h>
class Levenshtein  
{
public:
	int Get  (const char* a, const char* b);
	int Get  (const char* a, size_t aLen, const char* b, size_t bLen);

    int Get2 (char const *s, char const *t);
    int Get2 (char const *s, size_t n, char const *t, size_t dst);
	
	Levenshtein();
	virtual ~Levenshtein();


private:
    int Minimum (int a, int b, int c)
	{
		int mi = a;
		
		if (b < mi)		mi = b;
		if (c < mi)		mi = c;
		
		return mi;
	}

    int *GetCellPointer (int *pOrigin, size_t col, size_t row, size_t nCols)
	{ return pOrigin + col + (row * (nCols + 1)); }
    
	int GetAt (int *pOrigin, size_t col, size_t row, size_t nCols)
	{
		int *pCell = GetCellPointer (pOrigin, col, row, nCols);
		return *pCell;
	}

	void PutAt (int *pOrigin, size_t col, size_t row, size_t nCols, int x)
	{
		int *pCell = GetCellPointer (pOrigin, col, row, nCols);
		*pCell = x;
	}
};

#endif // !defined(LEVENSHTEIN_H)
