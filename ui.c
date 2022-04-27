// egl_bar.c - self-contained example of a vertical bar moving across the screen.
// compile with  gcc -Wall -O0 -g -o egl_bar egl_bar.c log.c -lX11 -lEGL -lGLESv2 -lm
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include "telem_data.h"
#include "shader.h"
#include "texture.h"
#include "font.h"
#include "render.h"
#include "shape.h"
#include "matrix.h"

#include "layout.h"
#include "widgets/widget_text.h"
#include "widgets/widget_bank_indicator.h"

static void log_egl_details(EGLDisplay egl_display, EGLConfig egl_conf) {
	printf("EGL Client APIs: %s\n", eglQueryString(egl_display, EGL_CLIENT_APIS));
	printf("EGL Vendor: %s\n", eglQueryString(egl_display, EGL_VENDOR));
	printf("EGL Version: %s\n", eglQueryString(egl_display, EGL_VERSION));
	printf("EGL Extensions: %s\n", eglQueryString(egl_display, EGL_EXTENSIONS));

	int i = -1;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_CONFIG_ID, &i);
	printf("EGL_CONFIG_ID = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_RED_SIZE, &i);
	printf("EGL_RED_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_GREEN_SIZE, &i);
	printf("EGL_GREEN_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_BLUE_SIZE, &i);
	printf("EGL_BLUE_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_ALPHA_SIZE, &i);
	printf("EGL_ALPHA_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_DEPTH_SIZE, &i);
	printf("EGL_DEPTH_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_LEVEL, &i);
	printf("EGL_LEVEL = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_NATIVE_RENDERABLE, &i);
	printf("EGL_NATIVE_RENDERABLE = %s\n", i ? "EGL_TRUE" : "EGL_FALSE");

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_NATIVE_VISUAL_TYPE, &i);
	printf("EGL_NATIVE_VISUAL_TYPE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_RENDERABLE_TYPE, &i);
	printf("EGL_RENDERABLE_TYPE = 0x%04x\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_SURFACE_TYPE, &i);
	printf("EGL_SURFACE_TYPE = 0x%04x\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_TYPE, &i);
	if (i == EGL_TRANSPARENT_RGB) {
		printf("EGL_TRANSPARENT_TYPE = EGL_TRANSPARENT_RGB\n");

		i = 0;
		eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_RED_VALUE, &i);
		printf("EGL_TRANSPARENT_RED = 0x%02x\n", i);

		i = 0;
		eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_GREEN_VALUE, &i);
		printf("EGL_TRANSPARENT_GREEN = 0x%02x\n", i);

		i = 0;
		eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_BLUE_VALUE, &i);
		printf("EGL_TRANSPARENT_BLUE = 0x%02x\n", i);
	} else {
		printf("EGL_TRANSPARENT_TYPE = EGL_NONE\n");
	}
}

void *render_thread_start(void *arg) {
	Display *x_display = XOpenDisplay(NULL);
	if (x_display == NULL) {
		printf("cannot connect to X server\n");
		return 1;
	}

	Window root = DefaultRootWindow(x_display);   // get the root window (usually the whole screen)
	int root_x, root_y;
	unsigned int root_w, root_h, root_border_width, root_depth;
	Window root_again;
	XGetGeometry(x_display, root, &root_again, &root_x, &root_y, &root_w, &root_h, &root_border_width, &root_depth);
	printf("Matching X11 root window geometry: +%d,%d %dx%d border %d, %dbpp\n\n",
	       root_x, root_y, root_w, root_h, root_border_width, root_depth);

	XSetWindowAttributes  swa;
	swa.event_mask  =  ExposureMask | ButtonPressMask | KeyPressMask;

	Window win = XCreateWindow(x_display, root,
	                           0, 0, 720 / 2, 576 / 2, 0,
	                           CopyFromParent, InputOutput,
	                           CopyFromParent, CWEventMask,
	                           &swa);

	XSetWindowAttributes xattr;

	xattr.override_redirect = False;
	XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

	/* This fails when there is no window manager running
	Atom atom;
	atom = XInternAtom(x_display, "_NET_WM_STATE_FULLSCREEN", True);
	XChangeProperty(
	   x_display, win,
	   XInternAtom(x_display, "_NET_WM_STATE", True),
	   XA_ATOM, 32, PropModeReplace,
	   (unsigned char*) &atom, 1);
	*/

	int one = 1;
	XChangeProperty(
			x_display, win,
			XInternAtom ( x_display, "_HILDON_NON_COMPOSITED_WINDOW", False ),
			XA_INTEGER,  32,  PropModeReplace,
			(unsigned char*) &one,  1);

	XWMHints hints;
	hints.input = True;
	hints.flags = InputHint;
	XSetWMHints(x_display, win, &hints);

	XMapWindow(x_display, win); // makes the window visible on the screen

	//// get identifiers for the provided atom name strings
	Atom wm_state = XInternAtom(x_display, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(x_display, "_NET_WM_STATE_FULLSCREEN", False);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));

	xev.type                 = ClientMessage;
	xev.xclient.window       = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format       = 32;
	xev.xclient.data.l[0]    = 1;
	xev.xclient.data.l[1]    = fullscreen;
	XSendEvent(                // set up event mask (which events we want to receive)
			x_display,
			DefaultRootWindow(x_display),
			False,
			SubstructureNotifyMask,
			&xev);


	///////  the egl part  //////////////////////////////////////////////////////////////////
	//  egl provides an interface to connect the graphics related functionality of openGL ES
	//  with the windowing interface and functionality of the native operation system (X11
	//  in our case.)

	EGLDisplay egl_display = eglGetDisplay((EGLNativeDisplayType) x_display);
	if (egl_display == EGL_NO_DISPLAY) {
		printf("Got no EGL display.\n");
		return 1;
	}

	EGLint egl_version_major, egl_version_minor;
	if (!eglInitialize(egl_display, &egl_version_major, &egl_version_minor)) {
		printf("Unable to initialize EGL\n");
		return 1;
	}
	printf("Initialized EGL version %d.%d\n", egl_version_major, egl_version_minor);

	EGLint egl_config_constraints[] = {
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_CONFIG_CAVEAT, EGL_NONE,
			EGL_SAMPLES, 4,
			EGL_NONE
	};

	EGLConfig egl_conf;
	EGLint num_config;
	if (!eglChooseConfig(egl_display, egl_config_constraints, &egl_conf, 1, &num_config)) {
		printf("Failed to choose config (eglError: %s)\n", eglGetError());
		return 1;
	}

	if (num_config != 1) {
		printf("Didn't get exactly one config, but %d\n", num_config);
		return 1;
	}

	EGLSurface egl_surface = eglCreateWindowSurface(egl_display, egl_conf, win, NULL);
	if (egl_surface == EGL_NO_SURFACE) {
		printf("Unable to create EGL surface (eglError: %s)\n", eglGetError());
		return 1;
	}

	//// egl-contexts collect all state descriptions needed required for operation
	EGLint ctxattr[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
	};
	EGLContext egl_context = eglCreateContext(egl_display, egl_conf, EGL_NO_CONTEXT, ctxattr);
	if (egl_context == EGL_NO_CONTEXT) {
		printf("Unable to create EGL context (eglError: %s)\n", eglGetError());
		return 1;
	}

	//// associate the egl-context with the egl-surface
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

	log_egl_details(egl_display, egl_conf);

	EGLint queriedRenderBuffer;
	if (eglQueryContext(egl_display, egl_context, EGL_RENDER_BUFFER, &queriedRenderBuffer)) {
		switch (queriedRenderBuffer) {
			case EGL_SINGLE_BUFFER: printf("Render Buffer: EGL_SINGLE_BUFFER\n"); break;
			case EGL_BACK_BUFFER: printf("Render Buffer: EGL_BACK_BUFFER\n"); break;
			case EGL_NONE: printf("Render Buffer: EGL_NONE\n"); break;
			default: printf("Render Buffer: unknown value %d\n", queriedRenderBuffer); break;
		}
	} else {
		printf("Failed to query EGL_RENDER_BUFFER: %d\n", eglGetError());
	}

	if (!eglSwapInterval(egl_display, 1)) {
		printf("eglSwapInterval failed: %d\n", eglGetError());
	} else {
		printf("Set swap interval\n");
	}


	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	font_init();
	shape_init();
	layout_init();

	struct widget_text text;
	widget_text_init(&text);
	widget_text_set(&text, "test");

	float transform[3][3];
	matrix_identity(transform);
	layout_add(&text, transform);

	struct widget_text data;
	widget_text_init(&data);
	data.color[0] = 0;
	data.align = FONT_ALIGN_H_CENTER | FONT_ALIGN_V_CENTER;

	matrix_translate(360, 288, transform);
	layout_add(&data, transform);

	struct widget_bank_indicator bank;
	widget_bank_indicator_init(&bank);

	matrix_translate(360, 144, transform);
	matrix_scale_multiply(100, 100, transform, transform);
	layout_add(&bank, transform);

	struct shape_context shape;
	shape_new(&shape);

	glLineWidth(2);

	// render loop
	for (;;) {
		XWindowAttributes gwa;
		XGetWindowAttributes(x_display, win, &gwa);

		gwa.width = 720;
		gwa.height = 576;

		float width = 720;
		float height = 576;

		glViewport(0, 0, gwa.width / 2, gwa.height / 2);

		render_load_identity();
		render_ortho(0, gwa.width, gwa.height, 0);

		glClear(GL_COLOR_BUFFER_BIT);

		char buf[2048];
		sprintf(buf,
				"Attitude: %f %f %f\n"
				"GPS: %d %d\n"
				"Airspeed: %f\n"
				"Groundspeed: %f\n"
				"Altitude: %f\n"
				"Climb rate: %f",
				telem_data.pitch, telem_data.roll, (telem_data.yaw) / M_PI * 180,
				telem_data.lat, telem_data.lon,
				telem_data.airspeed,
				telem_data.groundspeed,
				telem_data.altitude,
				telem_data.climbrate);
		widget_text_set(&data, buf);

		bank.bank_angle = telem_data.roll;

		layout_render();

		render_set_color(1, 0, 0, 0.5);

		render_push_matrix();

		render_translate(width / 2, height / 2);

		render_rotate(-telem_data.roll);
		render_translate(0, telem_data.pitch / M_PI * 180 * 5);
		shape_begin(&shape, GL_LINES);
		for(int i = -5; i <= 5; ++i) {
			shape_vertex(&shape, -100, i * 50);
			shape_vertex(&shape, 100, i * 50);
		}
		shape_end(&shape);
		shape_draw(&shape);

		shape_begin(&shape, GL_TRIANGLE_STRIP);
		shape_vertex(&shape, 0, 0);
		shape_vertex(&shape, 10, 10);
		shape_vertex(&shape, 20, 40);
		shape_vertex(&shape, 100, 0);
		shape_end(&shape);

		shape_draw(&shape);

		render_pop_matrix();

		eglSwapBuffers(egl_display, egl_surface);
	}
}
