#ifndef WORLDMAPSTATE_H
#define WORLDMAPSTATE_H
#include "cubeMap.h"
#include "voxel.h"
#include "graphics/worldMapScene.h"
#include "eventManager.h"
#include <memory>
#include <vector>
#include <map>
extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}
class WorldMapState: public Subscribable
{
	public:
		WorldMapState(Ogre::Root* ogre, Ogre::RenderWindow* window);
		~WorldMapState();
		void update(unsigned long delta);
		
		
	private:
		struct Plot {
			int x;
			int y;
			int z;
			std::string id;
			std::map<std::string, std::string> properties;
		};
		typedef std::vector<Plot> MapDefinition;
	
		bool loadWorldMap(std::string filename);
		bool loadWorldMap(MapDefinition mapDefinition);
		bool saveWorldMapToLua(std::string filename);
		int getIntFieldLua(const char *key, lua_State *L);
		std::string getStringFieldLua(const char *key, lua_State *L);
		bool resizeWorldMap(int x, int y, int z);
		void changeVoxelType(std::string newType, int x, int y, int z, int radius);
			
		std::unique_ptr<CubeMap<Voxel>> worldMap_;
		std::unique_ptr<WorldMapScene> scene_;
		
};

#endif /* WORLDMAPSTATE_H */ 
