
#include <eosio/eosio.hpp>
#include <document_graph/content_group.hpp>
#include <document_graph/content.hpp>

namespace hypha
{
    ContentGroupWrapper::ContentGroupWrapper(ContentGroup& cg) : m_contentGroup{cg} {}
    ContentGroupWrapper::~ContentGroupWrapper() {}

    std::pair<int64_t, Content *> ContentGroupWrapper::get(const std::string &label)
    {
        for (std::size_t i = 0; i < m_contentGroup.size(); ++i)
        {
            if (m_contentGroup[i].label == label)
            {
                return {(int64_t)i, &m_contentGroup[i]};
            }
        }

        return {-1, nullptr};
    }

    Content* ContentGroupWrapper::getOrFail(const std::string &label, const std::string &error)
    {
        auto [idx, item] = get(label);
        if (idx == -1)
        {
            eosio::check(false, error);
        }
        return item;
    }

    bool ContentGroupWrapper::exists(const std::string &label)
    {
        for (std::size_t i = 0; i < m_contentGroup.size(); ++i)
        {
            if (m_contentGroup[i].label == label)
            {
                return true;
            }
        }

        return false;
    }

    void ContentGroupWrapper::add(Content item)
    {
        m_contentGroup.push_back(item);
    }

    ContentGroup &ContentGroupWrapper::getItems()
    {
        return m_contentGroup;
    }

    const std::string ContentGroupWrapper::toString()
    {
        std::string results = "[";
        bool is_first = true;

        for (Content &content : m_contentGroup)
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