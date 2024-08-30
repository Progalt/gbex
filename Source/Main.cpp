
#include "GBEX/GBEX.h"

#include <cstdio>
#include <malloc.h>
#include <stdexcept>

#include <SDL3/SDL.h>
#include "Util.h"

#include "DebugUI/DebugUI.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

SDL_Window* window;
SDL_Renderer* renderer;

SDL_Texture* VRAMtilesDebug;
SDL_Texture* output;

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

    file = fopen("Tests/Super Mario Land 2.gb", "rb");
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

    renderer = SDL_CreateRenderer(window, NULL);

    SDL_SetRenderVSync(renderer, 1);

    VRAMtilesDebug = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 16 * 8, 24 * 8);
    SDL_SetTextureScaleMode(VRAMtilesDebug, SDL_SCALEMODE_NEAREST);

    output = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, gbex::GameboyScreenWidth, gbex::GameboyScreenHeight);
    SDL_SetTextureScaleMode(output, SDL_SCALEMODE_NEAREST);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

	gbex::gbex emulator;
    emulator.set_device_type(gbex::DeviceType::DMG);

    gbex::Palette originalLCD;
    originalLCD.colours[0] = gbex::Colour(0x8c, 0xad, 0x28);
    originalLCD.colours[1] = gbex::Colour(0x6c, 0x94, 0x21);
    originalLCD.colours[2] = gbex::Colour(0x42, 0x6b, 0x29);
    originalLCD.colours[3] = gbex::Colour(0x21, 0x42, 0x31);



    emulator.set_vsync_callback([&]() 
        {
            uint8_t* pixelBuffer = emulator.get_framebuffer();

            int pitch;
            uint8_t* pixels;
            SDL_LockTexture(output, NULL, (void**)&pixels, &pitch);

            for (uint32_t i = 0; i < gbex::GameboyScreenWidth; i++)
            {
                for (uint32_t j = 0; j < gbex::GameboyScreenHeight; j++)
                {
                    uint32_t coord = j * gbex::GameboyScreenWidth + i;
                    coord *= 4;

                    pixels[coord + 3] = pixelBuffer[coord + 0];
                    pixels[coord + 2] = pixelBuffer[coord + 1];
                    pixels[coord + 1] = pixelBuffer[coord + 2];
                    pixels[coord + 0] = pixelBuffer[coord + 3];

                }
            }

            SDL_UnlockTexture(output);

            SDL_RenderClear(renderer);

            ImGui_ImplSDL3_NewFrame();
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui::NewFrame();

            debugui_cpu(&emulator.m_CPU, &emulator.m_MMU);
            debugui_ppu(&emulator.m_PPU, &emulator.m_MMU, VRAMtilesDebug);
            debugui_debugger(&emulator);
            debugui_output(&emulator, output);

            ImGui::Render();
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

            SDL_RenderPresent(renderer);
        });

    emulator.load_cartridge(rom, rom_size);

    emulator.set_dmg_colour_palette(originalLCD);

    printf("ROM Title: %s\n", emulator.get_cartridge()->get_header().title);
    printf("New Licensee: %s\n", emulator.get_cartridge()->get_header().new_licensee);
    printf("SGB Flag: %02x\n", emulator.get_cartridge()->get_header().sgb_flag);
    printf("Cartridge Type: %02x\n", emulator.get_cartridge()->get_header().cartridge_type);
    printf("ROM Size: %02x\n", emulator.get_cartridge()->get_header().rom_size);

    emulator.set_breakpoint(0x0335);

    emulator.pause();


    bool running = true;
    while (running)
    {
 

        SDL_Event evnt;
        gbex::Joypad* joyp = emulator.get_joypad();
        while (SDL_PollEvent(&evnt))
        {
            ImGui_ImplSDL3_ProcessEvent(&evnt);

            switch (evnt.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
            {
                switch (evnt.key.key)
                {
                case SDLK_UP:
                    joyp->set_button_down(gbex::JoypadButton::UP);
                    break;
                case SDLK_DOWN:
                    joyp->set_button_down(gbex::JoypadButton::DOWN);
                    break;
                case SDLK_RIGHT:
                    joyp->set_button_down(gbex::JoypadButton::RIGHT);
                    break;
                case SDLK_LEFT:
                    joyp->set_button_down(gbex::JoypadButton::LEFT);
                    break;
                case SDLK_RETURN:
                    joyp->set_button_down(gbex::JoypadButton::START);
                    break;
                case SDLK_BACKSPACE:
                    joyp->set_button_down(gbex::JoypadButton::SELECT);
                    break;
                case SDLK_X:
                    joyp->set_button_down(gbex::JoypadButton::B);
                    break;
                case SDLK_C:
                    joyp->set_button_down(gbex::JoypadButton::A);
                    break;
                }
            }
                break;
            case SDL_EVENT_KEY_UP:
            {
                switch (evnt.key.key)
                {
                case SDLK_UP:
                    joyp->set_button_released(gbex::JoypadButton::UP);
                    break;
                case SDLK_DOWN:
                    joyp->set_button_released(gbex::JoypadButton::DOWN);
                    break;
                case SDLK_RIGHT:
                    joyp->set_button_released(gbex::JoypadButton::RIGHT);
                    break;
                case SDLK_LEFT:
                    joyp->set_button_released(gbex::JoypadButton::LEFT);
                    break;
                case SDLK_RETURN:
                    joyp->set_button_released(gbex::JoypadButton::START);
                    break;
                case SDLK_BACKSPACE:
                    joyp->set_button_released(gbex::JoypadButton::SELECT);
                    break;
                case SDLK_X:
                    joyp->set_button_released(gbex::JoypadButton::B);
                    break;
                case SDLK_C:
                    joyp->set_button_released(gbex::JoypadButton::A);
                    break;
                }
            }
                break;
            }
        }

       // try
        //{
            emulator.step();
        /*}
        catch (std::exception& e)
        {
            emu::FailureMessage("Error", e.what());
            printf("%s\n", e.what());
            running = false;
        }*/


    }

    printf("Emulator Terminated");
    free(rom);

    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(window);
    SDL_Quit();
	return 0;
}