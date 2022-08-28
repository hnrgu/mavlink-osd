#include <assert.h>
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


const char *mode_strs[] = {
    [0] = "MANUAL",
    [1] = "CIRCLE",
    [2] = "STABILIZE",
    [3] = "TRAINING",
    [4] = "ACRO",
    [5] = "FLY_BY_WIRE_A",
    [6] = "FLY_BY_WIRE_B",
    [7] = "CRUISE",
    [8] = "AUTOTUNE",
    [10] = "AUTO",
    [11] = "RTL",
    [12] = "LOITER",
    [13] = "TAKEOFF",
    [14] = "AVOID_ADSB",
    [15] = "GUIDED",
    [16] = "INITIALISING",
    [17] = "QSTABILIZE",
    [18] = "QHOVER",
    [19] = "QLOITER",
    [20] = "QLAND",
    [21] = "QRTL",
    [22] = "QAUTOTUNE",
    [23] = "QACRO",
    [24] = "THERMAL",
    [25] = "LOITER_ALT_QLAND",
};

#ifdef EGL_NO_X11
#define MAX_DRM_DEVICES 4
#include "bcm_host.h"
#else
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "telem_data.h"
#include "shader.h"
#include "texture.h"
#include "font.h"
#include "render.h"
#include "shape.h"
#include "matrix.h"
#include "utils.h"
#include "layout.h"
#include "widgets/widget_text.h"
#include "widgets/widget_bank_indicator.h"
#include "widgets/widget_attitude.h"
#include "widgets/widget_heading.h"
#include "widgets/widget_tape.h"

volatile int running = 1;

#define RENDER_WIDTH 720
#define RENDER_HEIGHT 576
#define SCALE_FACTOR 1

static void log_egl_details(EGLDisplay egl_display, EGLConfig egl_conf) {
	printf("[ui] EGL Client APIs: %s\n", eglQueryString(egl_display, EGL_CLIENT_APIS));
	printf("[ui] EGL Vendor: %s\n", eglQueryString(egl_display, EGL_VENDOR));
	printf("[ui] EGL Version: %s\n", eglQueryString(egl_display, EGL_VERSION));
	printf("[ui] EGL Extensions: %s\n", eglQueryString(egl_display, EGL_EXTENSIONS));

	int i = -1;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_CONFIG_ID, &i);
	printf("[ui] EGL_CONFIG_ID = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_RED_SIZE, &i);
	printf("[ui] EGL_RED_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_GREEN_SIZE, &i);
	printf("[ui] EGL_GREEN_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_BLUE_SIZE, &i);
	printf("[ui] EGL_BLUE_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_ALPHA_SIZE, &i);
	printf("[ui] EGL_ALPHA_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_DEPTH_SIZE, &i);
	printf("[ui] EGL_DEPTH_SIZE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_LEVEL, &i);
	printf("[ui] EGL_LEVEL = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_NATIVE_RENDERABLE, &i);
	printf("[ui] EGL_NATIVE_RENDERABLE = %s\n", i ? "EGL_TRUE" : "EGL_FALSE");

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_NATIVE_VISUAL_TYPE, &i);
	printf("[ui] EGL_NATIVE_VISUAL_TYPE = %d\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_RENDERABLE_TYPE, &i);
	printf("[ui] EGL_RENDERABLE_TYPE = 0x%04x\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_SURFACE_TYPE, &i);
	printf("[ui] EGL_SURFACE_TYPE = 0x%04x\n", i);

	i = 0;
	eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_TYPE, &i);
	if (i == EGL_TRANSPARENT_RGB) {
		printf("[ui] EGL_TRANSPARENT_TYPE = EGL_TRANSPARENT_RGB\n");

		i = 0;
		eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_RED_VALUE, &i);
		printf("[ui] EGL_TRANSPARENT_RED = 0x%02x\n", i);

		i = 0;
		eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_GREEN_VALUE, &i);
		printf("[ui] EGL_TRANSPARENT_GREEN = 0x%02x\n", i);

		i = 0;
		eglGetConfigAttrib(egl_display, egl_conf, EGL_TRANSPARENT_BLUE_VALUE, &i);
		printf("[ui] EGL_TRANSPARENT_BLUE = 0x%02x\n", i);
	} else {
		printf("[ui] EGL_TRANSPARENT_TYPE = EGL_NONE\n");
	}
}

void start_ui_thread(void *arg) {
#ifndef EGL_NO_X11
	Display *x_display = XOpenDisplay(NULL);
	if (x_display == NULL) {
		printf("[ui] cannot connect to X server\n");
		return;
	}

	Window root = DefaultRootWindow(x_display);   // get the root window (usually the whole screen)
	int root_x, root_y;
	unsigned int root_w, root_h, root_border_width, root_depth;
	Window root_again;
	XGetGeometry(x_display, root, &root_again, &root_x, &root_y, &root_w, &root_h, &root_border_width, &root_depth);
	printf("[ui] Matching X11 root window geometry: +%d,%d %dx%d border %d, %dbpp\n\n",
	       root_x, root_y, root_w, root_h, root_border_width, root_depth);

	XSetWindowAttributes  swa;
	swa.event_mask  =  ExposureMask | ButtonPressMask | KeyPressMask;

	Window win = XCreateWindow(x_display, root,
	                           0, 0, 873, 464, 0,
	                           CopyFromParent, InputOutput,
	                           CopyFromParent, CWEventMask,
	                           &swa);

    // Redirect Close
    Atom atomWmDeleteWindow = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display, win, &atomWmDeleteWindow, 1);

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
#else
	EGL_DISPMANX_WINDOW_T nativewindow;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_ELEMENT_HANDLE_T dispman_element;

	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
	bcm_host_init();

	graphics_get_display_size(0 /* LCD */, &nativewindow.width, &nativewindow.height);

	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
	dispman_update = vc_dispmanx_update_start( 0 );
	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
			5/*layer*/, &dst_rect, 0/*src*/,
			&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

	nativewindow.element = dispman_element;
	vc_dispmanx_update_submit_sync( dispman_update );

#endif

	///////  the egl part  //////////////////////////////////////////////////////////////////
	//  egl provides an interface to connect the graphics related functionality of openGL ES
	//  with the windowing interface and functionality of the native operation system (X11
	//  in our case.)
	eglBindAPI(EGL_OPENGL_ES2_BIT);
#ifndef EGL_NO_X11
	EGLDisplay egl_display = eglGetDisplay((EGLNativeDisplayType) x_display);
#else
	EGLDisplay egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif
	if (egl_display == EGL_NO_DISPLAY) {
		printf("[ui] Got no EGL display.\n");
		return;
	}

	EGLint egl_version_major, egl_version_minor;
	if (!eglInitialize(egl_display, &egl_version_major, &egl_version_minor)) {
		printf("[ui] Unable to initialize EGL\n");
		return;
	}
	printf("[ui] Initialized EGL version %d.%d\n", egl_version_major, egl_version_minor);

	EGLint egl_config_constraints[] = {
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_STENCIL_SIZE, 8,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_CONFIG_CAVEAT, EGL_NONE,
//			EGL_SAMPLES, 4,
			EGL_NONE
	};

	EGLConfig egl_conf;
	EGLint num_config;
	if (!eglChooseConfig(egl_display, egl_config_constraints, &egl_conf, 1, &num_config)) {
		printf("[ui] Failed to choose config (eglError: %s)\n", eglGetError());
		return;
	}

	if (num_config != 1) {
		printf("[ui] Didn't get exactly one config, but %d\n", num_config);
		return;
	}

#ifndef EGL_NO_X11
	EGLSurface egl_surface = eglCreateWindowSurface(egl_display, egl_conf, win, NULL);
#else
	EGLSurface egl_surface = eglCreateWindowSurface(egl_display, egl_conf, &nativewindow, NULL);
#endif
	if (egl_surface == EGL_NO_SURFACE) {
		printf("[ui] Unable to create EGL surface (eglError: %s)\n", eglGetError());
		return;
	}

	// egl-contexts collect all state descriptions needed required for operation
	EGLint ctxattr[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
	};
	EGLContext egl_context = eglCreateContext(egl_display, egl_conf, EGL_NO_CONTEXT, ctxattr);
	if (egl_context == EGL_NO_CONTEXT) {
		printf("[ui] Unable to create EGL context (eglError: %s)\n", eglGetError());
		return;
	}

	//// associate the egl-context with the egl-surface
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

	log_egl_details(egl_display, egl_conf);

	EGLint queriedRenderBuffer;
	if (eglQueryContext(egl_display, egl_context, EGL_RENDER_BUFFER, &queriedRenderBuffer)) {
		switch (queriedRenderBuffer) {
			case EGL_SINGLE_BUFFER: printf("[ui] Render Buffer: EGL_SINGLE_BUFFER\n"); break;
			case EGL_BACK_BUFFER: printf("[ui] Render Buffer: EGL_BACK_BUFFER\n"); break;
			case EGL_NONE: printf("[ui] Render Buffer: EGL_NONE\n"); break;
			default: printf("[ui] Render Buffer: unknown value %d\n", queriedRenderBuffer); break;
		}
	} else {
		printf("[ui] Failed to query EGL_RENDER_BUFFER: %d\n", eglGetError());
	}

	if (!eglSwapInterval(egl_display, 1)) {
		printf("[ui] eglSwapInterval failed: %d\n", eglGetError());
	} else {
		printf("[ui] Set swap interval\n");
	}

#ifdef MEASURE
    struct program measure_program;
    SHADER_LOAD(&measure_program, shaders_measure_vert, shaders_measure_frag);
#endif

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	font_init();
	shape_init();
	layout_init();

	float identity[3][3];
    float transform[3][3];
    matrix_identity(identity);

	// Attitude indicator
	struct widget_attitude_indicator attitude;
	widget_attitude_indicator_init(&attitude);
	layout_add(&attitude, 0.5, 0.5, 0, 0, identity);

	// Text
	struct widget_text text;
	widget_text_init(&text);
	widget_text_set(&text, "test");
	layout_add(&text, 0, 0, 0, 0, identity);

//	struct widget_text data;
//	widget_text_init(&data);
//	data.color[0] = 0;
//	data.align = FONT_ALIGN_H_CENTER | FONT_ALIGN_V_CENTER;
//	layout_add(&data, 0.5, 0.5, 0, 0, identity);

	// Bank indicator
	struct widget_bank_indicator bank;
	widget_bank_indicator_init(&bank);
	matrix_scale_multiply(100, 100, identity, transform);
	layout_add(&bank, 0.5, 0.25, 0, 0, transform);

	// Heading indicator
	struct widget_heading_indicator heading;
	widget_heading_indicator_init(&heading);
	layout_add(&heading, 0.5, 1, 0, 50, identity);

    // Armed status
    struct widget_text armed;
    widget_text_init(&armed);
    armed.align = FONT_ALIGN_H_RIGHT | FONT_ALIGN_V_TOP;
    layout_add(&armed, 0, 0, 150, 20, identity);

    // GS text
    struct widget_text gs;
    widget_text_init(&gs);
    gs.align = FONT_ALIGN_H_RIGHT | FONT_ALIGN_V_BOTTOM;
    layout_add(&gs, 0, 1, 150, -20, identity);

    // Flight mode
    struct widget_text mode;
    widget_text_init(&mode);
    mode.align = FONT_ALIGN_H_RIGHT | FONT_ALIGN_V_TOP;
    layout_add(&mode, 1, 0, -75, 20, identity);

    // VS text
    struct widget_text vs;
    widget_text_init(&vs);
    vs.align = FONT_ALIGN_H_LEFT | FONT_ALIGN_V_BOTTOM;
    layout_add(&vs, 1, 1, -150, -20, identity);

    // GPS text
    struct widget_text gps;
    widget_text_init(&gps);
    gps.align = FONT_ALIGN_H_RIGHT | FONT_ALIGN_V_TOP;
    layout_add(&gps, 1, 0, 0, 0, identity);

	// Altitude tape
	struct widget_tape altitude;
	altitude.decimals = 1;
	altitude.num_marks = 9;
	altitude.mark_spacing = 40;
	altitude.mark_length = 15;
	altitude.mark_value = 5;
	altitude.label_period = 2;
	altitude.label_gap = 5;
	altitude.width = 120;
	altitude.arrow_width = 8;
	altitude.arrow_height = 10;
	altitude.indicator_width = 72;
	altitude.indicator_height = 32;
	altitude.indicator_gap = 3;
	altitude.direction = 0;
	widget_tape_init(&altitude);

	layout_add(&altitude, 1, 0.5, -150, 0, identity);

	// Air speed tape
	struct widget_tape airspeed;
	airspeed.decimals = 1;
	airspeed.num_marks = 9;
	airspeed.mark_spacing = 40;
	airspeed.mark_length = 15;
	airspeed.mark_value = 10;
	airspeed.label_period = 2;
	airspeed.label_gap = 5;
	airspeed.width = 120;
	airspeed.arrow_width = 8;
	airspeed.arrow_height = 10;
	airspeed.indicator_width = 72;
	airspeed.indicator_height = 32;
	airspeed.indicator_gap = 3;
	airspeed.direction = 1;
	widget_tape_init(&airspeed);

    layout_add(&airspeed, 0, 0.5, 150, 0, identity);

	glLineWidth(2 * SCALE_FACTOR);

    int surf_width, surf_height;

    int overscan_left, overscan_top, overscan_right, overscan_bottom;
    float scale_x, scale_y;

    uint64_t prev_time = 0;

	// render loop
#ifdef EGL_NO_X11
	struct gbm_bo *bo = NULL;
#endif
	unsigned long long last_fps = 0;
	while (running) {
        uint64_t cur_time = get_monotonic_time();

        double fps = 1e9 / (cur_time - prev_time);
        prev_time = cur_time;
#ifndef EGL_NO_X11
        if(XPending(x_display) > 0) {
            XEvent event;
            XNextEvent(x_display, &event);
            if(event.type == ClientMessage) {
                if(event.xclient.data.l[0] == atomWmDeleteWindow) {
                    break;
                }
            }
        }

		XWindowAttributes gwa;
		XGetWindowAttributes(x_display, win, &gwa);
        surf_width = gwa.width;
        surf_height = gwa.height;
        overscan_left = 0;
        overscan_top = 0;
        overscan_right = 0;
        overscan_bottom = 0;
//        scale_x = 0.69;
        scale_x = 1;
        scale_y = 1;
#else
        surf_width = RENDER_WIDTH;
        surf_height = RENDER_HEIGHT;
        overscan_left = 7;
        overscan_top = 109;
        overscan_right = 16;
        overscan_bottom = 3;
        scale_x = 0.8;
        scale_y = 1;
#endif

        int physical_width = surf_width - overscan_left - overscan_right;
        int physical_height = surf_height - overscan_top - overscan_bottom;

//        glViewport(0, 0, surf_width, surf_height);
//        glClearColor(1.0, 0.0, 1.0, 1.0);
//        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(overscan_left, overscan_bottom, physical_width, physical_height);

        int logical_width = physical_width / scale_x;
        int logical_height = physical_height / scale_y;

		render_load_identity();
		render_ortho(0, logical_width, logical_height, 0);

        layout_set_logical_size(logical_width, logical_height);

        char buf[2048];
	if(get_monotonic_time() - last_fps > 1000000000LLU) {
		sprintf(buf, "%.0f", fps);
		widget_text_set(&text, buf);
		last_fps = get_monotonic_time();
	}


//		sprintf(buf,
//				"Attitude: %f %f %f\n"
//				"GPS: %d %d\n"
//				"Airspeed: %f\n"
//				"Groundspeed: %f\n"
//				"Altitude: %f\n"
//				"Climb rate: %f",
//				telem_data.pitch, telem_data.roll, (telem_data.yaw) / M_PI * 180,
//				telem_data.lat, telem_data.lon,
//				telem_data.airspeed,
//				telem_data.groundspeed,
//				telem_data.altitude,
//				telem_data.climbrate);
//		widget_text_set(&data, buf);

		bank.bank_angle = telem_get_180(TELEM_ROLL);
		attitude.attitude_pitch = telem_get_180(TELEM_PITCH);
		attitude.attitude_roll = telem_get_180(TELEM_ROLL);
		heading.heading = telem_get_360(TELEM_YAW);

		altitude.value = telem_get(TELEM_ALTITUDE);
		airspeed.value = telem_get(TELEM_AIRSPEED);

        if (telem_get_raw(TELEM_ARMED) > 0.5f) {
            widget_text_set(&armed, "ARMED");
        } else {
            widget_text_set(&armed, "DISARMED");
        }

        int flight_mode = round(telem_get_raw(TELEM_MODE));
        const char *mode_str = mode_strs[flight_mode];
        widget_text_set(&mode, mode_str);

        sprintf(buf, "GS %.0f", telem_get(TELEM_GROUNDSPEED));
        widget_text_set(&gs, buf);

        sprintf(buf, "VS %.0f", telem_get(TELEM_CLIMBRATE));
        widget_text_set(&vs, buf);

        sprintf(buf, "%.5f %.5f", telem_get(TELEM_LAT), telem_get(TELEM_LON));
        widget_text_set(&gps, buf);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		layout_render();

#ifdef MEASURE
        shader_enable(&measure_program);

        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        GLfloat buffer_data[] = {
            -2.0f, -2.0f,
             2.0f, -2.0f,
            -2.0f,  2.0f,
             2.0f,  2.0f
        };
        glUniform2f(shader_get_uniform(&measure_program, "u_window_size"), surf_width, surf_height);
        glBufferData(GL_ARRAY_BUFFER, sizeof(buffer_data), buffer_data, GL_STATIC_DRAW);
        glVertexAttribPointer(shader_get_attrib(&measure_program, "a_pos"), 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(shader_get_attrib(&measure_program, "a_pos"));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDeleteBuffers(1, &buffer);
#endif

        if(eglSwapBuffers(egl_display, egl_surface) != EGL_TRUE) {
            return;
        }
	}
}

void stop_ui_thread() {
    running = 0;
}
