#ifndef OPTIONS_H
#define OPTIONS_H

////////////////////////////////////////
// Load options of the game
// and check if there is an option file
// that defines option's values
////////////////////////////////////////
//use:
//
//Options keymap(std::map default);
//keymap.setConfigFile("keys.cfg");
//keymap.writeToFile("toto.cfg");
//keymap.getValue("a");
//keymap.setValue("a", "+walk");
//
//ATTENTION: doesn't support option values with spaces (like: name = value with spaces)

#include <algorithm>
#include <string>
#include <map>
#include <glog/logging.h>
#include <sstream>
#include <fstream>
#include <typeinfo>

class Options
{
	public:
		Options(std::map<std::string, std::string> = std::map<std::string, std::string>());
		~Options();
		
		bool setConfigFile(std::string);
		bool writeToFile(std::string);
		void showOptions() const;
		template <typename T>
		T getValue(const std::string& name) const
		{
			auto it = options.find(name);
			if (it == options.end()) {
				LOG(WARNING) << "Option with name: \"" << name << "\" not found";
				return T();
			} else {
				std::istringstream iss(it->second);
				T val;
				if (iss >> val != 0) {
					return val;
				} else {
					LOG(WARNING) << "couldn't convert: " << it->second << " to " << typeid(val).name();
					return T();
				}
			}

		};

		bool setValue(std::string, std::string);
		bool isEmpty() const;

	private:
		std::map<std::string, std::string> options; 



};

#endif
