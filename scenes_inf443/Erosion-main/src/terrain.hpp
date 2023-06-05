#pragma once

#include "cgp/cgp.hpp"


std::vector<float> intiate_terrain_height(float x, float y, float terrain_length);
float evaluate_terrain_height(int N, float x, float y, float terrain_length);

/** Compute a terrain mesh 
	The (x,y) coordinates of the terrain are set in [-length/2, length/2].
	The z coordinates of the vertices are computed using evaluate_terrain_height(x,y).
	The vertices are sampled along a regular grid structure in (x,y) directions. 
	The total number of vertices is N*N (N along each direction x/y) 	*/

void initiate_terrain_mesh(int N, float length);
cgp::mesh get_terrain_mesh(int N);
std::vector<cgp::vec3> trace_droplet(int N, float terrain_length);

