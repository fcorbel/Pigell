#include "colouredVoxel.h"
#include <sstream>
#include <glog/logging.h>

ColouredVoxel::ColouredVoxel(float red, float green, float blue):
	Voxel("coloured"),
	red_(red),
	green_(green),
	blue_(blue)
{
	if (red < 0.0f || red > 1.0f || green < 0.0f || green > 1.0f || blue < 0.0f || blue > 1.0f) {
		LOG(WARNING) << "Values to create a new ColouredVoxel must be between 0 and 1";
		red_ = 0;
		green_ = 0;
		blue_ = 0;
	}
}

ColouredVoxel::ColouredVoxel(Colour colour):
	Voxel("coloured"),
	red_(colour.red),
	green_(colour.green),
	blue_(colour.blue)
{
	if (colour.red < 0.0f || colour.red > 1.0f || colour.green < 0.0f || colour.green > 1.0f || colour.blue < 0.0f || colour.blue > 1.0f) {
		LOG(WARNING) << "Values to create a new ColouredVoxel must be between 0 and 1";
		red_ = 0;
		green_ = 0;
		blue_ = 0;
	}
	
}

ColouredVoxel::~ColouredVoxel()
{
	
}

std::string ColouredVoxel::getInfos()
{
	return "#" + getHexColour();
}

Colour ColouredVoxel::getColour() const
{
	Colour col;
	col.red = red_;
	col.green = green_;
	col.blue = blue_;
	return col;
}

std::string ColouredVoxel::getHexColour() const
{
	int r = red_*255;
	int g = green_*255;
	int b = blue_*255;

	std::stringstream stream;
	if (r == 0) {
		stream << "00";
	} else {
		stream << std::hex << r;
	}
	if (g == 0) {
		stream << "00";
	} else {
		stream << std::hex << g;
	}
	if (b == 0) {
		stream << "00";
	} else {
		stream << std::hex << b;
	}
	std::string result(stream.str());
	return result;
}


std::ostream& operator<< (std::ostream& os, const ColouredVoxel& obj)
{
	Colour col = obj.getColour();
	os << "Type: coloured || colour: r=" << col.red << " g=" << col.green << " b=" << col.blue;
	return os;
}
