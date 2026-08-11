#include "platform/android/touch.h"

#include <math.h>

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
static SDL_FingerID dpadFinger = -1, aFinger = -1, bFinger = -1;
static SDL_FingerID startFinger = -1, selectFinger = -1, lFinger = -1, rFinger = -1;

static void SetKey(u16 key, bool down)
{
    if (down) touchKeys |= key;
    else touchKeys &= ~key;
}

static void PointFromFinger(const SDL_TouchFingerEvent *finger, float *x, float *y)
{
    *x = finger->x * TOUCH_W;
    *y = finger->y * TOUCH_H;
}

static bool InCircle(float x, float y, float cx, float cy, float radius)
{
    float dx = x - cx, dy = y - cy;
    return dx * dx + dy * dy <= radius * radius;
}

static bool InRect(float x, float y, float cx, float cy, float halfW, float halfH)
{
    return x >= cx - halfW && x <= cx + halfW && y >= cy - halfH && y <= cy + halfH;
}

static u16 DpadForPoint(float x, float y)
{
    float dx = x - DPAD_CX, dy = y - DPAD_CY;
    if (fabsf(dx) <= 10.0f && fabsf(dy) <= 10.0f) return 0;
    if (fabsf(dx) >= fabsf(dy)) return dx < 0 ? DPAD_LEFT : DPAD_RIGHT;
    return dy < 0 ? DPAD_UP : DPAD_DOWN;
}

static void ReleaseFinger(SDL_FingerID id)
{
    if (id == dpadFinger) { dpadFinger = -1; touchKeys &= ~(DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT); }
    if (id == aFinger) { aFinger = -1; SetKey(A_BUTTON, false); }
    if (id == bFinger) { bFinger = -1; SetKey(B_BUTTON, false); }
    if (id == startFinger) { startFinger = -1; SetKey(START_BUTTON, false); }
    if (id == selectFinger) { selectFinger = -1; SetKey(SELECT_BUTTON, false); }
    if (id == lFinger) { lFinger = -1; SetKey(L_BUTTON, false); }
    if (id == rFinger) { rFinger = -1; SetKey(R_BUTTON, false); }
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
    if (event->type != SDL_FINGERDOWN && event->type != SDL_FINGERMOTION) return;

    float x, y;
    PointFromFinger(&event->tfinger, &x, &y);
    SDL_FingerID id = event->tfinger.fingerId;

    if (id == dpadFinger) {
        touchKeys &= ~(DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT);
        touchKeys |= DpadForPoint(x, y);
        return;
    }
    if (id == aFinger) { SetKey(A_BUTTON, InCircle(x, y, BUTTON_CX, BUTTON_CY, BUTTON_R)); return; }
    if (id == bFinger) { SetKey(B_BUTTON, InCircle(x, y, BUTTON2_CX, BUTTON2_CY, BUTTON2_R)); return; }
    if (id == startFinger || id == selectFinger || id == lFinger || id == rFinger) return;
    if (event->type != SDL_FINGERDOWN) return;

    if (InCircle(x, y, DPAD_CX, DPAD_CY, DPAD_R)) {
        dpadFinger = id; touchKeys |= DpadForPoint(x, y);
    } else if (InCircle(x, y, BUTTON_CX, BUTTON_CY, BUTTON_R)) {
        aFinger = id; SetKey(A_BUTTON, true);
    } else if (InCircle(x, y, BUTTON2_CX, BUTTON2_CY, BUTTON2_R)) {
        bFinger = id; SetKey(B_BUTTON, true);
    } else if (InRect(x, y, 213.0f, 225.0f, 28.0f, 10.0f)) {
        startFinger = id; SetKey(START_BUTTON, true);
    } else if (InRect(x, y, 151.0f, 225.0f, 28.0f, 10.0f)) {
        selectFinger = id; SetKey(SELECT_BUTTON, true);
    } else if (InRect(x, y, 96.0f, 28.0f, SHOULDER_W / 2.0f, SHOULDER_H / 2.0f)) {
        lFinger = id; SetKey(L_BUTTON, true);
    } else if (InRect(x, y, 330.0f, 28.0f, SHOULDER_W / 2.0f, SHOULDER_H / 2.0f)) {
        rFinger = id; SetKey(R_BUTTON, true);
    }
}

u16 AndroidTouch_GetKeys(void) { return touchKeys; }

static void DrawCircle(SDL_Renderer *renderer, int cx, int cy, int radius)
{
    for (int y = -radius; y <= radius; ++y) {
        int x = (int)sqrtf((float)(radius * radius - y * y));
        SDL_RenderDrawLine(renderer, cx - x, cy + y, cx + x, cy + y);
    }
}

static void DrawButton(SDL_Renderer *renderer, int cx, int cy, int radius, bool pressed)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, pressed ? 90 : 55);
    DrawCircle(renderer, cx, cy, radius);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, pressed ? 190 : 110);
    SDL_RenderDrawLine(renderer, cx - radius, cy, cx + radius, cy);
    SDL_RenderDrawLine(renderer, cx, cy - radius, cx, cy + radius);
}

void AndroidTouch_Draw(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    DrawButton(renderer, (int)DPAD_CX, (int)DPAD_CY, (int)DPAD_R, (touchKeys & (DPAD_UP|DPAD_DOWN|DPAD_LEFT|DPAD_RIGHT)) != 0);
    DrawButton(renderer, (int)BUTTON_CX, (int)BUTTON_CY, (int)BUTTON_R, (touchKeys & A_BUTTON) != 0);
    DrawButton(renderer, (int)BUTTON2_CX, (int)BUTTON2_CY, (int)BUTTON2_R, (touchKeys & B_BUTTON) != 0);

    SDL_Rect start = {185, 216, 56, 18}, select = {123, 216, 56, 18};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, (touchKeys & START_BUTTON) ? 190 : 70);
    SDL_RenderFillRect(renderer, &start);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, (touchKeys & SELECT_BUTTON) ? 190 : 70);
    SDL_RenderFillRect(renderer, &select);

    SDL_Rect l = {76, 14, 40, 28}, r = {310, 14, 40, 28};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, (touchKeys & L_BUTTON) ? 190 : 70);
    SDL_RenderFillRect(renderer, &l);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, (touchKeys & R_BUTTON) ? 190 : 70);
    SDL_RenderFillRect(renderer, &r);
}
