cmake_minimum_required(VERSION 3.16)

project(mavlink-osd C)

set(PROJECT_SOURCES
	main.c
)

add_executable(main ${PROJECT_SOURCES})
target_include_directories(main PUBLIC c_library_v2)
target_compile_options(main PUBLIC -Wno-address-of-packed-member)
