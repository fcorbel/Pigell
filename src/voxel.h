#ifndef VOXEL_H
#define VOXEL_H

#include <string>
#include <memory>

class Voxel
{
	public:
		Voxel(std::string id): id_(id) {}
		virtual ~Voxel() {}
	
		std::string getId() const { return id_; }
		virtual std::string getInfos();
		static std::shared_ptr<Voxel> createVoxel(const float red, const float green, const float blue);
		static std::shared_ptr<Voxel> createMatterVoxel(const std::string type);
	
	private:
		std::string id_;
};

#endif
