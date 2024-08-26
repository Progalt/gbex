
cmake_minimum_required(VERSION 3.15)

project(gbex VERSION 1.0)

add_subdirectory("Vendor/SDL")


set(SOURCES
	"Source/Main.cpp"
 "Source/GBEX/Cartridge.cpp" "Source/GBEX/MMU.cpp" "Source/GBEX/GBEX.cpp" "Source/GBEX/CPU.cpp" "Source/GBEX/Interrupts.cpp" "Source/GBEX/Opcodes.cpp" "Source/GBEX/PPU.cpp")

add_executable(gbex ${SOURCES})

target_link_libraries(gbex SDL3::SDL3)
target_compile_features(gbex PRIVATE cxx_std_17)