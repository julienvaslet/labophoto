#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 a_Vertex;
in vec4 a_Color;
in vec2 a_TexCoord0;
out vec4 fontColor;
out vec2 texCoord0;

void main( void )
{
	fontColor = a_Color;
	texCoord0 = a_TexCoord0;
	gl_Position = projection_matrix * modelview_matrix * vec4( a_Vertex, 1.0 );
}

