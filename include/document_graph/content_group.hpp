#pragma once
#include <document_graph/content.hpp>

namespace hypha
{

    using ContentGroup = std::vector<Content>;
    static const std::string CONTENT_GROUP_LABEL = std::string("content_group_label");

    class ContentGroupWrapper 
    {

    public:
        ContentGroupWrapper(ContentGroup& cg); 
        ~ContentGroupWrapper(); 

        std::pair<int64_t, Content*> get (const std::string &label);
        Content* getOrFail(const std::string &label, const std::string &error);
        void add(Content item);
        bool exists(const std::string &label);

        ContentGroup& getItems();
        const std::string toString();

    private:
        ContentGroup& m_contentGroup;
    };

} // namespace hypha