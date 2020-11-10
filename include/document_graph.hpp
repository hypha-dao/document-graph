#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <cstring>

using namespace eosio;
using namespace std;

namespace hyphaspace
{
    class document_graph
    {
    public:
        name contract;

        document_graph(const name &contract) : contract(contract){}
        ~document_graph(){}

        // flexvalue can any of these commonly used eosio data types
        // a checksum256 can be a link to another document, akin to an "edge" on a graph
        typedef std::variant<name, string, asset, time_point, int64_t, checksum256> flexvalue;
        const flexvalue DOES_NOT_EXIST = -4206942069;  // arbitrary, lazy, hopefully never used value

        // a single labeled flexvalue
        struct content
        {
            string label;
            flexvalue value;

            EOSLIB_SERIALIZE(content, (label)(value))
        };

        typedef vector<content> content_group;

        struct certificate
        {
            name certifier;
            string notes;
            time_point certification_date = current_time_point();

            EOSLIB_SERIALIZE(certificate, (certifier)(notes)(certification_date))
        };

        struct [[eosio::table, eosio::contract("docs")]] document
        {
            uint64_t id;
            checksum256 hash;
            name creator;
            vector<content_group> content_groups;

            vector<certificate> certificates;
            uint64_t primary_key() const { return id; }
            uint64_t by_creator() const { return creator.value; }
            checksum256 by_hash() const { return hash; }

            time_point created_date = current_time_point();
            uint64_t by_created() const { return created_date.sec_since_epoch(); }

            EOSLIB_SERIALIZE(document, (id)(hash)(creator)(content_groups)(certificates)(created_date))
        };

        typedef multi_index<name("documents"), document,
                            indexed_by<name("idhash"), const_mem_fun<document, checksum256, &document::by_hash>>,
                            indexed_by<name("bycreator"), const_mem_fun<document, uint64_t, &document::by_creator>>,
                            indexed_by<name("bycreated"), const_mem_fun<document, uint64_t, &document::by_created>>>
            document_table;

        
        // scopes: get_self() 
        struct [[eosio::table, eosio::contract("docs")]] edge
        {
            uint64_t id; 

            // these three additional indexes allow isolating/querying edges more precisely (less iteration)
            uint64_t from_node_edge_name_index;
            uint64_t from_node_to_node_index;
            uint64_t to_node_edge_name_index;
            uint64_t by_from_node_edge_name_index() const { return from_node_edge_name_index; }
            uint64_t by_from_node_to_node_index() const { return from_node_to_node_index; } 
            uint64_t by_to_node_edge_name_index() const { return to_node_edge_name_index; }

            checksum256 from_node;
            checksum256 by_from() const { return from_node; }

            checksum256 to_node;
            checksum256 by_to() const { return to_node; }

            name edge_name;
            uint64_t by_edge_name() const { return edge_name.value; }

            time_point created_date = current_time_point();
            uint64_t by_created() const { return created_date.sec_since_epoch(); }

            uint64_t primary_key() const { return id; }

            EOSLIB_SERIALIZE(edge, (id) (from_node_edge_name_index)(from_node_to_node_index)(to_node_edge_name_index)
                                        (from_node)(to_node)(edge_name)(created_date))
        };

        typedef multi_index<name("edges"), edge,
            indexed_by<name("fromnode"), const_mem_fun<edge, checksum256, &edge::by_from>>,
            indexed_by<name("tonode"), const_mem_fun<edge, checksum256, &edge::by_to>>,
            indexed_by<name("edgename"), const_mem_fun<edge, uint64_t, &edge::by_edge_name>>,
            indexed_by<name("bycreated"), const_mem_fun<edge, uint64_t, &edge::by_created>>,
            indexed_by<name("byfromedge"), const_mem_fun<edge, uint64_t, &edge::by_from_node_edge_name_index>>,
            indexed_by<name("byfromto"), const_mem_fun<edge, uint64_t, &edge::by_from_node_to_node_index>>,
            indexed_by<name("bytoedge"), const_mem_fun<edge, uint64_t, &edge::by_to_node_edge_name_index>>>
        edge_table;

        uint64_t to_uint64 (const checksum256 &document_hash);
        void create_edge (const checksum256 &from_node, const checksum256 &to_node, const name &edge_name);
        void create_edge (const checksum256 &from_node, const checksum256 &to_node, const name &edge_name, const bool strict);

        void remove_edge (const checksum256 &from_node, const checksum256 &to_node, const name &edge_name, const bool strict);
        void remove_edges (const checksum256 &from_node, const checksum256 &to_node, const bool strict);
        void remove_edges (const checksum256 &from_node, const name &edge_name, const bool strict);
        void remove_edges (const checksum256 &node, const bool strict);

        vector<edge> get_edges (const checksum256 &from_node, const name &edge_name, const bool strict);
        // vector<edge> get_edges (const checksum256 &to_node, const name &edge_name, const bool strict);
        // vector<edge> get_edges (const checksum256 &from_node, const checksum256 &to_node, const bool strict);

        edge get_edge (const checksum256 &from_node, const name &edge_name, const bool strict);
        // edge get_edge (const checksum256 &to_node, const name &edge_name, const bool strict);
        // edge get_edge (const checksum256 &from_node, const checksum256 &to_node, const bool strict);
        
        // Any account/member can creator a new document, support many options/constructors
        document create_document(const name &creator, const vector<content_group> &content_groups);
        document create_document(const name &creator, const content_group &content_group);
        document create_document(const name &creator, const content &content);
        document create_document(const name &creator, const string &content_label, const flexvalue &content_value);

        document get_or_create(const name &creator, const vector<content_group> &content_groups);
        document get_or_create(const name &creator, const content_group &content_group);
        document get_or_create(const name &creator, const content &content);
        document get_or_create(const name &creator, const string &content_label, const flexvalue &content_value);

        void erase_document(const checksum256 &document_hash);
    
        // Fork creates a new document (node in a graph) from an existing document.
        // The forked content should contain only new or updated entries to avoid data duplication. (lazily enforced?)
        document fork_document(const checksum256 &hash, const name &creator, const vector<content_group> &content_groups);
        document fork_document(const checksum256 &hash, const name &creator, const content &content);

        // Creates a 'certificate' on a specific fork.
        // A certificate can be customized based on the document, but it represents
        // the signatures, with notes/timestamp, and of course auth is enforced
        void certify_document(const name &certifier, const checksum256 &hash, const string &notes);

        certificate new_certificate(const name &certifier, const string &notes);
        content new_content(const string &label, const flexvalue &fv);

        // accessors
        document get_document (const checksum256 &hash);
        document get_parent (const document &document);
       
        content_group get_content_group(const vector<content_group> &content_groups, 
                                        const string &content_group_label, 
                                        const bool &strict);
        
        content_group get_content_group (const document &document, 
                                            const string &content_group_label, 
                                            const bool &strict);

        std::vector<content_group> get_content_groups_of_type(const vector<content_group> &content_groups, 
                                                const name &content_group_type,
                                                const bool &strict);

        flexvalue get_content (const content_group &content_group, const string& content_label, const bool &strict);
        flexvalue get_content(const document &document,
                                const string &content_group_label,
                                const string &content_label,
                                const bool &strict);
        content get_content_item(const content_group &content_group,
                                                                const string &content_label,
                                                                const bool &strict);

        static std::string to_string(const vector<document_graph::content_group> &content_groups);
        static std::string to_string(const document_graph::content_group &content_group);
        static std::string to_string(const document_graph::content &content);
        static std::string to_string(const document_graph::flexvalue &value);

        static checksum256 hash_document (const vector<content_group> &content_groups);
        static std::string to_hex(const char *d, uint32_t s);
        static std::string readable_hash (const checksum256 &proposal_hash);
        
        static uint64_t to_uint64 (const string &fingerprint);
        static uint64_t edge_id(checksum256 from_node, checksum256 to_node, name edge_name);
        static uint64_t hash(checksum256 from_node, checksum256 to_node);
        static uint64_t hash(checksum256 from_node, name edge_name); 
    };
}; // namespace hyphaspace