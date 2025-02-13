#version 100

// fCrosshatch.glsl
// License: MIT
// Author: pthrasher
// adapted by gre from https://gist.github.com/pthrasher/04fd9a7de4012cbb03f6


#ifdef GL_ES
// Set default precision to high
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


const vec2 center = vec2(0.5);
const float threshold = 3.0;
const float fadeEdge = 0.1;

float rand(vec2 co) {
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


vec4 transition(vec2 p) {
  float dist = distance(center, p) / threshold;
  float r = progress - min(rand(vec2(p.y, 0.0)), rand(vec2(0.0, p.x)));
  return mix(getFromColor(p), getToColor(p), mix(0.0, mix(step(dist, r), 1.0, smoothstep(1.0-fadeEdge, 1.0, progress)), smoothstep(0.0, fadeEdge, progress)));    
}


void
main(void) {
    gl_FragColor = transition(v_texcoord);
}
