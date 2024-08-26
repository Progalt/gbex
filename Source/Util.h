
#ifndef UTIL_H
#define UTIL_H

#include <SDL3/SDL.h>
#include <cstdio>

namespace emu
{
	inline void FailureMessage(const char* title, const char* message)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, NULL);
		printf("%s\n", message);
	}
}

#endif // UTIL_H