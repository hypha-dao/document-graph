#include <eosio/eosio.hpp>
#include <document_graph/content_wrapper.hpp>
#include <document_graph/content.hpp>

namespace hypha
{
    ContentWrapper::ContentWrapper(ContentGroups& cgs) : m_contentGroups{cgs} {}
    ContentWrapper::~ContentWrapper() {}

    std::pair<int64_t, ContentGroup *> ContentWrapper::getGroup(const std::string &label)
    {
        for (std::size_t i = 0; i < getContentGroups().size(); ++i)
        {
            for (Content &content : getContentGroups()[i])
            {
                if (content.label == CONTENT_GROUP_LABEL)
                {
                    eosio::check(std::holds_alternative<std::string>(content.value), "fatal error: " + CONTENT_GROUP_LABEL + " must be a string");
                    if (std::get<std::string>(content.value) == label)
                    {
                        return {(int64_t)i, &getContentGroups()[i]};
                    }
                }
            }
        }
        return {-1, nullptr};
    }

    ContentGroup *ContentWrapper::getGroupOrFail(const std::string &label, const std::string &error)
    {
        auto [idx, contentGroup] = getGroup(label);
        if (idx == -1)
        {
            eosio::check(false, error);
        }
        return contentGroup;
    }

    std::pair<int64_t, Content *> ContentWrapper::get(const std::string &groupLabel, const std::string &contentLabel)
    {
        auto [idx, contentGroup] = getGroup(groupLabel);
        if (idx == -1)
        {
            return {-1, nullptr};
        }

        for (std::size_t i = 0; i < contentGroup->size(); ++i)
        {
            if (contentGroup->at(i).label == contentLabel)
            {
                return {(int64_t)i, &contentGroup->at(i)};
            }
        }

        return {-1, nullptr};
    }

    Content *ContentWrapper::getOrFail(const std::string &groupLabel, const std::string &contentLabel, const std::string &error)
    {
        auto [idx, item] = get(groupLabel, contentLabel);
        if (idx == -1)
        {
            eosio::check(false, error);
        }
        return item;
    }

    bool ContentWrapper::exists(const std::string &groupLabel, const std::string &contentLabel)
    {
        auto [idx, item] = get(groupLabel, contentLabel);
        if (idx == -1)
        {
            return true;
        }
        return false;
    }

    void ContentWrapper::insertOrReplace(ContentGroup &contentGroup, Content &newContent)
    {
        auto is_key = [&newContent](auto &c) {
            return c.label == newContent.label;
        };
        //First let's check if key already exists
        auto content_itr = std::find_if(contentGroup.begin(), contentGroup.end(), is_key);

        if (content_itr == contentGroup.end())
        {
            contentGroup.push_back(Content{newContent.label, newContent.value});
        }
        else
        {
            content_itr->value = newContent.value;
        }
    }
}