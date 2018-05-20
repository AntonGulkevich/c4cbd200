#include "LogReader.h"
#include "Test.h"
#define MAX_LENGTH 1024
#define PERFORM_TEST
#define PRESS_ENTER_TO_CONT
/*
 * global flags:
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
 * count
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
  * braces
  * priority?
  */

  /*
   * refactoring:
   * class file metadata
   * class parse filter results metadata
   */

   /* Example:
	* "../../c4cbd200/log/Actors.txt" "*?**har*"
	*/

#undef PERFORM_TEST
#undef PRESS_ENTER_TO_CONT
int main(const int argc, char* argv[])
{
#ifdef PERFORM_TEST
	Test test;
	test.TestAll();
#endif
	CLogReader logReader;

	if (argc > 2)
	{
		if (logReader.Open(argv[1]))
		{
			if (!logReader.SetFilter(argv[2]))
				return -1;

			const auto line = static_cast<char *>(malloc(MAX_LENGTH + 1));
#ifdef PRESS_ENTER_TO_CONT
			while ([]()->bool { return getchar() != EOF; }())
			{
#endif
				if (logReader.GetNextLine(line, MAX_LENGTH))
				{
					printf_s("%s", line);
					free(line);
					return 0;
				}
				else
				{
					printf_s("none");
					free(line);
					return -1;
				}
#ifdef PRESS_ENTER_TO_CONT
			}
			free(line);
#endif
		}
	}
	else
	{
		printf_s(R"(Hint: "fullpath/filename.extension" "m*ask?*")");
		return -1;
	}

	return 0;
}
