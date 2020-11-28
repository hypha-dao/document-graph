#include <document_graph/edge.hpp>
#include <document_graph/util.hpp>
#include <document_graph/document.hpp>

namespace hypha
{
    Edge::Edge() {}
    Edge::Edge( const eosio::name &contract,
                const eosio::name &creator, 
                const eosio::checksum256 &from_node, 
                const eosio::checksum256 &to_node, 
                const eosio::name &edge_name) 
        : contract {contract}, creator {creator}, from_node {from_node}, to_node {to_node}, edge_name{edge_name}
    {
        id = concatHash (from_node, to_node, edge_name);
        from_node_edge_name_index = concatHash(from_node, edge_name);
        from_node_to_node_index = concatHash(from_node, to_node);
        to_node_edge_name_index = concatHash(to_node, edge_name);        
    }

    Edge::~Edge(){}

    Edge Edge::get (const eosio::name &_contract,
                    const eosio::checksum256 &_from_node, 
                    const eosio::checksum256 &_to_node, 
                    const eosio::name &_edge_name)
    {
        edge_table e_t (_contract, _contract.value);
        auto itr = e_t.find (concatHash (_from_node, _to_node, _edge_name));

        eosio::check (itr != e_t.end(), "edge does not exist: from " + readableHash(_from_node) 
                + " to " + readableHash(_to_node) + " with edge name of " + _edge_name.to_string());

        return *itr;
    }

    bool Edge::exists (const eosio::name &_contract,
                        const eosio::checksum256 &_from_node, 
                        const eosio::checksum256 &_to_node, 
                        const eosio::name &_edge_name)
    {
        edge_table e_t (_contract, _contract.value);
        auto itr = e_t.find (concatHash (_from_node, _to_node, _edge_name));
        if (itr != e_t.end()) return true;
        return false;
    }

    void Edge::emplace () 
    {
        require_auth (creator);

        edge_table e_t(contract, contract.value);
        e_t.emplace(contract, [&](auto &e) {
            e = *this;
            e.created_date = eosio::current_time_point();
        });
    }

    void Edge::erase ()
    {       
        edge_table e_t (contract, contract.value);
        auto itr = e_t.find (id);

        eosio::check (itr != e_t.end(), "edge does not exist: from " + readableHash(from_node) 
                + " to " + readableHash(to_node) + " with edge name of " + edge_name.to_string());
        e_t.erase (itr);
    }

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