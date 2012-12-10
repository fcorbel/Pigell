#ifndef MATTERVOXEL_H
#define MATTERVOXEL_H

#include "voxel.h"
#include <string>

class MatterVoxel: public Voxel
{
	public:
		MatterVoxel(std::string type);
		virtual ~MatterVoxel();

		virtual std::string getInfos();
		std::string getType() const;
	private:
		std::string type_;
};

#endif /* MATTERVOXEL_H */ 
