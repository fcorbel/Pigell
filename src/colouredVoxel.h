#ifndef COLOUREDVOXEL_H
#define COLOUREDVOXEL_H

#include "voxel.h"
#include <iostream>

struct Colour {
	float red;
	float green;
	float blue;
};

class ColouredVoxel: public Voxel
{
	public:
		ColouredVoxel(float red, float green, float blue);
		ColouredVoxel(Colour colour);
		~ColouredVoxel();
	
		virtual std::string getInfos();
		Colour getColour() const;
		std::string getHexColour() const;
		
	private:
		float red_;
		float green_;
		float blue_;
};

std::ostream& operator<< (std::ostream& os, const ColouredVoxel& obj);


#endif /* COLOUREDVOXEL_H */ 
