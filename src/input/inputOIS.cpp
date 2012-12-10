#include "inputOIS.h"
#include <glog/logging.h>
#include "../eventManager.h"


InputOIS::InputOIS():
	ois_{nullptr},
	keyboard_{nullptr},
	mouse_{nullptr},
	keymap_{nullptr}
{

}

InputOIS::~InputOIS()
{
	LOG(INFO) << "Delete InputOIS";
	if (ois_) {
		if (keyboard_) {
			ois_->destroyInputObject(keyboard_);
			keyboard_ = nullptr;
		}
		if (mouse_) {
			ois_->destroyInputObject(mouse_);
			mouse_ = nullptr;
		}
		OIS::InputManager::destroyInputSystem(ois_);
	}
	keymap_ = nullptr;

}

bool InputOIS::initializeWithOgre(const std::string windowData, int winHeight, int winWidth, Options *keymap)
{
/*windowData comes from Ogre::RenderWindow, should be a string
*/
	OIS::ParamList pl;
	pl.insert(std::make_pair(std::string("WINDOW"),windowData));
	ois_ = OIS::InputManager::createInputSystem(pl);

	keyboard_ = (OIS::Keyboard*)ois_->createInputObject(OIS::OISKeyboard, true);
	mouse_ = (OIS::Mouse*)ois_->createInputObject(OIS::OISMouse, true);
	mouse_->setEventCallback(this);
	keyboard_->setEventCallback(this);
	
	//for the GUI
	const OIS::MouseState &mouseState = mouse_->getMouseState();
	mouseState.width = winWidth;
	mouseState.height = winHeight;

	keymap_ = keymap;

	return true;
}

void InputOIS::capture()
{	
	//Call capture to update both buffered and unbuffered devices
	if(mouse_) {
		mouse_->capture();
	} else {
		LOG(WARNING) << "Mouse not initialized";
	}
	if(keyboard_) {
		keyboard_->capture();
	} else {
		LOG(WARNING) << "Keyboard not initialized";
	}
}

bool InputOIS::keyPressed(const OIS::KeyEvent &evt)
{
	std::string key_name;
	key_name = ((OIS::Keyboard*)(evt.device))->getAsString(evt.key);
	LOG(INFO) << "key pressed: " << key_name;	
	std::string newEvent = keymap_->getValue<std::string>(key_name);
	if (newEvent != "") {
		EventMgrFactory::getCurrentEvtMgr()->sendEvent(newEvent);
	}
	//for MyGUI
	Arguments arg;
	arg["key"] = (int)evt.key;
	arg["text"] = evt.text;
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("keyPressed", arg);
	return true;
}

bool InputOIS::keyReleased(const OIS::KeyEvent &evt)
{
	std::string key_name;
	key_name = ((OIS::Keyboard*)(evt.device))->getAsString(evt.key);
	LOG(INFO) << "key released: " << key_name;	
	std::string newEvent = keymap_->getValue<std::string>("-" + key_name);
	if (newEvent != "") {
		EventMgrFactory::getCurrentEvtMgr()->sendEvent(newEvent);
	}
	//for MyGUI
	Arguments arg;
	arg["key"] = (int)evt.key;
	arg["text"] = evt.text;
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("keyReleased", arg);	
	return true;
}

bool InputOIS::mouseMoved(const OIS::MouseEvent &evt)
{
	const OIS::MouseState& s = evt.state;
	if (s.Z.rel != 0) {
		LOG(INFO) << "Mouse scroll: " << s.Z.rel;
		if (s.Z.rel > 0) {
			//scroll up
			std::string newEvent = keymap_->getValue<std::string>("scrollUp");
			if (newEvent != "") {
				Arguments arg;
				arg["Zabs"] = s.Z.abs;
				arg["Zrel"] = s.Z.rel;				
				EventMgrFactory::getCurrentEvtMgr()->sendEvent(newEvent, arg);
			}
		} else {
			//scroll down
			std::string newEvent = keymap_->getValue<std::string>("scrollDown");
			if (newEvent != "") {
				Arguments arg;
				arg["Zabs"] = s.Z.abs;
				arg["Zrel"] = s.Z.rel;				
				EventMgrFactory::getCurrentEvtMgr()->sendEvent(newEvent, arg);
			}			
		}
	} else {
	
		//~ LOG(INFO) << "Mouse moved";
		Arguments arg;
		arg["Xabs"] = s.X.abs;
		arg["Yabs"] = s.Y.abs;
		arg["Zabs"] = s.Z.abs;
		arg["Xrel"] = s.X.rel;
		arg["Yrel"] = s.Y.rel;
		arg["Zrel"] = s.Z.rel;
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("mouseMoved", arg);
	}
	return true;
}

bool InputOIS::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	//for MyGUI
	const OIS::MouseState& s = evt.state;
	Arguments arg;
	arg["Xabs"] = s.X.abs;
	arg["Yabs"] = s.Y.abs;
	arg["Zabs"] = s.Z.abs;
	arg["Xrel"] = s.X.rel;
	arg["Yrel"] = s.Y.rel;
	arg["Zrel"] = s.Z.rel;
	arg["id"] = (int)id;
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("mousePressed", arg);

	//~ LOG(INFO) << "Mouse button pressed: " << id;
	//~ std::string buttonName;
	//~ switch(id) {
		//~ case 0:
			//~ buttonName = "Mb_Left";
			//~ break;
		//~ case 1:
			//~ buttonName = "Mb_Right";
			//~ break;
		//~ default:
			//~ break;
	//~ }
	//~ std::string message;
	//~ message = keymap->getValue<std::string>(buttonName);
	//~ if (message != "") {
		//~ EventManager::Arguments arg;
		//~ arg["pressed"] = true;
		//~ EventManager::sendEvent(message, arg);
	//~ }
//~ 
	//~ 
	return true;
}

bool InputOIS::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	//for MyGUI
	const OIS::MouseState& s = evt.state;
	Arguments arg;
	arg["Xabs"] = s.X.abs;
	arg["Yabs"] = s.Y.abs;
	arg["Zabs"] = s.Z.abs;
	arg["Xrel"] = s.X.rel;
	arg["Yrel"] = s.Y.rel;
	arg["Zrel"] = s.Z.rel;
	arg["id"] = (int)id;
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("mouseReleased", arg);

	//~ LOG(INFO) << "Mouse button released: " << id;
	//~ std::string buttonName;
	//~ switch(id) {
		//~ case 0:
			//~ buttonName = "Mb_Left";
			//~ break;
		//~ case 1:
			//~ buttonName = "Mb_Right";
			//~ break;
		//~ default:
			//~ break;
	//~ }
	//~ std::string message;
	//~ message = keymap->getValue<std::string>(buttonName);
	//~ if (message != "") {
		//~ EventManager::Arguments arg;
		//~ arg["pressed"] = false;
		//~ EventManager::sendEvent(message, arg);
	//~ }
	return true;
}
