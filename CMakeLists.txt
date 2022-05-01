cmake_minimum_required(VERSION 3.16)

project(mavlink C)

set(PROJECT_SOURCES
		main.c ui.c shader.c texture.c font.c telem_data.c widget.c render.c shape.c matrix.c layout.c
		widgets/widget_text.c widgets/widget_bank_indicator.c widgets/widget_attitude.c widgets/widget_heading.c widgets/widget_tape.c
		)

set(RESOURCES
		shaders/font.frag
		shaders/font.vert
		shaders/solid.frag
		shaders/solid.vert
		fonts/font.rgba
		)

foreach(file ${RESOURCES})
	string(REGEX REPLACE "[/.]" "_" file_stripped ${file})
	list(APPEND PROJECT_SOURCES ${file_stripped}.o)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_stripped}.o
			COMMAND ${CMAKE_LINKER} ARGS
			--relocatable --format binary --output ${CMAKE_CURRENT_BINARY_DIR}/${file_stripped}.o ${file}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
			)
endforeach()

find_package(PkgConfig REQUIRED)


add_executable(main ${PROJECT_SOURCES})
target_compile_options(main PUBLIC -Wno-address-of-packed-member -Ofast -pg -Ofast -fprofile-arcs -ftest-coverage)
target_link_options(main PUBLIC -pg -fprofile-arcs -ftest-coverage)

if(DEFINED RPI)
	add_compile_definitions(EGL_NO_X11)
	target_include_directories(main PUBLIC c_library_v2 /opt/vc/include)
	target_link_directories(main PUBLIC /opt/vc/lib)
	target_link_libraries(
			main
			PUBLIC
			m
			brcmGLESv2
			brcmEGL
			bcm_host
			pthread
	)
else()
	target_include_directories(main PUBLIC c_library_v2)
	target_link_libraries(main GLESv2 EGL X11 m)
endif()
