

#include <windows.h>
#include <string>
#include <vector>
#include <utility>

// Structure to hold Roman numeral mapping
const std::vector<std::pair<int, const char*>> ROMAN_NUMERALS = {
    {1000, "M"},
    {900, "CM"},
    {500, "D"},
    {400, "CD"},
    {100, "C"},
    {90, "XC"},
    {50, "L"},
    {40, "XL"},
    {10, "X"},
    {9, "IX"},
    {5, "V"},
    {4, "IV"},
    {1, "I"}
};

// Global variables
std::string g_input;
bool g_keyStates[256] = { false };  // Track key states
bool g_isProcessingPopup = false; // Flag to prevent multiple popups

// Function to convert integer to Roman numeral
std::string intToRoman(int num) {
    if (num <= 0 || num > 3999) {
        return "Invalid number (must be between 1 and 3999)";
    }

    std::string result;
    for (const auto& pair : ROMAN_NUMERALS) {
        while (num >= pair.first) {
            result += pair.second;
            num -= pair.first;
        }
    }
    return result;
}

// Function to show popup message
void showPopup(const std::string& message) {
    MessageBoxA(NULL, message.c_str(), "Roman Numeral Conversion",
        MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
}

// Keyboard hook procedure
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && !g_isProcessingPopup) {
        KBDLLHOOKSTRUCT* pKbhs = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN) {
            // Handle number keys (0-9)
            if (pKbhs->vkCode >= '0' && pKbhs->vkCode <= '9' && !g_keyStates[pKbhs->vkCode]) {
                g_keyStates[pKbhs->vkCode] = true;
                g_input += static_cast<char>(pKbhs->vkCode);
            }
            // Handle Enter key
            else if (pKbhs->vkCode == VK_RETURN && !g_keyStates[VK_RETURN] && !g_input.empty()) {
                g_keyStates[VK_RETURN] = true;
                g_isProcessingPopup = true;  // Set flag to prevent multiple popups

                try {
                    int number = std::stoi(g_input);
                    std::string roman = intToRoman(number);
                    std::string message = "Number: " + g_input + "\nRoman Numeral: " + roman;
                    showPopup(message);
                }
                catch (...) {
                    showPopup("Invalid input! Please enter a valid number.");
                }

                g_input.clear();
                g_isProcessingPopup = false;  // Reset flag after showing popup
            }
        }
        else if (wParam == WM_KEYUP) {
            g_keyStates[pKbhs->vkCode] = false;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_USER + 1:  // Tray icon message
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, L"Exit");
            SetForegroundWindow(hwnd);
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
                pt.x, pt.y, 0, hwnd, NULL);
            if (cmd == 1) {
                DestroyWindow(hwnd);
            }
            DestroyMenu(hMenu);
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"RomanNumeralConverter";

    // Register window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassEx(&wc);

    // Create hidden window
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"RomanNumeralConverter",
        WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        MessageBoxA(NULL, "Window Creation Failed!", "Error", MB_ICONERROR);
        return 0;
    }

    // Set up system tray icon
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"Roman Numeral Converter (Right-click to exit)");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Install keyboard hook
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!hHook) {
        MessageBoxA(NULL, "Failed to install keyboard hook!", "Error", MB_ICONERROR);
        return 0;
    }

    // Show startup message
    showPopup("Roman Numeral Converter is running!\nType a number and press Enter to convert.\nRight-click the tray icon to exit.");

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    UnhookWindowsHookEx(hHook);
    Shell_NotifyIcon(NIM_DELETE, &nid);
    return static_cast<int>(msg.wParam);
}