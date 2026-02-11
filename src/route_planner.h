/**
 * @file route_planner.h
 * @brief A* pathfinding algorithm implementation for route planning
 * 
 * This file contains the RoutePlanner class which implements the A* search
 * algorithm to find the optimal path between two points on the map.
 */

#ifndef ROUTE_PLANNER_H
#define ROUTE_PLANNER_H

#include <iostream>
#include <vector>
#include <string>
#include "route_model.h"


/**
 * @class RoutePlanner
 * @brief Implements A* pathfinding algorithm for route planning
 * 
 * The RoutePlanner class uses the A* search algorithm to find the shortest
 * path between two points on a RouteModel. It maintains an open list of
 * nodes to explore and calculates both actual (g) and heuristic (h) costs.
 */
class RoutePlanner {
  public:
    /**
     * @brief Constructs a RoutePlanner with start and end coordinates
     * @param model Reference to the RouteModel containing the map data
     * @param start_x Starting x-coordinate (normalized longitude)
     * @param start_y Starting y-coordinate (normalized latitude)
     * @param end_x Ending x-coordinate (normalized longitude)
     * @param end_y Ending y-coordinate (normalized latitude)
     * 
     * Initializes the route planner by finding the closest nodes on the road
     * network to the specified start and end coordinates.
     */
    RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y);
    
    /**
     * @brief Returns the total distance of the calculated path
     * @return The path distance in normalized units
     */
    float GetDistance() const {return distance;}
    
    /**
     * @brief Executes the A* search algorithm to find the optimal path
     * 
     * Performs A* search from the start node to the end node, updating
     * the model's path with the result and calculating the total distance.
     */
    void AStarSearch();
    
    /**
     * @brief Adds neighboring nodes to the open list
     * @param current_node Pointer to the current node being explored
     * 
     * Finds all neighbors of the current node, updates their g and h values,
     * sets their parent pointers, and adds them to the open list.
     */
    void AddNeighbors(RouteModel::Node *current_node);
    
    /**
     * @brief Calculates the heuristic (h) value for a node
     * @param node Pointer to the node to calculate h-value for
     * @return The heuristic cost estimate from this node to the goal
     * 
     * Uses Euclidean distance as the heuristic function for A* search.
     */
    float CalculateHValue(RouteModel::Node const *node);
    
    /**
     * @brief Reconstructs the final path from start to end
     * @param current_node Pointer to the end node
     * @return Vector of nodes representing the path from start to end
     * 
     * Traces back through parent pointers from the end node to the start node,
     * building the complete path and calculating the total distance.
     */
    std::vector<RouteModel::Node> ConstructFinalPath(RouteModel::Node *);
    
    /**
     * @brief Selects the next node to explore from the open list
     * @return Pointer to the node with the lowest f-value (g + h)
     * 
     * Finds and removes the most promising node from the open list based
     * on the sum of actual cost (g) and heuristic cost (h).
     */
    RouteModel::Node *NextNode();

  private:
    std::vector<RouteModel::Node*> open_list;  ///< List of nodes to be explored
    RouteModel::Node *start_node;              ///< Pointer to the starting node
    RouteModel::Node *end_node;                ///< Pointer to the goal node

    float distance = 0.0f;     ///< Total distance of the calculated path
    RouteModel &m_Model;       ///< Reference to the route model
};

#endif