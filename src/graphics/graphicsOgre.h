#ifndef GRAPHICSOGRE_H
#define GRAPHICSOGRE_H

#include <OGRE/Ogre.h>
#include <memory>
#include "../options.h"

class GraphicsOgre
{
	public:
		GraphicsOgre(const Options *config);
		~GraphicsOgre();
		void renderFrame();

		std::string getWindowAttribute() const;
		Ogre::Root* getOgre() const { return ogre_.get(); }
		Ogre::RenderWindow* getWindow() const { return window_; }
	private:
		void defineRessources();
	
		std::unique_ptr<Ogre::Root> ogre_;
		Ogre::RenderWindow *window_;
};
#endif
