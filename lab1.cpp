#define UNICODE
#define _UNICODE

#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <iomanip>

void AfiseazaValoare(LPCTSTR numeValoare, DWORD tip, const BYTE* data, DWORD dataSize)
{
	_tprintf(L"Name: %s\n", numeValoare[0] ? numeValoare : L"(Implicit)");

	switch (tip)
	{
	case REG_SZ:
		_tprintf(L"Type: REG_SZ | Data: %s\n", (LPCTSTR)data);
		break;

	case REG_EXPAND_SZ:
		_tprintf(L"Type: REG_EXPAND_SZ | Data: %s\n", (LPCTSTR)data);
		break;

	case REG_DWORD:
		if (dataSize >= sizeof(DWORD))
			_tprintf(L"Type: REG_DWORD | Data: %lu\n", *(DWORD*)data);
		break;

	case REG_QWORD:
		if (dataSize >= sizeof(ULONGLONG))
			_tprintf(L"Type: REG_QWORD | Data: %llu\n", *(ULONGLONG*)data);
		break;

	case REG_MULTI_SZ:
	{
		_tprintf(L"Type: REG_MULTI_SZ | Data:\n");

		LPCTSTR p = (LPCTSTR)data;
		while (*p)
		{
			_tprintf(L"  - %s\n", p);
			p += _tcslen(p) + 1;
		}
		break;
	}

	case REG_BINARY:
	{
		_tprintf(L"Type: REG_BINARY | Data(hex): ");

		for (DWORD i = 0; i < dataSize; i++)
			_tprintf(L"%02X ", data[i]);

		_tprintf(L"\n");
		break;
	}

	default:
		_tprintf(L"Type: %lu | Data: unsupported\n", tip);
		break;
	}

	_tprintf(L"-----------------------------\n");
}

int ReadSubKeyReg(HKEY rootKey, LPCTSTR subKey)
{
	HKEY hKey;

	LONG result = RegOpenKeyEx(
		rootKey,
		subKey,
		0,
		KEY_READ,
		&hKey
	);

	if (result != ERROR_SUCCESS)
	{
		_tprintf(L"ERROR: Cannot open key %s. Code: %ld\n", subKey, result);
		return 1;
	}

	DWORD nrValori = 0;
	DWORD maxValueNameLen = 0;
	DWORD maxDataLen = 0;

	result = RegQueryInfoKey(
		hKey,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&nrValori,
		&maxValueNameLen,
		&maxDataLen,
		NULL,
		NULL
	);

	if (result != ERROR_SUCCESS)
	{
		_tprintf(L"ERROR: Cannot query key info. Code: %ld\n", result);
		RegCloseKey(hKey);
		return 1;
	}

	_tprintf(L"Subkey: %s\n", subKey);
	_tprintf(L"Number of values: %lu\n", nrValori);
	_tprintf(L"=============================\n");

	for (DWORD index = 0; index < nrValori; index++)
	{
		DWORD valueNameSize = maxValueNameLen + 1;
		std::vector<TCHAR> valueName(valueNameSize);

		DWORD dataSize = maxDataLen;
		std::vector<BYTE> data(dataSize);

		DWORD type = 0;

		result = RegEnumValue(
			hKey,
			index,
			valueName.data(),
			&valueNameSize,
			NULL,
			&type,
			data.data(),
			&dataSize
		);

		if (result == ERROR_SUCCESS)
		{
			AfiseazaValoare(valueName.data(), type, data.data(), dataSize);
		}
		else
		{
			_tprintf(L"ERROR: reading index %lu. Code: %ld\n", index, result);
		}
	}

	RegCloseKey(hKey);
	return 0;
}

int main()
{
	LPCTSTR subKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

	ReadSubKeyReg(HKEY_CURRENT_USER, subKey);

	return 0;
}