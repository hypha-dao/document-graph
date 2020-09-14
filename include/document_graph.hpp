#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

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

        struct [[eosio::table, eosio::contract("multisig")]] document
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

        // Any account/member can creator a new document
        document create_document(const name &creator, const vector<content_group> &content_groups);

        void erase_document(const checksum256 &document_hash);

        // Transform a legacy object format to the new document format
        // void transform_document(const name &scope, const uint64_t &id);

        // Fork creates a new document (node in a graph) from an existing document.
        // The forked content should contain only new or updated entries to avoid data duplication. (lazily enforced?)
        document fork_document(const checksum256 &hash, const name &creator, const vector<content_group> &content_groups);

        // Creates a 'certificate' on a specific fork.
        // A certificate can be customized based on the document, but it represents
        // the signatures, with notes/timestamp, and of course auth is enforced
        void certify_document(const name &certifier, const checksum256 &hash, const string &notes);

        certificate new_certificate(const name &certifier, const string &notes);
        content new_content(const string &label, const flexvalue &fv);

        // accessors
        document get_document (const checksum256 &hash);
        document get_parent (const document &document);
        content_group get_content_group (const document &document, const string &content_group_label);
        flexvalue get_content (const content_group &content_group, const string& content_label);

        std::string to_string(const vector<document_graph::content_group> &content_groups);
        std::string to_string(const document_graph::content_group &content_group);
        std::string to_string(const document_graph::content &content);
        std::string to_string(const document_graph::flexvalue &value);

        static std::string to_hex(const char *d, uint32_t s);
    };
}; // namespace hyphaspace