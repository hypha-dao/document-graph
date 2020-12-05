#pragma once
#include <document_graph/content.hpp>

using std::string;

namespace hypha
{
    using ContentGroup = std::vector<Content>;
    using ContentGroups = std::vector<ContentGroup>;

    static const std::string CONTENT_GROUP_LABEL = std::string("content_group_label");

    class ContentWrapper
    {

    public:
        ContentWrapper(ContentGroups &cgs);
        ~ContentWrapper();

        // non-static definitions
        std::pair<int64_t, ContentGroup *> getGroup(const std::string &label);
        ContentGroup *getGroupOrFail(const std::string &label, const std::string &error);
        ContentGroup *getGroupOrFail(const std::string &groupLabel);

        std::pair<int64_t, Content *> get(const std::string &groupLabel, const std::string &contentLabel);
        Content *getOrFail(const std::string &groupLabel, const std::string &contentLabel, const std::string &error);
        Content *getOrFail(const std::string &groupLabel, const std::string &contentLabel);

        bool exists(const std::string &groupLabel, const std::string &contentLabel);

        static void insertOrReplace(ContentGroup &contentGroup, Content &newContent);

    private:
        ContentGroups &getContentGroups() { return m_contentGroups; }
        ContentGroups &m_contentGroups;
    };

} // namespace hypha