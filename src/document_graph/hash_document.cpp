#include <hash_document.hpp>
#include <logger/logger.hpp>

namespace hypha
{
    HashDocument::HashDocument(eosio::name contract, eosio::name creator, ContentGroups contentGroups)
    : Document(contract, creator, contentGroups)
    {
       // run the hashing steps
       hashContents();
    }

    HashDocument::HashDocument(eosio::name *contract, const eosio::checksum256 &hash)
    {
        // look up the document by the hash index
        document_table d_t(getContract(), getContract().value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(hash);
        EOS_CHECK(h_itr != hash_index.end(), "document not found: " + readableHash(hash));

        // convert the found hashed document to an integer and load 
        Document (*contract, h_itr->id);
        // this should never happen, only if hash algorithm somehow changed
        EOS_CHECK(hash == hash, "fatal error: provided and indexed hash does not match newly generated hash");

    }

      HashDocument HashDocument::getOrNew(eosio::name _contract, eosio::name _creator, ContentGroups contentGroups) 
    {
        HashDocument document{};
        document.content_groups = contentGroups;
        document.hashContents();

        Document::document_table d_t(_contract, _contract.value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(document.hash);

        // if this content exists already, return this one
        if (h_itr != hash_index.end())
        {
            document.contract = _contract;
            document.creator = h_itr->creator;
            document.created_date = h_itr->created_date;
            document.id = h_itr->id;
            return document;
        }

        return Document(_contract, _creator, contentGroups);
    }

    Document Document::getOrNew(eosio::name contract, eosio::name creator, ContentGroup contentGroup)
    {
        return getOrNew(contract, creator, rollup(contentGroup));
    }

    Document Document::getOrNew(eosio::name contract, eosio::name creator, Content content)
    {
        return getOrNew(contract, creator, rollup(content));
    }

    Document Document::getOrNew(eosio::name contract, eosio::name creator, const std::string &label, const Content::FlexValue &value)
    {
        return getOrNew(contract, creator, rollup(Content(label, value)));
    }

    bool HashDocument::isHash() {
        return (getHash() == DO_NOT_HASH);
    }

    bool HashDocument::exists(eosio::name contract, const eosio::checksum256& _hash)
    {
        document_table d_t(contract, contract.value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(_hash);

        if (h_itr != hash_index.end())
        {
            return true;
        }
        return false;
    }


}  
