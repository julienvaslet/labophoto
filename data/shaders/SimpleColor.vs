#version 130

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 a_Vertex;
in vec4 a_Color;
out vec4 color;

void main( void )
{
	color = a_Color;
	vec4 pos = modelview_matrix * vec4( a_Vertex, 1.0 );
	gl_Position = projection_matrix * pos;
}

