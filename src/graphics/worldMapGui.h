#ifndef WORLDMAPGUI_H
#define WORLDMAPGUI_H

#include "../eventManager.h"
#include <MYGUI/MyGUI.h>
#include <MYGUI/MyGUI_OgrePlatform.h>
#include "OGRE/Ogre.h"


class WorldMapGui: public Subscribable
{
	public:
		WorldMapGui(Ogre::RenderWindow* window, Ogre::SceneManager *sceneMgr);
		~WorldMapGui();

		bool hasFocus();
		std::string getSelectedMatter();
		int getRadius();
	private:
		void buttonClicked(MyGUI::WidgetPtr sender);

	
		MyGUI::Gui *myGUI_;
		MyGUI::OgrePlatform* platform_;
		int radius_;
		
		MyGUI::ButtonPtr exitBtn_;
		MyGUI::ButtonPtr resizeBtn_;
		MyGUI::ListBox *matterList_;
		MyGUI::ButtonPtr radio1_;
		MyGUI::ButtonPtr radio2_;
		MyGUI::ButtonPtr radio3_;
};

#endif
