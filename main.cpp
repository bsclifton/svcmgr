// main.cpp
// brian clifton 2003

#include "main.h"

HWND           g_Dialog;
NOTIFYICONDATA gni_Data;
SC_HANDLE      g_ServiceManager;
SERVICE_STATUS g_Status;
HICON          g_IconRunning, g_IconStopped;

#define MY_SERVICE_NAME L"MyService"
#define MY_SERVICE_NAME_FRIENDLY L"My Service"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmdline, int cmdshow) {
	if (!InitInstance(hInst, cmdshow)) {
		return 0;
	}

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		UpdateStatus(g_Dialog);
	}
	return (int)msg.wParam;
}

int WINAPI EventHandler(HWND a, UINT b, WPARAM c, LPARAM d) {
	int wmId, wmEvent;

	switch (b) {
	case SWM_TRAYMSG:
		switch (d) {
		case WM_LBUTTONDBLCLK:
			ShowWindow(a, SW_RESTORE);
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(a);
			break;
		}
		break;

	case WM_SYSCOMMAND:
		if ((c & 0xFFF0) == SC_MINIMIZE) {
			ShowWindow(a, SW_HIDE);
			return 1;
		}
		break;

	case WM_COMMAND:
		wmId = LOWORD(c);
		wmEvent = HIWORD(c);

		switch (wmId) {
		case SWM_SHOW:
			ShowWindow(a, SW_RESTORE);
			break;
		case SWM_HIDE:
		case IDOK:
			ShowWindow(a, SW_HIDE);
			break;
		case SWM_EXIT:
			DestroyWindow(a);
			break;
		case SWM_START:
		case BTN_START:
			ServiceStart(a);
			break;
		case BTN_RESTART:
			MessageBox(a, L"Not implemented yet", 0, 0);
			break;
		case SWM_STOP:
		case BTN_STOP:
			ServiceStop(a);
			break;
		case BTN_SAVE:
			Save(a);
			break;
		case BTN_RESET:
			Load(a);
			break;
		}
		return 1;

	case WM_CLOSE:
		DestroyWindow(a);
		break;

	case WM_DESTROY:
		gni_Data.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &gni_Data);
		PostQuitMessage(0);
		break;
	}
	return 0;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	// prepare for XP style controls
	InitCommonControls();

	// load icon
	g_IconRunning = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	g_IconStopped = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	// store instance handle and create dialog
	g_Dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, (DLGPROC)EventHandler);
	if (!g_Dialog) {
		return FALSE;
	}

	// service manager creation
	g_ServiceManager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (!g_ServiceManager) {
		return FALSE;
	}

	UpdateStatus(g_Dialog);

	// create tray item
	memset(&gni_Data, 0, sizeof(NOTIFYICONDATA));
	gni_Data.cbSize = sizeof(NOTIFYICONDATA);
	wcscpy_s(gni_Data.szTip, 128, MY_SERVICE_NAME_FRIENDLY);
	gni_Data.uID = TRAYICONID;
	gni_Data.uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP);

	gni_Data.hIcon = (g_Status.dwCurrentState == SERVICE_RUNNING) ? g_IconRunning : g_IconStopped;

	gni_Data.hWnd = g_Dialog;
	gni_Data.uCallbackMessage = SWM_TRAYMSG;

	// load settings from file
	Load(g_Dialog);

	// add to tray
	Shell_NotifyIcon(NIM_ADD, &gni_Data);
	return TRUE;
}

void Load(HWND hWnd) {
	// load settings from file
	if (!Config::Load()) {
		MessageBox(hWnd, L"This program was unable to load settings from disk", L"Error loading settings", MB_ICONERROR);
		return;
	}

	// load items into form
	wchar_t strTemp[64];
	SetDlgItemText(hWnd, TXT_HOST, Config::PaymentServiceHost);
	wsprintf(strTemp, L"%d", Config::PortNumber);
	SetDlgItemText(hWnd, TXT_PORTNO, strTemp);
	wsprintf(strTemp, L"%d", Config::MaxConnections);
	SetDlgItemText(hWnd, TXT_MAXCON, strTemp);
	SetDlgItemText(hWnd, TXT_SITE, Config::PaymentServiceSite);
	SetDlgItemText(hWnd, TXT_PAGE, Config::PaymentServicePage);
	SetDlgItemText(hWnd, TXT_DB, Config::ConnectionString);

	HWND cbLoggingEnabled = GetDlgItem(hWnd, CB_LOG);
	SendMessage(cbLoggingEnabled, BM_SETCHECK, (WPARAM)Config::WriteToLog ? 1 : 0, 0);
}

void Save(HWND hWnd) {
	wchar_t strTemp[2048];

	//get parameters from form
	GetDlgItemText(hWnd, TXT_HOST, strTemp, 2047);
	wcscpy_s(Config::PaymentServiceHost, 2048, strTemp);

	GetDlgItemText(hWnd, TXT_PORTNO, strTemp, 2047);
	Config::PortNumber = _wtoi(strTemp);
	if (Config::PortNumber < 0) {
		Config::PortNumber = 0;
	}

	GetDlgItemText(hWnd, TXT_MAXCON, strTemp, 2047);
	Config::MaxConnections = _wtoi(strTemp);
	if (Config::MaxConnections <= 0) {
		Config::MaxConnections = 1;
	}

	GetDlgItemText(hWnd, TXT_SITE, strTemp, 2047);
	wcscpy_s(Config::PaymentServiceSite, 2048, strTemp);

	GetDlgItemText(hWnd, TXT_PAGE, strTemp, 2047);
	wcscpy_s(Config::PaymentServicePage, 2048, strTemp);

	GetDlgItemText(hWnd, TXT_DB, strTemp, 2047);
	wcscpy_s(Config::ConnectionString, 2048, strTemp);

	HWND cbLoggingEnabled = GetDlgItem(hWnd, CB_LOG);
	Config::WriteToLog = (SendMessage(cbLoggingEnabled, BM_GETCHECK, 0, 0)) ? true : false;

	// save to file,refresh
	if (!Config::Save()) {
		MessageBox(hWnd, L"This program was unable to write settings to disk", L"Error saving settings", MB_ICONERROR);
	} else {
		Load(hWnd);
	}
}

void ServiceStart(HWND hWnd) {
	SC_HANDLE svc = OpenService(g_ServiceManager, MY_SERVICE_NAME, SERVICE_ALL_ACCESS);
	int result = StartService(svc, 0, 0);

	if (!result) {
		int errorCode = GetLastError();
		wchar_t strTitle[] = L"Error Starting Service";
		wchar_t strText[1024];

		switch (errorCode) {
			case ERROR_ACCESS_DENIED:
				wcscpy_s(strText, 1024, L"The handle does not have the SERVICE_START access right");
				break;
			case ERROR_INVALID_HANDLE:
				wcscpy_s(strText, 1024, L"The handle is invalid");
				break;
			case ERROR_FILE_NOT_FOUND:
				wcscpy_s(strText, 1024, L"The service binary file could not be found");
				break;
			case ERROR_PATH_NOT_FOUND:
				wcscpy_s(strText, 1024, L"The service binary file could not be found");
				break;
			case ERROR_SERVICE_ALREADY_RUNNING:
				wcscpy_s(strText, 1024, L"An instance of the service is already running");
				break;
			case ERROR_SERVICE_DATABASE_LOCKED:
				wcscpy_s(strText, 1024, L"The database is locked");
				break;
			default:
				wsprintf(strText, L"Unknown error: %d", errorCode);
				break;
		}
		MessageBox(hWnd, strText, strTitle, MB_ICONERROR);
	}
	CloseServiceHandle(svc);
}

void ServiceStop(HWND hWnd) {
	SC_HANDLE svc = OpenService(g_ServiceManager, MY_SERVICE_NAME, SERVICE_ALL_ACCESS);
	SERVICE_STATUS svc_status;

	int result = ControlService(svc, SERVICE_CONTROL_STOP, &svc_status);
	if (!result) {
		int  errorCode = GetLastError();
		wchar_t strTitle[] = L"Error Stopping Service";
		wchar_t strText[1024];

		switch (errorCode) {
			case ERROR_ACCESS_DENIED:
				wcscpy_s(strText, 1024, L"The handle does not have the required access");
				break;
			case ERROR_DEPENDENT_SERVICES_RUNNING:
				wcscpy_s(strText, 1024, L"The service cannot be stopped because other running services are dependent on it");
				break;
			case ERROR_INVALID_HANDLE:
				wcscpy_s(strText, 1024, L"The specified handle was not obtained using CreateService or OpenService, or the handle is no longer valid");
				break;
			case ERROR_INVALID_PARAMETER:
				wcscpy_s(strText, 1024, L"The requested control code is undefined");
				break;
			case ERROR_INVALID_SERVICE_CONTROL:
				wcscpy_s(strText, 1024, L"The requested control code is not valid, or it is unacceptable to the service");
				break;
			case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
				wcscpy_s(strText, 1024, L"The requested control code cannot be sent to the service because the state of the service is SERVICE_STOPPED, SERVICE_START_PENDING, or SERVICE_STOP_PENDING");
				break;
			case ERROR_SERVICE_NOT_ACTIVE:
				wcscpy_s(strText, 1024, L"The service has not been started");
				break;
			case ERROR_SERVICE_REQUEST_TIMEOUT:
				wcscpy_s(strText, 1024, L"The process for the service was started, but it did not call StartServiceCtrlDispatcher, or the thread that called StartServiceCtrlDispatcher may be blocked in a control handler function");
				break;
			case ERROR_SHUTDOWN_IN_PROGRESS:
				wcscpy_s(strText, 1024, L"The system is shutting down");
				break;
			default:
				wsprintf(strText, L"Unknown error: %d", errorCode);
				break;
		}
		MessageBox(hWnd, strText, strTitle, MB_ICONERROR);
	}
	CloseServiceHandle(svc);
}

void SetWindowCaption(HWND hWnd, const wchar_t* lpString, ...) {
	wchar_t strText[2048];
	wchar_t temp[2048];
	va_list	arg_list;

	va_start(arg_list, lpString);
	wvsprintf(temp, lpString, arg_list);
	va_end(arg_list);

	wsprintf(strText, MY_SERVICE_NAME_FRIENDLY" - %s", temp);

	GetWindowText(hWnd, temp, 2047);
	if (wcscmp(temp, strText)) {
		SetWindowText(hWnd, strText);
	}
	return;
}

// Name says it all
void ShowContextMenu(HWND hWnd) {
	HMENU hMenu;
	POINT pt;

	GetCursorPos(&pt);
	hMenu = CreatePopupMenu();

	if (hMenu) {
		UpdateStatus(hWnd);

		// show/hide
		if (IsWindowVisible(hWnd)){
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, L"Hide Control Panel");
		} else {
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, L"Open Control Panel");
		}

		// start/stop
		if (g_Status.dwCurrentState == SERVICE_RUNNING) {
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_STOP, L"Stop Service");
		} else if (g_Status.dwCurrentState == SERVICE_STOPPED) {
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_START, L"Start Service");
		}

		// exit
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, L"Exit");

		// must be set to foreground window
		SetForegroundWindow(hWnd);
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}

void UpdateStatus(HWND hWnd) {
	// get buttons
	HWND btnStart = GetDlgItem(hWnd, BTN_START);
	HWND btnStop = GetDlgItem(hWnd, BTN_STOP);

	// get status
	SC_HANDLE svc = OpenService(g_ServiceManager, MY_SERVICE_NAME, SERVICE_QUERY_STATUS);
	QueryServiceStatus(svc, &g_Status);
	CloseServiceHandle(svc);

	// set icon
	switch (g_Status.dwCurrentState) {
		case SERVICE_START_PENDING:
			EnableWindow(btnStart, 0);
			EnableWindow(btnStop, 0);
			SetWindowCaption(hWnd, L"Starting...");

			if (gni_Data.hIcon != g_IconStopped) {
				gni_Data.hIcon = g_IconStopped;
				Shell_NotifyIcon(NIM_MODIFY, &gni_Data);
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_IconStopped);
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_IconStopped);
			}
			break;

		case SERVICE_RUNNING:
			EnableWindow(btnStart, 0);
			EnableWindow(btnStop, 1);
			SetWindowCaption(hWnd, L"Running");

			if (gni_Data.hIcon != g_IconRunning) {
				gni_Data.hIcon = g_IconRunning;
				Shell_NotifyIcon(NIM_MODIFY, &gni_Data);
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_IconRunning);
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_IconRunning);
			}
			break;

		case SERVICE_STOP_PENDING:
			EnableWindow(btnStart, 0);
			EnableWindow(btnStop, 0);
			SetWindowCaption(hWnd, L"Stopping...");

			if (gni_Data.hIcon != g_IconStopped) {
				gni_Data.hIcon = g_IconStopped;
				Shell_NotifyIcon(NIM_MODIFY, &gni_Data);
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_IconStopped);
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_IconStopped);
			}
			break;

		case SERVICE_STOPPED:
		default:
			EnableWindow(btnStart, 1);
			EnableWindow(btnStop, 0);
			SetWindowCaption(hWnd, L"Stopped");

			if (gni_Data.hIcon != g_IconStopped) {
				gni_Data.hIcon = g_IconStopped;
				Shell_NotifyIcon(NIM_MODIFY, &gni_Data);
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_IconStopped);
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_IconStopped);
			}
			break;
	}
}
