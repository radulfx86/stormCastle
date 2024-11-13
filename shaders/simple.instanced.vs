#version 300 es
layout (location=0) in vec2 pos;
layout (location=1) in vec2 texPos;

struct InstanceInfo {
    vec2 pos;
    vec2 texPos;
    vec2 texSize;
    bool flip;
    bool enabled;
    vec2 texOrigin;
};
#define MAX_ELEMENTS (150u)

uniform InstanceInfo texInfo[MAX_ELEMENTS];
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 outTexPos;
void main() {
    if ( ! texInfo[gl_InstanceID].enabled )
    {
        return;
    }
    outTexPos = texInfo[gl_InstanceID].texOrigin + (texInfo[gl_InstanceID].texPos + texPos) * texInfo[gl_InstanceID].texSize;
    //outTexPos = texInfo[gl_InstanceID].texOrigin + texInfo[gl_InstanceID].texPos * texInfo[gl_InstanceID].texSize;
    //outTexPos = texPos * texInfo[gl_InstanceID].texSize;
    if ( texInfo[gl_InstanceID].flip )
    {
        vec2 tilePos = vec2(-pos.x, pos.y) + texInfo[gl_InstanceID].pos;
        gl_Position = projection * view * model * vec4(tilePos, 0, 1);
    }
    else
    {
        vec2 tilePos = pos + texInfo[gl_InstanceID].pos;
        gl_Position = projection * view * model * vec4(tilePos, 0, 1);
    }
}