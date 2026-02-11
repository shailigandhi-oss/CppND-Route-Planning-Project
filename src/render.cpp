#include "render.h"
#include <iostream>

static float RoadMetricWidth(Model::Road::Type type);
static io2d::rgba_color RoadColor(Model::Road::Type type);
static io2d::dashes RoadDashes(Model::Road::Type type);
static io2d::point_2d ToPoint2D( const Model::Node &node ) noexcept; 

Render::Render( RouteModel &model ):
    m_Model(model)
{
    BuildRoadReps();
    BuildLanduseBrushes();
}

io2d::interpreted_path Render::PathLine() const
{    
    if( m_Model.path.empty() )
        return {};

    const auto nodes = m_Model.path;    
    
    auto pb = io2d::path_builder{};
    pb.matrix(m_Matrix);
    pb.new_figure( ToPoint2D( m_Model.path[0]));

    for( int i=1; i< m_Model.path.size();i++ )
        pb.line( ToPoint2D(m_Model.path[i])); 

      
    return io2d::interpreted_path{pb};
}

io2d::interpreted_path Render::PathFromWay(const Model::Way &way) const
{    
    if( way.nodes.empty() )
        return {};

    const auto nodes = m_Model.Nodes().data();    
    
    auto pb = io2d::path_builder{};
    pb.matrix(m_Matrix);
    pb.new_figure( ToPoint2D(nodes[way.nodes.front()]) );
    for( auto it = ++way.nodes.begin(); it != std::end(way.nodes); ++it )
        pb.line( ToPoint2D(nodes[*it]) );     
    return io2d::interpreted_path{pb};
}

io2d::interpreted_path Render::PathFromMP(const Model::Multipolygon &mp) const
{
    const auto nodes = m_Model.Nodes().data();
    const auto ways = m_Model.Ways().data();

    auto pb = io2d::path_builder{};    
    pb.matrix(m_Matrix);    
    
    auto commit = [&](const Model::Way &way) {
        if( way.nodes.empty() )
            return;
        pb.new_figure( ToPoint2D(nodes[way.nodes.front()]) );
        for( auto it = ++way.nodes.begin(); it != std::end(way.nodes); ++it )
            pb.line( ToPoint2D(nodes[*it]) );        
        pb.close_figure();        
    };
    
    for( auto way_num: mp.outer )
        commit( ways[way_num] );
    for( auto way_num: mp.inner )
        commit( ways[way_num] );
    
    return io2d::interpreted_path{pb};
}

void Render::BuildRoadReps()
{
    using R = Model::Road;
    auto types = {R::Motorway, R::Trunk, R::Primary,  R::Secondary, R::Tertiary,
        R::Residential, R::Service, R::Unclassified, R::Footway};
    for( auto type: types ) {
        auto &rep = m_RoadReps[type];
        rep.brush = io2d::brush{ RoadColor(type) };
        rep.metric_width = RoadMetricWidth(type);  
        rep.dashes = RoadDashes(type);
    }
}

void Render::BuildLanduseBrushes()
{
    m_LanduseBrushes.insert_or_assign(Model::Landuse::Commercial, io2d::brush{io2d::rgba_color{233, 195, 196}});
    m_LanduseBrushes.insert_or_assign(Model::Landuse::Construction, io2d::brush{io2d::rgba_color{187, 188, 165}});
    m_LanduseBrushes.insert_or_assign(Model::Landuse::Grass, io2d::brush{io2d::rgba_color{197, 236, 148}});
    m_LanduseBrushes.insert_or_assign(Model::Landuse::Forest, io2d::brush{io2d::rgba_color{158, 201, 141}});    
    m_LanduseBrushes.insert_or_assign(Model::Landuse::Industrial, io2d::brush{io2d::rgba_color{223, 197, 220}});
    m_LanduseBrushes.insert_or_assign(Model::Landuse::Railway, io2d::brush{io2d::rgba_color{223, 197, 220}});
    m_LanduseBrushes.insert_or_assign(Model::Landuse::Residential, io2d::brush{io2d::rgba_color{209, 209, 209}});
}

static float RoadMetricWidth(Model::Road::Type type)
{
    switch( type ) {
        case Model::Road::Motorway:     return 6.f;
        case Model::Road::Trunk:        return 6.f;
        case Model::Road::Primary:      return 5.f;
        case Model::Road::Secondary:    return 5.f;    
        case Model::Road::Tertiary:     return 4.f;
        case Model::Road::Residential:  return 2.5f;
        case Model::Road::Unclassified: return 2.5f;            
        case Model::Road::Service:      return 1.f;
        case Model::Road::Footway:      return 0.f;
        default:                        return 1.f;            
    }
}

static io2d::rgba_color RoadColor(Model::Road::Type type)
{
    switch( type) {
        case Model::Road::Motorway:     return io2d::rgba_color{226, 122, 143};
        case Model::Road::Trunk:        return io2d::rgba_color{245, 161, 136};
        case Model::Road::Primary:      return io2d::rgba_color{249, 207, 144};
        case Model::Road::Secondary:    return io2d::rgba_color{244, 251, 173};    
        case Model::Road::Tertiary:     return io2d::rgba_color{244, 251, 173};
        case Model::Road::Residential:  return io2d::rgba_color{254, 254, 254};
        case Model::Road::Service:      return io2d::rgba_color{254, 254, 254};
        case Model::Road::Footway:      return io2d::rgba_color{241, 106, 96};    
        case Model::Road::Unclassified: return io2d::rgba_color{254, 254, 254};
        default:                        return io2d::rgba_color::grey;  
    }
}

static io2d::dashes RoadDashes(Model::Road::Type type)
{
    return type == Model::Road::Footway ? io2d::dashes{0.f, {1.f, 2.f}} : io2d::dashes{};   
}

static io2d::point_2d ToPoint2D( const Model::Node &node ) noexcept
{
    return io2d::point_2d(static_cast<float>(node.x), static_cast<float>(node.y));
}