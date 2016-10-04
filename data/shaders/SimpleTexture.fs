#version 130

uniform sampler2D texture0;

in vec2 texCoord0;
out vec4 outColor;

void main(void)
{
	outColor = texture( texture0, texCoord0 );
}

