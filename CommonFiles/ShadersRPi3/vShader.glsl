#version 100
// vShader.glsl

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

// attribute - linkage between a vertex shader and OpenGL ES for per-vertex data
// uniform   - value does not change across the primitive being processed, uniforms
//             form the linkage between a shader, OpenGL ES, and the application
// varying   - linkage between a vertex shader and a fragment shader for interpolated
//             data

uniform mat4   mvp_matrix;
attribute vec2 a_texcoord;
attribute vec4 a_position;
varying vec2   v_texcoord;


void
main() {
    // Calculate vertex position attribute screen space
    gl_Position = mvp_matrix * a_position;
    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
}
