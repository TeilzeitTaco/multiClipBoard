// MultiClipBoard software, 2019, TeilzeitTaco
// Licensed under the MIT license

#include <iostream>
#include "Windows.h"

#define TEST_VERSION

// Hide the console window
void hideWindow() {
    HWND consoleWindow = NULL;

    AllocConsole();
    consoleWindow = FindWindow("ConsoleWindowClass", NULL);
    ShowWindow(consoleWindow, SW_HIDE);

    return;
}

// Check if any number keys are pressed right now.
unsigned int getNumberKey(void) {
    // Number keys are 0x30 to 0x39...
    for (size_t i = 0x30; i < 0x40; i++) {
        if (GetAsyncKeyState(i) != 0) {
            return (i-0x30);
        }
    }

    // ...numpad keys are 0x60 to 0x69.
    for (size_t i = 0x60; i < 0x70; i++) {
        if (GetAsyncKeyState(i) != 0) {
            return (i-0x60);
        }
    }

    // Nothing, use default.
    return -1;
}

int main(void) {
    unsigned int result = 0;
    unsigned int slot = 0;
    INPUT ip[1];

    #ifndef TEST_VERSION
    hideWindow();
    #endif

    // Register CTRL+C hotkey.
    result = RegisterHotKey(NULL, 1, MOD_CONTROL, 0x43);
    if (result == 0) {
        return 1;
    }

    // Register CTRL+X hotkey.
    result = RegisterHotKey(NULL, 2, MOD_CONTROL, 0x58);
    if (result == 0) {
        return 1;
    }

    // Register CTRL+V hotkey.
    result = RegisterHotKey(NULL, 3, MOD_CONTROL, 0x56);
    if (result == 0) {
        return 1;
    }

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        if (msg.message == WM_HOTKEY) {
            switch(msg.lParam) {
                ////////////////////////////////////////////////////////////////
                case 4390914: // CTRL+C lParam int
                #ifdef TEST_VERSION
                std::cout << "CTRL+C triggered" << std::endl;
                #endif

                // PROCESS:
                // 1. Check if any number keys are pressed
                // 2. Safe current default keyboard content
                // 3. Temporarly unregister hotkeys
                // 4. Send simulated CTRL+C keypress
                // 5. Re-register hotkeys
                // 6. Copy new clipboard to number buffer
                // 7. Restore old clipboard

                // Step 1:
                slot = getNumberKey();
                #ifdef TEST_VERSION
                std::cout << "Slot selected: " << slot << std::endl;
                #endif

                // Step 3:
                result = UnregisterHotKey(NULL, 1);
                if (result == 0) {
                    return 1;
                }

                // Step 4:
                ip[0].type = INPUT_KEYBOARD;
                ip[0].ki.wVk = VK_CONTROL;
                ip[0].ki.dwFlags = 0;
                ip[1].type = INPUT_KEYBOARD;
                ip[1].ki.wVk = 0x43;
                ip[1].ki.dwFlags = 0;
                result = SendInput(2, ip, sizeof(ip));
                if (result != 2) {
                    return 2;
                }

                ip[0].ki.dwFlags = KEYEVENTF_KEYUP;
                ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
                result = SendInput(2, ip, sizeof(ip));
                if (result != 2) {
                    return 2;
                }

                // Step 5:
                result = RegisterHotKey(NULL, 1, MOD_CONTROL, 0x43);
                if (result == 0) {
                    return 1;
                }

                #ifdef TEST_VERSION
                std::cout << "Done processing keypress." << std::endl << std::endl;
                #endif
                break;

                ////////////////////////////////////////////////////////////////
                case 5767170: // CTRL+X lParam int
                #ifdef TEST_VERSION
                std::cout << "CTRL+X triggered" << std::endl;
                #endif

                // PROCESS:
                // 1. Check if any number keys are pressed
                // 2. Safe current default keyboard content
                // 3. Temporarly unregister hotkeys
                // 4. Send simulated CTRL+X keypress
                // 5. Re-register hotkeys
                // 6. Copy new clipboard to number buffer
                // 7. Restore old clipboard

                // Step 1:
                slot = getNumberKey();
                #ifdef TEST_VERSION
                std::cout << "Slot selected: " << slot << std::endl;
                #endif

                // Step 3:
                result = UnregisterHotKey(NULL, 2);
                if (result == 0) {
                    return 1;
                }

                // Step 4:
                ip[0].type = INPUT_KEYBOARD;
                ip[0].ki.wVk = VK_CONTROL;
                ip[0].ki.dwFlags = 0;
                ip[1].type = INPUT_KEYBOARD;
                ip[1].ki.wVk = 0x58;
                ip[1].ki.dwFlags = 0;
                result = SendInput(2, ip, sizeof(ip));
                if (result != 2) {
                    return 2;
                }

                ip[0].ki.dwFlags = KEYEVENTF_KEYUP;
                ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
                result = SendInput(2, ip, sizeof(ip));
                if (result != 2) {
                    return 2;
                }

                // Step 5:
                result = RegisterHotKey(NULL, 2, MOD_CONTROL, 0x58);
                if (result == 0) {
                    return 1;
                }

                #ifdef TEST_VERSION
                std::cout << "Done processing keypress." << std::endl << std::endl;
                #endif
                break;

                ////////////////////////////////////////////////////////////////
                case 5636098: // CTRL+V lParam int
                #ifdef TEST_VERSION
                std::cout << "CTRL+V triggered" << std::endl;
                #endif

                // PROCESS:
                // 1. Check if any number keys are pressed
                // 2. Safe current default keyboard content
                // 3. Set keyboard to number buffer
                // 4. Temporarly unregister hotkeys
                // 5. Send simulated CTRL+V keypress
                // 6. Re-register hotkeys
                // 7. Restore old clipboard

                // Step 1:
                slot = getNumberKey();
                #ifdef TEST_VERSION
                std::cout << "Slot selected: " << slot << std::endl;
                #endif

                // Step 3:
                result = UnregisterHotKey(NULL, 3);
                if (result == 0) {
                    return 1;
                }

                // Step 4:
                ip[0].type = INPUT_KEYBOARD;
                ip[0].ki.wVk = VK_CONTROL;
                ip[0].ki.dwFlags = 0;
                ip[1].type = INPUT_KEYBOARD;
                ip[1].ki.wVk = 0x56;
                ip[1].ki.dwFlags = 0;
                result = SendInput(2, ip, sizeof(ip));
                if (result != 2) {
                    return 2;
                }

                ip[0].ki.dwFlags = KEYEVENTF_KEYUP;
                ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
                result = SendInput(2, ip, sizeof(ip));
                if (result != 2) {
                    return 2;
                }

                // Step 5:
                result = RegisterHotKey(NULL, 3, MOD_CONTROL, 0x56);
                if (result == 0) {
                    return 1;
                }

                #ifdef TEST_VERSION
                std::cout << "Done processing keypress." << std::endl << std::endl;
                #endif
                break;
            }
        }
    }
}
