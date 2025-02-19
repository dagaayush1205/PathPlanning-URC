#include <iostream>
#include "astar.h"
#include <vector>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <librealsense2/rs.hpp>
#include <iostream>
#include <vector>
#include <rerun.hpp>
#include <Eigen/Dense>
#include <thread>
#include <mutex>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <Eigen/Core>
//for downsampling and filtering
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/voxel_grid.h>
#include <cstdlib>
#include <rerun/demo_utils.hpp>
#include <unordered_set>
#include <sstream>
#include "rerun.h"
#include "common.h"

using namespace std;


/*double heuristic(int x1, int y1, int x2, int y2)
{
	return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}
vector<Node>astar(const std::unordered_map<std::pair<int, int>, CellCost, pair_hash>& occupancyGrid, Node start, Node goal)
{
   priority_queue<Node*,vector<Node*>,comparenode>openlist;//priority list for 
   unordered_map<pair<int,int>,Node*, pair_hash>allNodes;
   //unordered_map<int, Node*>allNodes;//map for all the nodes
   start.h_cost = heuristic(start.x, start.y, goal.x, goal.y);//euclidean distance function h cost
   start.f_cost = start.g_cost + start.h_cost;
   openlist.push(&start);

   while(!openlist.empty())
   {
     //stack
     Node*current=openlist.top();
     openlist.pop();//read one by one
    //(check if the node has higher g cost than in all nodes, if yes it is outdated)
    pair<int,int>currentkey={current->x,current->y};
    if(allNodes.find(currentkey) != allNodes.end() && current->g_cost>allNodes[currentkey]->g_cost)
    {
	    continue;
    }
     if(current->x == goal.x && current->y == goal.y)
     {
	     vector <Node> path;
	     for(Node *n=current;n!=NULL;n=n->parent)
	     {
		     path.push_back(*n);//error
	     }
	     reverse(path.begin(), path.end()); //reverse path
	     return path;
     }
    //for variable square grid size
    int dx[] = {1,-1,0,0,1,-1,1,-1};
    int dy[] = {0,0,1,-1,1,-1,-1,1}; 
//
//ADD 2 ENTITIES-> MOVEMENT_COST AND OBSTACLE_COST TO THE G_COST.
//OBSTACLE_COST IS CALCULATED FROM
   int i; double newg_cost;
   double movement_cost;
   for(i=0; i<8; i++)
   {
	   int newx = current->x+dx[i];
	   int newy = current->y+dy[i];
	     if(i<4)
	     {
		     movement_cost=1.0;
	     }
	     else
	     {
		     movement_cost=1.414;
	     }
             
	     double obstacle_cost=1e6; //default cost for free cells
 
             
		     auto it = occupancyGrid.find({newx,newy});
		     if(it!=occupancyGrid.end()) 
		     {
			     obstacle_cost=it->second.cost; //variable cost assigned (in grid and within bounds)
		     }
		    else if(newx >= gridmap.min_x && newx <= gridmap.max_x && newy >= gridmap.min_y && newy <= gridmap.max_y)
		     {
			     obstacle_cost=0.0; //free space (if not in grid but within the bounds)
		     }
                     // Skip if the cost is high (blocked or out of bounds)
                     else {
    // If the cell is outside the boundary, assign a high cost (impassable)
                      obstacle_cost = 1e6;
                    }

                      
		     if(obstacle_cost>=1e6)
		     {
			     continue;
		     }
	     
	     //infinite cost for unexplored regions (out of bounds)
	     newg_cost=current->g_cost+movement_cost+obstacle_cost; 
	     Node* neighbour = new Node(newx, newy, newg_cost, heuristic(newx, newy, goal.x, goal.y), current);//create a new neighbour node
             //newx+newy shows collision
	     //int key=newx*grid[0].size()+newy;
	     pair<int, int>neighbour_key = {newx, newy};
	     if(allNodes.find(neighbour_key)==allNodes.end() || neighbour -> g_cost<allNodes.find(neighbour_key) -> second -> g_cost)//newx+newy is the unique key for map
	     {
               neighbour->f_cost=neighbour->g_cost+neighbour->h_cost;
	       openlist.push(neighbour);
	       //update all nodes
	       allNodes[neighbour_key]=neighbour;
	     }

     } 
   }
 //return std::vector<Node>();  //return empty if not found in while loop
 return {};
}*/
// Heuristic function (Euclidean Distance)
double heuristic(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// A* Algorithm with optimized memory handling and proper waypoint logic
vector<Node> astar(const unordered_map<pair<int, int>, CellCost, pair_hash>& occupancyGrid, Node start, Node goal) {
    priority_queue<shared_ptr<Node>, vector<shared_ptr<Node>>, comparenode> openList;
    unordered_map<pair<int, int>, shared_ptr<Node>, pair_hash> allNodes;
    unordered_set<pair<int, int>, pair_hash> visited;

    start.h_cost = heuristic(start.x, start.y, goal.x, goal.y);
    start.f_cost = start.g_cost + start.h_cost;

    auto startNode = make_shared<Node>(start);
    allNodes[{start.x, start.y}] = startNode;
    openList.push(startNode);

    const int dx[] = {1, -1, 0, 0, 1, -1, 1, -1};
    const int dy[] = {0, 0, 1, -1, 1, -1, -1, 1};

    while (!openList.empty()) {
        auto current = openList.top();
        openList.pop();

        pair<int, int> currentKey = {current->x, current->y};

        // If the node is already visited, skip it
        if (visited.find(currentKey) != visited.end()) {
            continue;
        }
        visited.insert(currentKey); // Mark node as visited

        cout << "Processing Node: (" << current->x << "," << current->y << ") "
             << "G: " << current->g_cost << " H: " << current->h_cost << " F: " << current->f_cost << endl;

        if (current->x == goal.x && current->y == goal.y) {
            vector<Node> path;
            for (auto n = current; n != nullptr; n = n->parent) {
                path.push_back(*n);
            }
            reverse(path.begin(), path.end());

            cout << "Path found: ";
            for (const Node& n : path) {
                cout << "(" << n.x << "," << n.y << ") ";
            }
            cout << endl;
            return path;
        }

        for (int i = 0; i < 8; i++) {
            int newX = current->x + dx[i];
            int newY = current->y + dy[i];
            double movementCost = (i < 4) ? 1.0 : 1.414;

            double obstacleCost = 1e6;  // Default high cost (impassable)
            auto it = occupancyGrid.find({newX, newY});
            if (it != occupancyGrid.end()) {
                obstacleCost = it->second.cost;  // Use actual cost if found
            } else {
                obstacleCost = 0.0;  // Free space
            }

            if (obstacleCost >= 1e6) {
                continue;  // Skip impassable nodes
            }

            double newGCost = current->g_cost + movementCost + obstacleCost;
            pair<int, int> neighborKey = {newX, newY};

            if (visited.find(neighborKey) == visited.end() && 
                (allNodes.find(neighborKey) == allNodes.end() || newGCost < allNodes[neighborKey]->g_cost)) {
                
                auto neighbor = make_shared<Node>(
                    newX, newY, newGCost, heuristic(newX, newY, goal.x, goal.y), current);

                neighbor->f_cost = neighbor->g_cost + neighbor->h_cost;
                openList.push(neighbor);
                allNodes[neighborKey] = neighbor;
            }
        }
    }

    cout << "No path found!" << endl;
    return {};
}

