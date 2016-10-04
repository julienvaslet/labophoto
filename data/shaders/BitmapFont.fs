#version 130

uniform sampler2D texture0;

in vec2 texCoord0;
in vec4 fontColor;
out vec4 fragColor;

void main(void)
{
	vec4 textureColor = texture2D( texture0, texCoord0 );
	
	if( textureColor.rgb != vec3( 1.0, 1.0, 1.0 ) )
		fragColor = vec4( textureColor.rgb, textureColor.r ) * fontColor;

	else
		fragColor = textureColor * fontColor;
}

