#pragma once
#include <eosio/name.hpp>
#include <eosio/crypto.hpp>

#include <document_graph/document.hpp>

namespace hypha
{
    class HashDocument : public Document
    {
    public:

        HashDocument(eosio::name contract, eosio::name creator, ContentGroups contentGroups);
        HashDocument(eosio::name *contract, const eosio::checksum256 &hash);

        // returns a document, saves to RAM if it doesn't already exist
        static HashDocument getOrNew(eosio::name contract, eosio::name creator, ContentGroups contentGroups);
        static HashDocument getOrNew(eosio::name contract, eosio::name creator, ContentGroup contentGroup);
        static HashDocument getOrNew(eosio::name contract, eosio::name creator, Content content);
        static HashDocument getOrNew(eosio::name contract, eosio::name creator, const std::string &label, const Content::FlexValue &value);

        static bool exists(eosio::name contract, const eosio::checksum256 &hash);

        const void hashContents();
        static const eosio::checksum256 hashContents(const ContentGroups &contentGroups);

        bool isHash();
        std::string getLabel();
        std::string getReadable();
    };
} // namespace hypha