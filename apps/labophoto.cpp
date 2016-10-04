#include <SDL2/SDL.h>
#include <opengl/Screen.h>
#include <opengl/OpenGL.h>
#include <game/Resource.h>

#include <labophoto/Labophoto.h>
#include <tools/logger/Stdout.h>

#include <unistd.h>

using namespace opengl;
using namespace game;
using namespace tools::logger;
using namespace labophoto;

int main( int argc, char ** argv )
{
	// Initialize standard-output logger
	new Stdout( "stdout", true );
	
	if( !Screen::initializeWindowed() )
	{
		Logger::get() << "Unable to initialize screen. Exiting.\n";
		return 1;
	}
	
	Labophoto * app = new Labophoto();
	
	/*if( argc >= 2 )
	{
		// Load argv[1]
		app->loadNegative( argv[1] );
	}*/
	
	//app->loadNegative( "test.jpg" );
	
	bool running = true;
	SDL_Event lastEvent;
	unsigned int lastDrawTicks = 0;
	
	while( running && app->isRunning() )
	{
		while( SDL_PollEvent( &lastEvent ) )
		{
			switch( lastEvent.type )
			{
				case SDL_QUIT:
				{
					running = false;
					break;
				}
				
				case SDL_WINDOWEVENT:
				{
					if( lastEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
					{
						Screen::get()->resize( lastEvent.window.data1, lastEvent.window.data2 );
						app->resizeView();
					}
					
					break;
				}

				default:
				{
					app->handleEvent( &lastEvent );
					break;
				}
			}
		}

		unsigned int ticks = SDL_GetTicks();
		
		if( ticks - lastDrawTicks > 30 )
		{
			Screen::get()->clear();

			app->render( ticks );
			Screen::get()->render();
			
			lastDrawTicks = ticks;
		}
		else
			usleep( 15 );
	}
	
	delete app;
	
	Resource::destroy();
	Screen::destroy();
	Logger::destroy();
	
	return 0;
}

