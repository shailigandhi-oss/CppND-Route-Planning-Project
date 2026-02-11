/**
 * @file model.h
 * @brief OpenStreetMap data model for representing map features
 * 
 * This file contains the Model class which parses and stores OpenStreetMap (OSM) data
 * including nodes, ways, roads, buildings, and various geographic features.
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>

/**
 * @class Model
 * @brief Main class for storing and managing OpenStreetMap data
 * 
 * The Model class parses OSM XML data and stores various geographic features
 * such as roads, buildings, water bodies, and land uses. It provides accessors
 * to retrieve these features for rendering or route planning.
 */
class Model
{
public:
    /**
     * @struct Node
     * @brief Represents a geographic coordinate point
     * 
     * A node contains x and y coordinates representing a point on the map.
     * These coordinates are normalized between 0 and 1.
     */
    struct Node {
        double x = 0.f;  ///< Normalized x-coordinate (longitude)
        double y = 0.f;  ///< Normalized y-coordinate (latitude)
    };
    
    /**
     * @struct Way
     * @brief Represents a path or area defined by a sequence of nodes
     * 
     * A way is an ordered list of node indices that define paths (roads, rivers)
     * or closed areas (buildings, parks).
     */
    struct Way {
        std::vector<int> nodes;  ///< Ordered list of node indices defining the way
    };
    
    /**
     * @struct Road
     * @brief Represents a road feature with classification
     * 
     * Contains the way index and road type classification according to
     * OpenStreetMap road hierarchy.
     */
    struct Road {
        /**
         * @enum Type
         * @brief Road classification types based on OSM standards
         */
        enum Type { Invalid, Unclassified, Service, Residential,
            Tertiary, Secondary, Primary, Trunk, Motorway, Footway };
        int way;     ///< Index to the way representing this road
        Type type;   ///< Classification type of the road
    };
    
    /**
     * @struct Railway
     * @brief Represents a railway line
     */
    struct Railway {
        int way;  ///< Index to the way representing this railway
    };    
    
    /**
     * @struct Multipolygon
     * @brief Represents a complex polygon with outer and inner boundaries
     * 
     * Used for features that may have holes or multiple parts, such as
     * buildings with courtyards or lakes with islands.
     */
    struct Multipolygon {
        std::vector<int> outer;  ///< Node indices forming the outer boundary
        std::vector<int> inner;  ///< Node indices forming inner boundaries (holes)
    };
    
    /**
     * @struct Building
     * @brief Represents a building structure
     */
    struct Building : Multipolygon {};
    
    /**
     * @struct Leisure
     * @brief Represents leisure areas like parks or sports facilities
     */
    struct Leisure : Multipolygon {};
    
    /**
     * @struct Water
     * @brief Represents water bodies like lakes or rivers
     */
    struct Water : Multipolygon {};
    
    /**
     * @struct Landuse
     * @brief Represents land usage areas with classification
     * 
     * Contains information about how land is used, such as commercial,
     * residential, or industrial areas.
     */
    struct Landuse : Multipolygon {
        /**
         * @enum Type
         * @brief Land usage classification types
         */
        enum Type { Invalid, Commercial, Construction, Grass, Forest, Industrial, Railway, Residential };
        Type type;  ///< Classification type of the land use
    };
    
    /**
     * @brief Constructs a Model from OSM XML data
     * @param xml Vector of bytes containing the OSM XML data
     * 
     * Parses the provided OSM XML data and populates all map features.
     */
    Model( const std::vector<std::byte> &xml );
    
    /**
     * @brief Returns the metric scale of the map
     * @return The scale factor for converting normalized coordinates to meters
     */
    auto MetricScale() const noexcept { return m_MetricScale; }    
    
    /**
     * @brief Returns all nodes in the model
     * @return Const reference to the vector of nodes
     */
    auto &Nodes() const noexcept { return m_Nodes; }
    
    /**
     * @brief Returns all ways in the model
     * @return Const reference to the vector of ways
     */
    auto &Ways() const noexcept { return m_Ways; }
    
    /**
     * @brief Returns all roads in the model
     * @return Const reference to the vector of roads
     */
    auto &Roads() const noexcept { return m_Roads; }
    
    /**
     * @brief Returns all buildings in the model
     * @return Const reference to the vector of buildings
     */
    auto &Buildings() const noexcept { return m_Buildings; }
    
    /**
     * @brief Returns all leisure areas in the model
     * @return Const reference to the vector of leisure areas
     */
    auto &Leisures() const noexcept { return m_Leisures; }
    
    /**
     * @brief Returns all water bodies in the model
     * @return Const reference to the vector of water bodies
     */
    auto &Waters() const noexcept { return m_Waters; }
    
    /**
     * @brief Returns all land use areas in the model
     * @return Const reference to the vector of land use areas
     */
    auto &Landuses() const noexcept { return m_Landuses; }
    
    /**
     * @brief Returns all railways in the model
     * @return Const reference to the vector of railways
     */
    auto &Railways() const noexcept { return m_Railways; }
    
private:
    /**
     * @brief Normalizes coordinates to the range [0, 1]
     * 
     * Adjusts all node coordinates based on the minimum and maximum
     * latitude and longitude values found in the data.
     */
    void AdjustCoordinates();
    
    /**
     * @brief Constructs polygon rings from node sequences
     * @param mp Reference to the multipolygon to build rings for
     * 
     * Processes the node indices to create proper polygon boundaries.
     */
    void BuildRings( Multipolygon &mp );
    
    /**
     * @brief Parses OSM XML data and populates model structures
     * @param xml Vector of bytes containing the OSM XML data
     */
    void LoadData(const std::vector<std::byte> &xml);
    
    std::vector<Node> m_Nodes;          ///< All nodes in the map
    std::vector<Way> m_Ways;            ///< All ways in the map
    std::vector<Road> m_Roads;          ///< All roads in the map
    std::vector<Railway> m_Railways;    ///< All railways in the map
    std::vector<Building> m_Buildings;  ///< All buildings in the map
    std::vector<Leisure> m_Leisures;    ///< All leisure areas in the map
    std::vector<Water> m_Waters;        ///< All water bodies in the map
    std::vector<Landuse> m_Landuses;    ///< All land use areas in the map
    
    double m_MinLat = 0.;      ///< Minimum latitude in the data
    double m_MaxLat = 0.;      ///< Maximum latitude in the data
    double m_MinLon = 0.;      ///< Minimum longitude in the data
    double m_MaxLon = 0.;      ///< Maximum longitude in the data
    double m_MetricScale = 1.f;///< Scale factor for metric conversions
};
