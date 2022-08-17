#version 100

precision mediump float;

uniform vec2 u_window_size;

#define INF (1.0 / 0.0)

void main() {
    float xlevel = floor((u_window_size.y - gl_FragCoord.y - 200.0) / 16.0);
    float xpow = xlevel < 0.0 ? INF : pow(2.0, xlevel);
    bool xstripe = mod(floor(gl_FragCoord.x / xpow + 0.01), 2.0) > 0.5;

    float ylevel = floor((gl_FragCoord.x - 200.0) / 16.0);
    float ypow = ylevel < 0.0 ? INF : pow(2.0, ylevel);
    bool ystripe = mod(floor((u_window_size.y - gl_FragCoord.y) / ypow + 0.01), 2.0) > 0.5;

    gl_FragColor = vec4(xstripe, ystripe, 0.0, 1.0);
}
