#ifndef GAME_H
#define GAME_H

#include <memory>
#include "options.h"
#include "cubeMap.h"
#include "voxel.h"
#include "graphics/graphicsOgre.h"
#include "input/inputOIS.h"
#include "eventManager.h"
#include "worldMapState.h"


class Game: public Subscribable
{
	public:
		Game();
		~Game();
		
		bool start();
		bool isRunning() const;
		void update();
		
			
	private:
		bool loadOptions();
		
		std::shared_ptr<Options> config_;
		std::shared_ptr<Options> keymap_; 
		bool running_;
		std::shared_ptr<CubeMap<Voxel>> currentMap_;
		std::unique_ptr<GraphicsOgre> graphics_;
		std::unique_ptr<InputOIS> input_;
		std::unique_ptr<WorldMapState> currentState_;
		
		//for the FPS
		Ogre::Timer timer_;
		int nbFrames_;
		unsigned long lastFpsCalcul_;
		unsigned long currentTime_;
		int fpsLimit_;
		unsigned long timeBetweenFrames_;
		unsigned long lastFrameTime_;};

#endif /* GAME_H */ 
