#include "platform/android/touch.h"

#include <math.h>
#include <string.h>

/* Logical control layout. Coordinates are expressed in the game's 426x240
 * logical coordinate space so the controls scale cleanly on every phone. */
#define TOUCH_W 426.0f
#define TOUCH_H 240.0f

#define DPAD_CX 54.0f
#define DPAD_CY 184.0f
#define DPAD_R  43.0f
#define BUTTON_CX 366.0f
#define BUTTON_CY 181.0f
#define BUTTON_R  27.0f
#define BUTTON2_CX 320.0f
#define BUTTON2_CY 202.0f
#define BUTTON2_R  23.0f
#define SHOULDER_W 40.0f
#define SHOULDER_H 28.0f

static u16 touchKeys;
static SDL_FingerID dpadFinger = -1;
static SDL_FingerID aFinger = -1;
static SDL_FingerID bFinger = -1;
static SDL_FingerID startFinger = -1;
static SDL_FingerID selectFinger = -1;
static SDL_FingerID lFinger = -1;
static SDL_FingerID rFinger = -1;

static void SetKey(u16 key, bool down)
{
    if (down)
        touchKeys |= key;
    else
        touchKeys &= ~key;
}

static void PointFromFinger(const SDL_TouchFingerEvent *finger, float *x, float *y)
{
    *x = finger->x * TOUCH_W;
    *y = finger->y * TOUCH_H;
}

static bool InCircle(float x, float y, float cx, float cy, float radius)
{
    float dx = x - cx;
    float dy = y - cy;
    return dx * dx + dy * dy <= radius * radius;
}

static bool InRect(float x, float y, float cx, float cy, float halfW, float halfH)
{
    return x >= cx - halfW && x <= cx + halfW && y >= cy - halfH && y <= cy + halfH;
}

static u16 DpadForPoint(float x, float y)
{
    float dx = x - DPAD_CX;
    float dy = y - DPAD_CY;
    float dead = 10.0f;
    u16 result = 0;

    if (fabsf(dx) > dead || fabsf(dy) > dead) {
        if (fabsf(dx) >= fabsf(dy))
            result |= dx < 0 ? DPAD_LEFT : DPAD_RIGHT;
        else
            result |= dy < 0 ? DPAD_UP : DPAD_DOWN;
    }

    return result;
}

static void ReleaseFinger(SDL_FingerID id)
{
    if (id == dpadFinger) {
        dpadFinger = -1;
        touchKeys &= ~(DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT);
    }
    if (id == aFinger) {
        aFinger = -1;
        SetKey(A_BUTTON, false);
    }
    if (id == bFinger) {
        bFinger = -1;
        SetKey(B_BUTTON, false);
    }
    if (id == startFinger) {
        startFinger = -1;
        SetKey(START_BUTTON, false);
    }
    if (id == selectFinger) {
        selectFinger = -1;
        SetKey(SELECT_BUTTON, false);
    }
    if (id == lFinger) {
        lFinger = -1;
        SetKey(L_BUTTON, false);
    }
    if (id == rFinger) {
        rFinger = -1;
        SetKey(R_BUTTON, false);
    }
}

void AndroidTouch_Init(void)
{
    touchKeys = 0;
    dpadFinger = aFinger = bFinger = startFinger = selectFinger = lFinger = rFinger = -1;
}

void AndroidTouch_HandleEvent(const SDL_Event *event)
{
    if (event->type == SDL_FINGERUP || event->type == SDL_FINGERCANCEL) {
        ReleaseFinger(event->tfinger.fingerId);
        return;
    }

    if (event->type != SDL_FINGERDOWN && event->type != SDL_FINGERMOTION)
        return;

    float x, y;
    PointFromFinger(&event->tfinger, &x, &y);
    SDL_FingerID id = event->tfinger.fingerId;

    if (id == dpadFinger) {
        touchKeys &= ~(DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT);
        touchKeys |= DpadForPoint(x, y);
        return;
    }
    if (id == aFinger) {
        SetKey(A_BUTTON, InCircle(x, y, BUTTON_CX, BUTTON_CY, BUTTON_R));
        return;
    }
    if (id == bFinger) {
        SetKey(B_BUTTON, InCircle(x, y, BUTTON2_CX, BUTTON2_CY, BUTTON2_R));
        return;
    }
    if (id == startFinger || id == selectFinger || id == lFinger || id == rFinger)
        return;

    if (event->type != SDL_FINGERDOWN)
        return;

    if (InCircle(x, y, DPAD_CX, DPAD_CY, DPAD_R)) {
        dpadFinger = id;
        touchKeys |= DpadForPoint(x, y);
    } else if (InCircle(x, y, BUTTON_CX, BUTTON_CY, BUTTON_R)) {
        aFinger = id;
        SetKey(A_BUTTON, true);
    } else if (InCircle(x, y, BUTTON2_CX, BUTTON2_CY, BUTTON2_R)) {
        bFinger = id;
        SetKey(B_BUTTON, true);
    } else if (InRect(x, y, 213.0f, 225.0f, 28.0f, 10.0f)) {
        startFinger = id;
        SetKey(START_BUTTON, true);
    } else if (InRect(x, y, 151.0f, 225.0f, 28.0f, 10.0f)) {
        selectFinger = id;
        SetKey(SELECT_BUTTON, true);
    } else if (InRect(x, y, 96.0f, 28.0f, SHOULDER_W / 2.0f, SHOULDER_H / 2.0f)) {
        lFinger = id;
        SetKey(L_BUTTON, true);
    } else if (InRect(x, y, 330.0f, 28.0f, SHOULDER_W / 2.0f, SHOULDER_H / 2.0f)) {
        rFinger = id;
        SetKey(R_BUTTON, true);
    }
}

u16 AndroidTouch_GetKeys(void)
{
    return touchKeys;
}
