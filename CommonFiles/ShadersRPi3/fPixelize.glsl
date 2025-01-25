#version 100

// fPixelize.glsl
// Author: gre
// License: MIT
// forked from https://gist.github.com/benraziel/c528607361d90a072e98


#ifdef GL_ES
// Set default precision to medium
precision highp int;
precision highp float;
#endif


uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float progress;

varying vec2 v_texcoord;



vec4
getFromColor(vec2 p) {
    vec4 tempc;
    tempc = texture2D(texture0, vec2(p.x, p.y));
    return tempc;
}

vec4
getToColor(vec2 p) {
    vec4 tempc;
    tempc = texture2D(texture1, vec2(p.x, p.y));
    return tempc;
}


// minimum number of squares (when the effect is at its higher level)
const ivec2 squaresMattribute = ivec2(20);
// zero disable the stepping
const int steps = 50;

float d = 0.0;
float dist =0.0;
vec2 squareSize = vec2(1.0, 1.0);
float squaresMin= 50.0;

vec4 transition(vec2 uv) {
  vec2 p = dist>0.0 ? (floor(uv / squareSize) + 0.5) * squareSize : uv;
  return mix(getFromColor(p), getToColor(p), progress);
}

void
main(void) {
    d = min(progress, 1.0 - progress);
    dist = steps>0 ? ceil(d * float(steps)) / float(steps) : d;
    squareSize = 2.0 * dist / vec2(squaresMin);
    gl_FragColor = transition(v_texcoord);
}
