#ifndef INTERCEPTION_H
#define INTERCEPTION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* InterceptionContext;
typedef int InterceptionDevice;

typedef struct
{
    unsigned short code;
    unsigned short state;
    unsigned int information;
} InterceptionKeyStroke;

typedef struct
{
    unsigned short state;
    short rolling;
    int x;
    int y;
    unsigned int information;
} InterceptionMouseStroke;

typedef union
{
    InterceptionKeyStroke key;
    InterceptionMouseStroke mouse;
} InterceptionStroke;

enum
{
    INTERCEPTION_KEY_DOWN = 0x00,
    INTERCEPTION_KEY_UP = 0x01
};

enum
{
    INTERCEPTION_FILTER_KEY_NONE = 0x0000,
    INTERCEPTION_FILTER_KEY_DOWN = 0x0001,
    INTERCEPTION_FILTER_KEY_UP = 0x0002
};

InterceptionContext interception_create_context(void);
void interception_destroy_context(InterceptionContext context);
InterceptionDevice interception_wait(InterceptionContext context);
int interception_receive(InterceptionContext context, InterceptionDevice device, InterceptionStroke *stroke, unsigned int nstroke);
int interception_send(InterceptionContext context, InterceptionDevice device, const InterceptionStroke *stroke, unsigned int nstroke);

void interception_set_filter(InterceptionContext context, int (*predicate)(InterceptionDevice device), unsigned short filter);

int interception_is_keyboard(InterceptionDevice device);

#ifdef __cplusplus
}
#endif

#endif
