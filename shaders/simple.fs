#version 300 es
//precision lowp float;
precision mediump float;

uniform sampler2D tex;
in vec2 outTexPos;
out vec4 color;
void main() {
    color = texture(tex, outTexPos);
    if ( color.a < 1.0 )
    {
       // color = vec4(1,0,0,1);
        discard;
    }
    if ( color.rgb == vec3(0) )
    {
        color = vec4(.5,.7,1,1);
    }
}
