
#include <document_graph.hpp>

namespace hyphaspace
{
    document_graph::content document_graph::new_content(const string &label, const flexvalue &fv)
    {
        content c{};
        c.label = label;
        c.value = fv;
        return c;
    }

    // unsure how erasing documents should be handled, re: approvals 
    // for now, permissions should be handled in the contract action rather than this class
    // TODO: come back to this in the future
    void document_graph::erase_document(const checksum256 &document_hash) 
    {
        document_table d_t(contract, contract.value);
        auto hash_index = d_t.get_index<name("idhash")>();
        auto h_itr = hash_index.find(document_hash);

        auto byte_arr = document_hash.extract_as_byte_array();
        string readable_hash = document_graph::to_hex((const char *)byte_arr.data(), byte_arr.size());
        check (h_itr != hash_index.end(), "Cannot erase document; does not exist: " + readable_hash);
        
        hash_index.erase (h_itr);
    }

    document_graph::document document_graph::create_document(const name &creator, const vector<content_group> &content_groups)
    {
        require_auth(creator);

        // TODO: error out if content is empty
        // TODO: ensure that the creator is authorized/member
        document document;
        document_table d_t(contract, contract.value);
        d_t.emplace(contract, [&](auto &d) {
            d.id = d_t.available_primary_key();
            d.creator = creator;
            d.content_groups = content_groups;

            // fingerprint the content object
            string string_data = document_graph::to_string(content_groups);

            // stamp document with string_data used for hash
            // -- helpful when debugging
            // content c;
            // c.label = "fingerprint";
            // c.value = string_data;
            // content_group cg;
            // cg.push_back (c);
            // d.contents.push_back(cg);

            checksum256 content_hash = eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());

            auto hash_index = d_t.get_index<name("idhash")>();
            auto h_itr = hash_index.find(content_hash);

            auto byte_arr = content_hash.extract_as_byte_array();
            string readable_hash = document_graph::to_hex((const char *)byte_arr.data(), byte_arr.size());

            // if this content exists already, error out and send back the hash of the existing document
            if (h_itr != hash_index.end())
            {
                check(false, "document exists already: " + readable_hash);
            }

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

            string string_data = document_graph::to_string(content_groups);
            d.hash = eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());

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

    void document_graph::certify_document(const name &certifier, const checksum256 &hash, const string &notes)
    {
        document_table d_t(contract, contract.value);
        auto hash_index = d_t.get_index<name("idhash")>();
        auto h_itr = hash_index.find(hash);
        check(h_itr != hash_index.end(), "document not found: " + document_graph::to_string(hash));

        // this is enforced in the new certificate function too, so might be duplicative
        require_auth(certifier);

        // TODO: should a certifier be able to sign the same document fork multiple times?
        hash_index.modify(h_itr, contract, [&](auto &d) {
            d.certificates.push_back(new_certificate(certifier, notes));
        });
    }

    document_graph::certificate document_graph::new_certificate(const name &certifier, const string &notes)
    {
        require_auth(certifier);
        certificate cert = certificate{};
        cert.certifier = certifier;
        cert.notes = notes;
        return cert;
    }

    // series of functions used to fingerprint each content object
    // TODO: is there a more performant way to achieve the same result?
    // TODO: put this into an interface or cleaner abstraction
    std::string document_graph::to_string(const document_graph::flexvalue &value)
    {
        if (std::holds_alternative<int64_t>(value))
        {
            return "[int64," + std::to_string(std::get<int64_t>(value)) + "]";
        }
        else if (std::holds_alternative<asset>(value))
        {
            return "[asset," + std::get<asset>(value).to_string() + "]";
        }
        else if (std::holds_alternative<time_point>(value))
        {
            return "[time_point," + std::to_string(std::get<time_point>(value).sec_since_epoch()) + "]";
        }
        else if (std::holds_alternative<string>(value))
        {
            return "[string," + std::get<string>(value) + "]";
        }
        else if (std::holds_alternative<checksum256>(value))
        {
            checksum256 cs_value = std::get<checksum256>(value);
            auto arr = cs_value.extract_as_byte_array();
            string str_value = document_graph::to_hex((const char *)arr.data(), arr.size());
            return "[checksum256," + str_value + "]";
        }
        else
        {
            return "[name," + std::get<name>(value).to_string() + "]";
        }
    }

    std::string document_graph::to_string(const document_graph::content &content)
    {
        return "{" + string(content.label + "=" + to_string(content.value)) + "}";
    }

    std::string document_graph::to_string(const document_graph::content_group &content_group)
    {
        string results = "[";
        bool is_first = true;

        for (const document_graph::content &content : content_group)
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                results = results + ",";
            }
            results = results + to_string(content);
        }

        results = results + "]";
        return results;
    }

    std::string document_graph::to_string(const vector<document_graph::content_group> &content_groups)
    {
        string results = "[";
        bool is_first = true;

        for (const document_graph::content_group &content_group : content_groups)
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                results = results + ",";
            }
            results = results + to_string(content_group);
        }

        results = results + "]";
        return results;
    }

    std::string document_graph::to_hex(const char *d, uint32_t s)
    {
        std::string r;
        const char *to_hex = "0123456789abcdef";
        auto c = reinterpret_cast<const uint8_t *>(d);
        for (auto i = 0; i < s; ++i)
            (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
        return r;
    }
} // namespace hyphaspace