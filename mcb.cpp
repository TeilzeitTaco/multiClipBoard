// MultiClipBoard software, 2019, TeilzeitTaco
// Licensed under the MIT license

#define TEST_VERSION
#define ERROR_VAL (unsigned int) -1

// Note: Maybe we want to switch to pure win32 prints.
#ifdef TEST_VERSION
#include <iostream>
#endif

#include "Windows.h"

// Does what it says. Duh.
char* readClipboard(void) {
    while(!OpenClipboard(NULL)) {
        Sleep(100);
    }

    HANDLE clipboardHandle = 0;
    clipboardHandle = GetClipboardData(CF_TEXT);
    if (clipboardHandle == NULL) {
        return NULL;
    }

    char* clipboardText = NULL;
    clipboardText = static_cast<char*>(GlobalLock(clipboardHandle));
	GlobalUnlock(clipboardHandle);
	CloseClipboard();

    #ifdef TEST_VERSION
    std::cout << "Reading string from clipboard: " << clipboardText << std::endl;
    #endif

    return clipboardText;
}

// Does what it says. Duh.
unsigned int writeClipboard(char clipboardText[]) {
    size_t len = strlen(clipboardText)+1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);

    #ifdef TEST_VERSION
    std::cout << "Writing string to clipboard: " << clipboardText << std::endl;
    #endif

    memcpy(GlobalLock(hMem), clipboardText, len);
    GlobalUnlock(hMem);

    while(!OpenClipboard(NULL)) {
        Sleep(100);
    }

    EmptyClipboard();
    if (!SetClipboardData(CF_TEXT, hMem)) { return -1; }
    CloseClipboard();
    return 0;
}

// Does what it says.
unsigned int hideWindow(void) {
    HWND consoleWindow = NULL;

    AllocConsole();
    consoleWindow = FindWindow("ConsoleWindowClass", NULL);
    if (consoleWindow == NULL) {
        return -1;
    }

    ShowWindow(consoleWindow, SW_HIDE);
    return 0;
}

// Check if any number keys are pressed.
unsigned int getNumber(void) {
    // 0x30 to 0x39 are normal number keys.
    for (unsigned int i = 0x30; i < 0x40; i++) {
        if (GetAsyncKeyState(i) != 0) {
            return (i-0x30);
        }
    }

    // 0x60 to 0x69 are numpad keys.
    for (unsigned int i = 0x60; i < 0x70; i++) {
        if (GetAsyncKeyState(i) != 0) {
            return (i-0x60);
        }
    }

    return -1;
}

// This function is called when we want to use the vanilla CTRl+C/X/V.
// It disables the associated hotkey, simulates a keypress and re-enables
// the hotkey. Pretty neat if you ask me!
unsigned int pressOriginalKey(int hotkeyID, WORD vk) {
    unsigned int result = 0;

    // Temporarly disable hotkey and block user input.
    if (!UnregisterHotKey(NULL, hotkeyID)) { return -1; }
    BlockInput(true);

    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    // Press the "Ctrl" key.
    ip.ki.wVk = VK_CONTROL;
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));

    // Press the specified key.
    ip.ki.wVk = vk;
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));

    // Release the specified key.
    ip.ki.wVk = vk;
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));

    // Release the "Ctrl" key.
    ip.ki.wVk = VK_CONTROL;
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));

    // Re-enable hotkey and input.
    if (!RegisterHotKey(NULL, hotkeyID, MOD_CONTROL | MOD_NOREPEAT, vk)) { return -1; }
    BlockInput(false);

    // If we don't sleep here the clipboard doesn't change.
    // Note: Find the optimal value for this!
    Sleep(75);
    return 0;
}

unsigned int main(void) {
    char* clipboardSlots[9] = {0};
    bool usedSlots[9] = {false}; // This array is used to tell which slots already have content

    #ifndef TEST_VERSION
    if (hideWindow() == ERROR_VAL) { return 1; }
    #endif

    // Register CTRL+C/X/V hotkeys..
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_NOREPEAT, 0x43)) { return 1; }
    if (!RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_NOREPEAT, 0x58)) { return 1; }
    if (!RegisterHotKey(NULL, 3, MOD_CONTROL | MOD_NOREPEAT, 0x56)) { return 1; }

    #ifdef TEST_VERSION
    std::cout << "C/X/V Hotkeys created." << std::endl;
    #endif

    // Create number hotkeys.
    for (size_t i = 0x30; i < 0x40; i++) {
        if (!RegisterHotKey(NULL, i, MOD_CONTROL | MOD_NOREPEAT, i)) { return 1; }
    }

    #ifdef TEST_VERSION
    std::cout << "Number hotkeys created." << std::endl;
    #endif

    MSG msg = {0};
    char* newSlotBuf = 0;
    char* clipboardRestoreBuf = 0;
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        if (msg.message == WM_HOTKEY) {
            #ifdef TEST_VERSION
            std::cout << "Processing lParam: " << msg.lParam << std::endl;
            #endif

            unsigned int number = getNumber();
            switch(msg.lParam) {
                ////////////////////////////////////////////////////////////////
                case 5767170:
                case 4390914: // CTRL+C lParam int
                #ifdef TEST_VERSION
                if (msg.lParam == 4390914) {
                    std::cout << "Ctrl+C and " << number << std::endl;
                } else {
                    std::cout << "Ctrl+X and " << number << std::endl;
                }
                #endif

                // No number key pressed, just simulate a normal keypress.
                if (number == ERROR_VAL) {
                    if (msg.lParam == 4390914) {
                        if (pressOriginalKey(1, 0x43) == ERROR_VAL) { return 1; } // Ctrl+C
                    } else {
                        if (pressOriginalKey(2, 0x58) == ERROR_VAL) { return 1; } // Ctrl+X
                    }
                    break;
                }

                // Save current clipboard to restore it later
                clipboardRestoreBuf = readClipboard();

                // Simulate keypress
                if (msg.lParam == 4390914) {
                    if (pressOriginalKey(1, 0x43) == ERROR_VAL) { return 1; } // Ctrl+C
                } else {
                    if (pressOriginalKey(2, 0x58) == ERROR_VAL) { return 1; } // Ctrl+X
                }

                // Read the result
                newSlotBuf = readClipboard();

                // Free memory if the slot was previously used
                if (usedSlots[number]) {
                    free(clipboardSlots[number]);
                } else {
                    usedSlots[number] = true;
                }
                clipboardSlots[number] = static_cast<char*>(malloc(strlen(newSlotBuf)+1));
                strcpy(clipboardSlots[number], newSlotBuf);

                #ifdef TEST_VERSION
                std::cout << "Slot now contains: " << clipboardSlots[number] << std::endl;
                #endif

                // Restore the old clipboard
                writeClipboard(clipboardRestoreBuf);
                break;

                ////////////////////////////////////////////////////////////////
                case 5636098: // CTRL+V lParam int
                #ifdef TEST_VERSION
                std::cout << "Ctrl+V and " << number << std::endl;
                #endif

                if (number == ERROR_VAL) {
                    if (pressOriginalKey(3, 0x56) == ERROR_VAL) { return 1; }
                    break;
                }

                // Nothing to paste.
                if (!usedSlots[number]) { break; }

                // Save current clipboard to restore it later
                clipboardRestoreBuf = readClipboard();

                // Write slot contents to clipboard and paste it.
                writeClipboard(clipboardSlots[number]);
                if (pressOriginalKey(3, 0x56) == ERROR_VAL) { return 1; }

                // Restore the old clipboard
                writeClipboard(clipboardRestoreBuf);
                break;
            }
        }
    }
}
