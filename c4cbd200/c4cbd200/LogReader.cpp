#include "LogReader.h"

#include <cstring>
#include <new>

enum class CLogReader::_PlaceholderToken
{
	undefined,
	zeroOrMore, // *
	anyOne, // ?
	symbolMatch, // characters 
};

enum class CLogReader::_Anchor
{
	none,
	start,
	end,
	exact,
};

class CLogReader::_QuantifiedToken
{

public:

	// deleted empty c-tor
	_QuantifiedToken() = delete;

	// explicit c-tor with token
	explicit _QuantifiedToken(const _PlaceholderToken ph, const char & refStr) noexcept : _quantifier(1u), _token(ph), _pStr(refStr) {}

	// r3
	_QuantifiedToken(const _QuantifiedToken &qft) noexcept = default;

	// r3
	_QuantifiedToken& operator=(const _QuantifiedToken& rqtf) = delete;

	// r3
	~_QuantifiedToken() noexcept = default;

	// r5
	_QuantifiedToken(const _QuantifiedToken &&qft) = delete;

	// r5
	_QuantifiedToken & operator=(const _QuantifiedToken && rqtf) = delete;

	// prefix inrc
	_QuantifiedToken& operator++ () noexcept
	{
		++_quantifier;
		return *this;
	}

	const char* operator[] (const DWORD pos) const
	{
		auto tmp = &(&_pStr)[pos];

		return pos < _quantifier ? &(&_pStr)[pos] : nullptr;
	}

	//
	auto GetToken() const noexcept { return _token; }

	//
	auto GetQuant() const noexcept { return _quantifier; }

	//
	auto GetAnchor() const noexcept { return _anchor; }

	//
	void SetAnchor(const _Anchor anchor) noexcept { _anchor = anchor; }

private:
	// token quantifier
	DWORD _quantifier{ 0ul };

	// token
	_PlaceholderToken _token{ _PlaceholderToken::undefined };

	// anchor
	_Anchor _anchor{ _Anchor::none };

	// reference
	const char & _pStr;
};

CLogReader::CLogReader() noexcept : _theLastToken(_PlaceholderToken::undefined)
{
	// find out count of threads 
	// create SYSTEM_INFO struct
	SYSTEM_INFO systemInfo;
	GetNativeSystemInfo(&systemInfo);
	_concurencyIndex = systemInfo.dwNumberOfProcessors << 1u;

	// initialize threads
}

CLogReader::~CLogReader() noexcept
{
	// close opened file and reset file info
	Close();

	// reset filter and parsed results
	ResetFilter();

	// close all threads

	// free estimated memory

}

bool CLogReader::Open(const char* filename) noexcept
{
	if (_isOpened)
		Close();

	// open file in read mode
	// in mt open with _fsopen and _SH_DENYWR
	const auto retC = fopen_s(&_fileStream, filename, "r");

	// mark attempt to open
	_isOpened = true;

	// check results
	if (!!retC)
		// reset info
		ResetFileInfo();

	/*
	 * prepare for multithreading:
	 *
	 * calc crc32 of the file ? (lock write access)
	 *
	 * find positions of all new line symbols
	 *
	 * calc count of new lines
	 *
	 * open in read only mode file for every thread pool
	 *
	 * split file on chunks for each thread (set the start read position to (lineCount / _concurencyIndex) for every worker
	 * or limit buffer size for every worker and set start read position next to it after an attempt
	 * to find next available new line offset)
	 *
	 * check ranges
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
	 *
	 * =threads=
	 * pause threads instead of creating them every iteration (perf>>> memory< complexity>)
	 * use static memory instead of dynamic (perf<<< memory>> complexity=)
	 * ssd/hdd modes (perf>>> memory complexity>)
	 *
	 * =algo=
	 * sort (perf?> memory> complexity>)
	 * indexing (perf?> memory> complexity>>)
	 * cache (perf>>> memory>> complexity>>)
	 */


	return !retC;
}

void CLogReader::Close() noexcept
{
	if (_fileStream)
		fclose(_fileStream);
	ResetFileInfo();
}

bool CLogReader::SetFilter(const char* filter) noexcept
{
	// reset
	ResetFilter();

	// duplicate
	_filter = _strdup(filter);

	// check _strdup result
	if (_filter == nullptr)
		// if dup failed
		return false;

	// parse filter
	const auto filterLen = strlen(filter);

	// save previous value
	auto previousToken = _theLastToken;

	//
	if (filterLen > 0)
	{
		for (auto i = 0u; i < filterLen; ++i)
		{
			switch (_filter[i])
			{
			case '*':
				_theLastToken = _PlaceholderToken::zeroOrMore;
				break;
			case '?':
				++_minimalMatchLength;
				_theLastToken = _PlaceholderToken::anyOne;
				break;
			default:
				++_minimalMatchLength;
				_theLastToken = _PlaceholderToken::symbolMatch;
				break;
			}

			if (previousToken != _theLastToken)
			{
				if (!InitQuantToken(_theLastToken, _filter[i]))
					return false;

				previousToken = _theLastToken;
			}
			else
			{
				// incr placeholder count
				++*(_quantifiedTokens[_placeHoldersIndex - 1]);
			}

		}
		//
		InitAnchors();
	}
	else
	{
		// empty token
		// todo: greedy or lazy?
		return false;
	}
	// todo: reset search results

	// rewind to the start of the file
	if (!!_fileStream)
		rewind(_fileStream);

	return true;
}

bool CLogReader::GetNextLine(char* buf, const int bufsize) noexcept
{
	while (fgets(buf, bufsize, _fileStream))
	{
		const auto srcLength = strlen(buf) - 1/*\n*/;
		// return false if source length
		// is zero
		// more than srcSize
		// less than minimal parse length 
		if (!srcLength || srcLength > static_cast<size_t>(bufsize) || srcLength < _minimalMatchLength)
			continue;

		/*if (PlainReqursionMatch(buf, _filter))
			return true;*/
		if (GrammarParse(buf, srcLength))
			return true;
	}
	return false;
}

void CLogReader::ResetFileInfo() noexcept
{
	_fileStream = nullptr;
	_isOpened = false;
	_filename = nullptr;
}

void CLogReader::ResetFilter() noexcept
{
	if (_filter)
		free(_filter);
	_filter = nullptr;

	_theLastToken = _PlaceholderToken::undefined;

	if (_placeHoldersIndex)
	{
		// reset parsed filter
		for (auto i = 0u; i < _placeHoldersIndex; ++i)
		{
			_quantifiedTokens[i]->~_QuantifiedToken();
			free(_quantifiedTokens[i]);
			_quantifiedTokens[i] = nullptr;
		}
	}

	_placeHoldersIndex = 0ul;
	_maxIndexes = 0ul;
	_minimalMatchLength = 0ul;
}
bool CLogReader::GrammarParse(const char* src, const DWORD srcLength) noexcept
{
	// pNELOnialisANDdySayANDySayANDdySay
	//0123456789012345678901234567890123456789
	// ?NELO*				"NELO"	must start from 2
	// ?NELO*AND?S*			"NELO"	must start from 2 and S must start from "AND" pos + "AND" length + 1 per each ?

	auto backSearch = false;
	auto retry = false;
	auto retryPosition = 0ul;
	auto retryIndex = 0ul;
	_maxIndexes = 0ul;
	auto strIndex = 0ul;
	for (auto i = 0ul; i < _placeHoldersIndex; ++i)
	{
		backSearch = false;
		_theLastToken = _quantifiedTokens[i]->GetToken();
		switch (_theLastToken)
		{
		case _PlaceholderToken::undefined: return false;
		case _PlaceholderToken::zeroOrMore:
		{

			auto minLength = 0ul;
			for (auto estTokens = i; estTokens < _placeHoldersIndex; ++estTokens)
			{

				switch (_quantifiedTokens[estTokens]->GetToken()) {
				case _PlaceholderToken::zeroOrMore: break;
				case _PlaceholderToken::anyOne:
				case _PlaceholderToken::symbolMatch: minLength += _quantifiedTokens[estTokens]->GetQuant(); break;
				default: return false;
				}
			}
			if (minLength > srcLength)
			{
				retry = false;
				break;
			}
			const auto possibleEnd = srcLength - minLength;
			if (possibleEnd < strIndex)
			{
				retry = false;
				break;
			}
			////save possible results results
			//for(auto possibleIndex = strIndex; possibleIndex < possibleEnd ; ++possibleIndex)
			//{
			//	// save indexes
			//	_indexesBack[_maxIndexes] = possibleIndex;
			//	_patternIndexes[_maxIndexes++] = i;
			//}
			retry = true;
			retryIndex = i;
			retryPosition = strIndex;
			break;
		}
		case _PlaceholderToken::anyOne:
		{
			strIndex += _quantifiedTokens[i]->GetQuant();
			if (strIndex > srcLength)
				return false;
			break;
		}
		case _PlaceholderToken::symbolMatch:
		{
			//todo
			auto matchCount = 0ul;

			// pattern string
			const auto ex = (*_quantifiedTokens[i])[0];
			// pattern string length
			const auto exLength = _quantifiedTokens[i]->GetQuant();

			switch (_quantifiedTokens[i]->GetAnchor())
			{
			case _Anchor::none:
			{
				// previous was `?`
				if (i != 0 && _quantifiedTokens[i - 1]->GetToken() == _PlaceholderToken::anyOne)
				{
					// match directly
					auto strT = &src[strIndex];
					if (memcmp(ex, &src[strIndex], exLength) != 0)
					{
						if (_maxIndexes > 1)
							--_maxIndexes;
						if (_maxIndexes)
						{
							strIndex = _indexesBack[_maxIndexes - 1] + 1/*instead of for(...)*/;
							i = _patternIndexes[_maxIndexes - 1];
							--_maxIndexes;
							backSearch = true;
						}
						else
						{
							if (retry)
							{
								if (retryPosition < srcLength)
								{
									strIndex = ++retryPosition/*skip 1 symbol*/;
									i = retryIndex;
									backSearch = true;
								}
								else
									retry = false;
							}
						}
					}
					else
					{
						++matchCount;
						strIndex += exLength;
					}

					break;
				}
				else
				{
					// finding substrings
					for (auto srcIndex = strIndex, exI = 0ul; srcIndex < srcLength; ++srcIndex)
					{
						// check length
						if (srcLength - srcIndex < exLength)
							break;

						// todo: remove
						const auto strCmp = &src[srcIndex];
						const auto resCmp = memcmp(&ex[0], strCmp, exLength);
						if (resCmp == 0)
						{
							//todo: ref this!
							// save indexes
							_indexesBack[_maxIndexes] = srcIndex + exLength;
							_patternIndexes[_maxIndexes++] = i;
							//
							++matchCount;
							strIndex += exLength;
						}
					}
					if (matchCount == 0)
					{
						if (_maxIndexes > 1)
							--_maxIndexes;
						if (_maxIndexes)
						{
							strIndex = _indexesBack[_maxIndexes - 1] + 1/*instead of for(...)*/;
							i = _patternIndexes[_maxIndexes - 1];
							--_maxIndexes;
							backSearch = true;
						}
						else
						{
							if (retry)
							{
								if (retryPosition < srcLength)
								{
									strIndex = ++retryPosition/*skip 1 symbol*/;
									i = retryIndex;
									backSearch = true;
								}
								else
									retry = false;
							}
						}
					}

					break;
				}
			}
			case _Anchor::start:
				if (exLength > srcLength)
					return false;
				if (memcmp(ex, src, exLength) != 0)
					return false;
				++matchCount;
				strIndex += exLength;
				break;
			case _Anchor::end:
			{
				const auto from = srcLength - exLength;
				const auto res = memcmp(ex, &src[from], exLength);
				if (res != 0 || from < strIndex)
				{
					if (_maxIndexes > 1)
						--_maxIndexes;
					if (_maxIndexes)
					{
						strIndex = _indexesBack[_maxIndexes - 1] + 1/*instead of for(...)*/;
						i = _patternIndexes[_maxIndexes - 1];
						// remove this try
						--_maxIndexes;
						backSearch = true;
					}
					else
					{
						if (retry)
						{
							if (retryPosition < srcLength)
							{
								strIndex = ++retryPosition/*skip 1 symbol*/;
								i = retryIndex;
								backSearch = true;
							}
							else
								retry = false;
						}
					}
				}
				else
				{
					++matchCount;
					strIndex += exLength;
					// save indexes
					//_indexesBack[_maxIndexes] = strIndex;
					//_patternIndexes[_maxIndexes++] = i;
				}
				break;
			}
			case _Anchor::exact:
			{
				if (exLength != srcLength || memcmp(ex, src, exLength) != 0)
					return false;
				++matchCount;
			}
			}
			if (matchCount == 0ul)
			{
				if (backSearch)
					break;
				return false;
			}
			break;
		}
		default: return false;
		}
	}
	return true;
}

void CLogReader::InitAnchors() noexcept
{
	const auto theLastindex = _placeHoldersIndex - 1;

	// start check
	if (_quantifiedTokens[0]->GetToken() == _PlaceholderToken::symbolMatch)
		_quantifiedTokens[0]->SetAnchor(theLastindex == 0 ? _Anchor::exact : _Anchor::start);
	// end check
	if (theLastindex && _quantifiedTokens[theLastindex]->GetToken() == _PlaceholderToken::symbolMatch)
		_quantifiedTokens[theLastindex]->SetAnchor(_Anchor::end);
}

bool CLogReader::InitQuantToken(const _PlaceholderToken token, const char& ref) noexcept
{
	if (_placeHoldersIndex == MAX_PAIRS)
		return false;
	_quantifiedTokens[_placeHoldersIndex] = static_cast<_QuantifiedToken *>(malloc(sizeof(_QuantifiedToken)));
	if (_quantifiedTokens[_placeHoldersIndex] == nullptr)
		return false;
	_quantifiedTokens[_placeHoldersIndex] = new (_quantifiedTokens[_placeHoldersIndex])_QuantifiedToken(token, ref);

	++_placeHoldersIndex;
	return true;
}

