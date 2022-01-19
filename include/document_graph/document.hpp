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
        Document(eosio::name contract, const uint64_t &id);
        ~Document();

        void emplace();
        void modify();

        /**
         * @brief Updates the document in the multi_index table with the given content groups
         * 
         * @param updatedData
         */
        void update(const eosio::name &updater, ContentGroups updatedData);

        static bool exists(eosio::name contract, const uint64_t &documentId);

   // static helpers
        static ContentGroups rollup(ContentGroup contentGroup);
        static ContentGroups rollup(Content content);
        static void insertOrReplace(ContentGroup &contentGroup, Content &newContent);

        static Document merge(Document original, Document &deltas);

        // vanilla accessors
        ContentWrapper getContentWrapper() { return ContentWrapper(content_groups); }
        ContentGroups &getContentGroups() { return content_groups; }
        const ContentGroups &getContentGroups() const { return content_groups; }

        const std::uint64_t &getId() const { return id; }
        const eosio::time_point &getCreated() const { return created_date; }
        const eosio::time_point &getUpdated() const { return updated_date; }
        const eosio::name &getCreator() const { return creator; }
        const eosio::name &getContract() const { return contract; }

        // This has to be public in order to be reachable by the abi-generator macro
        // indexes for table
        uint64_t by_created() const { return created_date.sec_since_epoch(); }
        uint64_t by_updated() const { return updated_date.sec_since_epoch(); }
        uint64_t by_creator() const { return creator.value; }

        // TODO: this should be implemented in HashDocument preferably, but I dunno how 
        //       to do that whilst also impacting the ABI correctly :| (help!)
        eosio::checksum256 by_hash() const
        {
            auto cgs = getContentGroups();
            ContentWrapper cw (cgs); 
            if (const auto [_, hash] = cw.get("DETAILS", "HASH"); hash)
            {
                return std::get<eosio::checksum256>(hash->value);
            }
            // default value, for cases where hash is not used, would be all zeros
            return eosio::checksum256{};
        }
    private:
        // members, with names as serialized - these must be public for EOSIO tables
        std::uint64_t id;
        eosio::name creator;
        ContentGroups content_groups;
        eosio::time_point created_date;
        eosio::time_point updated_date;

        // I think we should remove this- it shouldn't be saved
        // because it can be derived and it wastes 64-bytes per document
        eosio::name contract;

        // toString iterates through all content, all levels, concatenating all values
        // the resulting string is used for fingerprinting and hashing
        const std::string toString();
        static const std::string toString(const ContentGroups &contentGroups);
        static const std::string toString(const ContentGroup &contentGroup);

        EOSLIB_SERIALIZE(Document, (id)(creator)(content_groups)(updated_date)(created_date)(contract))

    public:
        // for unknown reason, primary_key() must be public
        uint64_t primary_key() const { return id; }

        typedef eosio::multi_index<eosio::name("documents"), Document,
                                   eosio::indexed_by<eosio::name("byhash"), eosio::const_mem_fun<Document, eosio::checksum256, &Document::by_hash>>,
                                   eosio::indexed_by<eosio::name("bycreator"), eosio::const_mem_fun<Document, uint64_t, &Document::by_creator>>,
                                   eosio::indexed_by<eosio::name("bycreated"), eosio::const_mem_fun<Document, uint64_t, &Document::by_created>>,
                                   eosio::indexed_by<eosio::name("byupdated"), eosio::const_mem_fun<Document, uint64_t, &Document::by_updated>>>
            document_table;
    };

} // namespace hypha