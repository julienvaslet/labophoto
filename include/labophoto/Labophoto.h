#ifndef __LABOPHOTO_LABOPHOTO_H
#define __LABOPHOTO_LABOPHOTO_H	1

#define CONTROL_PANEL_WIDTH			310.0f
#define WORKSPACE_BACKGROUND_COLOR	"444444"
#define WINDOW_BACKGROUND_COLOR		"888888"
#define UI_ELEMENT_BACKGROUND_COLOR	"668b8b"

#include <SDL2/SDL.h>
#include <opengl/ColoredRectangle.h>
#include <labophoto/Negative.h>
#include <labophoto/CropTool.h>
#include <opengl/ui/UserInterface.h>
#include <tools/camera/GphotoCamera.h>

using namespace tools::camera;
using namespace opengl;
using namespace opengl::ui;

namespace labophoto
{
	class Labophoto
	{
		protected:
			static Labophoto * instance;
			
			GphotoCamera * camera;
			UserInterface * ui;
			Negative * image;
			ColoredRectangle * background;
			CropTool * cropTool;
			
			void loadModeSelectionUI();
			void loadCameraSelectionUI();
			void loadCameraConfigurationUI();
			
			void reloadAvailableCameras();
			void reloadCameraConfiguration();
			void synchronizeCropToolLabels();
			
			void setMode( short int mode );
			void selectCamera( const string& port );
			void captureImage();
			void setCameraShutterSpeed( const string& value );
			void setCameraAperture( const string& value );
			void setCameraIso( const string& value );
			void setCameraWhiteBalance( const string& value );
			
			void setLoadingAnimation( bool status );
		
		public:
			Labophoto();
			~Labophoto();
			
			bool isRunning();
			void render( unsigned int ticks );
			void handleEvent( SDL_Event * event );
			
			void resizeView();
			void loadNegative( const string& path );
			
			static Labophoto * get();
			
			static bool reloadCameraListEvent( Element * element, const event::Event * event );
			static bool currentModeChangedEvent( Element * element, const event::Event * event );
			static bool cameraChangedEvent( Element * element, const event::Event * event );
			static bool takePreviewEvent( Element * element, const event::Event * event );
			static bool changeShutterSpeedEvent( Element * element, const event::Event * event );
			static bool changeApertureEvent( Element * element, const event::Event * event );
			static bool changeIsoEvent( Element * element, const event::Event * event );
			static bool changeWhiteBalanceEvent( Element * element, const event::Event * event );
			static bool changeColorInversionEvent( Element * element, const event::Event * event );
			static bool activeCropToolEvent( Element * element, const event::Event * event );
	};
}

#endif
