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

    ContentGroup *ContentWrapper::getGroupOrFail(const std::string &groupLabel)
    {
        return getGroupOrFail(groupLabel, "group: " + groupLabel + " is required but not found");
    }

    std::pair<int64_t, Content *> ContentWrapper::get(const std::string &groupLabel, const std::string &contentLabel)
    {
        auto [idx, contentGroup] = getGroup(groupLabel);
        
        return get(static_cast<size_t>(idx), contentLabel);
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

    Content *ContentWrapper::getOrFail(const std::string &groupLabel, const std::string &contentLabel)
    {
        return getOrFail(groupLabel, contentLabel, "group: " + groupLabel + "; content: " + contentLabel + 
            " is required but not found");
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

    std::pair<int64_t, Content *> ContentWrapper::get(size_t groupIndex, const std::string &contentLabel)
    {
      if (groupIndex < m_contentGroups.size()) {

        auto& contentGroup = m_contentGroups[groupIndex];

        for (size_t i = 0; i < contentGroup.size(); ++i)
        {
            if (contentGroup.at(i).label == contentLabel)
            {
                return {(int64_t)i, &contentGroup.at(i)};
            }
        }
      }

      return {-1, nullptr};
    }

    void ContentWrapper::removeGroup(const std::string &groupLabel)
    {
      auto [idx, grp] = getGroup(groupLabel);
      eosio::check(idx != -1, 
            "Can't remove unexisting group: " + groupLabel);
      removeGroup(static_cast<size_t>(idx));
    }

    void ContentWrapper::removeGroup(size_t groupIndex)
    {
      eosio::check(groupIndex >= m_contentGroups.size(), 
            "Can't remove invalid group index: " + std::to_string(groupIndex));
      
      m_contentGroups.erase(m_contentGroups.begin() + groupIndex);
    }

    void ContentWrapper::removeContent(const std::string &groupLabel, const std::string &contentLabel)
    {
      auto [gidx, contentGroup] = getGroup(groupLabel);

      eosio::check(gidx != -1, 
            "Can't remove content from unexisting group: " + groupLabel);
      
      removeContent(static_cast<size_t>(gidx), contentLabel);
    }

    void ContentWrapper::removeContent(size_t groupIndex, const std::string &contentLabel)
    {
      auto [cidx, content] = get(static_cast<size_t>(groupIndex), contentLabel);

      eosio::check(cidx != -1, 
            "Can't remove unexisting content [" + contentLabel + "]");

      removeContent(groupIndex, cidx);
    }

    void ContentWrapper::removeContent(size_t groupIndex, size_t contentIndex)
    {
      eosio::check(groupIndex < m_contentGroups.size(), 
            "Can't remove content from invalid group index [Out Of Rrange]: " + std::to_string(groupIndex));

      auto& contentGroup = m_contentGroups[groupIndex];

      eosio::check(contentIndex < contentGroup.size(), 
            "Can't remove invalid content index [Out Of Rrange]: " + std::to_string(contentIndex));

      contentGroup.erase(contentGroup.begin() + contentIndex);
    }


    void ContentWrapper::insertOrReplace(size_t groupIndex, const Content &newContent)
    {
      eosio::check(groupIndex < m_contentGroups.size(), 
            "Can't access invalid group index [Out Of Rrange]: " + std::to_string(groupIndex));
      
      auto& contentGroup = m_contentGroups[groupIndex];

      insertOrReplace(contentGroup, newContent);
    }

    string_view ContentWrapper::getGroupLabel(const ContentGroup &contentGroup)
    {
      for (auto& content : contentGroup) {
        if (content.label == CONTENT_GROUP_LABEL) {
          eosio::check(std::holds_alternative<std::string>(content.value), 
                       "fatal error: " + CONTENT_GROUP_LABEL + " must be a string");
          return content.getAs<string>();
        }
      }

      return {};
    }

    void ContentWrapper::insertOrReplace(ContentGroup &contentGroup, const Content &newContent)
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