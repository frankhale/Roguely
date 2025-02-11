cmake_minimum_required(VERSION 3.8)

#if(WIN32)
#	cmake_policy(SET CMP0091 NEW)
#	set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "target triplet" FORCE)
#	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#endif()

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(roguely CXX)

find_package(magic_enum CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(sol2 REQUIRED)
find_package(Lua REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
target_link_libraries(main
		PRIVATE
		)

set(Sources main.cpp engine.cpp engine.h)
add_executable(roguely ${Sources})
target_compile_features(roguely PUBLIC cxx_std_20)

target_include_directories(roguely
	PUBLIC
	${Boost_INCLUDE_DIRS}
	${LUA_INCLUDE_DIR}
)

target_link_libraries(roguely
	PUBLIC
	winmm.lib
	bcrypt.lib
	fmt::fmt
	${LUA_LIBRARIES}
	SDL2::SDL2 SDL2::SDL2main SDL2::SDL2-static
		$<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
		$<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
		$<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
	PRIVATE
	magic_enum::magic_enum
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/roguely.lua DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
