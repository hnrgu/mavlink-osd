#version 100

precision mediump float;

uniform mat3 u_transform;

attribute vec2 a_pos;
attribute vec2 a_uv;
attribute float a_char;

varying vec2 uv;

void main() {
    float a_char_float = a_char + 0.5;
    vec2 offset = vec2(floor(mod(a_char_float, 16.0)), floor(a_char_float / 16.0) - 2.0);
    vec3 pos = u_transform * vec3(a_pos, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.z);
    uv = (a_uv + offset) / vec2(16.0, 6.0);
}
