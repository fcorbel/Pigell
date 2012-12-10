#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <glog/logging.h>
#include <vector>
#include <memory>
#include <fstream>

template <typename T>
class CubeMap
{
	public:
		CubeMap(int x, int y, int z);

		bool resize (int x, int y, int z);
		T* getVoxel (int x, int y, int z) const;
		bool setVoxel(const std::shared_ptr<T> voxel, int x, int y, int z);
		bool setVoxel(T *voxel, int x, int y, int z);
		int getSizeX() const { return x_; }
		int getSizeY() const { return y_; }
		int getSizeZ() const { return z_; }
		bool writeToFile(std::string filename);
		
	private:
		std::vector<std::vector<std::vector <std::shared_ptr<T>>>> cubeMap_;
		int x_;
		int y_;
		int z_;

		bool validCoord(int x, int y, int z) const;

};

template <typename T>
CubeMap<T>::CubeMap(int x, int y, int z)
{
	if (!resize(x, y, z)) {
			x_ = 0;
			y_ = 0;
			z_ = 0;
	}
}

template <typename T>
bool CubeMap<T>::resize(int x, int y, int z)
{
	//check if new sizes are corrects
	if (x <= 0 || y <= 0 || z<= 0) {
		LOG(WARNING) << "Cannot resize cubeMap with the given values: x=" << x << " y=" << y << " z=" << z;
		return false;
	}

	cubeMap_.resize(x);
	for (auto &yVal : cubeMap_) {
		yVal.resize(y);
		for (auto &zVal : yVal) {
			zVal.resize(z);
		}
	}
	
	x_ = x;
	y_ = y;
	z_ = z;
	LOG(INFO) << "cubeMap resized to size: x=" << x << " y=" << y << " z=" << z;
	return true;
}

template <typename T>
T* CubeMap<T>::getVoxel(int x, int y, int z) const
{
	if (!validCoord(x, y, z)) {
		//~ LOG(WARNING) << "Coordinates out of range: x=" << x << " y=" << y << " z=" << z;
		return nullptr;
	}
	if (cubeMap_[x][y][z] == nullptr) {
		//~ LOG(WARNING) << "Trying to access a non-initialized voxel at: x=" << x << " y=" << y << " z=" << z;
		return nullptr;
	}
	return cubeMap_[x][y][z].get();
}

template <typename T>
bool CubeMap<T>::setVoxel(const std::shared_ptr<T> voxel, int x, int y, int z)
{
	if (!validCoord(x, y, z)) {
		LOG(WARNING) << "Cannot set voxel at the given coord: x=" << x << " y=" << y << " z=" << z;
		return false;
	}
	cubeMap_[x][y][z] = voxel;
	return true;
}

template <typename T>
bool CubeMap<T>::setVoxel(T *voxel, int x, int y, int z)
{
	if (!validCoord(x, y, z)) {
		LOG(WARNING) << "Cannot set voxel at the given coord: x=" << x << " y=" << y << " z=" << z;
		return false;
	}
	auto voxPtr = std::shared_ptr<T>(voxel);
	cubeMap_[x][y][z] = voxPtr;	
	return true;
}

template <typename T>
bool CubeMap<T>::validCoord(int x, int y, int z) const
{
	return (x >= 0 && y >= 0 && z >= 0 && x < x_ && y < y_ && z < z_);
}

template <typename T>
bool CubeMap<T>::writeToFile(std::string filename)
{
	std::ofstream file(filename.c_str());
	if (file.is_open()) {
		file << "#representation of the voxel map\n#From top to bottom\n\n";
		for (int i=z_-1; i>-1; --i) {
				for (int j=0; j<y_; ++j) {
					for (int k=0; k<x_; ++k) {
						if (cubeMap_[k][j][i]) {
							file << cubeMap_[k][j][i]->getInfos();
						} else {
							file << "NULL";
						}
						file << " ";
					}
					file << "\n";
				}
				file << "\n";
		}
		file.close();
	} else {
		LOG(WARNING) << "Unable to open file: " << filename;
		return false;
	}
	return true;
}
















#endif /* CUBEMAP_H */ 
