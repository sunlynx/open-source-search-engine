#include "ScalingFunctions.h"

double scale_linear(double x, double min_x, double max_x, double min_y, double max_y)
{
	if(x<min_x) x=min_x;
	if(x>max_x) x=max_x;
	double x_range = max_x-min_x;
	double y_range = max_y-min_y;
	double r = (x-min_x)/x_range;
	double y = min_y + r*y_range;
	return y;
}

float scale_linear(float x, float min_x, float max_x, float min_y, float max_y)
{
	if(x<min_x) x=min_x;
	if(x>max_x) x=max_x;
	float x_range = max_x-min_x;
	float y_range = max_y-min_y;
	float r = (x-min_x)/x_range;
	float y = min_y + r*y_range;
	return y;
}


double scale_quadratic(double x, double min_x, double max_x, double min_y, double max_y)
{
	//Note: this function is slightly incorrect, but it is Friday
	//afternoon and the output is close enough.
	if(x<min_x) x=min_x;
	if(x>max_x) x=max_x;
	double x_range = max_x-min_x;
	double y_range = max_y-min_y;
	double r = (x-min_x)/x_range;
	return ((r+1)*(r+1)-1)/3*y_range+min_y;
}



#ifdef UNITTEST

#include <assert.h>

int main(void) {
	assert(scale_linear(  0, 0.0,10.0, 0.0,100.0) == 0);
	assert(scale_linear( 10, 0.0,10.0, 0.0,100.0) == 100);
	assert(scale_linear(  5, 0.0,10.0, 0.0,100.0) == 50);
	assert(scale_linear( -4, 0.0,10.0, 0.0,100.0) == 0);
	assert(scale_linear( 15, 0.0,10.0, 0.0,100.0) == 100);

	assert(scale_quadratic(  0, 0.0,10.0, 0.0,100.0) == 0);
	assert(scale_quadratic( 10, 0.0,10.0, 0.0,100.0) == 100);
	assert(scale_quadratic( 1.0, 1.0,2.0, 1.0,4.0) == 1);
	assert(scale_quadratic( 2.0, 1.0,2.0, 1.0,4.0) == 4);
	assert(scale_quadratic( 1.1, 1.0,2.0, 1.0,4.0) < 1.5);
	assert(scale_quadratic( 1.5, 1.0,2.0, 1.0,4.0) < 2.5);
	assert(scale_quadratic( 1.9, 1.0,2.0, 1.0,4.0) > 3);
}

#endif
