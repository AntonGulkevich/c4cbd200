#pragma once
#include "LogReader.h"

class Test
{
public:
	Test() = default;
	~Test() = default;

	bool TestAll();

private:

	CLogReader logReader;
	bool Try(const char* str, const char * filter, bool expectRes);
};

