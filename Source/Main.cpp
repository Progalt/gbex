
#include "GBEX/GBEX.h"

#include <cstdio>
#include <malloc.h>
#include <stdexcept>

int main(int argc, char* argv[])
{

    FILE* file = NULL;
    uint8_t* rom;
    long rom_size;

    file = fopen("Tests/cpu_instrs/individual/02-interrupts.gb", "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Unable to open file\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);

    rom_size = ftell(file);
    if (rom_size == -1)
    {
        fprintf(stderr, "Error determining the file size\n");
        fclose(file);
        return 1;
    }

    rom = (uint8_t*)malloc(rom_size);
    if (rom == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    // Move the file pointer back to the beginning of the file
    fseek(file, 0, SEEK_SET);

    size_t read_size = fread(rom, 1, rom_size, file);
    if (read_size != rom_size) {
        fprintf(stderr, "Error reading the file\n");
        free(rom);
        fclose(file);
        return 1;
    }

    fclose(file);

	gbex::gbex emulator;
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
        try
        {
            emulator.step();
        }
        catch (std::exception& e)
        {
            printf("%s\n", e.what());
            running = false;
        }
    }

    printf("Emulator Terminated");
    free(rom);
	return 0;
}