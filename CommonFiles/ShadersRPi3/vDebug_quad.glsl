#version 100

attribute vec3 vPosition;
attribute vec2 vTexture;

varying vec2 TexCoords;

void
main() {
    TexCoords = vTexture;
    gl_Position = vec4(vPosition, 1.0);
}
