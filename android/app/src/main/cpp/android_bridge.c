#include <stdbool.h>
#include <jni.h>
#include <SDL.h>

static SDL_Keycode KeyForButton(jint button)
{
    switch (button) {
        case 0: return SDLK_c;         /* A */
        case 1: return SDLK_x;         /* B */
        case 2: return SDLK_RETURN;    /* Start */
        case 3: return SDLK_BACKSLASH; /* Select */
        case 4: return SDLK_s;         /* L */
        case 5: return SDLK_d;         /* R */
        case 6: return SDLK_UP;
        case 7: return SDLK_DOWN;
        case 8: return SDLK_LEFT;
        case 9: return SDLK_RIGHT;
        default: return SDLK_UNKNOWN;
    }
}

JNIEXPORT void JNICALL
Java_com_sat_1r_sa2_MainActivity_nativeSetKey(JNIEnv *env, jclass clazz, jint button, jboolean down)
{
    (void)env;
    (void)clazz;

    SDL_Keycode key = KeyForButton(button);
    if (key == SDLK_UNKNOWN) return;

    SDL_Event event;
    SDL_zero(event);
    event.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.timestamp = SDL_GetTicks();
    event.key.state = down ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = 0;
    event.key.keysym.sym = key;
    event.key.keysym.scancode = SDL_GetScancodeFromKey(key);
    event.key.keysym.mod = KMOD_NONE;
    SDL_PushEvent(&event);
}
