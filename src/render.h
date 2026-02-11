#pragma once

#include <unordered_map>
#include <io2d.h>
#include "route_model.h"

using namespace std::experimental;

class Render
{
public:
    Render(RouteModel &model );
    
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
    void BuildRoadReps();
    void BuildLanduseBrushes();
    
    template <typename T>
    void DrawBuildings(T &surface) const {
        for( auto &building: m_Model.Buildings() ) {
            auto path = PathFromMP(building);
            surface.fill(m_BuildingFillBrush, path);        
            surface.stroke(m_BuildingOutlineBrush, path, std::nullopt, m_BuildingOutlineStrokeProps);
        }
    }

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

    template <typename T>
    void DrawLeisure(T &surface) const {
        for( auto &leisure: m_Model.Leisures()) {
            auto path = PathFromMP(leisure);
            surface.fill(m_LeisureFillBrush, path);        
            surface.stroke(m_LeisureOutlineBrush, path, std::nullopt, m_LeisureOutlineStrokeProps);
        }
    }

    template <typename T>
    void DrawWater(T &surface) const {
        for( auto &water: m_Model.Waters())
            surface.fill(m_WaterFillBrush, PathFromMP(water));
    }

    template <typename T>
    void DrawLanduses(T &surface) const {
        for( auto &landuse: m_Model.Landuses() )
            if( auto br = m_LanduseBrushes.find(landuse.type); br != m_LanduseBrushes.end() )        
                surface.fill(br->second, PathFromMP(landuse));
    }

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

    template <typename T>
    void DrawPath(T &surface) const {
        io2d::render_props aliased{ io2d::antialias::none };
        io2d::brush foreBrush{ io2d::rgba_color::orange}; 
        float width = 5.0f;
        surface.stroke(foreBrush, PathLine(), std::nullopt, io2d::stroke_props{width});
    }

    io2d::interpreted_path PathFromWay(const Model::Way &way) const;
    io2d::interpreted_path PathFromMP(const Model::Multipolygon &mp) const;
    io2d::interpreted_path PathLine() const;

    RouteModel &m_Model;
    float m_Scale = 1.f;
    float m_PixelsInMeter = 1.f;
    io2d::matrix_2d m_Matrix;
    
    io2d::brush m_BackgroundFillBrush{ io2d::rgba_color{238, 235, 227} };
    
    io2d::brush m_BuildingFillBrush{ io2d::rgba_color{208, 197, 190} };
    io2d::brush m_BuildingOutlineBrush{ io2d::rgba_color{181, 167, 154} };
    io2d::stroke_props m_BuildingOutlineStrokeProps{1.f};
    
    io2d::brush m_LeisureFillBrush{ io2d::rgba_color{189, 252, 193} };
    io2d::brush m_LeisureOutlineBrush{ io2d::rgba_color{160, 248, 162} };
    io2d::stroke_props m_LeisureOutlineStrokeProps{1.f};

    io2d::brush m_WaterFillBrush{ io2d::rgba_color{155, 201, 215} };    
        
    io2d::brush m_RailwayStrokeBrush{ io2d::rgba_color{93,93,93} };
    io2d::brush m_RailwayDashBrush{ io2d::rgba_color::white };
    io2d::dashes m_RailwayDashes{0.f, {3.f, 3.f}};
    float m_RailwayOuterWidth = 3.f;
    float m_RailwayInnerWidth = 2.f;
    
    struct RoadRep {
        io2d::brush brush{io2d::rgba_color::black};
        io2d::dashes dashes{};
        float metric_width = 1.f;
    };
    std::unordered_map<Model::Road::Type, RoadRep> m_RoadReps;
    
    std::unordered_map<Model::Landuse::Type, io2d::brush> m_LanduseBrushes;
};