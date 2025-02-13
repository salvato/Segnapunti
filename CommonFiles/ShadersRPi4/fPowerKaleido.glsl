#version 300 es

// fPowerKaleido.glsl
// Name: Power Kaleido
// Author: Boundless
// License: MIT

#ifdef GL_ES
// Set default precision to high
precision highp int;
precision highp float;
#endif


#define PI 3.14159265358979
const float rad = 120.; // change this value to get different mirror effects
const float deg = rad / 180. * PI;

const float scale = 2.0;
const float z = 1.5;
const float ratio = 0.85;
float dist = scale / 10.;
const float speed = 5.0;


uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float progress;
in vec2 v_texcoord;
out vec4 glFragColor;


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


vec2
refl(vec2 p,vec2 o,vec2 n) {
    return 2.0*o+2.0*n*dot(p-o,n)-p;
}


vec2
rot(vec2 p, vec2 o, float a) {
    float s = sin(a);
    float c = cos(a);
    return o + mat2(c, -s, s, c) * (p - o);
}

vec4
mainImage(vec2 uv) {
    vec2 uv0 = uv;
    uv -= 0.5;
    uv.x *= ratio;
    uv *= z;
    uv = rot(uv, vec2(0.0), progress*speed);
    // uv.x = fract(uv.x/l/3.0)*l*3.0;
    float theta = progress*6.+PI/.5;
    for(int iter = 0; iter < 10; iter++) {
        for(float i = 0.; i < 2. * PI; i+=deg) {
            float ts = sign(asin(cos(i))) == 1.0 ? 1.0 : 0.0;
            if(((ts == 1.0) && (uv.y-dist*cos(i) > tan(i)*(uv.x+dist*+sin(i)))) || ((ts == 0.0) && (uv.y-dist*cos(i) < tan(i)*(uv.x+dist*+sin(i))))) {
                uv = refl(vec2(uv.x+sin(i)*dist*2.,uv.y-cos(i)*dist*2.), vec2(0.,0.), vec2(cos(i),sin(i)));
            }
        }
    }
    uv += 0.5;
    uv = rot(uv, vec2(0.5), progress*-speed);
    uv -= 0.5;
    uv.x /= ratio;
    uv += 0.5;
    uv = 2.*abs(uv/2.-floor(uv/2.+0.5));
    vec2 uvMix = mix(uv,uv0,cos(progress*PI*2.)/2.+0.5);
    vec4 color = mix(getFromColor(uvMix),getToColor(uvMix),cos((progress-1.)*PI)/2.+0.5);
    return color;
    
}

vec4
transition (vec2 uv) {
    vec4 color = mainImage(uv);
    return color;
}


void
main(void) {
    glFragColor = transition(v_texcoord);
}
