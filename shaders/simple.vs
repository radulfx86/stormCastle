#version 300 es
layout (location=0) in vec2 pos;
layout (location=1) in vec2 texPos;

struct TexInfo {
    vec2 texPos;
    vec2 texSize;
    bool flip;
    vec2 texOrigin;
};

uniform TexInfo texInfo;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 outTexPos;
void main() {
    outTexPos = texInfo.texOrigin + (texInfo.texPos + texPos) * texInfo.texSize;
    if ( texInfo.flip )
    {
        gl_Position = projection * view * model * vec4(-pos.x, pos.y, 0, 1);
    }
    else
    {
        gl_Position = projection * view * model * vec4(pos, 0, 1);
    }
}
