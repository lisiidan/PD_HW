#include <windows.h>

#pragma comment(lib, "advapi32.lib")

constexpr LPCWSTR SERVICE_NAME = L"HelloWorldService";

SERVICE_STATUS g_serviceStatus{};
SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
HANDLE g_stopEvent = nullptr;

void WriteToEventLog(LPCWSTR message)
{
	HANDLE eventSource = RegisterEventSourceW(nullptr, SERVICE_NAME);
	if (eventSource == nullptr)
		return;

	LPCWSTR messages[] = { message };

	ReportEventW(
		eventSource,
		EVENTLOG_INFORMATION_TYPE,
		0,
		1,
		nullptr,
		1,
		0,
		messages,
		nullptr
	);

	DeregisterEventSource(eventSource);
}

void SetStatus(DWORD state, DWORD errorCode = NO_ERROR, DWORD waitHint = 0)
{
	g_serviceStatus.dwCurrentState = state;
	g_serviceStatus.dwWin32ExitCode = errorCode;
	g_serviceStatus.dwWaitHint = waitHint;

	g_serviceStatus.dwControlsAccepted =
		(state == SERVICE_RUNNING) ? SERVICE_ACCEPT_STOP : 0;

	SetServiceStatus(g_statusHandle, &g_serviceStatus);
}

void Cleanup()
{
	if (g_stopEvent != nullptr)
	{
		CloseHandle(g_stopEvent);
		g_stopEvent = nullptr;
	}
}

void WINAPI ServiceControlHandler(DWORD controlCode)
{
	if (controlCode != SERVICE_CONTROL_STOP)
		return;

	if (g_serviceStatus.dwCurrentState != SERVICE_RUNNING)
		return;

	SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
	WriteToEventLog(L"End of the World!");

	if (g_stopEvent != nullptr)
		SetEvent(g_stopEvent);
}

void WINAPI ServiceMain(DWORD argc, LPWSTR* argv)
{
	g_statusHandle = RegisterServiceCtrlHandlerW(
		SERVICE_NAME,
		ServiceControlHandler
	);

	if (g_statusHandle == nullptr)
		return;

	g_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_serviceStatus.dwServiceSpecificExitCode = 0;
	g_serviceStatus.dwCheckPoint = 0;

	SetStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	g_stopEvent = CreateEventW(
		nullptr,
		TRUE,
		FALSE,
		nullptr
	);

	if (g_stopEvent == nullptr)
	{
		SetStatus(SERVICE_STOPPED, GetLastError());
		return;
	}

	WriteToEventLog(L"Hello World!");

	SetStatus(SERVICE_RUNNING);

	WaitForSingleObject(g_stopEvent, INFINITE);

	Cleanup();

	SetStatus(SERVICE_STOPPED);
}

int wmain()
{
	SERVICE_TABLE_ENTRYW serviceTable[] =
	{
		{ const_cast<LPWSTR>(SERVICE_NAME), ServiceMain },
		{ nullptr, nullptr }
	};

	StartServiceCtrlDispatcherW(serviceTable);

	return 0;
}