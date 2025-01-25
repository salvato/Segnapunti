#version 100


attribute vec4 vPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void
main() {
    gl_Position = lightSpaceMatrix * model * vPosition;
}
