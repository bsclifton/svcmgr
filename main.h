// main.h
// brian clifton 2003

#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include "config.h"
#include "_R/resource.h"

#ifndef _WIN32_IE              // force new ie includes
#define _WIN32_IE 0x0600       //
#endif                         //
#define WIN32_LEAN_AND_MEAN    // cut out bloat
#define TRAYICONID	1          // ID number for the Notify Icon
#define SWM_TRAYMSG	WM_APP     // the message ID sent to our window
#define SWM_SHOW	WM_APP + 1 // show the window
#define SWM_HIDE	WM_APP + 2 // hide the window
#define SWM_EXIT	WM_APP + 3 // close the window
#define SWM_START WM_APP + 4   // start service
#define SWM_STOP  WM_APP + 5   // stop service

extern int WINAPI EventHandler(HWND a,UINT b,WPARAM c,LPARAM d);
extern BOOL InitInstance(HINSTANCE hInstance,int nCmdShow);
extern void Load(HWND hWnd);
extern void Save(HWND hWnd);
extern void ServiceStart(HWND hWnd);
extern void ServiceStop(HWND hWnd);
extern void SetWindowCaption(HWND hWnd, const wchar_t* lpString,...);
extern void ShowContextMenu(HWND hWnd);
extern void UpdateStatus(HWND hWnd);
extern void ShowContextMenu(HWND hWnd);

#endif
