/*
 * Copyright (c) 2014 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include "res/resource.h"

void SetKeyDword(HKEY root, const char *sub, const char *name, DWORD value)
{
    HKEY hKey;

    if (RegCreateKey(root, sub, &hKey) != ERROR_SUCCESS)
        return;

    RegSetValueEx(hKey, name, 0, REG_DWORD, (const BYTE *)&value, sizeof value);
    RegCloseKey(hKey);
}

DWORD GetKeyDword(HKEY root, const char *sub, const char *name)
{
    DWORD buf;
    DWORD len = sizeof buf;
    HKEY hKey;

    if (RegOpenKeyEx(root, sub, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return 0;

    if (RegQueryValueEx(hKey, name, NULL, NULL, (BYTE *)&buf, &len) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 0;
    }

    RegCloseKey(hKey);
    return buf;
}

void SetKeyString(HKEY root, const char *sub, const char *name, const char *value)
{
    HKEY hKey;

    if (RegCreateKey(root, sub, &hKey) != ERROR_SUCCESS)
        return;

    RegSetValueEx(hKey, name, 0, REG_SZ, (const BYTE *)value, strlen(value));
    RegCloseKey(hKey);
}

const char *GetKeyString(HKEY root, const char *sub, const char *name)
{
    static char buf[512];
    DWORD len = sizeof buf;
    HKEY hKey;

    memset(buf, 0, sizeof buf);

    if (RegOpenKeyEx(root, sub, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return NULL;

    if (RegQueryValueEx(hKey, name, NULL, NULL, (BYTE *)buf, &len) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return NULL;
    }

    RegCloseKey(hKey);
    return buf;
}

BOOL RenegadeGetRegister()
{
    return GetKeyString(HKEY_CURRENT_USER, "Software\\Classes\\renegade\\shell\\open\\command", "") != NULL;
}

BOOL RenegadeRegister()
{
    char path[MAX_PATH];
    char buf[MAX_PATH + 32];
    GetModuleFileName(NULL, path, sizeof path);

    sprintf(buf, "\"%s\" \"%%1\"", path);

    SetKeyString(HKEY_CURRENT_USER, "Software\\Classes\\renegade", "URL Protocol", " ");
    SetKeyString(HKEY_CURRENT_USER, "Software\\Classes\\renegade\\shell\\open\\command", "", buf);
    return TRUE;
}

BOOL RenegadeUnregister()
{
    RegDeleteKey(HKEY_CURRENT_USER, "Software\\Classes\\renegade\\shell\\open\\command");
    RegDeleteKey(HKEY_CURRENT_USER, "Software\\Classes\\renegade\\shell\\open");
    RegDeleteKey(HKEY_CURRENT_USER, "Software\\Classes\\renegade\\shell");
    RegDeleteKey(HKEY_CURRENT_USER, "Software\\Classes\\renegade");
    return TRUE;
}

BOOL RenegadeGetMulti()
{
    return GetKeyDword(HKEY_CURRENT_USER, "Software\\RenLauncher", "Multi");
}

void RenegadeSetMulti(BOOL enabled)
{
    SetKeyDword(HKEY_CURRENT_USER, "Software\\RenLauncher", "Multi", (DWORD)enabled);
}

const char *RenegadeGetNickname()
{
    static char buf[512];
    const char *ret = GetKeyString(HKEY_CURRENT_USER, "Software\\RenLauncher", "Nickname");
    memset(buf, 0, sizeof buf);
    if (ret)
        strncpy(buf, ret, sizeof(buf) - 1);
    return buf;
}

const char *RenegadeGetServer()
{
    static char buf[512];
    const char *ret = GetKeyString(HKEY_CURRENT_USER, "Software\\RenLauncher", "Server");
    memset(buf, 0, sizeof buf);
    if (ret)
        strncpy(buf, ret, sizeof(buf) - 1);
    return buf;
}

void RenegadeSetNickname(const char *nickname)
{
    SetKeyString(HKEY_CURRENT_USER, "Software\\RenLauncher", "Nickname", nickname);
}

void RenegadeSetServer(const char *server)
{
    if (strlen(server) > 256)
        return;

    // make sure the address is safe, sort of
    for (int i = 0; i < strlen(server); i++)
        if (isblank(server[i]) || iscntrl(server[i]) || isspace(server[i]))
            return;

    SetKeyString(HKEY_CURRENT_USER, "Software\\RenLauncher", "Server", server);
}

void RenegadeSave(HWND hwnd)
{
    char buf[512];

    GetWindowText(GetDlgItem(hwnd, NICKNAME), buf, sizeof buf);
    RenegadeSetNickname(buf);

    GetWindowText(GetDlgItem(hwnd, SERVER), buf, sizeof buf);
    RenegadeSetServer(buf);

    RenegadeSetMulti(SendMessage(GetDlgItem(hwnd, MULTI), BM_GETCHECK, 0, 0) == BST_CHECKED);
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            {
                SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), "small"));
                SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), "large"));
                /*
                SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, IDOK), TRUE);
                SetFocus(GetDlgItem(hwnd, IDOK));
                */

                SetWindowText(GetDlgItem(hwnd, NICKNAME), RenegadeGetNickname());
                SetWindowText(GetDlgItem(hwnd, SERVER), RenegadeGetServer());

                if (RenegadeGetRegister())  {
                    ShowWindow(GetDlgItem(hwnd, REGISTER), SW_HIDE);
                    ShowWindow(GetDlgItem(hwnd, UNREGISTER), SW_SHOW);
                }

                if (RenegadeGetMulti())
                    PostMessage(GetDlgItem(hwnd, MULTI), BM_SETCHECK, BST_CHECKED, 0);

                return TRUE;
            }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    RenegadeSave(hwnd);
                    EndDialog(hwnd, 1);
                    break;
                case IDCANCEL:
                    RenegadeSave(hwnd);
                    EndDialog(hwnd, 0);
                    break;
                case REGISTER:
                    if (RenegadeRegister()) {
                        ShowWindow(GetDlgItem(hwnd, REGISTER), SW_HIDE);
                        ShowWindow(GetDlgItem(hwnd, UNREGISTER), SW_SHOW);
                    }
                    break;
                case UNREGISTER:
                    if (RenegadeUnregister()) {
                        ShowWindow(GetDlgItem(hwnd, UNREGISTER), SW_HIDE);
                        ShowWindow(GetDlgItem(hwnd, REGISTER), SW_SHOW);
                    }
                    break;
            }
            break;

        case WM_CLOSE:
            RenegadeSave(hwnd);
            EndDialog(hwnd, 0);
            break;
    }

    return FALSE;
}

void RenegadeLaunch(const char *path)
{
    char buf[MAX_PATH];
    const char *nickname = RenegadeGetNickname();
    const char *server = RenegadeGetServer();
    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;

    if (strlen(server) == 0 || strlen(nickname) == 0)
        return;

    sprintf(buf, "\"%s\" +connect %s +netplayername \"%s\" %s", path, server, nickname, RenegadeGetMulti() ? "+multi" : "");

    ZeroMemory(&sInfo, sizeof sInfo);
    sInfo.cb = sizeof sInfo;
    ZeroMemory(&pInfo, sizeof pInfo);

    if (CreateProcess(NULL, buf, NULL, NULL, TRUE, 0, NULL, NULL, &sInfo, &pInfo) == 0) {
        char msg[MAX_PATH * 2];
        sprintf(msg, "Failed to run %s", buf);
        MessageBox(NULL, msg, "RenLauncher - Error", MB_OK|MB_ICONERROR);
    }
}

int main(int argc, char **argv)
{
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, sizeof path);
    char *last = strrchr(path, '\\');
    if (last) *last = '\0';
    strcat(last, "\\game.exe");

    if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES) {
        MessageBox(NULL, "RenLauncher needs to be in the same directory as your Renegade game.exe, please move this executable to your game folder and try again.", "RenLauncher - Error", MB_OK|MB_ICONSTOP);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "renegade://", 11) == 0) {
            RenegadeSetServer(argv[i] + 11);

            if (strlen(RenegadeGetNickname()) > 0) {
                RenegadeLaunch(path);
                return 0;
            }
        }
    }

    if (DialogBox(NULL, MAKEINTRESOURCE(IDD_RENLAUNCHER), NULL, DialogProc))
        RenegadeLaunch(path);

    return 0;
}
