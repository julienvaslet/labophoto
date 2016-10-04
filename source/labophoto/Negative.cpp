#include <labophoto/Negative.h>
#include <opengl/Vector.h>
#include <opengl/Matrix.h>
#include <cmath>

#ifdef DEBUG0
#include <tools/logger/Logger.h>
using namespace tools::logger;
#endif

namespace labophoto
{	
	Negative::Negative() : TexturedRectangle( false ), program(NULL), vertices(NULL), textureCoordinates(NULL), indices(NULL), invertColors(true), rotation(0.0f)
	{
		this->initializeRendering();
	}
	
	Negative::~Negative()
	{
		if( this->indices != NULL )
		{
			delete this->indices;
			this->indices = NULL;
		}
	
		if( this->textureCoordinates != NULL )
		{
			delete this->textureCoordinates;
			this->textureCoordinates = NULL;
		}
	
		if( this->vertices != NULL )
		{
			delete this->vertices;
			this->vertices = NULL;
		}
	
		if( this->program != NULL )
		{
			delete this->program;
			this->program = NULL;
		}
	}
	
	void Negative::initializeRendering()
	{
		if( this->program == NULL )
		{
			this->program = new Program();
			
			this->program->loadVertexShaderFile( "data/labophoto/Negative.vs" );
			this->program->loadFragmentShaderFile( "data/labophoto/Negative.fs" );
			
			this->program->link( true );
		}
		
		if( this->vertices == NULL )
			this->vertices = new ArrayBufferObject();
			
		if( this->textureCoordinates == NULL )
			this->textureCoordinates = new ArrayBufferObject();
			
		if( this->indices == NULL )
			this->indices = new ElementArrayBufferObject();
	}
	
	void Negative::render( bool cropped )
	{
		vector<Point3D> vertices;
		vector<Point2D> textureCoordinates;
		vector<unsigned short int> indices;
		
		Texture2D * texture = this->getTile()->getTexture();
		unsigned int textureUnit = 0;
		
		if( this->program != NULL && texture != NULL )
		{
			// Push matrix
			Matrix modelview = Matrix::modelview;
			
			Rectangle::prepareRendering( vertices, indices );
			
			Matrix transformation = Matrix::identity();
			Matrix rotate = Matrix::rotationZ( this->rotation );

			if( cropped )
			{
				Matrix moveToCenter = Matrix::translation( texture->getWidth() / 2.0f, texture->getHeight() / 2.0f, 0.0f );
				Matrix moveBack = Matrix::translation( texture->getWidth() / -2.0f, texture->getHeight() / -2.0f, 0.0f );
				
				transformation.multiply( moveToCenter );
				transformation.multiply( rotate );
				transformation.multiply( moveBack );
				
				Vector positionUpLeft( this->view.getOrigin().getX(), this->view.getOrigin().getY(), this->view.getOrigin().getZ() );
				Vector positionUpRight( this->view.getOrigin().getX() + static_cast<float>( this->view.getWidth() ), this->view.getOrigin().getY(), this->view.getOrigin().getZ() );
				Vector positionBottomRight( this->view.getOrigin().getX() + static_cast<float>( this->view.getWidth() ), this->view.getOrigin().getY() + static_cast<float>( this->view.getHeight() ), this->view.getOrigin().getZ() );
				Vector positionBottomLeft( this->view.getOrigin().getX(), this->view.getOrigin().getY() + static_cast<float>( this->view.getHeight() ), this->view.getOrigin().getZ() );
			
				positionUpLeft *= transformation;
				positionUpRight *= transformation;
				positionBottomRight *= transformation;
				positionBottomLeft *= transformation;
			
				textureCoordinates.push_back( Point2D( positionUpLeft.getX() / static_cast<float>( texture->getWidth() ), 1.0f - ( positionUpLeft.getY() / static_cast<float>( texture->getHeight() ) ) ) );
				textureCoordinates.push_back( Point2D( positionUpRight.getX() / static_cast<float>( texture->getWidth() ), 1.0f - ( positionUpRight.getY() / static_cast<float>( texture->getHeight() ) ) ) );
				textureCoordinates.push_back( Point2D( positionBottomRight.getX() / static_cast<float>( texture->getWidth() ), 1.0f - ( positionBottomRight.getY() / static_cast<float>( texture->getHeight() ) ) ) );
				textureCoordinates.push_back( Point2D( positionBottomLeft.getX() / static_cast<float>( texture->getWidth() ), 1.0f - ( positionBottomLeft.getY() / static_cast<float>( texture->getHeight() ) ) ) );
			}
			else
			{
				Matrix rotate = Matrix::rotationZ( this->rotation );
				Matrix moveToCenter = Matrix::translation( this->getX() + (this->getWidth() / 2.0f), this->getY() + (this->getHeight() / 2.0f), 0.0f );
				Matrix moveBack = Matrix::translation( -1 * this->getX() + (this->getWidth() / -2.0f), -1 * this->getY() + (this->getHeight() / -2.0f), 0.0f );
				
				transformation.multiply( moveToCenter );
				transformation.multiply( rotate );
				transformation.multiply( moveBack );
				
				// Matrix sent to OpenGL (columnMajor)
				Matrix::modelview = transformation;
				
				textureCoordinates.push_back( Point2D( 0.0f, 1.0f ) );
				textureCoordinates.push_back( Point2D( 1.0f, 1.0f ) );
				textureCoordinates.push_back( Point2D( 1.0f, 0.0f ) );
				textureCoordinates.push_back( Point2D( 0.0f, 0.0f ) );
			}
			
			this->program->use( true );
			
			this->vertices->setData( vertices );
			this->textureCoordinates->setData( textureCoordinates );
			this->indices->setData( indices );
			
			this->program->sendUniform( "projection_matrix", Matrix::projection, false );
			this->program->sendUniform( "modelview_matrix", Matrix::modelview, false );
			this->program->sendUniform( "texture0", *texture, textureUnit );
			this->program->sendUniform( "colorInversion", this->invertColors ? 1 : 0 );
			this->program->sendVertexPointer( "a_Vertex", this->vertices );
			this->program->sendTextureCoordinatesPointer( "a_TexCoord0", this->textureCoordinates );

			this->indices->draw( OpenGL::Triangles );
			
			// Pop matrix
			Matrix::modelview = modelview;
		}
	}
		
	bool Negative::areColorsInverted()
	{
		return this->invertColors;
	}
	
	void Negative::setColorInversion( bool status )
	{
		this->invertColors = status;
	}

	unsigned int Negative::getNegativeWidth() const
	{
		unsigned int width = 0;
		
		if( this->tile->getTexture() != NULL )
			width = this->tile->getTexture()->getWidth();
		
		return width;
	}
	
	unsigned int Negative::getNegativeHeight() const
	{
		unsigned int height = 0;
		
		if( this->tile->getTexture() != NULL )
			height = this->tile->getTexture()->getHeight();
		
		return height;
	}
	
	float Negative::getRotation()
	{
		return this->rotation;
	}
	
	void Negative::setRotation( float angle )
	{
		if( angle >= 360.0f )
			angle = fmod( angle, 360.0f );
		
		this->rotation = angle;
	}
	
	const Rectangle& Negative::getView() const
	{
		return this->view;
	}
	
	void Negative::setView( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
	{
		this->view.getOrigin().moveTo( x, y, 0.0f );
		this->view.resize( width, height );
	}
}

