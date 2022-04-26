#version 100

precision mediump float;

uniform vec2 u_screen_size;

uniform vec2 u_offset;
uniform vec2 u_scale;

attribute vec2 a_pos;
attribute vec2 a_uv;

varying vec2 uv;

void main() {
  gl_Position = vec4(u_scale * a_pos / u_screen_size + u_offset * 2.0 - 1.0, 0, 1);
  gl_Position.y = -gl_Position.y;
  uv = a_uv / vec2(16.0, 6.0);
//  uv.y = 1.0 - uv.y;
}
