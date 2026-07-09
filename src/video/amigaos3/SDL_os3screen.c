/*
 Simple DirectMedia Layer
 Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_DRIVER_AMIGAOS3

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/wb.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>

#include "SDL_os3video.h"
#include "SDL_os3screen.h"
#include "SDL_os3window.h"

static ULONG FindBestCyberMode(int width, int height, int depth) {
    ULONG bestModeId = INVALID_ID;
    int bestHeight = -1; // Used to track the closest height found so far

    ULONG nextid = NextDisplayInfo(INVALID_ID);
    ULONG pixelFormat = 0;

    while (nextid != INVALID_ID) {
        if (IsCyberModeID(nextid)) {
            SDL_bool validMode = SDL_FALSE;

            pixelFormat = GetCyberIDAttr(CYBRIDATTR_PIXFMT, nextid);

            // Validate Depth & Pixel Format
            if (depth == 8) {
                if (pixelFormat == PIXFMT_LUT8) validMode = SDL_TRUE;
            } else if (depth == 24) {
                if ((pixelFormat == PIXFMT_BGR24) || (pixelFormat == PIXFMT_RGB24)) validMode = SDL_TRUE;
            } else if (depth == 32) {
                if ((pixelFormat == PIXFMT_ARGB32) || (pixelFormat == PIXFMT_BGRA32) || (pixelFormat == PIXFMT_RGBA32)) validMode = SDL_TRUE;
            }

            // Check Dimensions if Depth is valid
            if (validMode) {
                int modeWidth = GetCyberIDAttr(CYBRIDATTR_WIDTH, nextid);

                if (modeWidth == width) {
                    int modeHeight = GetCyberIDAttr(CYBRIDATTR_HEIGHT, nextid);

                    if (modeHeight == height) {
                        // Success: Exact match found. Halt the search immediately.
                        bestModeId = nextid;
                        break;
                    } else if (modeHeight > height) {
                        // Potential Match: Height is greater than requested.
                        // Update if it's our first valid fallback, or if it's closer to the target height than the previous best.
                        if ((bestModeId == INVALID_ID) || (modeHeight < bestHeight)) {
                            bestModeId = nextid;
                            bestHeight = modeHeight;
                        }
                    }
                }
            }
        }

        nextid = NextDisplayInfo(nextid);
    }

    return bestModeId;
}

struct Screen* OS3_LockPublicScreen(void)
{
    struct Screen* screen = NULL;

    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Trying to lock the public screen\n");

    screen = LockPubScreen(NULL);
    if (screen) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Locked the public screen: %p\n", screen);
    } else {
    	SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Failed to lock the public screen\n");
    }

    return screen;
}

void OS3_UnlockPublicScreen(struct Screen* screen)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Trying to unlock the public screen\n");

	if (screen) {
		SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Unlocking the public screen: %p\n", screen);
        UnlockPubScreen(NULL, screen);
    }
}

struct Screen* OS3_CreateCustomScreen(int width, int height)
{
	struct Screen* screen = NULL;
	ULONG modeId = INVALID_ID;
	int depth;

	depth = 32; // Try to create a 32-bit screen by default.

	SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Trying to create screen (%dx%d)\n", width, height);

	modeId = FindBestCyberMode(width, height, depth);
	if (modeId == INVALID_ID) {
		// If we can't find a suitable 32-bit mode, try 24-bit instead as a fallback option.
		depth = 24;

		modeId = FindBestCyberMode(width, height, depth);
	}

	if (modeId == INVALID_ID) {
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Could not find a suitable display mode\n");
	} else {
		screen = OpenScreenTags(NULL, SA_Depth, depth, SA_DisplayID, modeId,
				SA_Top, 0, SA_Left, 0, SA_Width, width, SA_Height, height, SA_Type,
				CUSTOMSCREEN, SA_Quiet, TRUE, SA_ShowTitle, FALSE, SA_Draggable, FALSE,
				SA_Exclusive, TRUE, SA_AutoScroll, FALSE, TAG_DONE);
	}

    if (screen) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Opened screen: %p - width=%d, height=%d, depth=%d\n",
        		screen, width, height, depth);
    } else {
    	SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Failed to create screen\n");
    }

	return screen;
}

void OS3_CloseCustomScreen(SDL_Window* sdlwin)
{
	SDL_WindowData* data = sdlwin->driverdata;

	if (data) {
		struct Screen* screen = data->customScreen;

		if (screen) {
			SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Trying to closing screen: %p\n", screen);

			if (CloseScreen(screen) == FALSE) {
				SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Screen has open window(s), cannot close\n");
			} else {
				SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Screen closed successfully\n");
				data->customScreen = NULL;
	        }
	    }
	}
}


#endif /* SDL_VIDEO_DRIVER_AMIGAOS3 */

