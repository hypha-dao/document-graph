#include <document_graph/document.hpp>
#include <document_graph/content_group.hpp>
#include <document_graph/util.hpp>
#include <eosio/crypto.hpp>

namespace hypha
{

    Document::~Document() {}
    Document::Document() {}

    Document::Document(eosio::name contract, eosio::name creator, std::vector<ContentGroup> contentGroups) 
        : contract{contract}, creator{creator}, content_groups{std::move(contentGroups)}
    {
        hash = hashContents();
    }

    Document::Document(eosio::name contract, eosio::name creator, ContentGroup contentGroup)
    {
        Document (contract, creator, rollup (contentGroup));
    }

    Document::Document(eosio::name contract, eosio::name creator, Content content)
    {
        Document (contract, creator, rollup (content));
    }

    Document::Document(eosio::name contract, eosio::name creator, const std::string &label, const Content::FlexValue &value)
    {
        Document (contract, creator, rollup (Content (label, value)));
    }

    Document::Document(eosio::name contract, const eosio::checksum256 &_hash) : contract{contract}
    {
        document_table d_t(contract, contract.value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(_hash);
        eosio::check(h_itr != hash_index.end(), "document not found: " + readableHash(_hash));

        id = h_itr->id;
        creator = h_itr->creator;
        created_date = h_itr->created_date;
        certificates = h_itr->certificates;
        content_groups = h_itr->content_groups;
        hash = hashContents();

        // this should never happen, only if hash algorithm somehow changed
        eosio::check (hash == _hash, "fatal error: provided and indexed hash does not match newly generated hash");
    }

    void Document::emplace () 
    {
        require_auth(creator);
        hash = hashContents();

        document_table d_t(contract, contract.value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(hash);

        // if this content exists already, error out and send back the hash of the existing document
        eosio::check(h_itr == hash_index.end(), "document exists already: " + readableHash(hash));

        d_t.emplace(contract, [&](auto &d) {
            id = d_t.available_primary_key(); 
            created_date = eosio::current_time_point();
            d = *this;
        });
    }

    Document Document::getOrNew (eosio::name _contract, eosio::name _creator, std::vector<ContentGroup> contentGroups) 
    {
        Document document {};
        document.content_groups = contentGroups;
        document.hash = document.hashContents();

        Document::document_table d_t(_contract, _contract.value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(document.getHash());

        // if this content exists already, return this one
        if (h_itr != hash_index.end()) {
            document.creator = h_itr->creator;
            document.created_date = h_itr->created_date;
            document.certificates = h_itr->certificates;
            document.id = h_itr->id;
            return document;
        }

        require_auth(_creator);
        return Document (_contract, _creator, contentGroups);
    }

    Document Document::getOrNew (eosio::name contract, eosio::name creator, ContentGroup contentGroup)
    {
        return getOrNew (contract, creator, rollup (contentGroup));
    }

    Document Document::getOrNew (eosio::name contract, eosio::name creator, Content content) 
    {
        return getOrNew (contract, creator, rollup (content));
    }

    Document Document::getOrNew (eosio::name contract, eosio::name creator, const std::string &label, const Content::FlexValue &value)
    {
        return getOrNew (contract, creator, rollup (Content (label, value)));
    }

    void Document::setCreator(eosio::name &creator)
    {
        creator = creator;
    }

    eosio::name Document::getCreator()
    {
        return creator;
    }

    eosio::checksum256 Document::getHash()
    {
        return hash;
    }

    void Document::add(ContentGroup cg)
    {
        content_groups.push_back(cg);
    }

    std::pair<int64_t, ContentGroupWrapper *> Document::getContentGroup(const std::string &label)
    {
        for (std::size_t i = 0; i < content_groups.size(); ++i)
        {
            for (const Content &content : content_groups[i])
            {
                if (content.label == CONTENT_GROUP_LABEL)
                {
                    eosio::check(std::holds_alternative<std::string>(content.value), "fatal error: " + CONTENT_GROUP_LABEL + " must be a string");
                    if (std::get<std::string>(content.value) == label)
                    {
                        ContentGroupWrapper cgw (content_groups[i]);
                        return {(int64_t)i, &cgw};
                    }
                }
            }
        }
        return {-1, nullptr};
    }

    ContentGroupWrapper* Document::getContentGroupOrFail(const std::string &label, const std::string &error)
    {
        auto [idx, contentGroup] = getContentGroup(label);
        if (idx == -1)
        {
            eosio::check(false, error);
        }
        return contentGroup;
    }

    Content::FlexValue Document::getValueOrFail(const std::string &contentGroupLabel, const std::string &contentLabel, const std::string &error)
    {
        auto contentGroup = getContentGroupOrFail(contentGroupLabel, error);
        auto contentItem = contentGroup->getOrFail(contentLabel, error); 
        return contentItem->value;
    }

    std::vector<ContentGroupWrapper> Document::getContentGroups()
    {
        std::vector<ContentGroupWrapper> contentGroups;
        for (ContentGroup &contentGroup : content_groups)
        {
            // auto cgw = ContentGroupWrapper (contentGroup);
            contentGroups.push_back(ContentGroupWrapper(contentGroup));
        }    
        
        return contentGroups;
    }

    // void Document::certify(const eosio::name &certifier, const std::string &notes)
    // {
    //     // check if document is already saved??
    //     document_table d_t(m_contract, m_contract.value);
    //     auto h_itr = hash_index.find(id);
    //     eosio::check(h_itr != d_t.end(), "document not found when attemption to certify: " + readableHash(geash()));

    //     require_auth(certifier);

    //     // TODO: should a certifier be able to sign the same document fork multiple times?
    //     d_t.modify(h_itr, m_contract, [&](auto &d) {
    //         d = std::move(this);
    //         d.certificates.push_back(new_certificate(certifier, notes));
    //     });
    // }

    const eosio::checksum256 Document::hashContents()
    {
        // save/cache the hash in the member 
        hash = hashContents (content_groups);
        return hash;
    }

    const std::string Document::toString ()
    {
        return toString (content_groups);
    }

    // static version cannot cache the hash in a member
    const eosio::checksum256 Document::hashContents (std::vector<ContentGroup> &contentGroups)
    {
        std::string string_data = toString(contentGroups);
        return eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());
    }

    const std::string Document::toString(std::vector<ContentGroup> &contentGroups)
    {
        std::string results = "[";
        bool is_first = true;

        for (ContentGroup &contentGroup : contentGroups)
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                results = results + ",";
            }
            results = results + toString(contentGroup);
        }

        results = results + "]";
        return results;
    }

    const std::string Document::toString(ContentGroup &contentGroup)
    {
        std::string results = "[";
        bool is_first = true;

        for (Content &content : contentGroup)
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                results = results + ",";
            }
            results = results + content.toString();
        }

        results = results + "]";
        return results;
    }

    std::vector<ContentGroup> Document::rollup (ContentGroup contentGroup)
    {
        std::vector<ContentGroup> contentGroups;
        contentGroups.push_back (contentGroup);
        return contentGroups;
    }

    ContentGroup Document::rollup (Content content)
    {
        ContentGroup contentGroup;
        contentGroup.push_back (content);
        return rollup (content);
    }

} // namespace hypha