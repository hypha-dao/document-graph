#pragma once

#include <cstring>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/name.hpp>

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

        void removeEdges(uint64_t nodeID);

        std::vector<Edge> getEdges(uint64_t fromNode, uint64_t toNode);
        std::vector<Edge> getEdgesOrFail(uint64_t fromNode, uint64_t toNode);

        std::vector<Edge> getEdgesFrom(uint64_t fromNode, const eosio::name &edgeName);
        std::vector<Edge> getEdgesFromOrFail(uint64_t fromNode, const eosio::name &edgeName);

        std::vector<Edge> getEdgesTo(uint64_t toNode, const eosio::name &edgeName);
        std::vector<Edge> getEdgesToOrFail(uint64_t toNode, const eosio::name &edgeName);

        Edge createEdge(eosio::name &creator, uint64_t fromNode, uint64_t toNode, const eosio::name &edgeName);

        bool hasEdges(uint64_t nodeID);
        
        void replaceNode(uint64_t oldNode, uint64_t newNode);
        void eraseDocument(uint64_t document_hash);
        void eraseDocument(uint64_t document_hash, const bool includeEdges);

    private:
        eosio::name m_contract;
    };
}; // namespace hypha

#define DECLARE_DOCUMENT_GRAPH(contract)\
using FlexValue = hypha::Content::FlexValue;\
using root_doc = hypha::Document;\
TABLE contract##_document : public root_doc {};\
using contract_document = contract##_document;\
using document_table =  eosio::multi_index<eosio::name("documents"), contract_document,\
                            eosio::indexed_by<name("bycreator"), eosio::const_mem_fun<root_doc, uint64_t, &root_doc::by_creator>>,\
                            eosio::indexed_by<name("bycreated"), eosio::const_mem_fun<root_doc, uint64_t, &root_doc::by_created>>>;\
using root_edge = hypha::Edge;\
TABLE contract##_edge : public root_edge {};\
using contract_edge = contract##_edge;\
using edge_table = eosio::multi_index<eosio::name("edges"), contract_edge,\
            eosio::indexed_by<eosio::name("fromnode"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_from>>,\
            eosio::indexed_by<eosio::name("tonode"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_to>>,\
            eosio::indexed_by<eosio::name("edgename"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_edge_name>>,\
            eosio::indexed_by<eosio::name("byfromname"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_from_node_edge_name_index>>,\
            eosio::indexed_by<eosio::name("byfromto"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_from_node_to_node_index>>,\
            eosio::indexed_by<eosio::name("bytoname"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_to_node_edge_name_index>>,\
            eosio::indexed_by<eosio::name("bycreated"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_created>>,\
            eosio::indexed_by<eosio::name("bycreator"), eosio::const_mem_fun<root_edge, uint64_t, &root_edge::by_creator>>>;