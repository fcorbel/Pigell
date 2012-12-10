#include "game.h"
#include <glog/logging.h>

Game::Game():
	config_{nullptr},
	keymap_{nullptr},
	running_{false},
	currentMap_{nullptr},
	nbFrames_{0},
	lastFpsCalcul_{0},
	currentTime_{0},
	fpsLimit_{90},
	timeBetweenFrames_{static_cast<unsigned long>(1000/fpsLimit_)},
	lastFrameTime_(0)

{
	graphics_ = nullptr;
	input_ = nullptr;
	currentState_ = nullptr;
	
	timer_.reset();
	EventMgrFactory::createEvtMgr("main");	
	loadOptions();
	
	subscribe("quitGame", [this](std::string eventName, Arguments args){ running_ = false; });
}

Game::~Game()
{
	config_->writeToFile("options.ini");
}

bool Game::start()
{
	//start gui (Ogre)
	graphics_ = std::unique_ptr<GraphicsOgre>(new GraphicsOgre(config_.get()));	
	//start input (OIS)
	input_ = std::unique_ptr<InputOIS>(new InputOIS());
	input_->initializeWithOgre(graphics_->getWindowAttribute(), 
		config_->getValue<int>("height"), 
		config_->getValue<int>("width"), 
		keymap_.get());

	//load a state
	currentState_ = std::unique_ptr<WorldMapState>(new WorldMapState(graphics_->getOgre(), graphics_->getWindow()));
	//load a map
	Arguments args;
	args["data"] = std::string("testMap.lua");
	EventMgrFactory::getCurrentEvtMgr()->sendEvent("loadWorldMap", args);	
	
	running_ = true;
	return true;
}

bool Game::isRunning() const
{
	return running_;
}

void Game::update()
{
	//~ LOG(INFO) << "Updating game";
	unsigned long timeLapsed = timer_.getMilliseconds() - lastFrameTime_;
	input_->capture();
	EventMgrFactory::getCurrentEvtMgr()->processEvents();
	currentState_->update(timeLapsed);
	if (timeLapsed >= timeBetweenFrames_) {
		graphics_->renderFrame();
		++nbFrames_;
		lastFrameTime_ = timer_.getMilliseconds();
	}
	//calcul framerate every 2sec
	//~ LOG(INFO) << timeBetweenFrames_;
	currentTime_ = timer_.getMilliseconds();
	if ((currentTime_ - lastFpsCalcul_) > 2000) {
		int fps = nbFrames_*1000 / (currentTime_ - lastFpsCalcul_); // *1000 because time is in ms, we want it in s
		LOG(INFO) << "FPS: " << fps << " nbFrames: " << nbFrames_;
		lastFpsCalcul_ = currentTime_;
		nbFrames_ = 0;
	}
}

bool Game::loadOptions()
{
	//Load general options
	std::map<std::string, std::string> defaults;
	defaults["appname"] = "Pigell";
	defaults["ogrePluginsFolder"] = "/usr/lib/x86_64-linux-gnu/OGRE-1.8.0";
	defaults["fullscreen"] = "0";
	defaults["width"] = "800";
	defaults["height"] = "600";
	config_ = std::make_shared<Options>(defaults);
	//get additionnal options from file
	config_->setConfigFile("options.ini");
	
	//load keymap options
	std::map<std::string, std::string> defaultKeymap;
	defaultKeymap["Escape"] = "quitGame";
	defaultKeymap["c"] = "changeBackgroundColour";
	defaultKeymap["Left"] = "startScrollCamLeft";
	defaultKeymap["-Left"] = "stopScrollCamLeft";
	defaultKeymap["Up"] = "startScrollCamUp";
	defaultKeymap["-Up"] = "stopScrollCamUp";
	defaultKeymap["Right"] = "startScrollCamRight";
	defaultKeymap["-Right"] = "stopScrollCamRight";
	defaultKeymap["Down"] = "startScrollCamDown";
	defaultKeymap["-Down"] = "stopScrollCamDown";
	defaultKeymap["scrollUp"] = "zoomCamera";
	defaultKeymap["scrollDown"] = "unzoomCamera";
	
	keymap_ = std::make_shared<Options>(defaultKeymap);
	
	return true;
}
