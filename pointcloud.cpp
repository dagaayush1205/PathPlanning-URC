#include <librealsense2/rs.hpp> // RealSense Cross Platform API
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <pcl/io/ply_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <thread>

using namespace std;
using namespace pcl;
using namespace Eigen;

// Grid map structure to hold occupancy grid and boundaries
struct Gridmap {
    vector<vector<bool>> occupancy_grid;
    float min_x, min_y, max_x, max_y;
};

// Function to create grid map from point cloud
Gridmap create_gridmap(const vector<Vector3f>& points, float grid_resolution, float height=2.0) {
    // Calculate boundaries based on points
    float min_x=numeric_limits<float>::max();
    float max_x=numeric_limits<float>::lowest();
    float min_y=numeric_limits<float>::max();
    float max_y=numeric_limits<float>::lowest();

    for (const auto& point : points) {
        min_x=min(min_x, point[0]);
        max_x=max(max_x, point[0]);
        min_y=min(min_y, point[1]);
        max_y=max(max_y, point[1]);
    }

    int x_bins = static_cast<int>((max_x-min_x)/grid_resolution)+1;
    int y_bins = static_cast<int>((max_y-min_y)/grid_resolution)+1;

    // Initialize the occupancy grid
    vector<vector<bool>> occupancy_grid(x_bins, vector<bool>(y_bins, false));

    // Fill the occupancy grid based on the height of points
    for (const auto& point : points) {
        int x_idx = static_cast<int>((point(0) - min_x) / grid_resolution);
        int y_idx = static_cast<int>((point(1) - min_y) / grid_resolution);

        if (x_idx >= 0 && x_idx < x_bins && y_idx >= 0 && y_idx < y_bins) {
            if (point(2) > height) { 
                occupancy_grid[x_idx][y_idx] = true;
            }
        }
    }
    return {occupancy_grid, min_x, min_y, max_x, max_y};
}

void draw_axes(int x_bins, int y_bins, float cellwidth, float cellheight) {
    glColor3f(0.0f,1.0f,0.0f); //axes green
    glBegin(GL_LINES);
    //x axis
    glVertex2f(-1.0f,0.0f);
    glVertex2f(1.0f,0.0f);
    //y axis
    glVertex2f(0.0f, -1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();

}

void gridmapglfw(const Gridmap& gridmap)
{
	float cellwidth=2.0/gridmap.occupancy_grid.size();
	float cellheight=2.0/gridmap.occupancy_grid[0].size();
	//float cellwidth=2.0/20;
	//float cellheight=2.0/20;
	glBegin(GL_POINTS);
	for(size_t i=0;i<gridmap.occupancy_grid.size();i++)
	{
		for(size_t j=0;j<gridmap.occupancy_grid[0].size();j++)
		{

		if(gridmap.occupancy_grid[i][j])
		{
                       glColor3f(1.0f,0.0f,0.0f);
		}
		else
		{  
			glColor3f(1.0f,1.0f,1.0f);
                 
		}
		      
            float x_start=-1.0f+i*cellwidth;
            float y_start=-1.0f+j*cellheight;
            float x_end=x_start+cellwidth;
            float y_end=y_start+cellheight;

            
            glVertex2f(x_start,y_start); 
            glVertex2f(x_end,y_start);   
           glVertex2f(x_end,y_end);     
            glVertex2f(x_start,y_end);   
            
		}
	}
	glEnd();
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        return -1;
    }

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 800, "Occupancy Grid Map", nullptr, nullptr);
    if (!window) {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Create a RealSense pipeline and start streaming
    rs2::pipeline pipe;
    //pipe.start();
    
    //COMMENT THIS AND UNCOMMENT PIPELINE.START() IF I WANT TO TAKE REALSENSE CAM FOOTAGE
    rs2::config cfg;
    cfg.enable_device_from_file("outdoors.bag");
    pipe.start(cfg); // Load from file

    // Declare pointcloud object and points for mapping
    rs2::pointcloud pc;
    rs2::points points;
    vector<Vector3f> point_vectors; // Vector to hold 3D points

        while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Get the next frame
        auto frames = pipe.wait_for_frames();
        auto depth = frames.get_depth_frame();
        pc.map_to(frames.get_color_frame());
        points = pc.calculate(depth);

        // Collect point cloud data
        point_vectors.clear();
        for (size_t i = 0; i < points.size(); ++i) {
            auto point = points.get_vertices()[i];
            if (point.z) {
                point_vectors.push_back(Vector3f(point.x, point.y, point.z));
            }
        }

        // Create grid map
        float grid_resolution = 0.1f;
        Gridmap gridmap = create_gridmap(point_vectors, grid_resolution);

        // Render the grid map
        gridmapglfw(gridmap);

	draw_axes(gridmap.occupancy_grid.size(), gridmap.occupancy_grid[0].size(), grid_resolution, grid_resolution);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

            // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

