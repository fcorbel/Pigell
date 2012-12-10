#include "camera.h"
#include <glog/logging.h>

Camera::Camera(Ogre::String camName, Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window):
	scene_{sceneMgr},
	rotX_{0},
	rotY_{0},
	direction_{Ogre::Vector3::ZERO},
	rotSpeed_{0.2},
	mvtSpeed_{0.00006}
{
	cameraNode_ = scene_->getRootSceneNode()->createChildSceneNode();	
	cameraYawNode_ = cameraNode_->createChildSceneNode();
	cameraPitchNode_ = cameraYawNode_->createChildSceneNode();
	cameraRollNode_ = cameraPitchNode_->createChildSceneNode();
	camera_ = scene_->createCamera(camName);
	cameraRollNode_->attachObject(camera_);

	viewport_ = window->addViewport(camera_);
	viewport_->setBackgroundColour(Ogre::ColourValue(0.4,0.4,0.4));
	camera_->setAspectRatio(Ogre::Real(viewport_->getActualWidth()) / Ogre::Real(viewport_->getActualHeight()));

	subscribe("changeBackgroundColour", [this](std::string eventName, Arguments args){
		viewport_->setBackgroundColour(Ogre::ColourValue(1,0,1));
	});
	subscribe("startScrollCamLeft", [this](std::string eventName, Arguments args){
		moveLeft(true);
	});
	subscribe("stopScrollCamLeft", [this](std::string eventName, Arguments args){
		moveLeft(false);
	});
	subscribe("startScrollCamUp", [this](std::string eventName, Arguments args){
		moveForward(true);
	});
	subscribe("stopScrollCamUp", [this](std::string eventName, Arguments args){
		moveForward(false);
	});
	subscribe("startScrollCamRight", [this](std::string eventName, Arguments args){
		moveRight(true);
	});
	subscribe("stopScrollCamRight", [this](std::string eventName, Arguments args){
		moveRight(false);
	});
	subscribe("startScrollCamDown", [this](std::string eventName, Arguments args){
		moveBackward(true);
	});
	subscribe("stopScrollCamDown", [this](std::string eventName, Arguments args){
		moveBackward(false);
	});
	subscribe("zoomCamera", [this](std::string eventName, Arguments args){
		zoom(boost::any_cast<int>(args["Zrel"]));
	});
	subscribe("unzoomCamera", [this](std::string eventName, Arguments args){
		zoom(boost::any_cast<int>(args["Zrel"]));
	});
}

Camera::~Camera()
{
	cameraNode_->removeAndDestroyAllChildren();
	scene_->destroyCamera(camera_);
}

void Camera::update(float time_lapsed)
{
	//update rotation of the camera
	cameraYawNode_->yaw(rotX_*time_lapsed);
	cameraPitchNode_->pitch(rotY_*time_lapsed);
	//std::cout << "rotX: " << rotX_ << " rotY: " << rotY_ << std::endl;
	rotX_ = 0;
	rotY_ = 0;
	//update position of the camera
	translate(direction_*time_lapsed);
}

void Camera::setPosition(float x, float y, float z)
{
	LOG(INFO) << "Set camera to position: " << x << "-" << y << "-" << z;
	cameraNode_->setPosition(x, y, z);
}

void Camera::rotate(int x, int y, int z)
{
	rotX_ = Ogre::Degree(x*rotSpeed_);
	rotY_ = Ogre::Degree(y*rotSpeed_);
}

void Camera::moveForward(bool start)
{
	if (start && (direction_.z != -mvtSpeed_)) {
		direction_.z += -mvtSpeed_;
	} else {
		direction_.z += mvtSpeed_;
	}	
}

void Camera::moveBackward(bool start)
{	
	if (start && (direction_.z != mvtSpeed_)) {
		direction_.z += mvtSpeed_;
	} else {
		direction_.z += -mvtSpeed_;
	}
}

void Camera::moveLeft(bool start)
{
	if (start && (direction_.x != -mvtSpeed_)) {
		direction_.x += -mvtSpeed_;
	} else {
		direction_.x += mvtSpeed_;
	}
}

void Camera::moveRight(bool start)
{
	if (start && (direction_.x != mvtSpeed_)) {
		direction_.x += mvtSpeed_;
	} else {
		direction_.x += -mvtSpeed_;
	}
}

void Camera::translate(Ogre::Vector3 newDirection)
{
	cameraNode_->translate(cameraYawNode_->getOrientation()*cameraPitchNode_->getOrientation()*newDirection, Ogre::SceneNode::TS_LOCAL);
}

void Camera::lookAt(float x, float y, float z)
{
	//~ Ogre::Vector3 target(x,y,z);
	//~ Ogre::Vector3 position = cameraNode->getPosition();
	//maybe shitty
	//~ camera->lookAt(x,y,z);	
}

void Camera::zoom(int zoomValue)
{
	LOG(INFO) << "zoom of value: " << zoomValue;
	Ogre::Vector3 dir = camera_->getRealDirection();
	cameraNode_->translate(dir*zoomValue);
}
