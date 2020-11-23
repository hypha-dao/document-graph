#pragma once
#include <eosio/name.hpp>
#include <eosio/time.hpp>
#include <eosio/crypto.hpp>

namespace hypha
{

    struct [[eosio::table, eosio::contract("docs")]] Edge
    {
        Edge();
        Edge(const eosio::checksum256 &from_node, const eosio::checksum256 &to_node, const eosio::name &edge_name);
        ~Edge();

        uint64_t id;

        // these three additional indexes allow isolating/querying edges more precisely (less iteration)
        uint64_t from_node_edge_name_index;
        uint64_t from_node_to_node_index;
        uint64_t to_node_edge_name_index;
        uint64_t by_from_node_edge_name_index() const { return from_node_edge_name_index; }
        uint64_t by_from_node_to_node_index() const { return from_node_to_node_index; }
        uint64_t by_to_node_edge_name_index() const { return to_node_edge_name_index; }

        eosio::checksum256 from_node;
        eosio::checksum256 by_from() const { return from_node; }

        eosio::checksum256 to_node;
        eosio::checksum256 by_to() const { return to_node; }

        eosio::name edge_name;
        uint64_t by_edge_name() const { return edge_name.value; }

        eosio::time_point created_date = current_time_point();
        uint64_t by_created() const { return created_date.sec_since_epoch(); }

        eosio::name creator;
        uint64_t by_creator() const { return creator.value; }

        uint64_t primary_key() const { return id; }

        private:
            uint64_t createID(const eosio::checksum256 &from_node, const eosio::checksum256 &to_node, const eosio::name &edge_name);

        EOSLIB_SERIALIZE(Edge, (id)(from_node_edge_name_index)(from_node_to_node_index)(to_node_edge_name_index)(from_node)(to_node)(edge_name)(created_date)(creator))
    };

} // namespace hypha