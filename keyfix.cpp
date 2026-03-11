#include <windows.h>
#include <atomic>

HHOOK hook;

std::atomic<bool> waitingE(false);

LARGE_INTEGER eTime;
LARGE_INTEGER freq;

void sendKey(WORD vk, bool down)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;

    if (!down)
        input.ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &input, sizeof(INPUT));
}

double elapsedMs()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (double)(now.QuadPart - eTime.QuadPart) * 1000.0 / freq.QuadPart;
}

LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

        if (kb->flags & LLKHF_INJECTED)
            return CallNextHookEx(hook, nCode, wParam, lParam);

        bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool keyUp   = (wParam == WM_KEYUP   || wParam == WM_SYSKEYUP);

        if (kb->vkCode == 'E')
        {
            if (keyDown)
            {
                if (waitingE.exchange(true))
                {
                    // repeat fail-open
                    sendKey('E', true);
                    waitingE = false;
                    return CallNextHookEx(hook, nCode, wParam, lParam);
                }

                QueryPerformanceCounter(&eTime);
                return 1;
            }

            if (keyUp)
            {
                if (waitingE.exchange(false))
                {
                    // quick tap fail-open
                    sendKey('E', true);
                }

                sendKey('E', false);
                return 1;
            }
        }

        if (kb->vkCode == 'D')
        {
            if (keyDown && waitingE.exchange(false))
            {
                sendKey('D', true);
                sendKey('E', true);
                return 1;
            }

            if (keyUp && waitingE.exchange(false))
            {
                // fail-open
                sendKey('E', true);
                return CallNextHookEx(hook, nCode, wParam, lParam);
            }
        }
    }

    return CallNextHookEx(hook, nCode, wParam, lParam);
}

DWORD WINAPI timerThread(LPVOID)
{
    while (true)
    {
        if (waitingE && elapsedMs() > 6.0)
        {
            if (waitingE.exchange(false))
            {
                sendKey('E', true);
            }
        }

        Sleep(1);
    }
}

int main()
{
    QueryPerformanceFrequency(&freq);

    hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, NULL, 0);

    CreateThread(NULL, 0, timerThread, NULL, 0, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }

    if (waitingE)
        sendKey('E', true);

    UnhookWindowsHookEx(hook);

    return 0;
}
