/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_THREAD_AMIGAOS3

#include <proto/exec.h>

#include "SDL_mutex.h"

struct SDL_mutex
{
    struct SignalSemaphore sem;
};

SDL_mutex *SDL_CreateMutex(void)
{
    SDL_mutex *mutex = (SDL_mutex *)SDL_malloc(sizeof(*mutex));
    if (mutex) {
        InitSemaphore(&mutex->sem);
    } else {
        SDL_OutOfMemory();
    }
    return mutex;
}

void SDL_DestroyMutex(SDL_mutex *mutex)
{
    if (mutex) {
        SDL_free(mutex);
    }
}

int SDL_LockMutex(SDL_mutex *mutex)
{
    if (!mutex) return SDL_SetError("Passed a NULL mutex");
    ObtainSemaphore(&mutex->sem);
    return 0;
}

int SDL_TryLockMutex(SDL_mutex *mutex)
{
    if (!mutex) return SDL_SetError("Passed a NULL mutex");
    return AttemptSemaphore(&mutex->sem) ? 0 : SDL_MUTEX_TIMEDOUT;
}

int SDL_UnlockMutex(SDL_mutex *mutex)
{
    if (!mutex) return SDL_SetError("Passed a NULL mutex");
    ReleaseSemaphore(&mutex->sem);
    return 0;
}

#endif /* SDL_THREAD_AMIGAOS3 */
