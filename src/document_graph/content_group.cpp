#include <eosio/eosio.hpp>
#include <document_graph/content_group.hpp>
#include <document_graph/content.hpp>

namespace hypha
{
    ContentWrapper::ContentWrapper(const ContentGroups& cgs) : m_contentGroups{cgs} {}
    ContentWrapper::~ContentWrapper() {}

    ContentGroup ContentWrapper::getGroup(const std::string &groupLabel)
    {
        for (std::size_t i = 0; i < m_contentGroups.size(); ++i)
        {
            for (const Content &content : m_contentGroups[i])
            {
                if (content.label == CONTENT_GROUP_LABEL)
                {
                    eosio::check(std::holds_alternative<std::string>(content.value), "fatal error: " + CONTENT_GROUP_LABEL + " must be a string");
                    if (std::get<std::string>(content.value) == groupLabel)
                    {
                        return m_contentGroups[i];
                    }
                }
            }
        }
        eosio::check(false, "no content with label found: " + groupLabel);
        return ContentGroup{};
    }

    Content ContentWrapper::getContent(const std::string &groupLabel, const std::string &contentLabel)
    {
        ContentGroup contentGroup = getGroup(groupLabel);
        for (std::size_t i = 0; i < contentGroup.size(); ++i)
        {
            if (contentGroup[i].label == contentLabel)
            {
                return contentGroup[i];
            }
        }
        return Content{};
    }    

    bool ContentWrapper::exists (const std::string &groupLabel, const std::string &contentLabel)
    {
        ContentGroup contentGroup = getGroup(groupLabel);
        for (std::size_t i = 0; i < contentGroup.size(); ++i)
        {
            if (contentGroup[i].label == contentLabel)
            {
                return true;
            }
        }
        return false;
    }

    eosio::asset ContentWrapper::getAsset(const std::string &groupLabel, const std::string &contentLabel)
    {
        Content::FlexValue flex = getContent(groupLabel, contentLabel).value;
        eosio::check(std::holds_alternative<eosio::asset>(flex),
                     "Content group: " + contentLabel + ", item: " + contentLabel +
                         " is not of expected type");
        return std::get<eosio::asset>(flex);
    }

    std::string ContentWrapper::getString(const std::string &groupLabel, const std::string &contentLabel)
    {
        Content::FlexValue flex = getContent(groupLabel, contentLabel).value;
        eosio::check(std::holds_alternative<std::string>(flex),
                     "Content group: " + contentLabel + ", item: " + contentLabel +
                         " is not of expected type");
        return std::get<std::string>(flex);
    }

    std::int64_t ContentWrapper::getInt(const std::string &groupLabel, const std::string &contentLabel)
    {
        Content::FlexValue flex = getContent(groupLabel, contentLabel).value;
        eosio::check(std::holds_alternative<std::int64_t>(flex),
                     "Content group: " + contentLabel + ", item: " + contentLabel +
                         " is not of expected type");
        return std::get<std::int64_t>(flex);
    }

    eosio::name ContentWrapper::getName(const std::string &groupLabel, const std::string &contentLabel)
    {
        Content::FlexValue flex = getContent(groupLabel, contentLabel).value;
        eosio::check(std::holds_alternative<eosio::name>(flex),
                     "Content group: " + contentLabel + ", item: " + contentLabel +
                         " is not of expected type");
        return std::get<eosio::name>(flex);
    }

    eosio::checksum256 ContentWrapper::getChecksum(const std::string &groupLabel, const std::string &contentLabel)
    {
        Content::FlexValue flex = getContent(groupLabel, contentLabel).value;
        eosio::check(std::holds_alternative<eosio::checksum256>(flex),
                     "Content group: " + contentLabel + ", item: " + contentLabel +
                         " is not of expected type");
        return std::get<eosio::checksum256>(flex);
    }

    // static
    ContentGroup ContentWrapper::getGroup(const ContentGroups &contentGroups, const std::string &groupLabel)
    {
        ContentWrapper cw (contentGroups);
        return cw.getGroup(groupLabel);
    }

    // static
    Content ContentWrapper::getContent(const ContentGroups &contentGroups, 
                                            const std::string &groupLabel,
                                            const std::string &contentLabel)
    {
        ContentWrapper cw (contentGroups);
        return cw.getContent(groupLabel, contentLabel);
    }

    // static
    eosio::asset ContentWrapper::getAsset(const ContentGroups &contentGroups,
                                               const std::string &groupLabel,
                                               const std::string &contentLabel)
    {
        ContentWrapper cw (contentGroups);
        return cw.getAsset (groupLabel, contentLabel);
    }

    // static
    std::string ContentWrapper::getString(const ContentGroups &contentGroups,
                                               const std::string &groupLabel,
                                               const std::string &contentLabel)
    {
        ContentWrapper cw (contentGroups);
        return cw.getString(groupLabel, contentLabel);
    }

    // static
    std::int64_t ContentWrapper::getInt(const ContentGroups &contentGroups,
                                             const std::string &groupLabel,
                                             const std::string &contentLabel)
    {
       ContentWrapper cw (contentGroups);
       return cw.getInt(groupLabel, contentLabel);
    }

    // static
    eosio::name ContentWrapper::getName(const ContentGroups &contentGroups,
                                             const std::string &groupLabel,
                                             const std::string &contentLabel)
    {
        ContentWrapper cw (contentGroups);
        return cw.getName(groupLabel, contentLabel);
    }

    // static
    eosio::checksum256 ContentWrapper::getChecksum(const ContentGroups &contentGroups,
                                             const std::string &groupLabel,
                                             const std::string &contentLabel)
    {
        ContentWrapper cw (contentGroups);
        return cw.getChecksum(groupLabel, contentLabel);
    }



    // std::pair<int64_t, Content *> ContentWrapper::get(const std::string &label)
    // {
    //     for (std::size_t i = 0; i < m_contentGroup.size(); ++i)
    //     {
    //         if (m_contentGroup[i].label == label)
    //         {
    //             return {(int64_t)i, &m_contentGroup[i]};
    //         }
    //     }

    //     return {-1, nullptr};
    // }

    // Content* ContentWrapper::getOrFail(const std::string &label, const std::string &error)
    // {
    //     auto [idx, item] = get(label);
    //     if (idx == -1)
    //     {
    //         eosio::check(false, error);
    //     }
    //     return item;
    // }

    // std::pair<int64_t, ContentGroup *> ContentWrapper::getConntGroup(const std::string &label)
    // {
    //     for (std::size_t i = 0; i < content_groups.size(); ++i)
    //     {
    //         for (const Content &content : content_groups[i])
    //         {
    //             if (content.label == CONTENT_GROUP_LABEL)
    //             {
    //                 eosio::check(std::holds_alternative<std::string>(content.value), "fatal error: " + CONTENT_GROUP_LABEL + " must be a string");
    //                 if (std::get<std::string>(content.value) == label)
    //                 {
    //                     ContentWrapper cgw (content_groups[i]);
    //                     return {(int64_t)i, &cgw};
    //                 }
    //             }
    //         }
    //     }
    //     return {-1, nullptr};
    // }

    // ContentWrapper* ContentWrapper::getContentGroupOrFail(const std::string &label, const std::string &error)
    // {
    //     auto [idx, contentGroup] = getContentGroup(label);
    //     if (idx == -1)
    //     {
    //         eosio::check(false, error);
    //     }
    //     return contentGroup;
    // }

} // namespace hypha