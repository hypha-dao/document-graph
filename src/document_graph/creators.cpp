#include <document_graph.hpp>

namespace hyphaspace {

    checksum256 document_graph::hash_document (const vector<content_group> &content_groups)
    {
        // fingerprint the content object
        string string_data = to_string(content_groups);

        // stamp document with string_data used for hash
        // -- helpful when debugging
        // content c;
        // c.label = "fingerprint";
        // c.value = string_data;
        // content_group cg;
        // cg.push_back (c);
        // d.contents.push_back(cg);

        return eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());
    }

    document_graph::document document_graph::create_document(const name &creator, const vector<content_group> &content_groups)
    {
        require_auth(creator);

        // TODO: error out if content is empty
        // TODO: ensure that the creator is authorized/member
        document document;
        document_table d_t(contract, contract.value);

        checksum256 content_hash = hash_document(content_groups);
        auto hash_index = d_t.get_index<name("idhash")>();
        auto h_itr = hash_index.find(content_hash);

        // if this content exists already, error out and send back the hash of the existing document
        check(h_itr == hash_index.end(), "document exists already: " + readable_hash(content_hash));

        d_t.emplace(contract, [&](auto &d) {
            d.id = d_t.available_primary_key();
            d.creator = creator;
            d.content_groups = content_groups;            

            // write a 'free' created receipt to the blockchain history logs
            action(
                permission_level{contract, name("active")},
                contract, name("created"),
                std::make_tuple(d.creator, content_hash))
                // std::make_tuple(d.hash, d.id, d.creator, d.content))  // TODO: troubleshoot "Error: inline action too big"
            .send();

            d.hash = content_hash;
            document = d;
        });
        return document;
    }

    document_graph::document document_graph::create_document(const name &creator, const content_group &content_group) 
    {
        vector<document_graph::content_group> content_groups;
        content_groups.push_back(content_group);
        return create_document (creator, content_groups);
    }

    document_graph::document document_graph::create_document(const name &creator, const content &content) 
    {
        document_graph::content_group cg;
        cg.push_back(content);
        return create_document (creator, cg);
    }

    document_graph::document document_graph::create_document(const name &creator, const string &content_label, const flexvalue &content_value) 
    {
        return create_document (creator, new_content(content_label, content_value));
    }

    document_graph::document document_graph::get_or_create(const name &creator, const vector<content_group> &content_groups)
    {
        require_auth(creator);

        // TODO: error out if content is empty
        // TODO: ensure that the creator is authorized/member
        document document;
        document_table d_t(contract, contract.value);

        checksum256 content_hash = hash_document(content_groups);

        auto hash_index = d_t.get_index<name("idhash")>();
        auto h_itr = hash_index.find(content_hash);

        // if this content exists already, return the document
        if (h_itr != hash_index.end() && h_itr->hash == content_hash) {
            return *h_itr;
        }

        d_t.emplace(contract, [&](auto &d) {
            d.id = d_t.available_primary_key();
            d.creator = creator;
            d.content_groups = content_groups;

            // write a 'free' created receipt to the blockchain history logs
            action(
                permission_level{contract, name("active")},
                contract, name("created"),
                std::make_tuple(d.creator, content_hash))
                // std::make_tuple(d.hash, d.id, d.creator, d.content))  // TODO: troubleshoot "Error: inline action too big"
            .send();

            d.hash = content_hash;
            document = d;
        });
        return document;
    }

    document_graph::document document_graph::get_or_create(const name &creator, const content_group &content_group)
    {
        vector<document_graph::content_group> content_groups;
        content_groups.push_back(content_group);
        return get_or_create (creator, content_groups);
    }

    document_graph::document document_graph::get_or_create(const name &creator, const content &content) 
    {
        document_graph::content_group cg;
        cg.push_back(content);
        return get_or_create (creator, cg);
    }

    document_graph::document document_graph::get_or_create(const name &creator, const string &content_label, const flexvalue &content_value) 
    {
        return get_or_create (creator, new_content(content_label, content_value));
    }

    document_graph::document document_graph::fork_document(const checksum256 &hash, const name &creator, const content &content)
    {
        content_group cg = content_group {};
        cg.push_back (content);
        vector<content_group> content_groups = vector<content_group> {};
        content_groups.push_back (cg);
        return fork_document(hash, creator, content_groups);
    }

    document_graph::document document_graph::fork_document(const checksum256 &hash, const name &creator, const vector<content_group> &content_groups)
    {
        require_auth(creator);

        document document;
        document_table d_t(contract, contract.value);
        auto hash_index = d_t.get_index<name("idhash")>();
        auto h_itr = hash_index.find(hash);
        check(h_itr != hash_index.end(), "document not found: " + document_graph::to_string(hash));

        // TODO: error out if content is empty
        // TODO: error out if content does not provide new/updated data to the forked object
        d_t.emplace(contract, [&](auto &d) {
            d.id = d_t.available_primary_key();
            d.creator = contract;

            d.content_groups = content_groups;

            // TODO: where should the parent hash go?
            content_group cg {};
            cg.push_back(document_graph::new_content("content_group_label", "system"));
            cg.push_back(document_graph::new_content("parent", hash));
            d.content_groups.push_back(cg);
            d.hash = hash_document(content_groups);

            document = d;
            // write a 'free' created receipt to the blockchain history logs
            action(
                permission_level{contract, name("active")},
                contract, name("created"),
                std::make_tuple(d.creator, d.hash))
                .send();
        });
        return document;
    }
}