#include "matterVoxel.h"


MatterVoxel::MatterVoxel(std::string type):
	Voxel("matter"),
	type_{type}
{
	
}

MatterVoxel::~MatterVoxel()
{
	
}

std::string MatterVoxel::getInfos()
{
	return getId() + ":" + type_;
}

std::string MatterVoxel::getType() const
{
	return type_;
}
