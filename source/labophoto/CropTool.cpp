#include <SDL2/SDL.h>

#include <labophoto/CropTool.h>
#include <opengl/Texture2D.h>
#include <opengl/Vector.h>
#include <cmath>

#ifdef DEBUG0
#include <tools/logger/Logger.h>
using namespace tools::logger;
#endif

#define CROP_RECTANGLE_BORDER_COLOR		"ffffff"
#define CROP_RECTANGLE_BORDER_SIZE		1.0f
#define CROP_RECTANGLE_BACKGROUND_COLOR	"00000088"
#define CROP_TOOL_MOUSE_MARGIN			5.0f

namespace labophoto
{
	CropTool::CropTool( Negative * negative ) : ColoredRectangle(), negative(negative), active(false), mouseCropPosition(MouseCropPosition::Outside), mousePosition(0.0f,0.0f), croppingStarted(false), rotationStarted(false), originalRotation(0.0f)
	{
	}
	
	CropTool::~CropTool()
	{
	}
	
	MouseCropPosition CropTool::getCropPosition( const Point2D& point )
	{
		MouseCropPosition position = MouseCropPosition::Outside;
		unsigned int textureWidth = this->negative->getNegativeWidth();

		if( textureWidth > 0 )
		{
			float ratio = static_cast<float>( this->getWidth() ) / static_cast<float>( textureWidth );
			float margin = CROP_TOOL_MOUSE_MARGIN / ratio;
			
			Rectangle view;
			view.getOrigin().moveTo( this->getOrigin().getX() + round( this->negative->getView().getX() * ratio ), this->getOrigin().getY() + round( this->negative->getView().getY() * ratio ), this->getOrigin().getZ() );
			view.resize( round( this->negative->getView().getWidth() * ratio ), round( this->negative->getView().getHeight() * ratio ) );
		
			if( this->isInCollision( point ) )
			{
				if( point.getX() - margin < view.getX() )
				{
					if( point.getY() - margin < view.getY() )
						position = MouseCropPosition::NorthWest;
						
					else if( point.getY() > view.getY() + view.getHeight() - margin )
						position = MouseCropPosition::SouthWest;
						
					else
						position = MouseCropPosition::West;
				}
				else if( point.getX() > view.getX() + view.getWidth() - margin )
				{
					if( point.getY() - margin < view.getY() )
						position = MouseCropPosition::NorthEast;
						
					else if( point.getY() > view.getY() + view.getHeight() - margin )
						position = MouseCropPosition::SouthEast;
						
					else
						position = MouseCropPosition::East;
				}
				else
				{
					if( point.getY() - margin < view.getY() )
						position = MouseCropPosition::North;
						
					else if( point.getY() > view.getY() + view.getHeight() - margin )
						position = MouseCropPosition::South;
						
					else
						position = MouseCropPosition::Inside;
				}
			}
		}
		
		return position;
	}
	
	bool CropTool::isActive() const
	{
		return this->active;
	}
	
	void CropTool::activate( bool status )
	{
		this->active = status;
		
		if( !this->active )
			this->setMousePosition( MouseCropPosition::Outside );
	}
	
	void CropTool::setMousePosition( MouseCropPosition position )
	{
		if( position != this->mouseCropPosition )
		{
			this->mouseCropPosition = position;
			SDL_SystemCursor cursorId = SDL_SYSTEM_CURSOR_ARROW;
			
			switch( this->mouseCropPosition )
			{
				case MouseCropPosition::Inside:
				{
					cursorId = SDL_SYSTEM_CURSOR_SIZEALL;
					break;
				}
				
				case MouseCropPosition::NorthWest:
				case MouseCropPosition::SouthEast:
				{
					cursorId = SDL_SYSTEM_CURSOR_SIZENWSE;
					break;
				}
				
				case MouseCropPosition::North:
				case MouseCropPosition::South:
				{
					cursorId = SDL_SYSTEM_CURSOR_SIZENS;
					break;
				}
				
				case MouseCropPosition::NorthEast:
				case MouseCropPosition::SouthWest:
				{
					cursorId = SDL_SYSTEM_CURSOR_SIZENESW;
					break;
				}
				
				case MouseCropPosition::East:
				case MouseCropPosition::West:
				{
					cursorId = SDL_SYSTEM_CURSOR_SIZEWE;
					break;
				}
				
				default:
				case MouseCropPosition::Outside:
				{
					cursorId = SDL_SYSTEM_CURSOR_ARROW;
					break;
				}
			}
			
			SDL_Cursor * newCursor = SDL_CreateSystemCursor( cursorId );
			SDL_Cursor * oldCursor = SDL_GetCursor();
			SDL_SetCursor( newCursor );
			SDL_FreeCursor( oldCursor );
		}
	}
	
	void CropTool::mousemove( const Point2D& mouse )
	{
		if( this->croppingStarted )
			this->resizeView( mouse );

		else if( this->rotationStarted )
			this->rotateView( mouse );
			
		else
		{
			MouseCropPosition position = this->getCropPosition( mouse );
			this->setMousePosition( position );
		}
	}
	
	void CropTool::startCropping( const Point2D& mouse )
	{
		if( !this->rotationStarted )
		{
			this->croppingStarted = true;
			this->originalView = Rectangle( this->negative->getView() );
			this->mousePosition = mouse;
		}
	}
	
	void CropTool::stopCropping( const Point2D& mouse )
	{
		if( this->croppingStarted )
		{
			this->croppingStarted = false;
			this->resizeView( mouse );
		}
	}
	
	void CropTool::startRotation( const Point2D& mouse )
	{
		if( !this->croppingStarted )
		{
			this->rotationStarted = true;
			this->originalRotation = this->negative->getRotation();
			this->mousePosition = mouse;
		}
	}
	
	void CropTool::stopRotation( const Point2D& mouse )
	{
		if( this->rotationStarted )
		{
			this->rotationStarted = false;
			this->rotateView( mouse );
		}
	}
	
	void CropTool::rotateView( const Point2D& point )
	{
		unsigned int textureWidth = this->negative->getNegativeWidth();
		unsigned int textureHeight = this->negative->getNegativeHeight();
		float ratio = 1.0f;
		
		if( textureWidth > 0 )
			ratio = static_cast<float>( this->getWidth() ) / static_cast<float>( textureWidth );
			
		Point2D center( textureWidth / 2.0f, textureHeight / 2.0f );
		Vector origin( ((this->mousePosition.getX() - this->getX()) / ratio) - center.getX(), ((this->mousePosition.getY() - this->getY()) / ratio) - center.getY(), 0.0f );
		Vector current( ((point.getX() - this->getX()) / ratio) - center.getX(), ((point.getY() - this->getY()) / ratio) - center.getY(), 0.0f );
		
		float determinant = origin.getX() * current.getY() - origin.getY() * current.getX();
		float angle = determinant < 0 ? 360.0f - origin.getAngle( current ) : origin.getAngle( current );
		this->negative->setRotation( this->originalRotation + angle );
	}
	
	void CropTool::resizeView( const Point2D& point )
	{
		Rectangle view( this->originalView );
		unsigned int textureWidth = this->negative->getNegativeWidth();
		float ratio = 1.0f;

		if( textureWidth > 0 )
			ratio = static_cast<float>( this->getWidth() ) / static_cast<float>( textureWidth );
		
		switch( this->mouseCropPosition )
		{
			case MouseCropPosition::Inside:
			{
				view.getOrigin().moveBy( (point.getX() - this->mousePosition.getX()) / ratio, (point.getY() - this->mousePosition.getY()) / ratio, 0.0f );
				
				if( view.getX() < 0.0f )
					view.getOrigin().setX( 0.0f );
					
				else if( view.getX() + view.getWidth() > this->negative->getNegativeWidth() )
					view.getOrigin().setX( this->negative->getNegativeWidth() - view.getWidth() - 1.0f );
					
				if( view.getY() < 0.0f )
					view.getOrigin().setY( 0.0f );
					
				else if( view.getY() + view.getHeight() > this->negative->getNegativeHeight() )
					view.getOrigin().setY( this->negative->getNegativeHeight() - view.getHeight() - 1.0f );
				
				break;
			}
			
			case MouseCropPosition::NorthWest:
			{
				float deltaHeight = (point.getY() - this->mousePosition.getY()) / ratio;
				float deltaWidth = (point.getX() - this->mousePosition.getX()) / ratio;
				
				if( deltaWidth + view.getX() > 0 )
				{
					if( deltaWidth < view.getWidth() )
					{
						view.getOrigin().moveBy( deltaWidth, 0.0f, 0.0f );
						view.resizeBy( -deltaWidth, 0 );
					}
					else
					{
						view.getOrigin().setX( view.getX() + view.getWidth() - 1.0f );
						view.setWidth( 1 );
					}
				}
				else
				{
					view.setWidth( view.getX() + view.getWidth() );
					view.getOrigin().setX( 0.0f );
				}
				
				if( deltaHeight + view.getY() > 0 )
				{
					if( deltaHeight < view.getHeight() )
					{
						view.getOrigin().moveBy( 0.0f, deltaHeight, 0.0f );
						view.resizeBy( 0, -deltaHeight );
					}
					else
					{
						view.getOrigin().setY( view.getY() + view.getHeight() - 1.0f );
						view.setHeight( 1 );
					}
				}
				else
				{
					view.setHeight( view.getY() + view.getHeight() );
					view.getOrigin().setY( 0.0f );
				}
				
				break;
			}
			
			case MouseCropPosition::SouthEast:
			{
				float deltaWidth = (point.getX() - this->mousePosition.getX()) / ratio;
				float deltaHeight = (point.getY() - this->mousePosition.getY()) / ratio;
				
				if( deltaWidth + view.getWidth() > 0 )
				{
					view.resizeBy( deltaWidth, 0 );
				
					if( view.getX() + view.getWidth() > this->negative->getNegativeWidth() )
						view.setWidth( this->negative->getNegativeWidth() - view.getX() );
				}
				else
					view.setWidth( 1 );
				
				if( deltaHeight + view.getHeight() > 0 )
				{
					view.resizeBy( 0, deltaHeight );
				
					if( view.getY() + view.getHeight() > this->negative->getNegativeHeight() )
						view.setHeight( this->negative->getNegativeHeight() - view.getY() );
				}
				else
					view.setHeight( 1 );
					
				break;
			}
			
			case MouseCropPosition::North:
			{
				float deltaHeight = (point.getY() - this->mousePosition.getY()) / ratio;
				
				if( deltaHeight + view.getY() > 0 )
				{
					if( deltaHeight < view.getHeight() )
					{
						view.getOrigin().moveBy( 0.0f, deltaHeight, 0.0f );
						view.resizeBy( 0, -deltaHeight );
					}
					else
					{
						view.getOrigin().setY( view.getY() + view.getHeight() - 1.0f );
						view.setHeight( 1 );
					}
				}
				else
				{
					view.setHeight( view.getY() + view.getHeight() );
					view.getOrigin().setY( 0.0f );
				}
				
				break;
			}
			
			case MouseCropPosition::South:
			{
				float deltaHeight = (point.getY() - this->mousePosition.getY()) / ratio;
				
				if( deltaHeight + view.getHeight() > 0 )
				{
					view.resizeBy( 0, deltaHeight );
				
					if( view.getY() + view.getHeight() > this->negative->getNegativeHeight() )
						view.setHeight( this->negative->getNegativeHeight() - view.getY() );
				}
				else
					view.setHeight( 1 );
				
				break;
			}
			
			case MouseCropPosition::NorthEast:
			{
				float deltaWidth = (point.getX() - this->mousePosition.getX()) / ratio;
				float deltaHeight = (point.getY() - this->mousePosition.getY()) / ratio;
				
				if( deltaWidth + view.getWidth() > 0 )
				{
					view.resizeBy( deltaWidth, 0 );
				
					if( view.getX() + view.getWidth() > this->negative->getNegativeWidth() )
						view.setWidth( this->negative->getNegativeWidth() - view.getX() );
				}
				else
					view.setWidth( 1 );
				
				if( deltaHeight + view.getY() > 0 )
				{
					if( deltaHeight < view.getHeight() )
					{
						view.getOrigin().moveBy( 0.0f, deltaHeight, 0.0f );
						view.resizeBy( 0, -deltaHeight );
					}
					else
					{
						view.getOrigin().setY( view.getY() + view.getHeight() - 1.0f );
						view.setHeight( 1 );
					}
				}
				else
				{
					view.setHeight( view.getY() + view.getHeight() );
					view.getOrigin().setY( 0.0f );
				}
					
				break;
			}
			
			case MouseCropPosition::SouthWest:
			{
				float deltaHeight = (point.getY() - this->mousePosition.getY()) / ratio;
				float deltaWidth = (point.getX() - this->mousePosition.getX()) / ratio;
				
				if( deltaWidth + view.getX() > 0 )
				{
					if( deltaWidth < view.getWidth() )
					{
						view.getOrigin().moveBy( deltaWidth, 0.0f, 0.0f );
						view.resizeBy( -deltaWidth, 0 );
					}
					else
					{
						view.getOrigin().setX( view.getX() + view.getWidth() - 1.0f );
						view.setWidth( 1 );
					}
				}
				else
				{
					view.setWidth( view.getX() + view.getWidth() );
					view.getOrigin().setX( 0.0f );
				}
				
				if( deltaHeight + view.getHeight() > 0 )
				{
					view.resizeBy( 0, deltaHeight );
				
					if( view.getY() + view.getHeight() > this->negative->getNegativeHeight() )
						view.setHeight( this->negative->getNegativeHeight() - view.getY() );
				}
				else
					view.setHeight( 1 );
					
				break;
			}
			
			case MouseCropPosition::East:
			{
				float deltaWidth = (point.getX() - this->mousePosition.getX()) / ratio;
				
				if( deltaWidth + view.getWidth() > 0 )
				{
					view.resizeBy( deltaWidth, 0 );
				
					if( view.getX() + view.getWidth() > this->negative->getNegativeWidth() )
						view.setWidth( this->negative->getNegativeWidth() - view.getX() );
				}
				else
					view.setWidth( 1 );
				
				break;
			}
			
			case MouseCropPosition::West:
			{
				float deltaWidth = (point.getX() - this->mousePosition.getX()) / ratio;
				
				if( deltaWidth + view.getX() > 0 )
				{
					if( deltaWidth < view.getWidth() )
					{
						view.getOrigin().moveBy( deltaWidth, 0.0f, 0.0f );
						view.resizeBy( -deltaWidth, 0 );
					}
					else
					{
						view.getOrigin().setX( view.getX() + view.getWidth() - 1.0f );
						view.setWidth( 1 );
					}
				}
				else
				{
					view.setWidth( view.getX() + view.getWidth() );
					view.getOrigin().setX( 0.0f );
				}
				
				break;
			}
			
			default:
			case MouseCropPosition::Outside:
			{
				// Do nothing.
				break;
			}
		}
		
		this->negative->setView( view.getX(), view.getY(), view.getWidth(), view.getHeight() );
	}
	
	void CropTool::render()
	{
		vector<Point3D> vertices;
		vector<Color> colors;
		vector<unsigned short int> indices;
		
		unsigned int textureWidth = this->negative->getNegativeWidth();

		if( textureWidth > 0 )
		{
			float ratio = static_cast<float>( this->getWidth() ) / static_cast<float>( textureWidth );
			
			Rectangle view;
			view.getOrigin().moveTo( this->getOrigin().getX() + round( this->negative->getView().getX() * ratio ), this->getOrigin().getY() + round( this->negative->getView().getY() * ratio ), this->getOrigin().getZ() );
			view.resize( round( this->negative->getView().getWidth() * ratio ), round( this->negative->getView().getHeight() * ratio ) );
			
			Color borderColor( CROP_RECTANGLE_BORDER_COLOR );
			Color backgroundColor( CROP_RECTANGLE_BACKGROUND_COLOR );
			
			ColoredRectangle rectangle( view, backgroundColor );
			
			// Cropped top area
			rectangle.getOrigin().moveTo( this->getOrigin().getX(), this->getOrigin().getY(), this->getOrigin().getZ() );
			rectangle.resize( this->getWidth(), view.getOrigin().getY() - this->getOrigin().getY() );
			rectangle.prepareRendering( vertices, colors, indices );
			
			// Cropped right area
			rectangle.getOrigin().moveTo( view.getOrigin().getX() + view.getWidth(), view.getOrigin().getY(), this->getOrigin().getZ() );
			rectangle.resize( this->getWidth() - (view.getWidth() + view.getOrigin().getX() - this->getOrigin().getX()), view.getHeight() );
			rectangle.prepareRendering( vertices, colors, indices );
			
			// Cropped bottom area
			rectangle.getOrigin().moveTo( this->getOrigin().getX(), view.getOrigin().getY() + view.getHeight(), this->getOrigin().getZ() );
			rectangle.resize( this->getWidth(), this->getHeight() - view.getHeight() - ( view.getOrigin().getY() - this->getOrigin().getY() ) );
			rectangle.prepareRendering( vertices, colors, indices );
			
			// Cropped left area
			rectangle.getOrigin().moveTo( this->getOrigin().getX(), view.getOrigin().getY(), this->getOrigin().getZ() );
			rectangle.resize( view.getOrigin().getX() - this->getOrigin().getX(), view.getHeight() );
			rectangle.prepareRendering( vertices, colors, indices );
			
			rectangle.setColor( borderColor );
			
			// Top border
			rectangle.getOrigin().moveTo( view.getOrigin().getX(), view.getOrigin().getY() - CROP_RECTANGLE_BORDER_SIZE, this->getOrigin().getZ() + 0.1f );
			rectangle.resize( view.getWidth(), CROP_RECTANGLE_BORDER_SIZE );
			rectangle.prepareRendering( vertices, colors, indices );
			
			// Right border
			rectangle.getOrigin().moveTo( view.getOrigin().getX() + view.getWidth(), view.getOrigin().getY() - CROP_RECTANGLE_BORDER_SIZE, this->getOrigin().getZ() + 0.1f );
			rectangle.resize( CROP_RECTANGLE_BORDER_SIZE, view.getHeight() + 2 * CROP_RECTANGLE_BORDER_SIZE );
			rectangle.prepareRendering( vertices, colors, indices );
			
			// Bottom border
			rectangle.getOrigin().moveTo( view.getOrigin().getX(), view.getOrigin().getY() + view.getHeight(), this->getOrigin().getZ() + 0.1f );
			rectangle.resize( view.getWidth(), CROP_RECTANGLE_BORDER_SIZE );
			rectangle.prepareRendering( vertices, colors, indices );
			
			// Left border
			rectangle.getOrigin().moveTo( view.getOrigin().getX() - CROP_RECTANGLE_BORDER_SIZE, view.getOrigin().getY() - CROP_RECTANGLE_BORDER_SIZE, this->getOrigin().getZ() + 0.1f );
			rectangle.resize( CROP_RECTANGLE_BORDER_SIZE, view.getHeight() + 2 * CROP_RECTANGLE_BORDER_SIZE );
			rectangle.prepareRendering( vertices, colors, indices );
			
			ColoredRectangle::render( vertices, colors, indices );
		}
	}
}

