#version 100

precision mediump float;

uniform mat3 u_transform;

attribute vec2 a_pos;

void main() {
    vec3 pos = u_transform * vec3(a_pos, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.z);
}
