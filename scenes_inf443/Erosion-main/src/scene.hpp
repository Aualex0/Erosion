#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"


// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// Variables associated to the GUI
struct gui_parameters {
	bool display_frame = true;
	bool display_wireframe = false;
	bool erosion = false;

	float h0 = 5.0f;
	int step_simmed = 10;
};

// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	camera_controller_first_person_euler camera_control;
	camera_projection_perspective camera_projection;
	window_structure window;

	mesh_drawable global_frame;          // The standard global frame
	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage

	numarray<std::vector<vec3>> drops_of_water;
	//std::vector<vec3> droplet_trace_positions;
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	std::vector<cgp::mesh_drawable> terrain;
	cgp::mesh_drawable sphere;
	cgp::skybox_drawable skybox;
	//cgp::mesh_drawable trunk;
	//cgp::mesh_drawable foliage;




	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();    // Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop
	void simulation_step();


	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();

};





