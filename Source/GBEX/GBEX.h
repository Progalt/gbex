
#ifndef GBEX_H
#define GBEX_H

#include "CPU.h"
#include "MMU.h"
#include "PPU.h"
#include "Joypad.h"
#include <functional>
#include "Timer.h"
#include <filesystem>

namespace gbex
{
	enum class DeviceType
	{
		DMG, 
		CGB
	};

	/*
		gbex is the Base emulator class and allows the full running from it. You most likely won't need to interact with any internal APIs or functions and do it all through this. 

		It also exposes any APIs that you need like the Joypad API. 
	*/
	class gbex
	{
	public:

		/*
			Set the device type. This must be specified before loading a cartridge.
			It can be DMG, CGB

			Defaults to DMG 
		*/
		void set_device_type(DeviceType type)
		{
			m_DeviceType = type;
		}

		/*
			Loads a cartridge into the emulator and preps it for being played
		*/
		bool load_cartridge(uint8_t* rom, const size_t size);

		/*
			Unloads a cartridge and frees all associated memory with it, if its a cartridge with Battery Backed SRAM it will also save before. 
		*/
		void unload_cartridge();

		/*
			Step the emulator, usually executes a CPU instruction, and then the associated cycles in the PPU, and the Timer. 
		*/
		void step();

		Cartridge* get_cartridge() const { return m_MMU.m_Cartridge; }

		/*
			Allows you to set a breakpoint. The emulator will pause when the CPU program counter reaches the target. 
		*/
		void set_breakpoint(uint16_t target_pc);

		void reset_breakpoint() { m_HitBreakpoint = false; }

		/*
			Sets a VBlank callback. This is called when the PPU enters the VBlank timing interval and signals the image is done drawing and ready to display externally to the emulator. 
		*/
		void set_vsync_callback(std::function<void()> callback)
		{
			m_VsyncCallback = callback;
		}

		void play()
		{
			m_Halted = false;
			m_Step = false;
			// m_HitBreakpoint = false;
		}

		void pause()
		{
			m_Halted = true;
		}

		void step_intruction()
		{
			m_Step = true;
			m_Halted = false;
		}

		bool is_halted() const 
		{
			return m_Halted;
		}

		/*
			Get a pointer to the internal framebuffer used to draw to so it can be displayed or outputted in some way. 
		*/
		uint8_t* get_framebuffer() const
		{
			return m_PPU.m_Framebuffer.get();
		}

		/*
			Get the Joypad. 

			One of the only externally available emulator system so you can funnel input events into the emulator. This can be updated at every step to provide the best results even if there is a slowdown. Alternatively it can be updated at every VBlank
		*/
		Joypad* get_joypad()
		{
			return &m_Joypad;
		}

		/*
			This allows you to override the default grey palette for something else when in DMG mode.
			Like for instance the classic LCD green
		*/
		void set_dmg_colour_palette(Palette palette)
		{
			m_PPU.m_Palette = palette;
		}

		/*
			Set if the Emulator has the ability to save games. This is only possible if the game has an MBC with battery backed RAM. 

			By Default saves are enabled on cartridges that support them. 
		*/
		void set_save_ability(bool savesAllowed)
		{
			m_SavesEnabled = savesAllowed;
		}

		/*
			This is the path saves will be written to.
		*/
		void set_save_path(const std::filesystem::path& path)
		{

		}

		CPU m_CPU;
		MMU m_MMU;
		PPU m_PPU;
		Joypad m_Joypad;
		Timer m_Timer;

	private:

		bool m_SavesEnabled = true;

		std::function<void()> m_VsyncCallback;

		uint16_t m_Breakpoint = 0xFFFF;
		bool m_HitBreakpoint = false;

		bool m_Step = false;
		bool m_Halted = false;

		DeviceType m_DeviceType = DeviceType::DMG;

	};
}

#endif // GBEX_H