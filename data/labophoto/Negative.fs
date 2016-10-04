#version 130

uniform sampler2D texture0;
uniform bool colorInversion;

in vec2 texCoord0;
out vec4 outColor;

void main(void)
{
	vec4 color = texture( texture0, texCoord0 );
	
	if( colorInversion )
		outColor = vec4( 1.0 - color.r, 1.0 - color.g, 1.0 - color.b, color.a );
	else
		outColor = color;
}

