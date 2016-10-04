#ifndef __LABOPHOTO_CROPTOOL_H
#define __LABOPHOTO_CROPTOOL_H	1

#include <opengl/ColoredRectangle.h>
#include <labophoto/Negative.h>

using namespace opengl;

namespace labophoto
{
	enum MouseCropPosition
	{
		Outside,
		Inside,
		NorthWest,
		North,
		NorthEast,
		East,
		SouthEast,
		South,
		SouthWest,
		West
	};
	
	class CropTool : public ColoredRectangle
	{
		protected:
			Negative * negative;
			bool active;
			MouseCropPosition mouseCropPosition;
			Point2D mousePosition;
			bool croppingStarted;
			bool rotationStarted;
			float originalRotation;
			Rectangle originalView;
			
			MouseCropPosition getCropPosition( const Point2D& point );
			void setMousePosition( MouseCropPosition position );
			void resizeView( const Point2D& point );
			void rotateView( const Point2D& point );
		
		public:
			CropTool( Negative * negative );
			virtual ~CropTool();
			
			bool isActive() const;
			void activate( bool status );
			
			void mousemove( const Point2D& mouse );
			void startCropping( const Point2D& mouse );
			void stopCropping( const Point2D& mouse );
			void startRotation( const Point2D& mouse );
			void stopRotation( const Point2D& mouse );
			
			void render();
	};
}

#endif
