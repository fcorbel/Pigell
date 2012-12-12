#include "worldMapScene.h"
#include <glog/logging.h>
#include <set>
#include <cmath>
#include "../matterVoxel.h"

WorldMapScene::WorldMapScene(Ogre::Root* ogre, Ogre::RenderWindow* window, const std::unique_ptr<CubeMap<Voxel>> *worldMap):
	ogre_(ogre),
	sceneMgr_{ogre_->createSceneManager(Ogre::ST_GENERIC)},
	worldMap_{worldMap},
	worldMapNode_{sceneMgr_->getRootSceneNode()->createChildSceneNode()},
	camera_{std::unique_ptr<Camera>(new Camera("mainCam", sceneMgr_, window))},
	cubeSize_{50.0f},
	chunkSize_{3},
	gui_{},
	mouseButtonPressed{false},
	selectedCube_{-1,-1,-1},
	selectionMarkNode_{sceneMgr_->getRootSceneNode()->createChildSceneNode()}
{
	textureAtlasInfos_["sea"] = 2;
	textureAtlasInfos_["plain"] = 3;
	textureAtlasInfos_["mountain"] = 4;
	textureAtlasInfos_["desert"] = 5;
	textureAtlasInfos_["ocean"] = 6;
	
	LOG(INFO) << "Creating a new scene: WorldMap";	
	//load camera
	camera_->getCameraPtr()->lookAt(0, -1, -0.5); 
	//load gui
	initGui(window);
	
	//create the basic cube entity
	Ogre::ManualObject* cube;
	cube = sceneMgr_->createManualObject("cube");
	cube->setDynamic(false);
	cube->begin("default", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	{
		cube->position(0,0,0); cube->textureCoord(1,1);
		cube->position(cubeSize_,0,0); cube->textureCoord(0,1);
		cube->position(cubeSize_,cubeSize_,0); cube->textureCoord(0,0);
		cube->position(0,cubeSize_,0); cube->textureCoord(1,0);
		cube->position(0,0,cubeSize_); 
		cube->position(cubeSize_,0,cubeSize_);
		cube->position(cubeSize_,cubeSize_,cubeSize_);
		cube->position(0,cubeSize_,cubeSize_);
		
		//front/back
		cube->triangle(4,5,6);
		cube->triangle(6,7,4);
		cube->triangle(1,0,2);
		cube->triangle(2,0,3);
		//top/down
		cube->triangle(7,6,2);
		cube->triangle(2,3,7);
		cube->triangle(1,5,4);
		cube->triangle(4,0,1);
		//left/right
		cube->triangle(7,3,0);
		cube->triangle(0,4,7);
		cube->triangle(1,2,6);
		cube->triangle(6,5,1);
	}
	cube->end();
	cube->convertToMesh("meshCube");
	
	//create the selection marker cube
	Ogre::ManualObject* selectionMarker;
	selectionMarker = sceneMgr_->createManualObject("selectionMarker");
	selectionMarker->setDynamic(false);
	selectionMarker->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
	{
		selectionMarker->position(0,0,0);
		selectionMarker->colour(Ogre::ColourValue(1, 0.0, 0.0));
		selectionMarker->position(cubeSize_,0,0);
		selectionMarker->position(cubeSize_,0,cubeSize_);
		selectionMarker->position(0,0,cubeSize_);

		selectionMarker->index(0);
		selectionMarker->index(1);
		selectionMarker->index(1);
		selectionMarker->index(2);
		selectionMarker->index(2);
		selectionMarker->index(3);
		selectionMarker->index(3);
		selectionMarker->index(0);
	}
	selectionMarker->end();
	selectionMarker->convertToMesh("meshSelectionMarker");
	subscribe("markerRadiusChanged", [this](std::string eventName, Arguments args){
		createSelectionMark(boost::any_cast<int>(args["radius"]));
	});
	subscribe("mapCreated", [this](std::string eventName, Arguments args){ drawMap(); });
	subscribe("mapResized", [this](std::string eventName, Arguments args){ drawMap(); });
	subscribe("cubeModified", [this](std::string eventName, Arguments args){
		//set the clean flag of the correct chunk to false
		int id = calculateChunkId(boost::any_cast<int>(args["x"]), boost::any_cast<int>(args["y"]), boost::any_cast<int>(args["z"]));
		
		for (ChunkInfos &chunk : chunkList) {
			if (chunk.id == id) {
				LOG(INFO) << "Chunk id=" << id << " is to be redrawn";
				chunk.clean = false;
				break;
			}
		}
		
		//~ ChunkInfos copy;
		//~ copy.id = id;
		//~ copy.clean = true;
		//~ auto res = std::find(chunkList.begin(), chunkList.end(), copy);
		//~ if (res == chunkList.end()) {
			//~ LOG(INFO) << "Chunk to modify not found in vector";
		//~ } else {
			//~ res->clean = false;
		//~ }
	});
}

WorldMapScene::~WorldMapScene()
{
	LOG(INFO) << "Deleting scene: WorldMap";
}

void WorldMapScene::update(unsigned long delta)
{
	camera_->update(delta);
	//check if some chunks need to be redrawn
	for (ChunkInfos &chunk : chunkList) {
		if (!chunk.clean) {
			LOG(INFO) << "Redraw chunk id=" << chunk.id;
			drawChunk(chunk.id);
			chunk.clean = true;
		}
	}
}

void WorldMapScene::drawMap()
{
	//draw the whole map from the cubeMap
	if (!worldMap_) { LOG(ERROR) << "No wolrdMap to draw"; }
	if (worldMapNode_->numChildren() > 0) {
		LOG(WARNING) << "A map appears to already be loaded";
		//clear the worldMapNode_
		destroyAllAttachedMovableObjects(worldMapNode_);
		worldMapNode_->removeAndDestroyAllChildren();
		chunkList.clear();
	}
	//draw by chunk
	int chunkId = 0;
	for (int z=0; z<=((*worldMap_)->getSizeZ()/chunkSize_); ++z) {
		for (int y=0; y<=((*worldMap_)->getSizeY()/chunkSize_); ++y) {
			for (int x=0; x<=((*worldMap_)->getSizeX()/chunkSize_); ++x) {
				LOG(INFO) << "IDchunks: " << x << " " << y << " " << z << " ->" << chunkId;
				int startX = x*chunkSize_;
				int startY = y*chunkSize_;
				int startZ = z*chunkSize_;
				LOG(INFO) << "Draw chunk id=" << chunkId << " at position " << startX << " " << startY << " " << startZ;
				drawChunk(startX, startY, startZ, startX+chunkSize_-1, startY+chunkSize_-1, startZ+chunkSize_-1);
				ChunkInfos newChunk;
				newChunk.id = chunkId;
				newChunk.clean = true;
				chunkList.push_back(newChunk);
				++chunkId;
			}
		}
	}	
	camera_->setPosition((*worldMap_)->getSizeX()*cubeSize_/2, 300, (*worldMap_)->getSizeZ()*cubeSize_/2+150);
}

void WorldMapScene::createCube(int x, int y, int z, std::string id)
{
	LOG(INFO) << "create a new entity for a voxel of type: " << id;
	//create a scene node for the cube
	Ogre::SceneNode *newNode = worldMapNode_->createChildSceneNode(
		Ogre::StringConverter::toString(x) + "-" + 
		Ogre::StringConverter::toString(y) + "-" + 
		Ogre::StringConverter::toString(z) + "_cubeNode");
	newNode->setPosition (x*cubeSize_, y*cubeSize_, z*cubeSize_);
	if (id == "matter") {
		Ogre::Entity *newCube = sceneMgr_->createEntity(
			Ogre::StringConverter::toString(x) + "-" + 
			Ogre::StringConverter::toString(y) + "-" + 
			Ogre::StringConverter::toString(z) + "_cube", "meshCube");
		//retrieve the type from the voxel
		MatterVoxel *matVox;
		matVox = static_cast<MatterVoxel*>((*worldMap_)->getVoxel(x, y, z));
		std::string matterType = matVox->getType();
		//~ LOG(INFO) << "MatterType: " << matterType;
		//set the right material
		newCube->getSubEntity(0)->setMaterialName(matterType);
		newNode->attachObject(newCube);
	} else {
		LOG(WARNING) << "Don't know how to draw that type of voxel: " << id;
	}
}

//draw a cube without hiden faces
void WorldMapScene::createCube2(int x, int y, int z, std::string id)
{
	LOG(INFO) << "create a new entity for a voxel of type: " << id;
	//create a scene node for the cube
	Ogre::SceneNode *newNode = worldMapNode_->createChildSceneNode(
		Ogre::StringConverter::toString(x) + "-" + 
		Ogre::StringConverter::toString(y) + "-" + 
		Ogre::StringConverter::toString(z) + "_cubeNode");
	newNode->setPosition (x*cubeSize_, y*cubeSize_, z*cubeSize_);
	if (id == "matter") {		
		Ogre::ManualObject* cubeOptimised;
		cubeOptimised = sceneMgr_->createManualObject(
			Ogre::StringConverter::toString(x) + "-" + 
			Ogre::StringConverter::toString(y) + "-" + 
			Ogre::StringConverter::toString(z) + "_cube");
		cubeOptimised->setDynamic(false);
		//retrieve the type from the voxel
		MatterVoxel *matVox;
		matVox = static_cast<MatterVoxel*>((*worldMap_)->getVoxel(x, y, z));
		std::string matterType = matVox->getType();

		cubeOptimised->begin(matterType, Ogre::RenderOperation::OT_TRIANGLE_LIST);
		//check if face is hidden by another cube
		int vertexCount = 0;
		//top
		if (!(*worldMap_)->getVoxel(x, y+1, z)) {
			cubeOptimised->position(0,cubeSize_,0); cubeOptimised->textureCoord(0,0);
			cubeOptimised->position(0,cubeSize_,cubeSize_); cubeOptimised->textureCoord(0,1);
			cubeOptimised->position(cubeSize_,cubeSize_,cubeSize_); cubeOptimised->textureCoord(1,1);
			cubeOptimised->position(cubeSize_,cubeSize_,0); cubeOptimised->textureCoord(1,0);

			cubeOptimised->triangle(vertexCount,vertexCount+1,vertexCount+2);
			cubeOptimised->triangle(vertexCount+2,vertexCount+3,vertexCount);
			vertexCount += 4;
		}
		//left
		if (!(*worldMap_)->getVoxel(x-1, y, z)) {
			cubeOptimised->position(0,0,0); cubeOptimised->textureCoord(0,1);
			cubeOptimised->position(0,0,cubeSize_); cubeOptimised->textureCoord(1,1);
			cubeOptimised->position(0,cubeSize_,cubeSize_); cubeOptimised->textureCoord(1,0);
			cubeOptimised->position(0,cubeSize_,0); cubeOptimised->textureCoord(0,0);

			cubeOptimised->triangle(vertexCount,vertexCount+1,vertexCount+2);
			cubeOptimised->triangle(vertexCount+2,vertexCount+3,vertexCount);
			vertexCount += 4;

		}
		//bottom
		if (!(*worldMap_)->getVoxel(x, y-1, z)) {
			cubeOptimised->position(0,0,0); cubeOptimised->textureCoord(0,1);
			cubeOptimised->position(cubeSize_,0,0); cubeOptimised->textureCoord(1,1);
			cubeOptimised->position(cubeSize_,0,cubeSize_); cubeOptimised->textureCoord(1,0);
			cubeOptimised->position(0,0,cubeSize_); cubeOptimised->textureCoord(0,0);

			cubeOptimised->triangle(vertexCount,vertexCount+1,vertexCount+2);
			cubeOptimised->triangle(vertexCount+2,vertexCount+3,vertexCount);
			vertexCount += 4;
		}
		//back
		if (!(*worldMap_)->getVoxel(x, y, z-1)) {
			cubeOptimised->position(0,0,0); cubeOptimised->textureCoord(1,1);
			cubeOptimised->position(0,cubeSize_,0); cubeOptimised->textureCoord(1,0);
			cubeOptimised->position(cubeSize_,cubeSize_,0); cubeOptimised->textureCoord(0,0);
			cubeOptimised->position(cubeSize_,0,0); cubeOptimised->textureCoord(0,1);

			cubeOptimised->triangle(vertexCount,vertexCount+1,vertexCount+2);
			cubeOptimised->triangle(vertexCount+2,vertexCount+3,vertexCount);
			vertexCount += 4;
		}
		//right
		if (!(*worldMap_)->getVoxel(x+1, y, z)) {
			cubeOptimised->position(cubeSize_,0,0); cubeOptimised->textureCoord(1,1);
			cubeOptimised->position(cubeSize_,cubeSize_,0); cubeOptimised->textureCoord(1,0);
			cubeOptimised->position(cubeSize_,cubeSize_,cubeSize_); cubeOptimised->textureCoord(0,0);
			cubeOptimised->position(cubeSize_,0,cubeSize_); cubeOptimised->textureCoord(0,1);

			cubeOptimised->triangle(vertexCount,vertexCount+1,vertexCount+2);
			cubeOptimised->triangle(vertexCount+2,vertexCount+3,vertexCount);
			vertexCount += 4;
		}
		//front
		if (!(*worldMap_)->getVoxel(x, y, z+1)) {
			cubeOptimised->position(0,0,cubeSize_); cubeOptimised->textureCoord(0,1);
			cubeOptimised->position(cubeSize_,0,cubeSize_); cubeOptimised->textureCoord(1,1);
			cubeOptimised->position(cubeSize_,cubeSize_,cubeSize_); cubeOptimised->textureCoord(1,0);
			cubeOptimised->position(0,cubeSize_,cubeSize_); cubeOptimised->textureCoord(0,0);

			cubeOptimised->triangle(vertexCount,vertexCount+1,vertexCount+2);
			cubeOptimised->triangle(vertexCount+2,vertexCount+3,vertexCount);
			vertexCount += 4;
		}

		
		cubeOptimised->end();	
		
		newNode->attachObject(cubeOptimised);
	} else {
		LOG(WARNING) << "Don't know how to draw that type of voxel: " << id;
	}
}

//draw a chunk of the map as one mesh (using a texture atlas)
void WorldMapScene::drawChunk(int startX, int startY, int startZ, int endX, int endY, int endZ)
{
	//check values
	if (startX>endX || startY>endY || startZ>endZ) {
		LOG(WARNING) << "Trying to draw a chunk with incorrect coordinates: from " << startX << "-" << startY << "-" << startZ << " to " << endX << "-" << endY << "-" << endZ;
		return;
	}
	std::string nodeName = Ogre::StringConverter::toString(startX) + "-" + 
							Ogre::StringConverter::toString(startY) + "-" + 
							Ogre::StringConverter::toString(startZ) + "_chunkNode";
	Ogre::SceneNode *chunkNode = nullptr;
	if (!sceneMgr_->hasSceneNode(nodeName)) {
		LOG(INFO) << "Create a new node for the chunk";
		chunkNode = worldMapNode_->createChildSceneNode(nodeName);
		chunkNode->setPosition (startX*cubeSize_, startY*cubeSize_, startZ*cubeSize_);	
		//~ chunkNode->showBoundingBox(true);	
	} else {
		LOG(INFO) << "clear what's attached to the scene node";
		chunkNode = sceneMgr_->getSceneNode(nodeName);
		destroyAllAttachedMovableObjects(chunkNode);
	}
	LOG(INFO) << "Draw chunk of coord: [" << startX << "," << startY << "," << startZ << "] [" << endX << "," << endY << "," << endZ << "]";
	LOG(INFO) << "Try to draw a chunk of size: " << endX-startX+1 << "*" << endY-startY+1 << "*" << endZ-startZ+1;
	Ogre::ManualObject* chunkObject;
	chunkObject = sceneMgr_->createManualObject(
			Ogre::StringConverter::toString(startX) + "-" + 
			Ogre::StringConverter::toString(startY) + "-" + 
			Ogre::StringConverter::toString(startZ) + "_chunk");
	chunkObject->setDynamic(false);
	//draw all visible faces
	chunkObject->begin("default", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	int vertexCount = 0;
	for (int i=startX; i <=endX; ++i) {
		for (int j=startY; j <=endY; ++j) {
			for (int k=startZ; k<=endZ; ++k) {
				//~ LOG(INFO) << "Try to draw element at: " << i << "-" << j << "-" << k;
				//position in actual chunk
				int posX = i-startX+1;
				int posY = j-startY+1;
				int posZ = k-startZ+1;
				if (Voxel *vox = (*worldMap_)->getVoxel(i, j, k)) {
					if (vox->getId() == "matter") {
						MatterVoxel *matVox;
						matVox = static_cast<MatterVoxel*>(vox);
						std::string matterType = matVox->getType();
						int atlasNb = 1; //TODO adjust this number to the row of the material
						int nbRaw = 8;
						auto it = textureAtlasInfos_.find(matterType);
						if (it != textureAtlasInfos_.end()) {
							atlasNb = it->second;
						}
						//get the right texture coordinates
						float x0 = 0;
						float x1 = 1;
						float y0 = (float)(atlasNb-1) / nbRaw;
						float y1 = (float)atlasNb / nbRaw;
						//~ LOG(INFO) << x0 << " " << x1 << " " << y0 << " " << y1;
						//create a representation for that voxel
						//top
						if (!(*worldMap_)->getVoxel(i, j+1, k)) {
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*posY,cubeSize_*(posZ-1)); chunkObject->textureCoord(x0,y0);
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*posY,cubeSize_*posZ); chunkObject->textureCoord(x0,y1);
							chunkObject->position(cubeSize_*posX,cubeSize_*posY,cubeSize_*posZ); chunkObject->textureCoord(x1,y1);
							chunkObject->position(cubeSize_*posX,cubeSize_*posY,cubeSize_*(posZ-1)); chunkObject->textureCoord(x1,y0);
														
							chunkObject->triangle(vertexCount,vertexCount+1,vertexCount+2);
							chunkObject->triangle(vertexCount+2,vertexCount+3,vertexCount);
							vertexCount += 4;
						}
						//left
						if (!(*worldMap_)->getVoxel(i-1, j, k)) {
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*(posY-1),cubeSize_*(posZ-1)); chunkObject->textureCoord(0,1);
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*(posY-1),cubeSize_*posZ); chunkObject->textureCoord(1,1);
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*posY,cubeSize_*posZ); chunkObject->textureCoord(1,0);
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*posY,cubeSize_*(posZ-1)); chunkObject->textureCoord(0,0);
				
							chunkObject->triangle(vertexCount,vertexCount+1,vertexCount+2);
							chunkObject->triangle(vertexCount+2,vertexCount+3,vertexCount);
							vertexCount += 4;
				
						}
						//bottom
						if (!(*worldMap_)->getVoxel(i, j-1, k)) {
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*(posY-1),cubeSize_*(posZ-1)); chunkObject->textureCoord(0,1);
							chunkObject->position(cubeSize_*posX,cubeSize_*(posY-1),cubeSize_*(posZ-1)); chunkObject->textureCoord(1,1);
							chunkObject->position(cubeSize_*posX,cubeSize_*(posY-1),cubeSize_*posZ); chunkObject->textureCoord(1,0);
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*(posY-1),cubeSize_*posZ); chunkObject->textureCoord(0,0);
				
							chunkObject->triangle(vertexCount,vertexCount+1,vertexCount+2);
							chunkObject->triangle(vertexCount+2,vertexCount+3,vertexCount);
							vertexCount += 4;
						}
						//back
						if (!(*worldMap_)->getVoxel(i, j, k-1)) {
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*(posY-1),cubeSize_*(posZ-1)); chunkObject->textureCoord(1,1);
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*posY,cubeSize_*(posZ-1)); chunkObject->textureCoord(1,0);
							chunkObject->position(cubeSize_*posX,cubeSize_*posY,cubeSize_*(posZ-1)); chunkObject->textureCoord(0,0);
							chunkObject->position(cubeSize_*posX,cubeSize_*(posY-1),cubeSize_*(posZ-1)); chunkObject->textureCoord(0,1);
				
							chunkObject->triangle(vertexCount,vertexCount+1,vertexCount+2);
							chunkObject->triangle(vertexCount+2,vertexCount+3,vertexCount);
							vertexCount += 4;
						}
						//right
						if (!(*worldMap_)->getVoxel(i+1, j, k)) {
							chunkObject->position(cubeSize_*posX,cubeSize_*(posY-1),cubeSize_*(posZ-1)); chunkObject->textureCoord(1,1);
							chunkObject->position(cubeSize_*posX,cubeSize_*posY,cubeSize_*(posZ-1)); chunkObject->textureCoord(1,0);
							chunkObject->position(cubeSize_*posX,cubeSize_*posY,cubeSize_*posZ); chunkObject->textureCoord(0,0);
							chunkObject->position(cubeSize_*posX,cubeSize_*(posY-1),cubeSize_*posZ); chunkObject->textureCoord(0,1);
				
							chunkObject->triangle(vertexCount,vertexCount+1,vertexCount+2);
							chunkObject->triangle(vertexCount+2,vertexCount+3,vertexCount);
							vertexCount += 4;
						}
						//front
						if (!(*worldMap_)->getVoxel(i, j, k+1)) {
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*(posY-1),cubeSize_*posZ); chunkObject->textureCoord(0,1);
							chunkObject->position(cubeSize_*posX,cubeSize_*(posY-1),cubeSize_*posZ); chunkObject->textureCoord(1,1);
							chunkObject->position(cubeSize_*posX,cubeSize_*posY,cubeSize_*posZ); chunkObject->textureCoord(1,0);
							chunkObject->position(cubeSize_*(posX-1),cubeSize_*posY,cubeSize_*posZ); chunkObject->textureCoord(0,0);
				
							chunkObject->triangle(vertexCount,vertexCount+1,vertexCount+2);
							chunkObject->triangle(vertexCount+2,vertexCount+3,vertexCount);
							vertexCount += 4;
						}
					}
				}
			}
		}
	}
	chunkObject->end();
	chunkNode->attachObject(chunkObject);
}

void WorldMapScene::drawChunk(int id)
{
	//calculate the coordinates of this chunk
	int nbChunkX = std::ceil((*worldMap_)->getSizeX()/(float)chunkSize_);
	int nbChunkY = std::ceil((*worldMap_)->getSizeY()/(float)chunkSize_);
	int nbChunkZ = std::ceil((*worldMap_)->getSizeZ()/(float)chunkSize_);
	
	int chunkId = 0;
	for (int z=0; z<nbChunkZ; ++z) { //not very efficient
		for (int y=0; y<nbChunkY; ++y) {
			for (int x=0; x<nbChunkX; ++x) {
				if (chunkId == id) {
					int xCoord = x*chunkSize_;
					int yCoord = y*chunkSize_;
					int zCoord = z*chunkSize_;
					LOG(INFO) << "Draw chunk at " << xCoord << " " << yCoord << " " << zCoord;
					drawChunk(xCoord, yCoord, zCoord, xCoord+chunkSize_-1, yCoord+chunkSize_-1, zCoord+chunkSize_-1);
					return;	
				}
				++chunkId;
			}
		}
	}
	
}

void WorldMapScene::createSelectionMark(int radius)
{
	//destroy current mark
	destroyAllAttachedMovableObjects(selectionMarkNode_);
	selectionMarkNode_->removeAndDestroyAllChildren();
	//create a new mark
	int xSize = radius-1;
	for (int x=-xSize; x<=xSize; ++x) {
		int ySize = xSize - std::abs(x);
		LOG(INFO) << "xSize: " << xSize << " ySize: " << ySize;
		for (int y=-ySize; y<=ySize; ++y) {
			LOG(INFO) << "x=" << x << " y=" << y;
			std::string entityName = "selectionMark" + Ogre::StringConverter::toString(x) + "-" + Ogre::StringConverter::toString(y);
			Ogre::Entity *mark = sceneMgr_->createEntity(entityName, "meshSelectionMarker");
			Ogre::SceneNode *node = selectionMarkNode_->createChildSceneNode();
			node->setPosition(x*cubeSize_, 0, y*cubeSize_);
			node->attachObject(mark);
		}
	}
}


bool WorldMapScene::initGui(Ogre::RenderWindow *window)
{
	gui_ = std::unique_ptr<WorldMapGui>(new WorldMapGui(window, sceneMgr_));
	
	subscribe("mouseMoved", [this](std::string eventName, Arguments args){
		if (!gui_->hasFocus()) checkSelection(boost::any_cast<int>(args["Xabs"]), boost::any_cast<int>(args["Yabs"]));
	});
	subscribe("selectedCubeUpdated", [this](std::string eventName, Arguments args){
		updateSelection();
	});
	subscribe("mousePressed", [this](std::string eventName, Arguments args){
		if (!gui_->hasFocus()){
			std::string matter = gui_->getSelectedMatter();
			if ((matter != "") && (selectedCube_.x != -1)) {
				Arguments arg;
				arg["matter"] = matter;
				arg["x"] = selectedCube_.x;
				arg["y"] = selectedCube_.y;
				arg["z"] = selectedCube_.z;
				arg["radius"] = gui_->getRadius();
				EventMgrFactory::getCurrentEvtMgr()->sendEvent("changeVoxelType", arg);
			}
			mouseButtonPressed = true;
		}
	});
	subscribe("mouseReleased", [this](std::string eventName, Arguments args){
		mouseButtonPressed = false;
	});	
	return true;
}

void WorldMapScene::checkSelection(int x, int y)
{
	//check if mouse is under a chunk	
	int height = camera_->getCameraPtr()->getViewport()->getActualHeight();
	int width = camera_->getCameraPtr()->getViewport()->getActualWidth();
	Ogre::Ray ray = camera_->getCameraPtr()->getCameraToViewportRay(x/float(width), y/float(height));
	Ogre::RaySceneQuery *rayQuery = sceneMgr_->createRayQuery(ray);
	rayQuery->setSortByDistance(true);
	Ogre::RaySceneQueryResult res = rayQuery->execute();
	
	//only interested in the first _chunk
	for (auto &ent : res) {
		Ogre::MovableObject *obj = ent.movable;
		if (obj) {
			//~ LOG(INFO) << "Something under the mouse: " << obj->getName();
			if (obj->getName().find("_chunk") != std::string::npos) {
				//calculate which cube it is
				Ogre::Vector3 point = rayQuery->getRay().getPoint(ent.distance);
				//~ LOG(INFO) << xCoord << " " << zCoord;
				Coordinates newSelec;
				newSelec.x = (point.x-0.0001f) / cubeSize_;
				newSelec.y = 0;
				newSelec.z = (point.z-0.0001f) / cubeSize_;
				if (!(newSelec.x == selectedCube_.x && newSelec.y == selectedCube_.y && newSelec.z == selectedCube_.z)) {
					selectedCube_ = newSelec;
					EventMgrFactory::getCurrentEvtMgr()->sendEvent("selectedCubeUpdated");
					if (mouseButtonPressed) {
						//maybe it's a mouse drag
						std::string matter = gui_->getSelectedMatter();
						if (matter != "") {
							Arguments arg;
							arg["matter"] = matter;
							arg["x"] = selectedCube_.x;
							arg["y"] = selectedCube_.y;
							arg["z"] = selectedCube_.z;
							arg["radius"] = gui_->getRadius();
							EventMgrFactory::getCurrentEvtMgr()->sendEvent("changeVoxelType", arg);
						}
					}
				}
				return;		
			}
		
		}
	}
	//~ LOG(INFO) << "Nothing under the mouse";
	if (selectedCube_.x > -1) {
		selectedCube_.x = -1;
		selectedCube_.y = -1;
		selectedCube_.z = -1;
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("selectedCubeUpdated");	
	}
}

void WorldMapScene::updateSelection()
{
	selectionMarkNode_->setVisible(false);
	if (selectedCube_.x != -1) {
		//set the selection mark node to the right position
		selectionMarkNode_->setPosition(selectedCube_.x*cubeSize_, cubeSize_ + 1.0f, selectedCube_.z*cubeSize_);
		selectionMarkNode_->setVisible(true);
		
		
		//TODO update gui
		//~ LOG(INFO) << selectedChunk_->getName();
		//get the voxel corresponding this entity
		//~ Coordinates coord = entityNameToCoordinates(selectedChunk_->getName());
		//~ Voxel *vox = (*worldMap_)->getVoxel(coord.x, coord.y, coord.z);
		//~ if (vox->getId() == "matter") {
			//~ MatterVoxel *matVox = static_cast<MatterVoxel*>(vox);
			//~ LOG(INFO) << matVox->getType();
		//~ }
	}
	
	
}

Coordinates WorldMapScene::entityNameToCoordinates(std::string entityName)
{
	Coordinates coord;	
	size_t found;
	size_t posFirstDash;
	size_t posSecondDash;

	//get position of the first "-"
	found = entityName.find('-');
	if (found == std::string::npos) {
		LOG(ERROR) << "string not correctly formated: " << entityName;
	}
	posFirstDash = found;
	//get position of the second "-"
	found = entityName.find('-', posFirstDash+1);
	if (found == std::string::npos) {
		LOG(ERROR) << "string not correctly formated: " << entityName;
	}
	posSecondDash = found;
	if (entityName.find('_') != std::string::npos) {
		LOG(ERROR) << "This format of string is not supported: " << entityName;
	}
	
	std::istringstream strX(entityName.substr(0, posFirstDash));
	std::istringstream strY(entityName.substr(posFirstDash+1, posSecondDash));
	std::istringstream strZ(entityName.substr(posSecondDash+1));	
	strX >> coord.x;
	strY >> coord.y;
	strZ >> coord.z;
	LOG(INFO) << "Coordonates converted from string: x= " << coord.x << " y= " << coord.y << " z= " << coord.z;	
	return coord;
}

void WorldMapScene::destroyAllAttachedMovableObjects(Ogre::SceneNode *node)
{
	if(!node) return;
	// Destroy all the attached objects
	Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();
	while (itObject.hasMoreElements()) {
		node->getCreator()->destroyMovableObject(itObject.getNext());
	}
	// Recurse to child SceneNodes
	Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();
	while (itChild.hasMoreElements()) {
		Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
		destroyAllAttachedMovableObjects( pChildNode );
	}
}

int WorldMapScene::calculateChunkId(int x, int y, int z)
{
	int chunkX = x/chunkSize_;
	int chunkY = y/chunkSize_;
	int chunkZ = z/chunkSize_;
	int nbChunkX = std::ceil((*worldMap_)->getSizeX()/(float)chunkSize_);
	int nbChunkY = std::ceil((*worldMap_)->getSizeY()/(float)chunkSize_);
	int result = chunkX + chunkY*nbChunkX + chunkZ*nbChunkX*nbChunkY;
	//~ LOG(INFO) << "chunk id for cube at " << x << " " << y << " " << z << " --> " << result;
	return result;
}
