#include "LogReader.h"

#include <Windows.h> // GetSystemInfo

enum class CLogReader::_PlaceholderToken
{
	undefined,
	anyOneOrMore, // *
	anyOne, // ?
	symbolMatch, // any characters 
};

struct CLogReader::_QuantifiedToken
{
	uint32_t quantifier{ 0u };
	_PlaceholderToken token{ _PlaceholderToken::undefined };

	_QuantifiedToken& operator++ ()
	{
		++quantifier;
		return *this;
	}
};

CLogReader::CLogReader(): _theLastToken(_PlaceholderToken::undefined)
{
	// find out count of threads 
	// create SYSTEM_INFO struct
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	_concurencyIndex = systemInfo.dwNumberOfProcessors << 1u;

	// initialize threads
}

bool CLogReader::Open(const char* filename)
{
	if (_isOpened)
		Close();

	// open file in read mode
	// in mt open with _fsopen and _SH_DENYWR
	_theLastErrorCode = fopen_s(&_fileStream, filename, "r");

	// mark attempt to open
	_isOpened = true;

	// check results
	if (!!_theLastErrorCode)
		// reset info
		Reset();

	/*
	 * prepare for multithreading:
	 * 
	 * calc crc32 of the file
	 * 
	 * find positions of all new line symbols
	 * 
	 * calc count of new lines
	 * 
	 * open in read only mode file for every thread pool
	 * 
	 * set the start read position to (lineCount / _concurencyIndex) for every worker
	 * or limit buffer size for every worker and set start read position next to it after attempt
	 * to find next available new line offset
	 * 
	 * start parsing
	 * 
	 * after thread has done its job [from, to) line index
	 * synchronize results and the range
	 * 
	 * if results are positive, check if previous range pending parsing, 
	 * if not, synchronize threads, save their states and return first result 
	 * if pending, start parsing next chunk if it is available or pause thread
	 * 
	 * ==optimizations==
	 * pause threads instead of creating them every iteration (perf>>> memory< complexity>)
	 * use static memory instead of dynamic (perf<<< memory>> complexity>)
	 * ssd/hdd modes
	 * 
	 * 
	 */


	return !_theLastErrorCode;
}

void CLogReader::Close()
{
	if (_fileStream)
		fclose(_fileStream); 
	Reset();
}

bool CLogReader::SetFilter(const char* filter)
{
	// reset
	ResetFilter();
	// duplicate
	_filter = _strdup(filter);

	// check dup results
	if (_filter == nullptr)
		// if dup failed
		return false;

	// parse filter
	auto filterLen = strlen(filter);

	auto previousToken = _theLastToken;

	//
	if (filterLen > 0)
	{
		for (auto i=0u; i < filterLen; ++i)
		{
			switch (_filter[i])
			{
			case '*': 
				_theLastToken = _PlaceholderToken::anyOneOrMore;
				break;
			case '?':
				_theLastToken = _PlaceholderToken::anyOne;
				break;
			default: 
				_theLastToken = _PlaceholderToken::symbolMatch;;
			}

			if (previousToken != _theLastToken)
			{
				_quantifiedTokens[_placeHoldersIndex++] = new _QuantifiedToken();
				previousToken = _theLastToken;
			}
			else
			{
				++(_quantifiedTokens[_placeHoldersIndex - 1]);
			}
		}
	}
	else
	{
		//empty token
	}
	//todo: reset search results


	return true;
}

bool CLogReader::GetNextLine(char* buf, const int bufsize)
{
	while(fgets(buf, bufsize, _fileStream))
	{
		printf("%s", buf);
		return true;
	}
	return false;
}

void CLogReader::Reset()
{
	_fileStream = nullptr;
	_isOpened = false;
	_filename = nullptr;
}

void CLogReader::ResetFilter()
{
	if (_filter)
		free(_filter);
	_filter = nullptr;
}
