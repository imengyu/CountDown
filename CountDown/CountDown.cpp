// CountDown.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "CountDown.h"
#include <shellapi.h>
#include <shlwapi.h>
#include <windowsx.h>
// 开启可视化效果  
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name = 'Microsoft.Windows.Common-Controls' version = '6.0.0.0' \
processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")

#define TIMER_WORK 10019

HINSTANCE hInst; 
WCHAR fullPath[MAX_PATH];
WCHAR fullIniPath[MAX_PATH];

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,  _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;

	//获取完整ini路径
	GetModuleFileName(0, fullPath, MAX_PATH);
	GetModuleFileName(0, fullIniPath, MAX_PATH);
	PathRenameExtension(fullIniPath, (LPWSTR)L".ini");

	HWND hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_APP), GetDesktopWindow(), (DLGPROC)DlgProc);
	if (!hDlg)
		return 0;
	if(!AdjustToken())
		MessageBox(0, L"获取权限失败！", L"错误", MB_ICONERROR);

	ShowWindow(hDlg, SW_SHOW);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COUNTDOWN));
    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//配置：
int mainFontSize = 26;
int mainWidth = 540;
int mainHeight = 228;
int countDownSecond = 60;
int countDownSecondCount = 60;
bool couldCancel = true;
bool mainTextFlash = false;
bool enterSecond = false;
bool topMost = false;
WCHAR cuntDownAction[256];
WCHAR tipTitle[256];
WCHAR tipText[256];
LPWSTR tipIcon = IDI_EXCLAMATION;
int tipIconMb = MB_ICONEXCLAMATION;

bool firstShow = true;
bool firstKeyCancel = false;
HFONT hFont = NULL;
HBRUSH hRedBrush = NULL;

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG: {
		hRedBrush = CreateSolidBrush(RGB(255, 0, 0));
		//加载配置
		LoadCfg();
		//设置大小
		if (mainWidth != 540 || mainHeight != 228)  SetWindowPos(hDlg, NULL, 0, 0, mainWidth, mainHeight, SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOZORDER);
		// 设置对话框的图标
		HICON hIcon = LoadIcon(0, tipIcon);
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		//设置字体
		hFont = CreateFontW(mainFontSize, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");//创建字体
		SendDlgItemMessage(hDlg, IDC_STATIC_TIPTEXT, WM_SETFONT, (WPARAM)hFont, TRUE);//发送设置字体消息
		UpdateWindow(hDlg);
		break;
	}
	case WM_DESTROY: {
		DeleteObject(hFont);//删除所创建字体对象
		DeleteObject(hRedBrush);
		
		PostQuitMessage(0);
		break;
	}
	case WM_SHOWWINDOW: {
		if (wParam && firstShow) {
			Start(hDlg);
			firstShow = false;
		}
		break;
	}
	case WM_SYSCOMMAND: {
		if (couldCancel && wParam == SC_CLOSE)
			Cancel(hDlg);//退出
		break;
	}
	case WM_COMMAND: {
		switch (wParam)
		{
		case IDC_CANCEL: if (couldCancel)  Cancel(hDlg); break;
		default: break;
		}
		break;
	}
	case WM_KEYDOWN: {
		short ctrl = GetKeyState(VK_CONTROL);
		if (wParam == 0x51 && ctrl) {//Ctrl+Q
			if (!firstKeyCancel) {
				firstKeyCancel = true;
				Stop(hDlg);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_TIPTEXT), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_WTIP), SW_SHOW);
				SetDlgItemText(hDlg, IDC_STATIC_WTIP, L"再次按 Ctrl+Q 退出程序，按Ctrl+A 继续程序运行 ");
			}
			else Cancel(hDlg);
		}
		else if (wParam == 0x41 && ctrl && firstKeyCancel) {//Ctrl+A
			firstKeyCancel = false;
			ShowWindow(GetDlgItem(hDlg, IDC_STATIC_TIPTEXT), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_STATIC_WTIP), SW_HIDE);
			StartTimer(hDlg);
		}
		else if (wParam == 0x44 && ctrl) {//Ctrl+D
			countDownSecondCount = 1;
			ShowWindow(GetDlgItem(hDlg, IDC_STATIC_TIPTEXT), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_STATIC_WTIP), SW_SHOW);
			SetDlgItemText(hDlg, IDC_STATIC_WTIP, L"倒计时提前了！！！ ");
		}
		else if (wParam == 0x53 && ctrl) {//Ctrl+S
			Stop(hDlg);
			ShowWindow(hDlg, SW_HIDE);
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CONFIG), hDlg, ConfigDlgProc);
			ShowWindow(hDlg, SW_SHOW);
			StartTimer(hDlg);
		}
		break;
	}
	case WM_TIMER: {
		if (wParam == TIMER_WORK)RunTick(hDlg);
		break;
	}
	case WM_ENDSESSION:
	case WM_QUERYENDSESSION: {
		PostQuitMessage(0);
		break;
	}
	case WM_CTLCOLORSTATIC: {//拦截WM_CTLCOLORSTATIC消息 
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_STATIC_TIPTEXT))//获得指定标签句柄用来对比
		{
			if (!mainTextFlash && countDownSecondCount > 10) 	
				SetTextColor((HDC)wParam, RGB(0, 0, 0));//设置文本颜色
			else {
				if (countDownSecondCount % 2 == 0) {
					if (mainTextFlash) {
						SetTextColor((HDC)wParam, RGB(255, 255, 255));//设置文本颜色
						SetBkColor((HDC)wParam, RGB(255, 0, 0));
						return (INT_PTR)hRedBrush;
					}
					else {
						SetTextColor((HDC)wParam, RGB(255, 0, 0));//设置文本颜色
					}
				}
				else {				
					SetTextColor((HDC)wParam, RGB(0, 0, 0));//设置文本颜色
				}
			}
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_STATIC_WTIP) || (HWND)lParam == GetDlgItem(hDlg, IDC_STATIC))  SetTextColor((HDC)wParam, RGB(255, 0, 0));
		if (mainTextFlash && countDownSecondCount % 2 == 0) return (INT_PTR)hRedBrush;
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}
	case WM_CTLCOLORDLG: {
		if(mainTextFlash && countDownSecondCount % 2 == 0) return (INT_PTR)hRedBrush;
		else return (INT_PTR)(HBRUSH)GetStockObject(WHITE_BRUSH);
	}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK EnterSecDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG: {
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		WCHAR secText[32];
		_itow_s(countDownSecond, secText, 10);
		SetDlgItemText(hDlg, IDC_EDIT_SEC, secText);
		break;
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE) 
			EndDialog(hDlg, wParam);
		break;
	}
	case WM_COMMAND: {
		switch (wParam)
		{
		case IDCANCEL:EndDialog(hDlg, wParam); break;
		case IDOK: {
			WCHAR enterSec[32];
			GetDlgItemText(hDlg, IDC_EDIT_SEC, enterSec, 32);
			if (!StrEqual(enterSec, L"")) {
				countDownSecond = _wtoi(enterSec);
				EndDialog(hDlg, wParam);
			}
			else if (MessageBox(hDlg, L"如果您不输入倒计时则程序会使用默认倒计时，点击“是”使用默认，点击“否”重新输入", L"提示", MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {
				EndDialog(hDlg, wParam);
			}
			break;
		}
		default: break;
		}
		break;
	}
	case WM_ENDSESSION:
	case WM_QUERYENDSESSION: {
		PostQuitMessage(0);
		break;
	}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG: {
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONFIG));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		
		WCHAR secText[32];
		_itow_s(countDownSecond, secText, 10);
		SetDlgItemText(hDlg, IDC_EDIT_SEC, secText);

		SetDlgItemText(hDlg, IDC_EDIT_TITLE, tipTitle);
		SetDlgItemText(hDlg, IDC_EDIT_TEXT, tipText);

		HWND hLevel = GetDlgItem(hDlg, IDC_COMBO_ICON);
		ComboBox_AddString(hLevel, L"错误");
		ComboBox_AddString(hLevel, L"警告");
		ComboBox_AddString(hLevel, L"提示");
		ComboBox_AddString(hLevel, L"消息");

		if(tipIcon == IDI_ERROR)
			ComboBox_SetCurSel(hLevel, 0);
		else if (tipIcon == IDI_EXCLAMATION) 
			ComboBox_SetCurSel(hLevel, 1);
		else if (tipIcon == IDI_INFORMATION) 
			ComboBox_SetCurSel(hLevel, 2);
		else if (tipIcon == IDI_ASTERISK)
			ComboBox_SetCurSel(hLevel, 3);

		HWND hCmd = GetDlgItem(hDlg, IDC_COMBO_CMD);
		ComboBox_AddString(hCmd, L"关机");
		ComboBox_AddString(hCmd, L"重启");
		ComboBox_AddString(hCmd, L"注销");
		if (StrEqual(cuntDownAction, L"关机")) 
			ComboBox_SetCurSel(hCmd, 0);
		else if (StrEqual(cuntDownAction, L"重启"))
			ComboBox_SetCurSel(hCmd, 1);
		else if (StrEqual(cuntDownAction, L"注销")) 
			ComboBox_SetCurSel(hCmd, 2);
		else {
			ComboBox_AddString(hCmd, cuntDownAction);
			ComboBox_SetCurSel(hCmd, 3);
		}

		if (couldCancel) CheckDlgButton(hDlg, IDC_CHECK_CANCANCEL, TRUE);
		if (enterSecond) CheckDlgButton(hDlg, IDC_CHECK_CANENTER, TRUE);
		if (topMost) CheckDlgButton(hDlg, IDC_CHECK_TOPMOST, TRUE);

		break;
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE) SendMessage(hDlg, WM_COMMAND, IDCANCEL, NULL);
		break;
	}
	case WM_COMMAND: {
		switch (wParam)
		{
		case IDCANCEL: {
			if (MessageBox(hDlg, L"真的要放弃所作的更改吗？", L"提示", MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
				EndDialog(hDlg, wParam);
			break;
		}
		case IDOK: {

			WCHAR secText[32];
			GetDlgItemText(hDlg, IDC_EDIT_SEC, secText, 32);
			if (!StrEqual(secText,L"")) {
				int v = _wtoi(secText);
				if (v <= 0 || v > 86401) {
					MessageBox(hDlg, L"倒计时时间秒数必须介于（0-86401之间）", L"错误", MB_ICONHAND);
					return 0;
				}
				else countDownSecond = v;
			}

			WCHAR buf[256];
			GetDlgItemText(hDlg, IDC_EDIT_TITLE, buf, 256);
			wcscpy_s(tipTitle, buf);
			GetDlgItemText(hDlg, IDC_EDIT_TEXT, buf, 256);
			wcscpy_s(tipText, buf);

			ComboBox_GetText(GetDlgItem(hDlg, IDC_COMBO_CMD), buf, 256);
			wcscpy_s(cuntDownAction, buf);

			int sel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_ICON));

			couldCancel = IsDlgButtonChecked(hDlg, IDC_CHECK_CANCANCEL);
			enterSecond = IsDlgButtonChecked(hDlg, IDC_CHECK_CANENTER);
			topMost = IsDlgButtonChecked(hDlg, IDC_CHECK_TOPMOST);

			SaveConfig(sel);
			if (MessageBox(hDlg, L"设置成功保存，但是必须重启软件才能生效。 点击“是”重启软件。", L"提示", MB_YESNO) == IDYES) {
				PostQuitMessage(0);
				ShellExecute(hDlg, L"open", fullPath, 0, 0, SW_NORMAL);
			}
			EndDialog(hDlg, wParam);
			break;
		}
		case IDC_ABOUT: {
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, AboutDlgProc);
			break;
		}
		default: break;
		}
		break;
	}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG: {
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ABOUT));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE) SendMessage(hDlg, WM_COMMAND, IDOK, NULL);
		break;
	}
	case WM_COMMAND: {
		switch (wParam)
		{
		case IDOK:  EndDialog(hDlg, wParam); break;
		default: break;
		}
		break;
	}
	}
	return (INT_PTR)FALSE;
}

void LoadCfg() {

	WCHAR buf[256];
	GetPrivateProfileStringW(L"CountDown", L"CountDownSecond", L"60", buf, 256, fullIniPath);
	countDownSecond = _wtoi(buf);
	if (countDownSecond <= 0 || countDownSecond > 86401) {
		MessageBox(0, L"倒计时时间秒数必须介于（0-86401之间）", L"错误", MB_ICONERROR);
		PostQuitMessage(0);
	}
	countDownSecondCount = countDownSecond;

	int nWidth = GetSystemMetrics(SM_CXSCREEN);
	int nHeight = GetSystemMetrics(SM_CYSCREEN);

	//540 x 228
	GetPrivateProfileStringW(L"CountDown", L"WndMainWidth", L"540", buf, 256, fullIniPath);
	mainWidth = _wtoi(buf);
	if (mainWidth < 540 && mainWidth > nWidth) mainWidth = 540;
	GetPrivateProfileStringW(L"CountDown", L"WndMainHeight", L"228", buf, 256, fullIniPath);
	mainHeight = _wtoi(buf);
	if (mainHeight < 228 && mainHeight > nHeight) mainHeight = 228;
	GetPrivateProfileStringW(L"CountDown", L"FontSize", L"26", buf, 256, fullIniPath);
	mainFontSize = _wtoi(buf);
	if (mainFontSize < 18 && mainFontSize > 45) mainFontSize = 540;

	WCHAR buf2[16];
	GetPrivateProfileStringW(L"CountDown", L"CouldCancel", L"1", buf2, 16, fullIniPath);
	couldCancel = StrEqual(buf2, L"1") || StrEqual(buf2, L"TRUE") || StrEqual(buf2, L"true");

	GetPrivateProfileStringW(L"CountDown", L"EnterSecond", L"0", buf2, 16, fullIniPath);
	enterSecond = StrEqual(buf2, L"1") || StrEqual(buf2, L"TRUE") || StrEqual(buf2, L"true");

	GetPrivateProfileStringW(L"CountDown", L"TopMost", L"0", buf2, 16, fullIniPath);
	topMost = StrEqual(buf2, L"1") || StrEqual(buf2, L"TRUE") || StrEqual(buf2, L"true");

	GetPrivateProfileStringW(L"CountDown", L"FlashingText", L"0", buf2, 16, fullIniPath);
	mainTextFlash = StrEqual(buf2, L"1") || StrEqual(buf2, L"TRUE") || StrEqual(buf2, L"true");

	GetPrivateProfileStringW(L"CountDown", L"CountDownAction", L"msg 倒计时结束！", buf, 256, fullIniPath);
	if (StrEqual(buf, L"")) {
		MessageBox(0, L"没有定义操作", L"错误", MB_ICONERROR);
		PostQuitMessage(0);
	}
	wcscpy_s(cuntDownAction, buf);

	GetPrivateProfileStringW(L"CountDown", L"TipTitle", L"倒计时", buf, 256, fullIniPath);
	wcscpy_s(tipTitle, buf);

	GetPrivateProfileStringW(L"CountDown", L"TipText", L"倒计时：%s 后执行某个操作。", buf, 256, fullIniPath);
	wcscpy_s(tipText, buf);

	GetPrivateProfileStringW(L"CountDown", L"TipIcon", L"警告", buf2, 16, fullIniPath);
	if (StrEqual(buf2, L"错误")) {
		tipIcon = IDI_ERROR;
		tipIconMb = MB_ICONERROR;
	}else if (StrEqual(buf2, L"提示")) {
		tipIcon = IDI_INFORMATION;
		tipIconMb = MB_ICONINFORMATION;
	} else if (StrEqual(buf2, L"消息")) {
		tipIcon = IDI_ASTERISK;
		tipIconMb = MB_ICONASTERISK;
	}
}
void SaveConfig(int icon) {
	WCHAR secText[32];
	_itow_s(countDownSecond, secText, 10);
	WritePrivateProfileStringW(L"CountDown", L"CountDownSecond", secText, fullIniPath);
	WritePrivateProfileStringW(L"CountDown", L"CouldCancel", couldCancel ? L"TRUE" : L"FALSE", fullIniPath);
	WritePrivateProfileStringW(L"CountDown", L"EnterSecond", enterSecond ? L"TRUE" : L"FALSE", fullIniPath);
	WritePrivateProfileStringW(L"CountDown", L"TopMost", topMost ? L"TRUE" : L"FALSE", fullIniPath);
	WritePrivateProfileStringW(L"CountDown", L"CountDownAction", cuntDownAction, fullIniPath);
	WritePrivateProfileStringW(L"CountDown", L"TipTitle", tipTitle, fullIniPath);
	WritePrivateProfileStringW(L"CountDown", L"TipText", tipText, fullIniPath);
	const wchar_t* iconStr = 0;
	switch (icon)
	{
	case 0:  iconStr = L"错误"; break;
	default:
	case 1: iconStr = L"警告";  break;
	case 2:  iconStr = L"提示"; break;
	case 3:  iconStr = L"信息"; break;
	}
	WritePrivateProfileStringW(L"CountDown", L"TipIcon", iconStr, fullIniPath);
}
void Start(HWND hDlg) {
	SetWindowText(hDlg, tipTitle);
	SetDlgItemText(hDlg, IDC_STATIC_TIPTEXT, L"");
	Static_SetIcon(GetDlgItem(hDlg, IDC_STATIC_TIPICON),LoadIcon(NULL, tipIcon));

	if (!couldCancel) {
		EnableWindow(GetDlgItem(hDlg, IDC_CANCEL), FALSE);
		LONG oldLog = GetClassLong(hDlg, GCL_STYLE);
		oldLog |= CS_NOCLOSE;
		SetClassLong(hDlg, GCL_STYLE, oldLog);
	}
	if (topMost) {
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	if (enterSecond) {
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ENTER_SEC), hDlg, EnterSecDlgProc);
	}
	StartTimer(hDlg);
}
void StartTimer(HWND hDlg) {
	SetTimer(hDlg, TIMER_WORK, 1000, NULL);
}
void Stop(HWND hDlg) {
	KillTimer(hDlg, TIMER_WORK);
}
void Cancel(HWND hDlg) {
	Stop(hDlg);
	DestroyWindow(hDlg);
	PostQuitMessage(0);
}
void RunTick(HWND hDlg) {
	if (countDownSecondCount > 0) {
		WCHAR timeStr[32];
		WCHAR finalStr[300];
		memset(finalStr, 0, sizeof(finalStr));
		memset(timeStr, 0, sizeof(timeStr));

		GetSecTimeStr(countDownSecondCount, timeStr, 32);

		swprintf_s(finalStr, tipText, timeStr);
		SetDlgItemText(hDlg, IDC_STATIC_TIPTEXT, finalStr);

		if (mainTextFlash) InvalidWindow(hDlg);

		countDownSecondCount--;
	}
	else {
		Stop(hDlg);
		ShowWindow(hDlg, SW_HIDE);
		RunCmd();
		PostQuitMessage(0);
	}
}
void RunCmd() {
	if (wcsstr(cuntDownAction, L"msg") == cuntDownAction && wcslen(cuntDownAction) >= 5) {//Msgbox
		LPWSTR newStr = (LPWSTR)((DWORD)cuntDownAction + sizeof(WCHAR) * 4);
		MessageBox(GetDesktopWindow(), newStr, tipTitle, tipIconMb);
	}
	else if (StrEqual(cuntDownAction, L"关机"))
		ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
	else if (StrEqual(cuntDownAction, L"重启"))
		ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
	else if (StrEqual(cuntDownAction, L"注销")) 
		ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, 0);
	else _wsystem(cuntDownAction);
}
void InvalidWindow(HWND hDlg) {
	RECT rc = { 0 };
	GetWindowRect(hDlg, &rc);
	rc.top = rc.left = 0;
	InvalidateRect(hDlg, &rc, TRUE);
}

//帮助函数
bool AdjustToken() {
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	TOKEN_PRIVILEGES oldtp;
	DWORD dwSize = sizeof(TOKEN_PRIVILEGES);
	LUID luid;
	TOKEN_PRIVILEGES tkp = { 0 };

	ZeroMemory(&tp, sizeof(tp));
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) return true;
		else return false;
	}
	if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid))
	{
		CloseHandle(hToken);
		return false;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) {
		CloseHandle(hToken);
		return false;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		CloseHandle(hToken);
		return false;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) {
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);
	return true;
}
bool StrEqual(const wchar_t*a, const wchar_t*b) {
	return wcscmp(a,b)==0;
}
void GetSecTimeStr(int second, LPWSTR timeStr, size_t max) {
	WCHAR buf[16];
	int d = 0, h = 0, m = 0, s = 0;
	if (second > 86400) { d = second / 86400; second -= d * 86400; }
	if (second > 3600) { h = second / 3600; second -= h * 3600; }
	if (second > 60) { m = second / 60; second -= m * 60; }
	s = second;
	if (d > 0) {
		swprintf_s(buf, L" %d 天", d);
		wcscat_s(timeStr, max, buf);
	}
	if (h > 0) {
		swprintf_s(buf, L" %d 时", h);
		wcscat_s(timeStr, max, buf);
	}
	if (m > 0) {
		swprintf_s(buf, L" %d 分", m);
		wcscat_s(timeStr, max, buf);
	}
	if (s >= 0) {
		swprintf_s(buf, L" %d 秒", s);
		wcscat_s(timeStr, max, buf);
	}
}