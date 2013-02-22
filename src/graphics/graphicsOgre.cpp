#include "graphicsOgre.h"
#include <glog/logging.h>

GraphicsOgre::GraphicsOgre(const Options *config):
	ogre_{new Ogre::Root("", "")},
	window_{nullptr}
{
	LOG(INFO) << "Initializing Ogre";
	//use OpenGL
	const std::string pluginsFolder = config->getValue<std::string>("ogrePluginsFolder");
	ogre_->loadPlugin(pluginsFolder + "RenderSystem_GL");
	const Ogre::RenderSystemList renderList = ogre_->getAvailableRenderers();
	Ogre::RenderSystemList::const_iterator r_it;
	r_it = renderList.begin();
	ogre_->setRenderSystem(*r_it);
	ogre_->initialise(false);

	//create manualy the window
	Ogre::NameValuePairList opts;
	opts.insert(Ogre::NameValuePairList::value_type("vsync", "false"));
	opts.insert(Ogre::NameValuePairList::value_type("top", "10"));
	opts.insert(Ogre::NameValuePairList::value_type("left", "10"));
	window_ = ogre_->createRenderWindow(
		config->getValue<std::string>("appname"), 
		config->getValue<int>("width"), 
		config->getValue<int>("height"), 
		config->getValue<bool>("fullscreen"), 
		&opts);
	//initialize ressources	
	defineRessources();
	Ogre::ResourceGroupManager::getSingleton().loadResourceGroup("General");
}

GraphicsOgre::~GraphicsOgre()
{
	LOG(INFO) << "Destroying Ogre";
}

void GraphicsOgre::renderFrame()
{
	Ogre::WindowEventUtilities::messagePump();
	ogre_->renderOneFrame();
}

std::string GraphicsOgre::getWindowAttribute() const
{
	if (!window_) {
		LOG(ERROR) << "Window not created, unable to get its attributes";
		return "";
	}
	size_t windowHandle;
	window_->getCustomAttribute("WINDOW", &windowHandle);
	return Ogre::StringConverter::toString(windowHandle);	
}

void GraphicsOgre::defineRessources()
{
	Ogre::String secName, typeName, archName;
	Ogre::ConfigFile cf;
	Ogre::String resource_path = "./";
	cf.load(resource_path + "resources.cfg");

	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}


