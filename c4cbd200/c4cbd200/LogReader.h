#pragma once

#include <Windows.h>
#include <cstdio>

#define MAX_PAIRS 256

class Test;

class CLogReader final
{
#pragma region private_types

private:

	// 
	enum class _PlaceholderToken;

	//
	enum class _Anchor;

	//
	class _QuantifiedToken;
#pragma endregion 

public:

	// C-tor
	CLogReader() noexcept;

	// D-tor
	~CLogReader() noexcept;

	// открытие файла, false - ошибка
	bool Open(const char *filename) noexcept;

	// закрытие файла
	void Close() noexcept;

	// установка фильтра строк, false - ошибка
	bool SetFilter(const char* filter) noexcept;

	// запрос очередной найденной строки
	// buf - буфер, bufsize - максимальная длина
	// false - конец файла или ошибка
	bool GetNextLine(char *buf, const int bufsize)noexcept;

#pragma region private_members

private:

	// search filter
	char * _filter{ nullptr };

	// full path to the file
	char * _filename{ nullptr };

	// count of parallel jobs
	DWORD _concurencyIndex{ 2ul };

	// true if current filestream is opened
	bool _isOpened{ false };

	// current filestream
	FILE * _fileStream{ nullptr };

	// the last parsed token
	_PlaceholderToken _theLastToken{};

	// compiled filter
	_QuantifiedToken * _quantifiedTokens[MAX_PAIRS] = { nullptr };

	// count of parsed lexemes 
	DWORD _placeHoldersIndex{ 0ul };

	// min length of match string
	DWORD _minimalMatchLength{ 0ul };

	//todo: ref this!
	DWORD _indexesBack[MAX_PAIRS] = { 0 };
	DWORD _patternIndexes[MAX_PAIRS] = { 0 };
	DWORD _maxIndexes = { 0 };
	friend Test;
	//

#pragma endregion 

#pragma region private_methods

private:

	// reset file metadata
	void ResetFileInfo()noexcept;

	// reset filter metadata
	void ResetFilter()noexcept;

	// parse buf
	// return true on match, false on missmatch 
	// non reqursive
	[[nodiscard]] bool GrammarParse(const char * src, DWORD srcLength)noexcept;

	// 
	void InitAnchors()noexcept;

	// initialize memory block for new _QuantifiedToken with parameters
	// return true on success, false on failure
	[[nodiscard]] bool InitQuantToken(const _PlaceholderToken token, const char & ref)noexcept;


#pragma endregion 

public:
	// r3
	CLogReader(const CLogReader &qft) = delete;

	// r3
	CLogReader & operator=(const CLogReader & rqtf) = delete;

	// r5
	CLogReader(const CLogReader &&qft) = delete;

	// r5
	CLogReader & operator=(const CLogReader && rqtf) = delete;
};
