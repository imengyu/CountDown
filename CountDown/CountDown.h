#pragma once

#include "resource.h"


INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EnterSecDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void LoadCfg();
void SaveConfig(int icon);
void Start(HWND hDlg);
void StartTimer(HWND hDlg);
void Stop(HWND hDlg);
void Cancel(HWND hDlg);
void RunTick(HWND hDlg);
void RunCmd();

void InvalidWindow(HWND hDlg);

bool AdjustToken();

bool StrEqual(const wchar_t * a, const wchar_t * b);
void GetSecTimeStr(int second, LPWSTR timeStr, size_t max);
