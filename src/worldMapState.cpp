#include "worldMapState.h"
#include "eventManager.h"
#include <glog/logging.h>

WorldMapState::WorldMapState(Ogre::Root* ogre, Ogre::RenderWindow* window):
	worldMap_{},
	scene_{std::unique_ptr<WorldMapScene>(new WorldMapScene(ogre, window, &worldMap_))}
{
	LOG(INFO) << "Creating a new state: WorldMap";
	subscribe("loadWorldMap", [this](std::string eventName, Arguments args){
		loadWorldMap(boost::any_cast<std::string>(args["data"]));
	});
	subscribe("resizeWorldMap", [this](std::string eventName, Arguments args){
		resizeWorldMap(boost::any_cast<int>(args["X"]), boost::any_cast<int>(args["Y"]), boost::any_cast<int>(args["Z"]));
	});
	subscribe("changeVoxelType", [this](std::string eventName, Arguments args){
		changeVoxelType(boost::any_cast<std::string>(args["matter"]),
						boost::any_cast<int>(args["x"]),
						boost::any_cast<int>(args["y"]),
						boost::any_cast<int>(args["z"]),
						boost::any_cast<int>(args["radius"]));
	});
}

WorldMapState::~WorldMapState()
{
	LOG(INFO) << "Deleting state: WorldMap";
}

void WorldMapState::update(unsigned long delta)
{
	scene_->update(delta);
}

bool WorldMapState::loadWorldMap(std::string filename)
{
	LOG(INFO) << "trying to load a map from file: " << filename;
	//read from a file and create a MapDefinition
	lua_State *L = lua_open();
	if (luaL_loadfile(L, filename.c_str()) || lua_pcall(L, 0, 0, 0)) {
		LOG(ERROR) << lua_tostring(L, -1);
		return false;
    }
	MapDefinition newMap;    
	lua_getglobal(L, "mapDef");
	lua_pushnil(L);
	while(lua_next(L, -2)) {
	    //cycle through all the plots
	    if(lua_istable(L, -1)) {
			int x = getIntFieldLua("x", L);
			int y = getIntFieldLua("y", L);
	        int z = getIntFieldLua("z", L);
	        std::string id = getStringFieldLua("id", L);
			std::map<std::string, std::string> props;
	        //get properties	        
        	lua_pushstring(L, "properties");
			lua_gettable(L, -2);
			if (!lua_istable(L, -1)) {
				LOG(WARNING) << "Properties field not present or not well formated";
			} else {
				//TODO put values in field
				lua_pushnil(L);
				std::string key = "";
				std::string value = "";
				while (lua_next(L, -2)) {
					key = (std::string)lua_tostring(L, -2);
					value = (std::string)lua_tostring(L, -1);
					props[key] = value;
					lua_pop(L, 1);
				}				
			}
			lua_pop(L, 1);
	        //~ LOG(INFO) << "OKAY! " << x << " " << y << " " << z << ": " << id << "( " << props["matterType"] << " )";
        	Plot plot;
			plot.x = x;
			plot.y = y;
			plot.z = z;
			plot.id = id;
			plot.properties = props;
			newMap.push_back(plot);
	    }
	    lua_pop(L, 1);
	}
	lua_pop(L, 1);
	lua_close(L);
	return loadWorldMap(newMap);
}

bool WorldMapState::loadWorldMap(MapDefinition mapDefinition)
{
	//check the size of the map to create
	if (mapDefinition.empty()) {
		return false;
	}
	int xSize = 0;
	int ySize = 0;
	int zSize = 0;
	for (Plot &elem : mapDefinition) {
		if (elem.x > xSize) {
			xSize = elem.x;
		}
		if (elem.y > ySize) {
			ySize = elem.y;
		}
		if (elem.z > zSize) {
			zSize = elem.z;
		}
	}
	xSize++;
	ySize++;
	zSize++;
	LOG(INFO) << "The new world map to create is of size: " << xSize << "*" << ySize << "*" << zSize;
	worldMap_ = std::unique_ptr<CubeMap<Voxel>>(new CubeMap<Voxel>(xSize, ySize, zSize));

	//create the right voxels and fil the map
	for (Plot &elem : mapDefinition) {
		if (elem.id == "matter") {
			if (elem.properties["matterType"] != "") {
				auto vox = Voxel::createMatterVoxel(elem.properties["matterType"]);
				worldMap_->setVoxel(vox, elem.x, elem.y, elem.z);				
			} else {
				LOG(WARNING) << "A matter plot doesn't have a matterType field: " << elem.x << "*" << elem.y << "*" << elem.z;
			}			
		} else {
			LOG(WARNING) << "Don't know how to create voxel of id = " << elem.id;
		}
	}
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("mapCreated");
	return true;
}

int WorldMapState::getIntFieldLua(const char *key, lua_State *L)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (!lua_isnumber(L, -1)) {
		LOG(WARNING) << "Trying to access a field in Lua that is not an int: " << key;
		return -1;
	}
	int value = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return value;
}

std::string WorldMapState::getStringFieldLua(const char *key, lua_State *L)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (!lua_isstring(L, -1)) {
		LOG(WARNING) << "Trying to access a field in Lua that is not a string: " << key;
		return "";
	}
	std::string value = (std::string)lua_tostring(L, -1);
	lua_pop(L, 1);
	return value;
}

bool WorldMapState::resizeWorldMap(int x, int y, int z)
{
	//check values are correct
	if ((x <= 0) || (y <= 0) || (z <= 0)) {
		LOG(WARNING) << "Map resized failed: size <=0";
		return false;
	}
	LOG(INFO) << "Trying to resize the world map";
	if ((x > worldMap_->getSizeX()) || (y > worldMap_->getSizeY()) || (z > worldMap_->getSizeZ())) {
		//map bigger then before --> have to fill the new space
		worldMap_->resize(x, y, z);
		for (int i=0; i<x; ++i) {
			for (int j=0; j<y; ++j) {
				for (int k=0; k<z; ++k) {
					if (!worldMap_->getVoxel(i, j, k)) {
						//create a new voxel
						auto vox = Voxel::createMatterVoxel("ocean");
						worldMap_->setVoxel(vox, i, j, k);
					}
				}
			}
		}
	} else {
		//map smaller then before
		worldMap_->resize(x, y, z);
	}
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("mapResized");
	return true;
}

void WorldMapState::changeVoxelType(std::string newType, int x, int y, int z, int radius)
{
	//~ LOG(INFO) << "Change voxel " << x << "-" << y << "-" << z << " to type " << newType;
	int xSize = radius-1;
	for (int i=-xSize; i<=xSize; ++i) {
		int ySize = xSize - std::abs(i);
		LOG(INFO) << "xSize: " << xSize << " ySize: " << ySize;
		for (int j=-ySize; j<=ySize; ++j) {
		LOG(INFO) << "x=" << i << " y=" << j;
		auto vox = Voxel::createMatterVoxel(newType);
		worldMap_->setVoxel(vox, x+i, y, z+j);
		Arguments arg;
		arg["x"] = x+i;
		arg["y"] = y;
		arg["z"] = z+j;
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("cubeModified", arg);
		}
	}	
}
