#version 300 es
//precision lowp float;
precision mediump float;

uniform sampler2D tex;
uniform vec4 textColor;
in vec2 outTexPos;
out vec4 color;
void main() {
    color = texture(tex, outTexPos);
    if ( color.a < 1.0 )
    {
       // color = vec4(0,1,0,1);
        discard;
    }
    color = textColor;
}
