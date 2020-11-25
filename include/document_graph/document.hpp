#pragma once
#include <eosio/multi_index.hpp>
#include <variant>
#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

#include <document_graph/content.hpp>
#include <document_graph/content_group.hpp>

namespace hypha
{
    // unused for now, but leaving in the data structure for the future
    struct Certificate
    {
        Certificate() {}
        Certificate(const eosio::name &certifier, const std::string notes) : certifier{certifier}, notes{notes}{}

        eosio::name certifier;
        std::string notes;
        eosio::time_point certification_date = eosio::current_time_point();

        EOSLIB_SERIALIZE(Certificate, (certifier)(notes)(certification_date))
    };

    struct [[eosio::table, eosio::contract("docs")]] Document
    {
        public:
        Document();
        Document(eosio::name contract, eosio::name creator, std::vector<ContentGroup> contentGroups);
        Document(eosio::name contract, const eosio::checksum256 &hash);
        ~Document();

        void emplace ();

        static Document getOrCreate (eosio::name contract, eosio::name creator, std::vector<ContentGroup> contentGroups);
        uint64_t primary_key() const { return id; }
        uint64_t by_creator() const { return creator.value; }
        eosio::checksum256 by_hash() const { return hash; }
        uint64_t by_created() const { return created_date.sec_since_epoch(); }
        
        void setCreator(eosio::name & creator);
        eosio::name getCreator();
        eosio::checksum256 getHash();
        std::vector<ContentGroupWrapper> getContentGroups();
        std::pair<int64_t, ContentGroupWrapper *> getContentGroup(const std::string &label);
        ContentGroupWrapper * getContentGroupOrFail(const std::string &label, const std::string &error);
        Content::FlexValue getValueOrFail(const std::string &contentGroupLabel, const std::string &contentLabel, const std::string &error);
        void add(ContentGroup cg);

        // certificates are not yet used
        void certify(const eosio::name &certifier, const std::string &notes);

        // members, with names as serialized - these must be public for EOSIO tables
        std::uint64_t id;
        eosio::checksum256 hash;
        eosio::name creator;
        std::vector<ContentGroup> content_groups;
        std::vector<Certificate> certificates;
        eosio::time_point created_date;

        const eosio::checksum256 hashContents();

        typedef eosio::multi_index<eosio::name("documents"), Document,
                eosio::indexed_by<eosio::name("idhash"), eosio::const_mem_fun<Document, eosio::checksum256, &Document::by_hash>>,
                eosio::indexed_by<eosio::name("bycreator"), eosio::const_mem_fun<Document, uint64_t, &Document::by_creator>>,
                eosio::indexed_by<eosio::name("bycreated"), eosio::const_mem_fun<Document, uint64_t, &Document::by_created>>>
        document_table;

        private: 
            eosio::name m_contract;
            const std::string toString();
            const std::string toString(ContentGroup contentGroup);

        EOSLIB_SERIALIZE(Document, (id)(hash)(creator)(content_groups)(certificates)(created_date))
    };

} // namespace hypha