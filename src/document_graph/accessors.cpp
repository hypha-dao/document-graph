#include <document_graph.hpp>

namespace hyphaspace
{
    document_graph::document document_graph::get_document(const checksum256 &hash)
    {
        document_table d_t(contract, contract.value);
        auto hash_index = d_t.get_index<name("idhash")>();
        auto h_itr = hash_index.find(hash);

        auto byte_arr = hash.extract_as_byte_array();
        string readable_hash = document_graph::to_hex((const char *)byte_arr.data(), byte_arr.size());

        // if this content exists already, error out and send back the hash of the existing document
        check(h_itr != hash_index.end(), "document does not exist: " + readable_hash);
        return *h_itr;
    }

    document_graph::document document_graph::get_parent(const document &document)
    {
        auto content_group = get_content_group(document, "system", false);
        auto content_value = get_content(content_group, "parent", false);
        check(std::holds_alternative<checksum256>(content_value), "fatal error: system::parent content item is not a checksum256");
        return get_document(std::get<checksum256>(content_value));
    }

    document_graph::content_group document_graph::get_content_group(const vector<content_group> &content_groups, 
                                                                    const string &content_group_label,
                                                                    const bool &strict)
    {
        for (const document_graph::content_group &content_group : content_groups)
        {
            for (const document_graph::content &content : content_group)
            {
                if (content.label == "content_group_label")
                {
                    check(std::holds_alternative<string>(content.value), "fatal error: content_group_label must be a string");
                    if (std::get<string>(content.value) == content_group_label)
                    {
                        return content_group;
                    }
                }
            }
        }
        check (!strict, "content_group_label required for at least one content_group: " + content_group_label);
        return document_graph::content_group{};
    }

     document_graph::content_group document_graph::get_content_group(const document &document, 
                                                                    const string &content_group_label,
                                                                    const bool &strict)
    {
        return get_content_group (document.content_groups, content_group_label, strict);
    }

    std::vector<document_graph::content_group> document_graph::get_content_groups_of_type(const vector<content_group> &content_groups, 
                                                                            const name &content_group_type,
                                                                            const bool &strict)
    {
        vector<content_group> typed_content_groups;
        bool found = false;
        for (const document_graph::content_group &content_group : content_groups)
        {
            for (const document_graph::content &content : content_group)
            {
                if (content.label == "content_group_type")
                {
                    check(std::holds_alternative<name>(content.value), "fatal error: content_group_type must be a name");
                    if (std::get<name>(content.value) == content_group_type)
                    {
                        typed_content_groups.push_back (content_group);
                        found = true;
                        break;
                    }
                }
            }
        }
        if (strict && !found) 
        {
            check (false, "content_group_type with provided value is required in at least one content_group: " + content_group_type.to_string());
        }        
        return typed_content_groups;
    }

    document_graph::flexvalue document_graph::get_content(const content_group &content_group,
                                                          const string &content_label,
                                                          const bool &strict)
    {
        for (const document_graph::content &content : content_group)
        {
            if (content.label == content_label)
            {
                return content.value;
            }
        }
        check (!strict, "content_label required: " + content_label);
        return DOES_NOT_EXIST;
    }

    document_graph::flexvalue document_graph::get_content(const document &document,
                                                          const string &content_group_label,
                                                          const string &content_label,
                                                          const bool &strict)
    {
        return get_content ( get_content_group (document, content_group_label, strict), 
                             content_label,
                             strict);
    }

    document_graph::content document_graph::get_content_item(const content_group &content_group,
                                                                const string &content_label,
                                                                const bool &strict)
    {
        for (const document_graph::content &content : content_group)
        {
            if (content.label == content_label)
            {
                return content;
            }
        }
        check (!strict, "content_label required: " + content_label);
        return content{};
    }

} // namespace hyphaspace