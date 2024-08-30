
cmake_minimum_required(VERSION 3.15)

project(gbex VERSION 1.0)

add_subdirectory("Vendor/SDL")

set(IMGUI_PATH "Vendor/imgui")

set(IMGUI_SOURCES 
	"${IMGUI_PATH}/imgui.cpp"
	"${IMGUI_PATH}/imgui_demo.cpp"
	"${IMGUI_PATH}/imgui_draw.cpp"
	"${IMGUI_PATH}/imgui_tables.cpp"
	"${IMGUI_PATH}/imgui_widgets.cpp"
	"${IMGUI_PATH}/backends/imgui_impl_sdl3.cpp"
	"${IMGUI_PATH}/backends/imgui_impl_sdlrenderer3.cpp"
)


set(SOURCES
	"Source/Main.cpp"
 "Source/GBEX/Cartridge.cpp" "Source/GBEX/MMU.cpp" "Source/GBEX/GBEX.cpp" "Source/GBEX/CPU.cpp" "Source/GBEX/Interrupts.cpp" "Source/GBEX/Opcodes.cpp" "Source/GBEX/PPU.cpp" "Source/GBEX/Joypad.cpp" "Source/GBEX/Timer.cpp")

add_executable(gbex ${SOURCES} ${IMGUI_SOURCES})

include_directories(gbex ${IMGUI_PATH})

target_link_libraries(gbex SDL3::SDL3)
target_compile_features(gbex PRIVATE cxx_std_17)