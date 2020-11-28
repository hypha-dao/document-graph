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

        ContentWrapper(const ContentGroups& cgs);
        ~ContentWrapper();

        // non-static definitions
        ContentGroup getGroup(const string &groupLabel);
        Content getContent(const std::string &groupLabel, const std::string &contentLabel);
        bool exists (const std::string &groupLabel, const std::string &contentLabel);
        
        eosio::asset getAsset(const std::string &groupLabel, const std::string &contentLabel);
        std::string getString(const std::string &groupLabel, const std::string &contentLabel);
        eosio::name getName(const std::string &groupLabel, const std::string &contentLabel);
        std::int64_t getInt(const std::string &groupLabel, const std::string &contentLabel);
        eosio::checksum256 getChecksum(const std::string &groupLabel, const std::string &contentLabel);

      
        // series of static methods that instantiate an instance and call the non-static member
        // I imagine there is a smarter way to do this...
        static ContentGroup getGroup(const ContentGroups &contentGroups, const string &groupLabel);

        static Content getContent(const ContentGroups &contentGroups,
                                  const std::string &groupLabel,
                                  const std::string &contentLabel);

         static Content::FlexValue getValue(const ContentGroups &contentGroups,
                                           const std::string &groupLabel,
                                           const std::string &contentLabel);

        // not exactly sure how to do a template, using each type below
        // template <typename T>
        // static T get(const ContentGroups &contentGroups,
        //              const std::string &groupLabel,
        //              const std::string &contentLabel);

        static eosio::asset getAsset(const ContentGroups &contentGroups,
                                     const std::string &groupLabel,
                                     const std::string &contentLabel);

        static std::string getString(const ContentGroups &contentGroups,
                                     const std::string &groupLabel,
                                     const std::string &contentLabel);

        static eosio::name getName(const ContentGroups &contentGroups,
                                   const std::string &groupLabel,
                                   const std::string &contentLabel);

        static std::int64_t getInt(const ContentGroups &contentGroups,
                                   const std::string &groupLabel,
                                   const std::string &contentLabel);

        static eosio::checksum256 getChecksum(const ContentGroups &contentGroups,
                                   const std::string &groupLabel,
                                   const std::string &contentLabel);

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
            const ContentGroups& m_contentGroups;
    };

} // namespace hypha