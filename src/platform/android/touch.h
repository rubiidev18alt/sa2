#ifndef PLATFORM_ANDROID_TOUCH_H
#define PLATFORM_ANDROID_TOUCH_H

#include <SDL.h>
#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

void AndroidTouch_Init(void);
void AndroidTouch_HandleEvent(const SDL_Event *event);
u16 AndroidTouch_GetKeys(void);

#ifdef __cplusplus
}
#endif

#endif
