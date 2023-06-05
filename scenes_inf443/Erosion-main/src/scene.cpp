#include "scene.hpp"

#include "terrain.hpp"

using namespace cgp;


float terrain_length = 15;
int N_terrain_samples = 250;
std::vector<cgp::mesh> terrain_mesh;
int simmed = 0;
int current = 10;

void scene_structure::initialize()
{
	camera_control.initialize(inputs, window); // Give access to the inputs and window global state to the camera controler
	camera_control.camera_model.set_rotation_axis({0,0,1});
	camera_control.camera_model.look_at({ 15.0f,6.0f,6.0f }, {0,0,0});
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	// CREATION DE LA SKYBOX 

	image_structure image_skybox_template = image_load_file(project::path + "assets/skybox_01.png");
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);
	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(image_grid[1], image_grid[7], image_grid[5], image_grid[3], image_grid[10], image_grid[4]);


	initiate_terrain_mesh(N_terrain_samples, terrain_length);
	
	
	/*for (int i=0; i<500000; ++i) {
		drops_of_water.push_back(trace_droplet(N_terrain_samples, terrain_length));
	}*/
	mesh droplet = mesh_primitive_sphere(0.02f, {0,0,0});

	sphere.initialize_data_on_gpu(droplet);
	sphere.material.color = {0,0,1};

	mesh ter_mesh = get_terrain_mesh(N_terrain_samples);
	terrain_mesh.push_back(ter_mesh);
	terrain_mesh[0].fill_empty_field();

	cgp::mesh_drawable ter;
	terrain.push_back(ter);
	terrain[0].initialize_data_on_gpu(terrain_mesh[0]);
	terrain[0].material.color = { 1.0f,1.0f,1.0f };
	terrain[0].material.phong.specular = 0.1f; // non-specular grass is treated in shader
	terrain[0].texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/texture_grass.jpg",
		GL_REPEAT,
		GL_REPEAT);
	terrain[0].supplementary_texture["pierre"].load_and_initialize_texture_2d_on_gpu(project::path + "assets/rock.jpg", GL_REPEAT, GL_REPEAT);
	terrain[0].supplementary_texture["neige"].load_and_initialize_texture_2d_on_gpu(project::path + "assets/neige.jpg", GL_REPEAT, GL_REPEAT);
}

void scene_structure::simulation_step() {
	if (simmed < 10 && gui.erosion) {
		for (int i=0; i<50000; ++i) {
			drops_of_water.push_back(trace_droplet(N_terrain_samples, terrain_length));
		}

		mesh ter_mesh = get_terrain_mesh(N_terrain_samples);
		terrain_mesh.push_back(ter_mesh);
		terrain_mesh[terrain.size() - 1].fill_empty_field();

		cgp::mesh_drawable ter;
		terrain.push_back(ter);
		terrain[terrain.size() - 1].initialize_data_on_gpu(terrain_mesh[terrain.size() - 1]);
		terrain[terrain.size() - 1].material.color = { 1.0f,1.0f,1.0f };
		terrain[terrain.size() - 1].material.phong.specular = 0.1f; // non-specular grass is treated in shader
		terrain[terrain.size() - 1].texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/texture_grass.jpg",
			GL_REPEAT,
			GL_REPEAT);
		terrain[terrain.size() - 1].supplementary_texture["pierre"].load_and_initialize_texture_2d_on_gpu(project::path + "assets/rock.jpg", GL_REPEAT, GL_REPEAT);
		terrain[terrain.size() - 1].supplementary_texture["neige"].load_and_initialize_texture_2d_on_gpu(project::path + "assets/neige.jpg", GL_REPEAT, GL_REPEAT);
		
		draw(terrain[terrain.size() - 1], environment);
		simmed += 1;
	} else {
		if (current != gui.step_simmed && gui.erosion) {
			draw(terrain[gui.step_simmed], environment);
		}
	}
	
}


void scene_structure::display_frame()
{
	environment.uniform_generic.uniform_float["h0"] = gui.h0;
	environment.uniform_generic.uniform_int["diffus"] = gui.step_simmed;


	// DRAW SKYBOX
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  // re-activate depth-buffer write

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	
	if (gui.display_frame)
		draw(global_frame, environment);

	draw(terrain[terrain.size() - 1], environment);
	/*for (std::vector<cgp::vec3> trace : drops_of_water) {
		for (cgp::vec3 vect : trace) {
			sphere.model.translation = vect;
			draw(sphere, environment);
		}
	}*/
	
	if (gui.display_wireframe) {
		draw_wireframe(terrain[terrain.size() - 1], environment);
	}

	simulation_step();

}


void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	ImGui::SliderFloat("Isotherme 0°C", &gui.h0, -5.0f, 15.0f);
	if (simmed == 0) {ImGui::Checkbox("Érosion", &gui.erosion);}
	if (simmed == 10) {ImGui::SliderInt("Étapes d'érosion", &gui.step_simmed, 0, 10);}
}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}

