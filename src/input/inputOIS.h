#ifndef INPUTOIS_H
#define INPUTOIS_H

#include <string>
#include "OIS/OISInputManager.h"
#include "OIS/OISException.h"
#include "OIS/OISKeyboard.h"
#include "OIS/OISMouse.h"
#include "OIS/OISEvents.h"

#include "../options.h"



class InputOIS :
	public OIS::KeyListener,
	public OIS::MouseListener
{
	public:
		InputOIS();
		virtual ~InputOIS();


		bool initializeWithOgre(const std::string windowData, int winHeight, int winWidth, Options *keymap);
		void capture();

		// KeyListener
		virtual bool keyPressed(const OIS::KeyEvent&);
		virtual bool keyReleased(const OIS::KeyEvent&);

		// MouseListener
		virtual bool mouseMoved(const OIS::MouseEvent&);
		virtual bool mousePressed(const OIS::MouseEvent&, OIS::MouseButtonID);
		virtual bool mouseReleased(const OIS::MouseEvent&, OIS::MouseButtonID);

	private:
		OIS::InputManager *ois_;
		OIS::Keyboard *keyboard_;
		OIS::Mouse *mouse_;
		Options *keymap_;

};

#endif

