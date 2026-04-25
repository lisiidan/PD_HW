#include <windows.h>
#include <setupapi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#pragma comment(lib, "setupapi.lib")

void PrintBoth(std::wofstream& out, const std::wstring& text)
{
	std::wcout << text;
	out << text;
}

void PrintProperty(
	HDEVINFO hDevInfo,
	SP_DEVINFO_DATA& deviceInfoData,
	DWORD propertyId,
	const wchar_t* label,
	std::wofstream& out)
{
	DWORD dataType = 0;
	DWORD requiredSize = 0;

	SetupDiGetDeviceRegistryPropertyW(
		hDevInfo,
		&deviceInfoData,
		propertyId,
		&dataType,
		nullptr,
		0,
		&requiredSize
	);

	PrintBoth(out, L"  " + std::wstring(label) + L": ");

	if (requiredSize == 0)
	{
		PrintBoth(out, L"(not available)\n");
		return;
	}

	std::vector<BYTE> buffer(requiredSize);

	BOOL ok = SetupDiGetDeviceRegistryPropertyW(
		hDevInfo,
		&deviceInfoData,
		propertyId,
		&dataType,
		buffer.data(),
		requiredSize,
		&requiredSize
	);

	if (!ok)
	{
		PrintBoth(out, L"(read error)\n");
		return;
	}

	switch (dataType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		PrintBoth(out, reinterpret_cast<wchar_t*>(buffer.data()));
		PrintBoth(out, L"\n");
		break;

	case REG_MULTI_SZ:
	{
		wchar_t* p = reinterpret_cast<wchar_t*>(buffer.data());

		while (*p)
		{
			PrintBoth(out, std::wstring(p) + L" ");
			p += wcslen(p) + 1;
		}

		PrintBoth(out, L"\n");
		break;
	}

	case REG_DWORD:
		if (buffer.size() >= sizeof(DWORD))
			PrintBoth(out, std::to_wstring(*reinterpret_cast<DWORD*>(buffer.data())) + L"\n");
		else
			PrintBoth(out, L"(invalid DWORD)\n");
		break;

	default:
		PrintBoth(out, L"(unsupported type: " + std::to_wstring(dataType) + L")\n");
		break;
	}
}

void ListAllDevices(std::wofstream& out)
{
	HDEVINFO hDevInfo = SetupDiGetClassDevsW(
		nullptr,
		nullptr,
		nullptr,
		DIGCF_PRESENT | DIGCF_ALLCLASSES
	);

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		PrintBoth(out, L"Error: could not retrieve device list.\n");
		return;
	}

	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	DWORD index = 0;

	while (SetupDiEnumDeviceInfo(hDevInfo, index, &deviceInfoData))
	{
		PrintBoth(out, L"\n========================================\n");
		PrintBoth(out, L"Device nr: " + std::to_wstring(index) + L"\n");

		PrintProperty(hDevInfo, deviceInfoData, SPDRP_FRIENDLYNAME, L"Friendly Name", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_DEVICEDESC, L"Description", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_MFG, L"Manufacturer", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_CLASS, L"Class", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_CLASSGUID, L"Class GUID", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_HARDWAREID, L"Hardware ID", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_COMPATIBLEIDS, L"Compatible IDs", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_ENUMERATOR_NAME, L"Enumerator", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_LOCATION_INFORMATION, L"Location", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_SERVICE, L"Service", out);
		PrintProperty(hDevInfo, deviceInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, L"PDO Name", out);

		index++;
	}

	PrintBoth(out, L"\nTotal devices found: " + std::to_wstring(index) + L"\n");

	SetupDiDestroyDeviceInfoList(hDevInfo);
}

int wmain()
{
	std::wofstream out("devices_output.txt");

	if (!out.is_open())
	{
		std::wcerr << L"Error: could not open output file.\n";
		return 1;
	}

	ListAllDevices(out);

	out.close();
	return 0;
}