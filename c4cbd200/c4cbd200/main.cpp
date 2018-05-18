#include "LogReader.h"

#define MAX_LENGTH 1024

/*
 * flags:
 * case sensitivity
 * multiline
 * lazy
 * greedy
 * ...
 * 
 * quantifiers:
 * zero or one
 * zero or more
 * one or more
 * ranges
 * ...
 * 
 * meta:
 * new line
 * character
 * digit
 * ...
 * 
 * groups:
 * named
 * unnamed
 * ...
 * 
 * anchors:
 * start
 * stop
 * pos
 * ...
 * 
 */

/*
 * format:
 * shileding 
 */

/*
 * refactoring:
 * class _Hint
 */

int main (int argc, char* argv[])
{
	CLogReader logReader;

	if (argc > 2)
	{
		if (logReader.Open(argv[1]))
		{
			auto line = new char[MAX_LENGTH];
			/*
			cимвол '*' - последовательность любых символов неограниченной длины;
			cимвол "?" - один любой символ;
			*/
			logReader.SetFilter("*?**har*");
			while ([]()->bool { return getchar()!=EOF; }())
			{
				logReader.GetNextLine(line, MAX_LENGTH);
			}
			delete[]line;
		}
	}
	return 0;
}
