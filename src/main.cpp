#include <glog/logging.h>
#include "game.h"


int main(int argc, char* argv[])
{
	//initialize logger
	FLAGS_logtostderr = true;
	google::InitGoogleLogging(argv[0]);
	google::InstallFailureSignalHandler();
	LOG(INFO) << "Program started";
	Game pigell;
	if (pigell.start()) {
		while (pigell.isRunning()) {
			pigell.update();
		}
	}
	LOG(INFO) << "Program stoped";
	return 0;
}
