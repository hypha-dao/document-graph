#include <document_graph/edge.hpp>
#include <document_graph/util.hpp>
#include <document_graph/document.hpp>

namespace hypha
{
    Edge::Edge() {}
    Edge::Edge(const eosio::name &creator, 
                const eosio::checksum256 &from_node, 
                const eosio::checksum256 &to_node, 
                const eosio::name &edge_name) 
        : creator {creator}, from_node {from_node}, to_node {to_node}, edge_name{edge_name}
    {
        auto new_edge_id = concatHash (from_node, to_node, edge_name);
        id = new_edge_id;
      
        from_node_edge_name_index = concatHash(from_node, edge_name);
        from_node_to_node_index = concatHash(from_node, to_node);
        to_node_edge_name_index = concatHash(to_node, edge_name);        
    }

    Edge::~Edge(){}
    
    uint64_t Edge::primary_key() const { return id; }
    uint64_t Edge::by_from_node_edge_name_index() const { return from_node_edge_name_index; }
    uint64_t Edge::by_from_node_to_node_index() const { return from_node_to_node_index; }
    uint64_t Edge::by_to_node_edge_name_index() const { return to_node_edge_name_index; }
    uint64_t Edge::by_edge_name() const { return edge_name.value; }
    uint64_t Edge::by_created() const { return created_date.sec_since_epoch(); }
    uint64_t Edge::by_creator() const { return creator.value; }

    eosio::checksum256 Edge::by_from() const { return from_node; }
    eosio::checksum256 Edge::by_to() const { return to_node; }
}