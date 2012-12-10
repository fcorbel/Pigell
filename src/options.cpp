#include "options.h"


Options::Options(std::map<std::string, std::string> defaults)
{
	options = defaults;
}

Options::~Options()
{

}

bool Options::setConfigFile(std::string file)
{
	//check if file exist and is readable
	std::ifstream config_file( file.c_str() );
	if (config_file) {
		std::string line;
		//read all lines
		while (std::getline(config_file, line)) {
			//remove all spaces
			line.erase(std::remove(line.begin(), line.end(), ' '),line.end());
			//remove comments (#)
			size_t com = line.find('#');
			if (com != std::string::npos) {
				line.erase(com);
			}
			size_t equal = line.find_first_of('=');
			if (equal != std::string::npos) {
				options[line.substr(0, equal)] = line.substr(equal+1);
			}
		}
	} else {
		LOG(WARNING) << "Config file: " << file << " doesn't exist or is not readable";
		return false;
	}

	return true;
}

bool Options::writeToFile(std::string file_name)
{
	std::ofstream file(file_name.c_str());
	if (file.is_open()) {
		file << "#Config file generated automatically\n";
		file << "#Usage:\n";
		file << "#this is a comment\n";
		file << "#option=value\n\n";
		std::map<std::string, std::string>::iterator it;
		for (it=options.begin(); it != options.end(); ++it) {
			file << it->first << "=" << it->second << "\n";
		}
		file.close();
	} else {
		LOG(WARNING) << "Unable to open file: " << file_name;
		return false;
	}

	return true;
}

void Options::showOptions() const
{	
		if (!isEmpty()) {
			LOG(INFO) << "== List of all options ==";
			std::map<std::string, std::string>::const_iterator it;
			for (it = options.begin(); it != options.end(); ++it) {
				LOG(INFO) << it->first << " = " << it->second;
			}
		}
}

bool Options::setValue(std::string name, std::string val)
{
	options[name] = val;
	return true;
}

bool Options::isEmpty() const
{
	return options.empty();
}
