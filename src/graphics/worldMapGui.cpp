#include "worldMapGui.h"
#include <glog/logging.h>
#include "../tools.h"


WorldMapGui::WorldMapGui(Ogre::RenderWindow* window, Ogre::SceneManager *sceneMgr):
	radius_{1}
{
	platform_ = new MyGUI::OgrePlatform();
	platform_->initialise(window, sceneMgr);
	myGUI_ = new MyGUI::Gui();
	myGUI_->initialise();
	MyGUI::LayoutManager::getInstance().loadLayout("mapSceneGui.xml");	

	//set callbacks of the GUI, callback functions must be: void (MyGUI::WidgetPtr)
	exitBtn_ = myGUI_->findWidget<MyGUI::Button>("bt_exit");
	exitBtn_->eventMouseButtonClick += MyGUI::newDelegate(this, &WorldMapGui::buttonClicked);
	resizeBtn_ = myGUI_->findWidget<MyGUI::Button>("bt_resize");
	resizeBtn_->eventMouseButtonClick += MyGUI::newDelegate(this, &WorldMapGui::buttonClicked);
	matterList_ = myGUI_->findWidget<MyGUI::ListBox>("list_matter");
	matterList_->setIndexSelected(0);
	radio1_= myGUI_->findWidget<MyGUI::Button>("radio_1");
	radio1_->setStateSelected(true);
	Arguments arg;
	arg["radius"] = radius_;
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("markerRadiusChanged", arg);
	radio1_->eventMouseButtonClick += MyGUI::newDelegate(this, &WorldMapGui::buttonClicked);
	radio2_= myGUI_->findWidget<MyGUI::Button>("radio_2");
	radio2_->eventMouseButtonClick += MyGUI::newDelegate(this, &WorldMapGui::buttonClicked);
	radio3_= myGUI_->findWidget<MyGUI::Button>("radio_3");
	radio3_->eventMouseButtonClick += MyGUI::newDelegate(this, &WorldMapGui::buttonClicked);
	saveBtn_ = myGUI_->findWidget<MyGUI::Button>("bt_save");
	saveBtn_->eventMouseButtonClick += MyGUI::newDelegate(this, &WorldMapGui::buttonClicked);
	loadBtn_ = myGUI_->findWidget<MyGUI::Button>("bt_load");
	loadBtn_->eventMouseButtonClick += MyGUI::newDelegate(this, &WorldMapGui::buttonClicked);
	
	//updates to MyGUI
	subscribe("mouseMoved", [](std::string eventName, Arguments args){
		MyGUI::InputManager::getInstance().injectMouseMove(
			boost::any_cast<int>(args["Xabs"]), 
			boost::any_cast<int>(args["Yabs"]), 
			boost::any_cast<int>(args["Zabs"])
		);
	});
	subscribe("mousePressed", [](std::string eventName, Arguments args){
		MyGUI::InputManager::getInstance().injectMousePress(
			boost::any_cast<int>(args["Xabs"]), 
			boost::any_cast<int>(args["Yabs"]), 
			MyGUI::MouseButton::Enum(boost::any_cast<int>(args["id"]))
		);
	});
	subscribe("mouseReleased", [](std::string eventName, Arguments args){
		MyGUI::InputManager::getInstance().injectMouseRelease(
			boost::any_cast<int>(args["Xabs"]), 
			boost::any_cast<int>(args["Yabs"]), 
			MyGUI::MouseButton::Enum(boost::any_cast<int>(args["id"]))
		);		
	});
	subscribe("keyPressed", [](std::string eventName, Arguments args){
		MyGUI::InputManager::getInstance().injectKeyPress(
			(MyGUI::KeyCode::Enum)boost::any_cast<int>(args["key"]),
			(MyGUI::Char)boost::any_cast<unsigned int>(args["text"])
		);
	});
	subscribe("keyReleased", [](std::string eventName, Arguments args){
		MyGUI::InputManager::getInstance().injectKeyRelease(
			(MyGUI::KeyCode::Enum)boost::any_cast<int>(args["key"])
		);
	});

}

WorldMapGui::~WorldMapGui()
{
	if (myGUI_) {
		myGUI_->shutdown();
		delete myGUI_;
	}
	if (platform_) {
		platform_->shutdown();
		delete platform_;
	}
	
}

bool WorldMapGui::hasFocus()
{
	return MyGUI::InputManager::getInstance().isFocusMouse() || MyGUI::InputManager::getInstance().isFocusKey();
}

std::string WorldMapGui::getSelectedMatter()
{
	unsigned int index = matterList_->getIndexSelected();
	if (index == MyGUI::ITEM_NONE) { //doesn't seem to work
		return "";
	} else {
		return static_cast<std::string>(matterList_->getItemNameAt(index));
	}
}

int WorldMapGui::getRadius()
{
	return radius_;
}

void WorldMapGui::buttonClicked(MyGUI::WidgetPtr sender)
{
	if (sender == exitBtn_) {
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("quitGame");			
	} else if (sender == resizeBtn_) {
		//check correct values
		std::string sizeX = myGUI_->findWidget<MyGUI::Edit>("edit_sizeX")->getCaption();
		std::string sizeZ = myGUI_->findWidget<MyGUI::Edit>("edit_sizeZ")->getCaption();
		if (!tools::is_number(sizeX) || !tools::is_number(sizeZ)) {
			LOG(WARNING) << "New sizes are incorrect";
			return;
		}
		Arguments arg;
		arg["X"] = tools::stringToInt(sizeX);
		arg["Y"] = 1;
		arg["Z"] = tools::stringToInt(sizeZ);
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("resizeWorldMap", arg);
	} else if ((sender == radio1_) || (sender == radio2_) || (sender == radio3_)) {
		radio1_->setStateSelected(false);
		radio2_->setStateSelected(false);
		radio3_->setStateSelected(false);
		static_cast<MyGUI::Button*>(sender)->setStateSelected(true);
		if (sender == radio1_) radius_ = 1;
		if (sender == radio2_) radius_ = 2;
		if (sender == radio3_) radius_ = 3;
		Arguments arg;
		arg["radius"] = radius_;
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("markerRadiusChanged", arg);
	} else if (sender == saveBtn_) {
		Arguments args;
		args["filename"] = std::string(myGUI_->findWidget<MyGUI::Edit>("edit_save")->getCaption());
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("saveWorldMap", args);
	} else if (sender == loadBtn_) {
		Arguments args;
		args["data"] = std::string(myGUI_->findWidget<MyGUI::Edit>("edit_load")->getCaption());
		EventMgrFactory::getCurrentEvtMgr()->sendEvent("loadWorldMap", args);
	} else {
		LOG(WARNING) << "Unknown button clicked";
	}
	
}

