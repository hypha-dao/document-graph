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
        std::pair<int64_t, Content *> get(const std::string &groupLabel, const std::string &contentLabel);
        Content *getOrFail(const std::string &groupLabel, const std::string &contentLabel, const std::string &error);
        bool exists(const std::string &groupLabel, const std::string &contentLabel);
        
        static void insertOrReplace(ContentGroup &contentGroup, Content &newContent);

        // prior version of getters used the std::pair as a return type
        // std::pair<int64_t, Content*> get (const std::string &label);
        // Content* getOrFail(const std::string &label, const std::string &error);

        // void add(Content item);
        // bool exists(const std::string &label);

        // ContentGroup& getItems();
        // const std::string toString();

        // static (stateless) parsers and finders
        // used to find data within ContentGroups
        // static std::pair<int64_t, Content *> get(ContentGroups contentGroups, const std::string &label);

        // static Content::FlexValue getValueOrFail(const ContentGroups &contentGroups,
        //                                          const string &contentGroupLabel,
        //                                          const string &contentLabel);

        // static ContentWrapper* getContentGroupOrFail(const string &label);

    private:
        ContentGroups &getContentGroups() { return m_contentGroups; }
        ContentGroups &m_contentGroups;
    };

} // namespace hypha