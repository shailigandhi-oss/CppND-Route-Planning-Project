/**
 * @file route_model.h
 * @brief Extended model class for route planning functionality
 * 
 * This file contains the RouteModel class which extends the base Model class
 * with additional features required for pathfinding using the A* algorithm.
 */

#ifndef ROUTE_MODEL_H
#define ROUTE_MODEL_H

#include <limits>
#include <cmath>
#include <unordered_map>
#include "model.h"
#include <iostream>

/**
 * @class RouteModel
 * @brief Extended Model class with pathfinding capabilities
 * 
 * RouteModel extends the base Model class by adding route planning features.
 * It maintains a graph of nodes with neighbor relationships and provides
 * methods to find the closest node to a given coordinate.
 */
class RouteModel : public Model {

  public:
    /**
     * @class Node
     * @brief Extended node class for A* pathfinding
     * 
     * This class extends Model::Node with additional attributes required for
     * the A* search algorithm, including parent pointers, cost values, and
     * neighbor relationships.
     */
    class Node : public Model::Node {
      public:
        Node * parent = nullptr;                              ///< Pointer to parent node in the path
        float h_value = std::numeric_limits<float>::max();    ///< Heuristic cost to the goal
        float g_value = 0.0;                                  ///< Actual cost from the start
        bool visited = false;                                 ///< Flag indicating if node has been visited
        std::vector<Node *> neighbors;                        ///< Pointers to neighboring nodes

        /**
         * @brief Finds and populates the neighbors vector
         * 
         * Searches for all nodes connected to this node via roads and adds
         * them to the neighbors vector.
         */
        void FindNeighbors();
        
        /**
         * @brief Calculates Euclidean distance to another node
         * @param other The other node to measure distance to
         * @return The Euclidean distance between the two nodes
         */
        float distance(Node other) const {
            return std::sqrt(std::pow((x - other.x), 2) + std::pow((y - other.y), 2));
        }

        /**
         * @brief Default constructor
         */
        Node(){}
        
        /**
         * @brief Constructs a Node with specific parameters
         * @param idx The index of this node in the model's node vector
         * @param search_model Pointer to the parent RouteModel
         * @param node The base Model::Node to initialize from
         */
        Node(int idx, RouteModel * search_model, Model::Node node) : Model::Node(node), parent_model(search_model), index(idx) {}

      private:
        int index;                            ///< Index of this node in the model's node vector
        
        /**
         * @brief Finds a neighbor node from a list of candidate indices
         * @param node_indices Vector of node indices to search through
         * @return Pointer to the found neighbor node, or nullptr if not found
         */
        Node * FindNeighbor(std::vector<int> node_indices);
        
        RouteModel * parent_model = nullptr;  ///< Pointer to the parent RouteModel
    };

    /**
     * @brief Constructs a RouteModel from OSM XML data
     * @param xml Vector of bytes containing the OSM XML data
     * 
     * Initializes the model and creates the node-to-road mapping required
     * for pathfinding.
     */
    RouteModel(const std::vector<std::byte> &xml);
    
    /**
     * @brief Finds the closest node to the given coordinates
     * @param x The x-coordinate (normalized longitude)
     * @param y The y-coordinate (normalized latitude)
     * @return Reference to the closest node
     * 
     * Uses Euclidean distance to find the nearest node on the road network.
     */
    Node &FindClosestNode(float x, float y);
    
    /**
     * @brief Returns the vector of all nodes in the route model
     * @return Reference to the nodes vector
     */
    auto &SNodes() { return m_Nodes; }
    
    std::vector<Node> path;  ///< The calculated path from start to end
    
  private:
    /**
     * @brief Creates a hashmap mapping nodes to roads
     * 
     * Builds an index that maps each node to all roads that pass through it,
     * enabling efficient neighbor lookup during pathfinding.
     */
    void CreateNodeToRoadHashmap();
    
    std::unordered_map<int, std::vector<const Model::Road *>> node_to_road;  ///< Maps node indices to roads
    std::vector<Node> m_Nodes;  ///< All nodes in the route model with pathfinding attributes

};

#endif
