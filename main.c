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
    const char *ret = GetKeyString(HKEY_CURRENT_USER, "Software\\RenLauncher", "Nickname");
    return ret ? ret : "";
}

const char *RenegadeGetServer()
{
    const char *ret = GetKeyString(HKEY_CURRENT_USER, "Software\\RenLauncher", "Server");
    return ret ? ret : "";
}

void RenegadeSetNickname(const char *nickname)
{
    SetKeyString(HKEY_CURRENT_USER, "Software\\RenLauncher", "Nickname", nickname);
}

void RenegadeSetServer(const char *server)
{
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
                    if (RenegadeRegister())
                    {
                        ShowWindow(GetDlgItem(hwnd, REGISTER), SW_HIDE);
                        ShowWindow(GetDlgItem(hwnd, UNREGISTER), SW_SHOW);
                    }
                    break;
                case UNREGISTER:
                    if (RenegadeUnregister())
                    {
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

int start()
{
    // check that game.exe exists in current directory

    // check if command line has address & complain if no nickname set?

    // if has address, remember to save the last server!

    if (DialogBox(NULL, MAKEINTRESOURCE(IDD_RENLAUNCHER), NULL, DialogProc))
    {
        // launch
    }

    return 0;
}
