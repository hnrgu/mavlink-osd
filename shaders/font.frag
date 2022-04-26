#version 100

precision mediump float;

uniform vec4 u_color;
uniform sampler2D u_texture;

varying vec2 uv;

void main() {
    gl_FragColor = u_color * texture2D(u_texture, uv);
}
