#pragma once

#include <stdint.h>
#include <cstdio>

#define MAX_PAIRS 256

class CLogReader final
{
#pragma region private_types

private:

	// 
	enum class _PlaceholderToken;

	//
	struct _QuantifiedToken;

#pragma endregion 

public:

	// C-tor
	CLogReader();

	// D-tor
	~CLogReader() = default;

	// открытие файла, false - ошибка
	bool Open(const char *filename);

	// закрытие файла
	void Close();

	// установка фильтра строк, false - ошибка
	bool SetFilter(const char* filter);

	// запрос очередной найденной строки
	// buf - буфер, bufsize - максимальная длина
	// false - конец файла или ошибка
	bool GetNextLine(char *buf, const int bufsize);

#pragma region private_members

private:

	// search filter
	char * _filter{ nullptr };

	// full path to the file
	char * _filename{ nullptr };

	// the last error code
	uint32_t _theLastErrorCode{ 0u };

	// count of parallel jobs
	uint32_t _concurencyIndex{ 2u };

	// true if current filestream is opened
	bool _isOpened{ false };

	// current filestream
	FILE * _fileStream{ nullptr };

	// the last parsed token
	_PlaceholderToken _theLastToken{};

	//
	_QuantifiedToken * _quantifiedTokens[MAX_PAIRS];

	//
	uint32_t _placeHoldersIndex{ 0u };

#pragma endregion 

#pragma region private_methods

private:

	//
	void Reset();

	//
	void ResetFilter();

	//

#pragma endregion 

};