#include <document_graph/document.hpp>
#include <document_graph/content_group.hpp>
#include <eosio/crypto.hpp>

namespace hypha
{

    Document::~Document() {}
    Document::Document() {}

    Document::Document(const uint64_t id, const eosio::name creator, std::vector<ContentGroup> contentGroups) : id{id}, creator{creator}
    {
        content_groups = contentGroups;
        hash = hash_contents();
    }

    const eosio::checksum256 Document::hash_contents()
    {
        std::string string_data = toString();
        return eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());
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

    const std::string Document::toString()
    {
        std::string results = "[";
        bool is_first = true;

        for (ContentGroup contentGroup : content_groups)
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

    const std::string Document::toString(ContentGroup contentGroup)
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
} // namespace hypha