#ifndef __LABOPHOTO_NEGATIVE_H
#define __LABOPHOTO_NEGATIVE_H	1

#include <opengl/TexturedRectangle.h>

using namespace opengl;

namespace labophoto
{
	class Negative : public TexturedRectangle
	{
		protected:
			Program * program;
			ArrayBufferObject * vertices;
			ArrayBufferObject * textureCoordinates;
			ElementArrayBufferObject * indices;
			
			bool invertColors;
			float rotation;
			Rectangle view;
			
			void initializeRendering();
		
		public:
			Negative();
			virtual ~Negative();
			
			bool areColorsInverted();
			void setColorInversion( bool status );
			
			unsigned int getNegativeWidth() const;
			unsigned int getNegativeHeight() const;
			
			float getRotation();
			void setRotation( float angle );
			const Rectangle& getView() const;
			void setView( unsigned int x, unsigned int y, unsigned int width, unsigned int height );
			
			void render( bool cropped = true );
	};
}

#endif
