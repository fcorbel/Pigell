#include "voxel.h"
#include "colouredVoxel.h"
#include "matterVoxel.h"

std::string Voxel::getInfos()
{
	return std::string{"raw"};
}

std::shared_ptr<Voxel> Voxel::createVoxel(const float red, const float green, const float blue)
{
	return std::shared_ptr<Voxel>(new ColouredVoxel(red, green, blue));
}

std::shared_ptr<Voxel> Voxel::createMatterVoxel(const std::string type)
{
		return std::make_shared<MatterVoxel>(type);
}
