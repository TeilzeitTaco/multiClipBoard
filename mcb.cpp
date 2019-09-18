// MultiClipBoard software, 2019, TeilzeitTaco
// Licensed under the MIT license
// Compile with: "cl /EHsc mcb.cpp User32.lib"

#define TEST_VERSION
#define ERROR_VAL static_cast<unsigned int>(-1)

// You may want to tweak this according to your needs.
// If it is higher, it will take longer until a number hotkey
// is triggered.
#define FALLTROUGH_DELAY 25

// Note: Maybe we want to switch to pure win32 prints.
#ifdef TEST_VERSION
#include <iostream>
#endif

#include "Windows.h"


char* clipboardSlots[10] = {0}; // This array holds our clipboard strings
bool usedSlots[10] = {false};   // This array is used to tell which slots already have content


// Does what it says. Duh.
char* readClipboard(void) {
    while(!OpenClipboard(NULL)) {
        Sleep(50);
    }

    HANDLE clipboardHandle = 0;
    clipboardHandle = GetClipboardData(CF_TEXT);
    if (clipboardHandle == NULL) { return NULL; }

    char* clipboardText = NULL;
    clipboardText = static_cast<char*>(GlobalLock(clipboardHandle));
	GlobalUnlock(clipboardHandle);
	CloseClipboard();

    char* returnedBuffer = NULL;
    returnedBuffer = static_cast<char*>(malloc(strlen(clipboardText)+1));
    strcpy(returnedBuffer, clipboardText);

    #ifdef TEST_VERSION
    std::cout << "[CLIPBOARD]: Reading string from clipboard: " << returnedBuffer << std::endl;
    #endif

    return returnedBuffer;
}

// Does what it says. Duh.
unsigned int writeClipboard(char clipboardText[]) {
    size_t len = strlen(clipboardText)+1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);

    #ifdef TEST_VERSION
    std::cout << "[CLIPBOARD]: Writing string to clipboard: " << clipboardText << std::endl;
    #endif

    memcpy(GlobalLock(hMem), clipboardText, len);
    GlobalUnlock(hMem);

    while(!OpenClipboard(NULL)) {
        Sleep(50);
    }

    EmptyClipboard();
    if (!SetClipboardData(CF_TEXT, hMem)) { return -1; }
    CloseClipboard();
    return 0;
}

// Does what it says.
unsigned int hideWindow(void) {
    HWND consoleWindow = NULL;

    AllocConsole(); // Allocate console if we don't already have one.
    consoleWindow = FindWindow("ConsoleWindowClass", NULL);
    if (consoleWindow == NULL) { return -1; }

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

    // Inject keypresses
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
    if (!RegisterHotKey(NULL, hotkeyID, MOD_CONTROL, vk)) { return -1; }
    BlockInput(false);

    // If we don't sleep here the clipboard doesn't change.
    // Note: Find the optimal value for this!
    Sleep(100);
    return 0;
}


// Keyboard hook to detect replacement hotkeys
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static unsigned int index = 0;
    static BYTE stub[255] = {0};  // This just exists so we can call ToAscii.
    static bool watching = false; // This gets set to one when the user presses the spacebar

    static char* str = "cbX ";
    static char* num = "0123456789";
    static char slot = 0;

    if ((nCode >= 0) && (wParam == WM_KEYDOWN)) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT *) lParam;
        WORD conversionBuf = 0;

        ToAscii(pKeyBoard->vkCode, pKeyBoard->scanCode, stub, &conversionBuf, 0);
        char real = 0;
        real = static_cast<char>(conversionBuf);

        // The user pressed space, now we have to watch if the entered string is
        // one of our strings.
        if (((real == 0x20) || (real == 0xD)) && (!watching)) {
            #ifdef TEST_VERSION
            std::cout << "[KBDHOOK]: Space or Enter was pressed, start watching..." << std::endl;
            #endif

            watching = true;
            return 0;
        }

        if (!watching) {
            return 0;
        }

        // Check if we got the right char.
        if (index == 2) {
            if (strchr(num, real) == NULL) {
                #ifdef TEST_VERSION
                std::cout << "[KBDHOOK]: Number key mismatch. Restarting." << std::endl;
                #endif

                watching = false;
                index = 0;
                return 0;
            }

            slot = real-0x30;
        } else {
            if (str[index] != real) {
                #ifdef TEST_VERSION
                std::cout << "[KBDHOOK]: Key mismatch. Restarting." << std::endl;
                #endif

                watching = false;
                index = 0;
                return 0;
            }
        }

        if (index < strlen(str)-1) {
            #ifdef TEST_VERSION
            std::cout << "[KBDHOOK]: Sucess, incrementing." << std::endl;
            #endif

            index++;
            return 0;
        }

        #ifdef TEST_VERSION
        std::cout << "[KBDHOOK]: Sucess, shortcut phrase entered." << std::endl;
        std::cout << "[KBDHOOK]: Now pasting slot " << static_cast<unsigned int>(slot) << std::endl;
        #endif

        // Press backspace four times to
        // delete the entered shortcut
        BlockInput(true);
        for (size_t i = 0; i < 3; i++) {
            // Inject keypresses
            INPUT ip;
            ip.type = INPUT_KEYBOARD;
            ip.ki.wScan = 0;
            ip.ki.time = 0;
            ip.ki.dwExtraInfo = 0;

            // Press backspace key
            ip.ki.wVk = VK_BACK;
            ip.ki.dwFlags = 0;
            SendInput(1, &ip, sizeof(INPUT));

            Sleep(50);
        }
        BlockInput(false);

        // Nothing to paste.
        if (!usedSlots[slot]) { return 1; }

        // Save current clipboard to restore it later
        char* clipboardRestoreBuf = 0;
        clipboardRestoreBuf = readClipboard();

        // Write slot contents to clipboard and paste it.
        writeClipboard(clipboardSlots[slot]);
        if (pressOriginalKey(3, 0x56) == ERROR_VAL) { return 1; }

        // Restore the old clipboard
        writeClipboard(clipboardRestoreBuf);
        free(clipboardRestoreBuf);

        watching = true;
        index = 0;
        return 0;
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

unsigned int main(void) {
    #ifndef TEST_VERSION
    if (hideWindow() == ERROR_VAL) { return 1; }
    #endif

    // Register CTRL+C/X/V hotkeys..
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL, 0x43)) { return 1; }
    if (!RegisterHotKey(NULL, 2, MOD_CONTROL, 0x58)) { return 1; }
    if (!RegisterHotKey(NULL, 3, MOD_CONTROL, 0x56)) { return 1; }

    #ifdef TEST_VERSION
    std::cout << "[MAIN]: C/X/V Hotkeys created." << std::endl;
    #endif

    // Create number hotkeys.
    for (size_t i = 0x30; i < 0x40; i++) {
        if (!RegisterHotKey(NULL, i, MOD_CONTROL, i)) { return 1; }
    }

    #ifdef TEST_VERSION
    std::cout << "[MAIN]: Number hotkeys created." << std::endl;
    #endif

    // Install keyboard hook
    HINSTANCE hExe = GetModuleHandle(NULL);
    if (!hExe) { return 1; }

    HHOOK hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC) LowLevelKeyboardProc, hExe, 0);
    if (!hKeyHook) { return 1; }

    #ifdef TEST_VERSION
    std::cout << "[MAIN]: Keyboard hook installed." << std::endl;
    #endif

    MSG msg = {0};
    char* newSlotBuf = 0;
    char* clipboardRestoreBuf = 0;
    unsigned char numberHotkeyFallthrough = 0;
    unsigned int countingWithlParam = 0;
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        if (msg.message == WM_HOTKEY) {
            #ifdef TEST_VERSION
            std::cout << "[HOTKEY]: Processing lParam: " << msg.lParam << std::endl;
            #endif

            unsigned int number = getNumber();
            switch(msg.lParam) {
                ////////////////////////////////////////////////////////////////
                case 3211266:
                case 3276802:
                case 3342338:
                case 3407874:
                case 3473410:
                case 3538946:
                case 3604482:
                case 3670018:
                case 3735554:
                case 3145730:

                if ((msg.lParam != countingWithlParam) || (countingWithlParam == 0)) {
                    #ifdef TEST_VERSION
                    std::cout << "[HOTKEY]: Setting new countingWithlParam." << std::endl;
                    #endif

                    numberHotkeyFallthrough = 0;
                    countingWithlParam = msg.lParam;
                }

                if (numberHotkeyFallthrough < FALLTROUGH_DELAY) {
                    #ifdef TEST_VERSION
                    std::cout << "[HOTKEY]: Increasing fallthrough counter to " << static_cast<int>(numberHotkeyFallthrough)+1 << std::endl;
                    #endif

                    numberHotkeyFallthrough++;
                    break;
                }

                #ifdef TEST_VERSION
                std::cout << "[HOTKEY]: Number hotkey fallthrough triggered!" << std::endl;
                std::cout << "[HOTKEY]: Hotkey ID: " << (msg.lParam >> 16) << std::endl;
                #endif

                // Here we detect if the user holds down a number hotkey.
                // If he does for long enough, we simulate a real hotkey press.
                // This is pretty much only so we can copy text from browsers
                // while still allowing the use of normal hotkeys.

                numberHotkeyFallthrough = 0;
                pressOriginalKey((msg.lParam >> 16), (msg.lParam >> 16));
                break;

                ////////////////////////////////////////////////////////////////
                case 5767170: // CTRL+V lParam int
                case 4390914: // CTRL+C lParam int
                #ifdef TEST_VERSION
                if (msg.lParam == 4390914) {
                    std::cout << "[HOTKEY]: Ctrl+C and " << number << std::endl;
                } else {
                    std::cout << "[HOTKEY]: Ctrl+X and " << number << std::endl;
                }
                #endif

                // No number key pressed, just simulate a normal keypress.
                numberHotkeyFallthrough = 0;
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
                    #ifdef TEST_VERSION
                    std::cout << "[HOTKEY]: Freeing slot..." << std::endl;
                    #endif

                    free(clipboardSlots[number]);
                } else {
                    usedSlots[number] = true;
                }
                clipboardSlots[number] = static_cast<char*>(malloc(strlen(newSlotBuf)+1));
                strcpy(clipboardSlots[number], newSlotBuf);

                #ifdef TEST_VERSION
                std::cout << "[HOTKEY]: Slot now contains: " << clipboardSlots[number] << std::endl;
                #endif

                // Restore the old clipboard
                writeClipboard(clipboardRestoreBuf);
                free(clipboardRestoreBuf);
                break;

                ////////////////////////////////////////////////////////////////
                case 5636098: // CTRL+V lParam int
                #ifdef TEST_VERSION
                std::cout << "[HOTKEY]: Ctrl+V and " << number << std::endl;
                #endif

                numberHotkeyFallthrough = 0;
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
                free(clipboardRestoreBuf);
                break;
            }
        }
    }
}
