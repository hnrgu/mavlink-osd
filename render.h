#ifndef RENDER_H
#define RENDER_H

#define RENDER_STENCIL_DISABLE 0
#define RENDER_STENCIL_INSIDE 1
#define RENDER_STENCIL_OUTSIDE 2

extern float render_current_color[4];
extern float render_current_transform[3][3];

void render_push_matrix();
void render_pop_matrix();
void render_load_identity();
void render_set_color(float r, float g, float b, float a);
void render_transform(float transform[3][3]);
void render_translate(float x, float y);
void render_rotate(float rotate);
void render_scale(float x, float y);
void render_ortho(float left, float right, float bottom, float top);
void render_stencil_begin();
void render_stencil_end();
void render_stencil_mode(int mode);

#endif
