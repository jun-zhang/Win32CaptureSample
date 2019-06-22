#pragma once
#include <dwmapi.h>

struct Window
{
public:
    Window(nullptr_t) {}
    Window(HWND hwnd, std::wstring& title, std::wstring& className)
    {
        m_hwnd = hwnd;
        m_title = title;
        m_className = className;
    }

    HWND Hwnd() const noexcept { return m_hwnd; }
    std::wstring Title() const noexcept { return m_title; }
    std::wstring ClassName() const noexcept { return m_className; }

private:
    HWND m_hwnd;
    std::wstring m_title;
    std::wstring m_className;
};

std::wstring GetClassName(HWND hwnd)
{
    const DWORD TITLE_SIZE = 1024;
    WCHAR windowTitle[TITLE_SIZE];

    ::GetClassName(hwnd, windowTitle, TITLE_SIZE);

    std::wstring title(&windowTitle[0]);
    return title;
}

std::wstring GetWindowText(HWND hwnd)
{
    const DWORD TITLE_SIZE = 1024;
    WCHAR windowTitle[TITLE_SIZE];

    ::GetWindowText(hwnd, windowTitle, TITLE_SIZE);

    std::wstring title(&windowTitle[0]);
    return title;
}

bool IsCapturableWindow(Window const& window)
{
    HWND hwnd = window.Hwnd();
    HWND shellWindow = GetShellWindow();

    auto title = window.Title();
    auto className = window.ClassName();

    if (hwnd == shellWindow)
    {
        return false;
    }

    if (title.length() == 0)
    {
        return false;
    }

    if (!IsWindowVisible(hwnd))
    {
        return false;
    }

    if (GetAncestor(hwnd, GA_ROOT) != hwnd)
    {
        return false;
    }

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (!((style & WS_DISABLED) != WS_DISABLED))
    {
        return false;
    }

    DWORD cloaked = FALSE;
    HRESULT hrTemp = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (SUCCEEDED(hrTemp) &&
        cloaked == DWM_CLOAKED_SHELL)
    {
        return false;
    }

    return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto class_name = GetClassName(hwnd);
    auto title = GetWindowText(hwnd);

    auto window = Window(hwnd, title, class_name);

    if (!IsCapturableWindow(window))
    {
        return TRUE;
    }

    std::vector<Window>& windows = *reinterpret_cast<std::vector<Window>*>(lParam);
    windows.push_back(window);

    return TRUE;
}

const std::vector<Window> EnumerateWindows()
{
    std::vector<Window> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

    return windows;
}