#ifndef WORLDMAPSCENE_H
#define WORLDMAPSCENE_H

#include "OGRE/Ogre.h"
#include <memory>
#include <vector>
#include "camera.h"
#include "../eventManager.h"
#include "../cubeMap.h"
#include "../voxel.h"
#include "worldMapGui.h"

struct Coordinates {
	int x;
	int y;
	int z;
};

struct ChunkInfos {
	int id;
	bool clean;
};

class WorldMapScene: public Subscribable
{
	public:
		WorldMapScene(Ogre::Root *ogre, Ogre::RenderWindow *window, const std::unique_ptr<CubeMap<Voxel>> *worldMap);
		~WorldMapScene();
		void update(unsigned long delta);
		
		void drawMap();
	private:
		void createCube(int x, int y, int z, std::string id);
		void createCube2(int x, int y, int z, std::string id);
		void drawChunk(int startX, int startY, int startZ, int endX, int endY, int endZ);
		void drawChunk(int id);
		void createSelectionMark(int radius);
		
		bool initGui(Ogre::RenderWindow *window);
		void checkSelection(int x, int y);
		void updateSelection();
		Coordinates entityNameToCoordinates(std::string entityName);
		void destroyAllAttachedMovableObjects(Ogre::SceneNode *node);
		int calculateChunkId(int x, int y, int z);
		
		
		Ogre::Root *ogre_;
		Ogre::SceneManager *sceneMgr_;
		const std::unique_ptr<CubeMap<Voxel>> *worldMap_;
		Ogre::SceneNode *worldMapNode_;
		std::unique_ptr<Camera> camera_;
		float cubeSize_;
		int chunkSize_;
		std::vector<ChunkInfos> chunkList;
		std::map<std::string, int> textureAtlasInfos_;		
		//gui
		std::unique_ptr<WorldMapGui> gui_;
		bool mouseButtonPressed;
		Coordinates selectedCube_;
		Ogre::SceneNode *selectionMarkNode_;
};

#endif /* WORLDMAPSCENE_H */ 
