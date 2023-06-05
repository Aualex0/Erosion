
#include "scene.hpp"
#include "terrain.hpp"
#include <ctime>
#include <cstdlib>
#include <map>



using namespace cgp;


std::vector<float> offset;
std::map<std::tuple<float, float>, float> deformations;
bool first_mesh_creation = true;
mesh terrain; // temporary terrain storage (CPU only)

// Evaluate 3D position of the terrain for any (x,y)
std::vector<float> initiate_terrain_height(float x, float y, float terrain_length)
{
    float z = -10.0f;
    
    //perlin noise
    float u = (x+terrain_length/2)/terrain_length;
    float v = (y+terrain_length/2)/terrain_length;
    float const noise1 = 15.0f*noise_perlin({(u + offset[0])/100, (v + offset[1])/100}, 2, 0.6f, 70.0f);
    float const noise2 = 3.0f*noise_perlin({(u + offset[0])/100, (v + offset[1])/100}, 2, 0.6f, 400.0f);
    float const noise3 = 0.25f*noise_perlin({(u + offset[0])/100, (v + offset[1])/100}, 2, 0.6f, 1500.0f);
    
    float const noise = noise1 + noise2 + noise3;
    z += noise;

    std::vector<float> res;
    res.push_back(z);
    res.push_back(noise/10.0f);

    return res;
}

float evaluate_terrain_height(int N, float x, float y, float terrain_length) 
{
    // Access table coordinates (ku,kv) 
    float ku = static_cast<int>((x/terrain_length + 0.5f)*(N-1.0f));
    float kv = static_cast<int>((y/terrain_length + 0.5f)*(N-1.0f));

    float percX = (x/terrain_length + 0.5f)*(N-1.0f) - ku;
    float percY = (y/terrain_length + 0.5f)*(N-1.0f) - kv;

    if (ku<0 || kv<0 || ku>=N-2 || ku>=N-2) {return 0.0f;}
    return terrain.position[kv+N*ku].z*percX*percY + terrain.position[kv+1+N*ku].z*percX*(1-percY) + terrain.position[kv+N*(ku+1)].z*(1-percX)*percY + terrain.position[kv+1+N*(ku+1)].z*(1-percX)*(1-percY);
}

void update_terrain_height(int N, float x, float y, float value, float terrain_length)
{
    // Access table coordinates (ku,kv)  
    float ku = static_cast<int>((x/terrain_length + 0.5f)*(N-1.0f));
    float kv = static_cast<int>((y/terrain_length + 0.5f)*(N-1.0f));

    if (ku<3 || kv<3 || ku>=N-5 || ku>=N-5) {return;}
    terrain.position[kv+N*ku].z+=value;
    terrain.position[kv+1+N*ku].z+=value;
    terrain.position[kv+1+N*(ku+1)].z+=value;
    terrain.position[kv+N*(ku+1)].z+=value;

    terrain.position[kv-1+N*ku].z+=0.8*value;
    terrain.position[kv+N*(ku-1)].z+=0.8*value;
    terrain.position[kv+2+N*ku].z+=0.8*value;
    terrain.position[kv+1+N*(ku-1)].z+=0.8*value;
    terrain.position[kv+2+N*(ku+1)].z+=0.8*value;
    terrain.position[kv+1+N*(ku+2)].z+=0.8*value;
    terrain.position[kv-1+N*(ku+1)].z+=0.8*value;
    terrain.position[kv+N*(ku+2)].z+=0.8*value;

    terrain.position[kv-2+N*ku].z+=0.5*value;
    terrain.position[kv+N*(ku-2)].z+=0.5*value;
    terrain.position[kv+3+N*ku].z+=0.5*value;
    terrain.position[kv+1+N*(ku-2)].z+=0.5*value;
    terrain.position[kv+3+N*(ku+1)].z+=0.5*value;
    terrain.position[kv+1+N*(ku+3)].z+=0.5*value;
    terrain.position[kv-2+N*(ku+1)].z+=0.5*value;
    terrain.position[kv+N*(ku+3)].z+=0.5*value;
    terrain.position[kv-1+N*(ku-1)].z+=0.5*value;
    terrain.position[kv+2+N*(ku+2)].z+=0.5*value;
    terrain.position[kv+2+N*(ku-1)].z+=0.5*value;
    terrain.position[kv-1+N*(ku+2)].z+=0.5*value;

    terrain.position[kv-3+N*ku].z+=0.2*value;
    terrain.position[kv+N*(ku-3)].z+=0.2*value;
    terrain.position[kv+4+N*ku].z+=0.2*value;
    terrain.position[kv+1+N*(ku-3)].z+=0.2*value;
    terrain.position[kv+4+N*(ku+1)].z+=0.2*value;
    terrain.position[kv+1+N*(ku+4)].z+=0.2*value;
    terrain.position[kv-3+N*(ku+1)].z+=0.2*value;
    terrain.position[kv+N*(ku+4)].z+=0.2*value;
    terrain.position[kv-1+N*(ku-2)].z+=0.2*value;
    terrain.position[kv-1+N*(ku+3)].z+=0.2*value;
    terrain.position[kv+2+N*(ku-2)].z+=0.2*value;
    terrain.position[kv+2+N*(ku+3)].z+=0.2*value;
    terrain.position[kv-2+N*(ku-1)].z+=0.2*value;
    terrain.position[kv+3+N*(ku-1)].z+=0.2*value;
    terrain.position[kv-2+N*(ku+2)].z+=0.2*value;
    terrain.position[kv+3+N*(ku+2)].z+=0.2*value;
}

void initiate_terrain_mesh(int N, float terrain_length)
{
    srand (static_cast <unsigned> (time(0)));
    offset.push_back(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/99)));
    offset.push_back(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/99)));

    terrain.position.resize(N*N);
    terrain.uv.resize(N*N);
    terrain.color.resize(N*N);

    // Fill terrain geometry
    for(int ku=0; ku<N; ++ku)
    {
        for(int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            float u = ku/(N-1.0f);
            float v = kv/(N-1.0f);

            // Compute the real coordinates (x,y) of the terrain in [-terrain_length/2, +terrain_length/2]
            float x = (u - 0.5f) * terrain_length;
            float y = (v - 0.5f) * terrain_length;

            // Compute the surface height function at the given sampled coordinate
            std::vector<float> noised_height = initiate_terrain_height(x,y,terrain_length);
            float z = noised_height[0];
            float noise = noised_height[1];

            //adapt color
            terrain.color[kv+N*ku] = vec3(1,1,1);

            // Store vertex coordinates
            terrain.position[kv+N*ku] = {x,y,z};
            terrain.uv[kv+N*ku] = {u*10, v*10};
        }
    }
}

mesh get_terrain_mesh(int N) {
    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    for(int ku=0; ku<N-1; ++ku)
    {
        for(int kv=0; kv<N-1; ++kv)
        {
            unsigned int idx = kv + N*ku; // current vertex offset

            uint3 triangle_1 = {idx, idx+1+N, idx+1};
            uint3 triangle_2 = {idx, idx+N, idx+1+N};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

    // need to call this function to fill the other buffer with default values (normal, color, etc)
	terrain.fill_empty_field(); 


    return terrain;
}

std::vector<vec3> trace_droplet(int N_terrain_samples, float terrain_length) 
{
    std::vector<vec3> positions;
    float mineraux = 0.0f;
    cgp::vec2 inertia = vec2(0,0);
    float step = terrain_length/N_terrain_samples/4.0f;
    float speed = 0.0f;
    
    float current_x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(terrain_length-0.2f))) - (terrain_length-0.2f)/2.0f;
    float current_y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(terrain_length-0.2f))) - (terrain_length-0.2f)/2.0f;
    positions.push_back(vec3(current_x, current_y, evaluate_terrain_height(N_terrain_samples, current_x, current_y, terrain_length)));

    for(int i=0; i<50; ++i)
    {
        cgp::vec3 A = vec3(current_x + step, current_y + step, evaluate_terrain_height(N_terrain_samples, current_x + step, current_y + step, terrain_length)); 
        cgp::vec3 B = vec3(current_x - step, current_y + step, evaluate_terrain_height(N_terrain_samples, current_x - step, current_y + step, terrain_length));
        cgp::vec3 C = vec3(current_x + step, current_y - step, evaluate_terrain_height(N_terrain_samples, current_x + step, current_y - step, terrain_length));
        cgp::vec3 D = vec3(current_x - step, current_y - step, evaluate_terrain_height(N_terrain_samples, current_x - step, current_y - step, terrain_length));
        vec3 N = cross(A - D, B - C); //perform cross product of two lines on plane
        if(norm(N) > 0)
        {
            N = normalize(N);  //normalize
        }

        speed = std::min(0.9f, (static_cast<float>(sqrt(1-N.z*N.z)) + speed)/2.0f);


        float rX = 0.8f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.3f-0.7f)));
        float rY = 0.8f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.3f-0.f)));
        float future_x = current_x + (step*N.x + speed*inertia.x)*rX;
        float future_y = current_y + (step*N.y + speed*inertia.y)*rY;

        //cgp::vec2 proj = vec2(N.x, N.y);
        float value;
        bool toBreak = false;
        if (evaluate_terrain_height(N_terrain_samples, future_x,  future_y, terrain_length) - evaluate_terrain_height(N_terrain_samples, current_x, current_y, terrain_length) > 0) {
            value = std::min(std::min(0.001f, abs(evaluate_terrain_height(N_terrain_samples,future_x,future_y,terrain_length)-evaluate_terrain_height(N_terrain_samples, current_x, current_y, terrain_length))/2.0f),mineraux); 
            mineraux -= value; 
            if (future_x == current_x && future_y == current_y) {toBreak = true;}
        } else {
            value = std::max(static_cast<float>(-0.001f*sqrt(1-N.z*N.z)), (evaluate_terrain_height(N_terrain_samples,future_x,future_y,terrain_length)-evaluate_terrain_height(N_terrain_samples, current_x, current_y, terrain_length))/2.0f);
            mineraux -= value;
        }   //Ici, attention à ce que en descendant le point en question, on ne le descende pas en dessous du poit sur lequel on va : cela pourrait causer des problèmes
        if (mineraux < 0.0f) {toBreak = true;} 

        update_terrain_height(N_terrain_samples, current_x, current_y, value, terrain_length);

        A = vec3(current_x + step, current_y + step, evaluate_terrain_height(N_terrain_samples, current_x + step, current_y + step, terrain_length)); 
        B = vec3(current_x - step, current_y + step, evaluate_terrain_height(N_terrain_samples, current_x - step, current_y + step, terrain_length));
        C = vec3(current_x + step, current_y - step, evaluate_terrain_height(N_terrain_samples, current_x + step, current_y - step, terrain_length));
        D = vec3(current_x - step, current_y - step, evaluate_terrain_height(N_terrain_samples, current_x - step, current_y - step, terrain_length));
        N = cross(A - D, B - C); //perform cross product of two lines on plane
        if(norm(N) > 0)
        {
            N = normalize(N);  //normalize
        }
        current_x = future_x;
        current_y = future_y;
        inertia = 0.9f*inertia + step*vec2(N.x,N.y);

        if ((current_x < -terrain_length + 0.1f || current_x > terrain_length - 0.1f) || (current_y < -terrain_length + 0.1f || current_y > terrain_length - 0.1f)) {break;}
        if (toBreak) {break;}

        positions.push_back(vec3(current_x, current_y, evaluate_terrain_height(N_terrain_samples, current_x, current_y, terrain_length)));
    }

    return positions;
}

