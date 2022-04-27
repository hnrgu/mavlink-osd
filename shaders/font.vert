#version 100

precision mediump float;

uniform mat3 u_transform;

attribute vec2 a_pos;
attribute vec2 a_uv;

varying vec2 uv;

void main() {
    vec3 pos = u_transform * vec3(a_pos, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.z);
    uv = a_uv / vec2(16.0, 6.0);
}
