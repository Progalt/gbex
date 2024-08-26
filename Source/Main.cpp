
#include "GBEX/GBEX.h"

#include <cstdio>
#include <malloc.h>
#include <stdexcept>

#include <SDL3/SDL.h>
#include "Util.h"

SDL_Window* window;

int main(int argc, char* argv[])
{

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Failed to initialise SDL3\n");
        return 1;
    }

    FILE* file = NULL;
    uint8_t* rom;
    long rom_size;

    file = fopen("Tests/cpu_instrs.gb", "rb");
    if (file == NULL)
    {
        emu::FailureMessage("Error", "Unable to open ROM file, is the path correct?");
        return 1;
    }

    fseek(file, 0, SEEK_END);

    rom_size = ftell(file);
    if (rom_size == -1)
    {
        emu::FailureMessage("Internal Error", "Failed to determine file size");
        fclose(file);
        return 1;
    }

    rom = (uint8_t*)malloc(rom_size);
    if (rom == NULL) {
        emu::FailureMessage("Error", "Failed to allocate memory for ROM, make sure you have enough memory available.");
        fclose(file);
        return 1;
    }

    // Move the file pointer back to the beginning of the file
    fseek(file, 0, SEEK_SET);

    size_t read_size = fread(rom, 1, rom_size, file);
    if (read_size != rom_size) {
        emu::FailureMessage("Error", "Failed to read ROM from file.");
        free(rom);
        fclose(file);
        return 1;
    }

    fclose(file);

    window = SDL_CreateWindow("Emulator", 1200, 800, 0);

    if (!window)
    {
        emu::FailureMessage("Error", "Failed to craete window");
        return 1;
    }

	gbex::gbex emulator;
    emulator.set_device_type(gbex::DeviceType::DMG);

    emulator.set_vsync_callback([&]() 
        {

        });

    emulator.load_cartridge(rom, rom_size);

    printf("ROM Title: %s\n", emulator.get_cartridge()->get_header().title);
    printf("New Licensee: %s\n", emulator.get_cartridge()->get_header().new_licensee);
    printf("SGB Flag: %02x\n", emulator.get_cartridge()->get_header().sgb_flag);
    printf("Cartridge Type: %02x\n", emulator.get_cartridge()->get_header().cartridge_type);
    printf("ROM Size: %02x\n", emulator.get_cartridge()->get_header().rom_size);

    // emulator.set_breakpoint(0xC65C);

    bool running = true;
    while (running)
    {
        SDL_Event evnt;
        while (SDL_PollEvent(&evnt))
        {
            switch (evnt.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            }
        }

        try
        {
            emulator.step();
        }
        catch (std::exception& e)
        {
            emu::FailureMessage("Error", e.what());
            printf("%s\n", e.what());
            running = false;
        }
    }

    printf("Emulator Terminated");
    free(rom);

    SDL_DestroyWindow(window);
    SDL_Quit();
	return 0;
}