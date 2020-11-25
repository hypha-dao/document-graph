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
        eosio::name m_contract;

        DocumentGraph(const eosio::name &contract) : m_contract(contract){}
        ~DocumentGraph(){}
 
        typedef eosio::multi_index<eosio::name("edges"), Edge,
            eosio::indexed_by<eosio::name("fromnode"), eosio::const_mem_fun<Edge, eosio::checksum256, &Edge::by_from>>,
            eosio::indexed_by<eosio::name("tonode"), eosio::const_mem_fun<Edge, eosio::checksum256, &Edge::by_to>>,
            eosio::indexed_by<eosio::name("edgename"), eosio::const_mem_fun<Edge, uint64_t, &Edge::by_edge_name>>,
            eosio::indexed_by<eosio::name("byfromname"), eosio::const_mem_fun<Edge, uint64_t, &Edge::by_from_node_edge_name_index>>,
            eosio::indexed_by<eosio::name("byfromto"), eosio::const_mem_fun<Edge, uint64_t, &Edge::by_from_node_to_node_index>>,
            eosio::indexed_by<eosio::name("bytoname"), eosio::const_mem_fun<Edge, uint64_t, &Edge::by_to_node_edge_name_index>>,
            eosio::indexed_by<eosio::name("bycreated"), eosio::const_mem_fun<Edge, uint64_t, &Edge::by_created>>,
            eosio::indexed_by<eosio::name("bycreator"), eosio::const_mem_fun<Edge, uint64_t, &Edge::by_creator>>>
        edge_table;

        // always strict (fails if edge does not exist)
        void removeEdge (const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode, const eosio::name &edgeName);
        void removeEdges (const eosio::checksum256 &node);

        std::vector<Edge> getEdges (const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode);
        std::vector<Edge> getEdgesOrFail (const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode);

        std::vector<Edge> getEdgesFrom (const eosio::checksum256 &fromNode, const eosio::name &edgeName);
        std::vector<Edge> getEdgesFromOrFail (const eosio::checksum256 &fromNode, const eosio::name &edgeName);

        std::vector<Edge> getEdgesTo (const eosio::checksum256 &toNode, const eosio::name &edgeName);
        std::vector<Edge> getEdgesToOrFail (const eosio::checksum256 &toNode, const eosio::name &edgeName);

        // Any account/member can creator a new document, support many options/constructors
        // Document createDocument(eosio::name &creator, std::vector<ContentGroup> &contentGroups);
        // Document createDocument(eosio::name &creator, ContentGroup &contentGroup);
        // Document createDocument(eosio::name &creator, Content &content);
        // Document createDocument(eosio::name &creator, const std::string &label, const Content::FlexValue &value);

        // only for testing
        // Document createDocument(eosio::name &creator);

        Edge createEdge (eosio::name &creator, const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode, const eosio::name &edgeName);

        // Document getOrCreate(const eosio::name &creator, const std::vector<ContentGroup> &contentGroups);
        // Document getOrCreate(const eosio::name &creator, const ContentGroup &contentGroup);
        // Document getOrCreate(const eosio::name &creator, const Content &content);
        // Document getOrCreate(const eosio::name &creator, const std::string &label, const Content::FlexValue &value);

        void eraseDocument(const eosio::checksum256 &document_hash);
    
        // // Fork creates a new document (node in a graph) from an existing document.
        // // The forked content should contain only new or updated entries to avoid data duplication. (lazily enforced?)
        // document fork_document(const checksum256 &hash, const name &creator, const vector<content_group> &content_groups);
        // document fork_document(const checksum256 &hash, const name &creator, const content &content);

        // // Creates a 'certificate' on a specific fork.
        // // A certificate can be customized based on the document, but it represents
        // // the signatures, with notes/timestamp, and of course auth is enforced
        // void certify_document(const name &certifier, const checksum256 &hash, const string &notes);

        // certificate new_certificate(const name &certifier, const string &notes);
        // content new_content(const string &label, const flexvalue &fv);

        // // accessors
        // document get_document (const checksum256 &hash);
        // document get_parent (const document &document);
       
        // content_group get_content_group(const vector<content_group> &content_groups, 
        //                                 const string &content_group_label, 
        //                                 const bool &strict);
        
        // content_group get_content_group (const document &document, 
        //                                     const string &content_group_label, 
        //                                     const bool &strict);

        // std::vector<content_group> get_content_groups_of_type(const vector<content_group> &content_groups, 
        //                                         const name &content_group_type,
        //                                         const bool &strict);

        // flexvalue get_content (const content_group &content_group, const string& content_label, const bool &strict);
        // flexvalue get_content(const document &document,
        //                         const string &content_group_label,
        //                         const string &content_label,
        //                         const bool &strict);
        // content get_content_item(const content_group &content_group,
        //                                                         const string &content_label,
        //                                                         const bool &strict);

        // static std::string to_string(const vector<document_graph::content_group> &content_groups);
        // static std::string to_string(const document_graph::content_group &content_group);
        // static std::string to_string(const document_graph::content &content);
        // static std::string to_string(const document_graph::flexvalue &value);

        // static checksum256 hash_document (const vector<content_group> &content_groups);
        // static std::string to_hex(const char *d, uint32_t s);
        // static std::string readable_hash (const checksum256 &proposal_hash);
        
        // // uint64_t to_uint64 (const checksum256 &document_hash);
        // static uint64_t to_uint64 (const string &fingerprint);
        // static uint64_t edge_id(checksum256 from_node, checksum256 to_node, name edge_name);
        // static uint64_t hash(checksum256 from_node, checksum256 to_node);
        // static uint64_t hash(checksum256 from_node, name edge_name); 
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