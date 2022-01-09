#pragma once

#include <variant>

#include <eosio/multi_index.hpp>
#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

#include <document_graph/content.hpp>
#include <document_graph/content_wrapper.hpp>

namespace hypha
{
    // unused for now, but leaving in the data structure for the future
    struct Document
    {
    public:
        Document();

        // these constructors populate a Document instance and emplace
        Document(eosio::name contract, eosio::name creator, ContentGroups contentGroups);
        Document(eosio::name contract, eosio::name creator, ContentGroup contentGroup);
        Document(eosio::name contract, eosio::name creator, Content content);
        Document(eosio::name contract, eosio::name creator, const std::string &label, const Content::FlexValue &value);

        // this constructor reads the hash from the table and populates the object from storage
        Document(eosio::name contract, uint64_t id);
        ~Document();

        // actual sha256 of "DO_NOT_HASH" is d15ddfec8899ee7bf14aea64c77719d99101ae034fadd02e455476d59a8ec0f9
        // when the document.hash value is set to this, the document contents is never hashed
        // need to hardcode that to this constant
        const eosio::checksum256 DO_NOT_HASH = eosio::fixed_bytes<32UL>{};

        void emplace();

        /**
         * @brief Updates the document in the multi_index table with the given content groups
         * 
         * @param updatedData
         */
        void update(const eosio::name &updater, ContentGroups updatedData);

        static bool exists(eosio::name contract, uint64_t _id);

        // certificates are not yet used
        void certify(const eosio::name &certifier, const std::string &notes);

        // static helpers
        static ContentGroups rollup(ContentGroup contentGroup);
        static ContentGroups rollup(Content content);
        static void insertOrReplace(ContentGroup &contentGroup, Content &newContent);

        static Document merge(Document original, Document &deltas);

        // vanilla accessors
        ContentWrapper getContentWrapper() { return ContentWrapper(content_groups); }
        ContentGroups &getContentGroups() { return content_groups; }
        const ContentGroups &getContentGroups() const { return content_groups; }
        const eosio::checksum256 &getHash() const { return hash; }
        const eosio::time_point &getCreated() const { return created_date; }
        const eosio::name &getCreator() const { return creator; }
        const eosio::name &getContract() const { return contract; }

        // This has to be public in order to be reachable by the abi-generator macro
        // indexes for table
        uint64_t by_created() const { return created_date.sec_since_epoch(); }
        uint64_t by_updated() const { return updated_date.sec_since_epoch(); }
        uint64_t by_creator() const { return creator.value; }

        eosio::checksum256 by_hash() const { return hash; }

        inline uint64_t getID() { return id; }

    private:
        // members, with names as serialized - these must be public for EOSIO tables
        std::uint64_t id;
        eosio::checksum256 hash = DO_NOT_HASH;
        eosio::name creator;
        ContentGroups content_groups;
        eosio::time_point created_date;
        eosio::time_point updated_date;
        eosio::name contract;

        // toString iterates through all content, all levels, concatenating all values
        // the resulting string is used for fingerprinting and hashing
        const std::string toString();
        static const std::string toString(const ContentGroups &contentGroups);
        static const std::string toString(const ContentGroup &contentGroup);

        EOSLIB_SERIALIZE(Document, (id)(hash)(creator)(content_groups)(created_date)(updated_date)(contract))

    public:
        // for unknown reason, primary_key() must be public
        uint64_t primary_key() const { return id; }

        typedef eosio::multi_index<eosio::name("documents"), Document,
                                   eosio::indexed_by<eosio::name("idhash"), eosio::const_mem_fun<Document, eosio::checksum256, &Document::by_hash>>,
                                   eosio::indexed_by<eosio::name("bycreator"), eosio::const_mem_fun<Document, uint64_t, &Document::by_creator>>,
                                   eosio::indexed_by<eosio::name("bycreated"), eosio::const_mem_fun<Document, uint64_t, &Document::by_created>>,
                                   eosio::indexed_by<eosio::name("byupdated"), eosio::const_mem_fun<Document, uint64_t, &Document::by_updated>>>

            document_table;
    };
} // namespace hypha