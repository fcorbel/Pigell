#ifndef CAMERA_H
#define CAMERA_H

#include "OGRE/Ogre.h"
#include "../eventManager.h"


class Camera: public Subscribable
{
	public:
		Camera(Ogre::String camName, Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window);
		~Camera();

		void update(float speed_factor);
		Ogre::Camera* getCameraPtr() const { return camera_; }
		void setPosition(float x, float y, float z);
		void rotate(int x, int y, int z);
		
		
		void moveForward(bool start);
		void moveBackward(bool start);
		void moveLeft(bool start);
		void moveRight(bool start);
		
		void translate(Ogre::Vector3 newDirection);
		void lookAt(float x, float y, float z);
		void zoom(int zoomValue);
		//~ void changeBackgroundColour(std::string, EventManager::Arguments);


	private:
		Ogre::SceneManager* scene_;
		Ogre::Viewport* viewport_;
		Ogre::SceneNode* cameraNode_;
		Ogre::SceneNode* cameraYawNode_;
		Ogre::SceneNode* cameraPitchNode_;
		Ogre::SceneNode* cameraRollNode_;
		Ogre::Camera* camera_;
		Ogre::Radian rotX_, rotY_;
		Ogre::Vector3 direction_;
		float rotSpeed_;
		float mvtSpeed_;
};

#endif
