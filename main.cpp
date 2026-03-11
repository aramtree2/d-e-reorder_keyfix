#include <iostream>
#include <chrono>
#include "interception.h"

using namespace std;
using namespace std::chrono;

const int KEY_E = 18;
const int KEY_D = 32;

int main()
{
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

    context = interception_create_context();

    interception_set_filter(
        context,
        interception_is_keyboard,
        INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP
    );

    bool waitingE = false;
    auto eTime = high_resolution_clock::now();

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {
        InterceptionKeyStroke &key = *(InterceptionKeyStroke *)&stroke;
        bool isDown = !(key.state & INTERCEPTION_KEY_UP);

        // E down
        if (key.code == KEY_E && isDown)
        {
            waitingE = true;
            eTime = high_resolution_clock::now();
            continue;
        }

        // D down while waiting
        if (waitingE && key.code == KEY_D && isDown)
        {
            InterceptionKeyStroke d = key;
            interception_send(context, device, (InterceptionStroke *)&d, 1);

            InterceptionKeyStroke e{};
            e.code = KEY_E;
            e.state = INTERCEPTION_KEY_DOWN;
            interception_send(context, device, (InterceptionStroke *)&e, 1);

            waitingE = false;
            continue;
        }

        // timeout
        if (waitingE)
        {
            auto now = high_resolution_clock::now();
            auto diff = duration_cast<milliseconds>(now - eTime).count();

            if (diff > 6)
            {
                InterceptionKeyStroke e{};
                e.code = KEY_E;
                e.state = INTERCEPTION_KEY_DOWN;
                interception_send(context, device, (InterceptionStroke *)&e, 1);
                waitingE = false;
            }
        }

        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);
}
