#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/name.hpp>
#include <cstring>

#include <document_graph/content.hpp>
#include <document_graph/document.hpp>
#include <document_graph/edge.hpp>

namespace hypha
{
    class DocumentGraph
    {
    public:
        DocumentGraph(const eosio::name &contract) : m_contract(contract) {}
        ~DocumentGraph() {}

        void removeEdges(const eosio::checksum256 &node);

        std::vector<Edge> getEdges(const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode);
        std::vector<Edge> getEdgesOrFail(const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode);

        std::vector<Edge> getEdgesFrom(const eosio::checksum256 &fromNode, const eosio::name &edgeName);
        std::vector<Edge> getEdgesFromOrFail(const eosio::checksum256 &fromNode, const eosio::name &edgeName);

        std::vector<Edge> getEdgesTo(const eosio::checksum256 &toNode, const eosio::name &edgeName);
        std::vector<Edge> getEdgesToOrFail(const eosio::checksum256 &toNode, const eosio::name &edgeName);

        Edge createEdge(eosio::name &creator, const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode, const eosio::name &edgeName);

        Document updateDocument(const eosio::name &updater,
                                const eosio::checksum256 &doc_hash,
                                ContentGroups content_groups);

        void replaceNode(const eosio::checksum256 &oldNode, const eosio::checksum256 &newNode);
        void eraseDocument(const eosio::checksum256 &document_hash);
        void eraseDocument(const eosio::checksum256 &document_hash, const bool includeEdges);

    private:
        eosio::name m_contract;
    };
}; // namespace hypha

// #define DECLARE_DOCUMENT(contract)\
// using flexvalue = hypha::document_graph::flexvalue;\
// using root_doc = hypha::document_graph::document;\
// TABLE contract##_document : public root_doc {};\
// using contract_document = contract##_document;\
// using document_table =  multi_index<name("documents"), contract_document,\
//                             indexed_by<name("idhash"), const_mem_fun<root_doc, checksum256, &root_doc::by_hash>>,\
//                             indexed_by<name("bycreator"), const_mem_fun<root_doc, uint64_t, &root_doc::by_creator>>,\
//                             indexed_by<name("bycreated"), const_mem_fun<root_doc, uint64_t, &root_doc::by_created>>>;\
// using root_edge = hypha::document_graph::edge;\
// TABLE contract##_edge : public hypha::document_graph::edge {};\
// using contract_edge = contract##_edge;\
// using edge_table = multi_index<name("edges"), contract_edge,\
//             indexed_by<name("fromnode"), const_mem_fun<root_edge, checksum256, &root_edge::by_from>>,\
//             indexed_by<name("tonode"), const_mem_fun<root_edge, checksum256, &root_edge::by_to>>,\
//             indexed_by<name("edgename"), const_mem_fun<root_edge, uint64_t, &root_edge::by_edge_name>>,\
//             indexed_by<name("byfromname"), const_mem_fun<root_edge, uint64_t, &root_edge::by_from_node_edge_name_index>>,\
//             indexed_by<name("byfromto"), const_mem_fun<root_edge, uint64_t, &root_edge::by_from_node_to_node_index>>,\
//             indexed_by<name("bytoname"), const_mem_fun<root_edge, uint64_t, &root_edge::by_to_node_edge_name_index>>,\
//             indexed_by<name("bycreated"), const_mem_fun<root_edge, uint64_t, &root_edge::by_created>>,\
//             indexed_by<name("bycreator"), const_mem_fun<root_edge, uint64_t, &root_edge::by_creator>>>;