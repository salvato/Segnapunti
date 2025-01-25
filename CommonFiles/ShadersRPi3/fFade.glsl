#version 100

// fFade.glsl
// author: gre
// license: MIT

#ifdef GL_ES
// Set default precision to high
precision highp int;
precision highp float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;
varying vec2      v_texcoord;
uniform float     progress;


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


vec4 transition (vec2 uv) {
  return mix(
    getFromColor(uv),
    getToColor(uv),
    progress
  );
}

void
main(void) {
    gl_FragColor = transition(v_texcoord);
}
