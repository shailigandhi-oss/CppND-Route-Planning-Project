/**
 * @file render.h
 * @brief Rendering engine for visualizing OpenStreetMap data and routes
 * 
 * This file contains the Render class which uses the IO2D library to draw
 * maps, routes, and various geographic features on a graphical surface.
 */

#pragma once

#include <unordered_map>
#include <io2d.h>
#include "route_model.h"

using namespace std::experimental;

/**
 * @class Render
 * @brief Handles the rendering of map features and calculated routes
 * 
 * The Render class is responsible for drawing all visual elements of the map
 * including roads, buildings, water bodies, and the calculated route path.
 * It uses the IO2D library for 2D graphics rendering.
 */
class Render
{
public:
    /**
     * @brief Constructs a Render object with the given route model
     * @param model Reference to the RouteModel containing map data
     */
    Render(RouteModel &model );
    
    /**
     * @brief Renders the complete map and route to the given surface
     * @tparam T The surface type (must support IO2D operations)
     * @param surface Reference to the rendering surface
     * 
     * This method draws all map features in the correct order:
     * background, land uses, leisure areas, water, railways, roads,
     * buildings, and finally the calculated route with start/end markers.
     */
    template <typename T>
    void Display( T &surface ) {
        m_Scale = static_cast<float>(std::min(surface.dimensions().x(), surface.dimensions().y()));    
        m_PixelsInMeter = static_cast<float>(m_Scale / m_Model.MetricScale()); 
        m_Matrix = io2d::matrix_2d::create_scale({m_Scale, -m_Scale}) *
                   io2d::matrix_2d::create_translate({0.f, static_cast<float>(surface.dimensions().y())});
        
        surface.paint(m_BackgroundFillBrush);        
        DrawLanduses(surface);
        DrawLeisure(surface);
        DrawWater(surface);    
        DrawRailways(surface);
        DrawHighways(surface);    
        DrawBuildings(surface);  
        DrawPath(surface);
        DrawStartPosition(surface);   
        DrawEndPosition(surface);
    }
    
private:
    /**
     * @brief Initializes road rendering representations
     * 
     * Sets up brushes, line widths, and dash patterns for different road types.
     */
    void BuildRoadReps();
    
    /**
     * @brief Initializes brushes for different land use types
     * 
     * Creates color brushes for commercial, residential, forest areas, etc.
     */
    void BuildLanduseBrushes();
    
    /**
     * @brief Draws all buildings on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     */
    template <typename T>
    void DrawBuildings(T &surface) const {
        for( auto &building: m_Model.Buildings() ) {
            auto path = PathFromMP(building);
            surface.fill(m_BuildingFillBrush, path);        
            surface.stroke(m_BuildingOutlineBrush, path, std::nullopt, m_BuildingOutlineStrokeProps);
        }
    }

    /**
     * @brief Draws all roads (highways) on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders roads with appropriate styling based on their classification.
     */
    template <typename T>
    void DrawHighways(T &surface) const {
        auto ways = m_Model.Ways().data();
        for( auto road: m_Model.Roads() )
            if( auto rep_it = m_RoadReps.find(road.type); rep_it != m_RoadReps.end() ) {
                auto &rep = rep_it->second;   
                auto &way = ways[road.way];
                auto width = rep.metric_width > 0.f ? (rep.metric_width * m_PixelsInMeter) : 1.f;
                auto sp = io2d::stroke_props{width, io2d::line_cap::round};
                surface.stroke(rep.brush, PathFromWay(way), std::nullopt, sp, rep.dashes);        
            }
    }

    /**
     * @brief Draws all railway lines on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders railways with a distinctive dashed line pattern.
     */
    template <typename T>
    void DrawRailways(T &surface) const {     
        auto ways = m_Model.Ways().data();
        for( auto &railway: m_Model.Railways() ) {
            auto &way = ways[railway.way];
            auto path = PathFromWay(way);
            surface.stroke(m_RailwayStrokeBrush, path, std::nullopt, io2d::stroke_props{m_RailwayOuterWidth * m_PixelsInMeter});
            surface.stroke(m_RailwayDashBrush, path, std::nullopt, io2d::stroke_props{m_RailwayInnerWidth * m_PixelsInMeter}, m_RailwayDashes);
        }
    }

    /**
     * @brief Draws all leisure areas on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders parks, sports facilities, and other leisure areas.
     */
    template <typename T>
    void DrawLeisure(T &surface) const {
        for( auto &leisure: m_Model.Leisures()) {
            auto path = PathFromMP(leisure);
            surface.fill(m_LeisureFillBrush, path);        
            surface.stroke(m_LeisureOutlineBrush, path, std::nullopt, m_LeisureOutlineStrokeProps);
        }
    }

    /**
     * @brief Draws all water bodies on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders lakes, rivers, and other water features.
     */
    template <typename T>
    void DrawWater(T &surface) const {
        for( auto &water: m_Model.Waters())
            surface.fill(m_WaterFillBrush, PathFromMP(water));
    }

    /**
     * @brief Draws all land use areas on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders different land use types with appropriate colors.
     */
    template <typename T>
    void DrawLanduses(T &surface) const {
        for( auto &landuse: m_Model.Landuses() )
            if( auto br = m_LanduseBrushes.find(landuse.type); br != m_LanduseBrushes.end() )        
                surface.fill(br->second, PathFromMP(landuse));
    }

    /**
     * @brief Draws the starting position marker on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders a green square marker at the start of the calculated path.
     */
    template <typename T>
    void DrawStartPosition(T &surface) const {
        if (m_Model.path.empty()) return;

        io2d::render_props aliased{ io2d::antialias::none };
        io2d::brush foreBrush{ io2d::rgba_color::green };

        auto pb = io2d::path_builder{}; 
        pb.matrix(m_Matrix);

        pb.new_figure({(float) m_Model.path.front().x, (float) m_Model.path.front().y});
        float constexpr l_marker = 0.01f;
        pb.rel_line({l_marker, 0.f});
        pb.rel_line({0.f, l_marker});
        pb.rel_line({-l_marker, 0.f});
        pb.rel_line({0.f, -l_marker});
        pb.close_figure();
        
        surface.fill(foreBrush, pb);
        surface.stroke(foreBrush, io2d::interpreted_path{pb}, std::nullopt, std::nullopt, std::nullopt, aliased);
    }

    /**
     * @brief Draws the ending position marker on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders a red square marker at the end of the calculated path.
     */
    template <typename T>
    void DrawEndPosition(T &surface) const {
        if (m_Model.path.empty()) return;
        io2d::render_props aliased{ io2d::antialias::none };
        io2d::brush foreBrush{ io2d::rgba_color::red };

        auto pb = io2d::path_builder{}; 
        pb.matrix(m_Matrix);

        pb.new_figure({(float) m_Model.path.back().x, (float) m_Model.path.back().y});
        float constexpr l_marker = 0.01f;
        pb.rel_line({l_marker, 0.f});
        pb.rel_line({0.f, l_marker});
        pb.rel_line({-l_marker, 0.f});
        pb.rel_line({0.f, -l_marker});
        pb.close_figure();
        
        surface.fill(foreBrush, pb);
        surface.stroke(foreBrush, io2d::interpreted_path{pb}, std::nullopt, std::nullopt, std::nullopt, aliased);
    }

    /**
     * @brief Draws the calculated route path on the surface
     * @tparam T The surface type
     * @param surface Reference to the rendering surface
     * 
     * Renders the path found by the A* algorithm as an orange line.
     */
    template <typename T>
    void DrawPath(T &surface) const {
        io2d::render_props aliased{ io2d::antialias::none };
        io2d::brush foreBrush{ io2d::rgba_color::orange}; 
        float width = 5.0f;
        surface.stroke(foreBrush, PathLine(), std::nullopt, io2d::stroke_props{width});
    }

    /**
     * @brief Converts a Way to an IO2D path
     * @param way The way to convert
     * @return An IO2D interpreted path object
     */
    io2d::interpreted_path PathFromWay(const Model::Way &way) const;
    
    /**
     * @brief Converts a Multipolygon to an IO2D path
     * @param mp The multipolygon to convert
     * @return An IO2D interpreted path object
     */
    io2d::interpreted_path PathFromMP(const Model::Multipolygon &mp) const;
    
    /**
     * @brief Creates an IO2D path from the calculated route
     * @return An IO2D interpreted path representing the route
     */
    io2d::interpreted_path PathLine() const;

    RouteModel &m_Model;           ///< Reference to the route model
    float m_Scale = 1.f;           ///< Scaling factor for rendering
    float m_PixelsInMeter = 1.f;   ///< Conversion factor from meters to pixels
    io2d::matrix_2d m_Matrix;      ///< Transformation matrix for coordinate mapping
    
    io2d::brush m_BackgroundFillBrush{ io2d::rgba_color{238, 235, 227} };  ///< Background color brush
    
    io2d::brush m_BuildingFillBrush{ io2d::rgba_color{208, 197, 190} };      ///< Building fill color
    io2d::brush m_BuildingOutlineBrush{ io2d::rgba_color{181, 167, 154} };   ///< Building outline color
    io2d::stroke_props m_BuildingOutlineStrokeProps{1.f};                    ///< Building outline stroke properties
    
    io2d::brush m_LeisureFillBrush{ io2d::rgba_color{189, 252, 193} };       ///< Leisure area fill color
    io2d::brush m_LeisureOutlineBrush{ io2d::rgba_color{160, 248, 162} };    ///< Leisure area outline color
    io2d::stroke_props m_LeisureOutlineStrokeProps{1.f};                     ///< Leisure area outline stroke properties

    io2d::brush m_WaterFillBrush{ io2d::rgba_color{155, 201, 215} };         ///< Water body fill color
        
    io2d::brush m_RailwayStrokeBrush{ io2d::rgba_color{93,93,93} };          ///< Railway outer stroke color
    io2d::brush m_RailwayDashBrush{ io2d::rgba_color::white };               ///< Railway dash line color
    io2d::dashes m_RailwayDashes{0.f, {3.f, 3.f}};                           ///< Railway dash pattern
    float m_RailwayOuterWidth = 3.f;                                         ///< Railway outer line width
    float m_RailwayInnerWidth = 2.f;                                         ///< Railway inner line width
    
    /**
     * @struct RoadRep
     * @brief Rendering representation for a road type
     * 
     * Contains styling information for rendering roads of different classifications.
     */
    struct RoadRep {
        io2d::brush brush{io2d::rgba_color::black};  ///< Road color brush
        io2d::dashes dashes{};                        ///< Dash pattern for the road
        float metric_width = 1.f;                     ///< Width of the road in meters
    };
    std::unordered_map<Model::Road::Type, RoadRep> m_RoadReps;  ///< Road rendering styles by type
    
    std::unordered_map<Model::Landuse::Type, io2d::brush> m_LanduseBrushes;  ///< Brushes for land use types
};