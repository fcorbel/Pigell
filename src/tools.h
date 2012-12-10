#include <sstream>

namespace tools {

bool is_number(const std::string& s) {
	return !s.empty() && std::find_if(s.begin(), 
	s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

int stringToInt(std::string s) {
	int result;
	std::istringstream str(s);
	str >> result;
	return result;
}

};
