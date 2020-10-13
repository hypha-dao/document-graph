#include <document_graph.hpp>

namespace hyphaspace
{
    void document_graph::create_edge (const checksum256 &from_node, const checksum256 &to_node, const name &edge_name)
    {
        // these functions will assert failure if the documents do not exist
        get_document(from_node);
        get_document(to_node);

        // add the edge
        edge_table e_t (contract, contract.value);
        e_t.emplace (contract, [&](auto &e) {
            // if this specific edge exists already, this will fail
            e.id    = edge_id (from_node, to_node, edge_name);
            e.from_node = from_node;
            e.to_node = to_node;
            e.edge_name = edge_name;
        });
    }

    // find the specific edge of this from_node, to_node, and edge_name
    // if strict, the function will assert failure if no edge is found
    void document_graph::remove_edge (const checksum256 &from_node, const checksum256 &to_node, const name &edge_name, const bool strict)
    {
        edge_table e_t (contract, contract.value);
        auto itr = e_t.find (edge_id (from_node, to_node, edge_name));

        if (strict) {
            check (itr != e_t.end(), "edge does not exist: from " + readable_hash(from_node) 
                + " to " + readable_hash(to_node) + " with edge name of " + edge_name.to_string());

            e_t.erase (itr);
        }
    }

    // iterate through all edges of this from_node, find edges with the provided edge_name, and erase
    // if strict, the function will assert failure if no edges are found
    void document_graph::remove_edges (const checksum256 &from_node, const name &edge_name, const bool strict)
    {
        edge_table e_t (contract, contract.value);
        
        auto from_node_index = e_t.get_index<name("fromnode")>();
        auto itr = from_node_index.find (from_node);

        bool found = false;
        while (itr != from_node_index.end() && itr->from_node == from_node) {
            if (itr->edge_name == edge_name) {
                itr = from_node_index.erase (itr);
                found = true;
            } else {
                itr++;
            }
        } 

        if (strict) {
            check (found , "edge does not exist: from " + readable_hash(from_node) + " with edge name of " + edge_name.to_string() );
        }
    }

    // iterate through all edges of this from_node, find edges with the provided edge_name, and erase
    // if strict, the function will assert failure if no edges are found
    void document_graph::remove_edges (const checksum256 &from_node, const checksum256 &to_node, const bool strict)
    {
        edge_table e_t (contract, contract.value);
        
        auto from_node_index = e_t.get_index<name("fromnode")>();
        auto itr = from_node_index.find (from_node);

        bool found = false;
        while (itr != from_node_index.end() && itr->from_node == from_node) {
            if (itr->to_node == to_node) {
                itr = from_node_index.erase (itr);
                found = true;
            } else {
                itr++;
            }
        } 

        if (strict) {
            check (found , "edge does not exist: from " + readable_hash(from_node) + " to " + readable_hash(to_node));
        }
    }

    // iterate through all edges of this node, to and from, and erase
    // if strict, the function will assert failure if no edges are found
    void document_graph::remove_edges (const checksum256 &node, const bool strict)
    {
        edge_table e_t (contract, contract.value);
        
        auto from_node_index = e_t.get_index<name("fromnode")>();
        auto from_itr = from_node_index.find (node);

        bool found = false;
        while (from_itr != from_node_index.end() && from_itr->from_node == node) {
            from_itr = from_node_index.erase (from_itr);
            found = true;
        } 

        auto to_node_index = e_t.get_index<name("tonode")>();
        auto to_itr = to_node_index.find (node);

        while (to_itr != to_node_index.end() && to_itr->to_node == node) {
            to_itr = to_node_index.erase (to_itr);
            found = true;
        } 

        if (strict) {
            check (found , "no edges exist on document: " + readable_hash(node));
        }
    }
}