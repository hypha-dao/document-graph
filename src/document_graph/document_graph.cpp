
#include <document_graph/document_graph.hpp>
#include <document_graph/document.hpp>
// #include <document_graph/content.hpp>

namespace hypha
{
    // Document DocumentGraph::create_document(eosio::name &creator, std::vector<ContentGroup> &content_groups) {

    //     require_auth(creator);

    //     Document* document = new Document (content_groups);
    //     document->setCreator(creator);

    //     // checksum256 content_hash = hash_document(content_groups);
    //     // auto hash_index = d_t.get_index<name("idhash")>();
    //     // auto h_itr = hash_index.find(content_hash);

    //     // if this content exists already, error out and send back the hash of the existing document
    //     // check(h_itr == hash_index.end(), "document exists already: " + readable_hash(content_hash));

    //     DocumentGraph::document_table d_t(m_contract, m_contract.value);
    //     d_t.emplace(m_contract, [&](auto &d) {
    //         d.id = d_t.available_primary_key();
    //         d.creator = document->getCreator();
    //         // d.content_groups = document->getContentGroups();
    //         d.created_date =   current_time_point();
    //         d.hash = document->getHash();;
    //     });

    //     return *document;
    // }

    Document DocumentGraph::create_document(eosio::name &creator)
    {

        require_auth(creator);

        // Document* document = new Document (content);
        // document->setCreator(creator);

        // checksum256 content_hash = hash_document(content_groups);
        // auto hash_index = d_t.get_index<name("idhash")>();
        // auto h_itr = hash_index.find(content_hash);

        // if this content exists already, error out and send back the hash of the existing document
        // check(h_itr == hash_index.end(), "document exists already: " + readable_hash(content_hash));

        Content c{};
        c.label = "my label from content1";
        c.value = "my string value from content1";

        std::vector<Content> cg{};
        cg.push_back(c);

        std::vector<ContentGroup> cgs;
        cgs.push_back(cg);

        DocumentGraph::document_table d_t(m_contract, m_contract.value);
        Document document(d_t.available_primary_key(), creator, cgs);

        d_t.emplace(m_contract, [&](auto &d) {
            d = std::move(document);
            d.created_date = current_time_point();
        });

        return document;
    }

    // document_graph::content document_graph::new_content(const string &label, const flexvalue &fv)
    // {
    //     content c{};
    //     c.label = label;
    //     c.value = fv;
    //     return c;
    // }

    // // unsure how erasing documents should be handled, re: approvals
    // // for now, permissions should be handled in the contract action rather than this class
    // void document_graph::erase_document(const checksum256 &document_hash)
    // {
    //     document_table d_t(contract, contract.value);
    //     auto hash_index = d_t.get_index<name("idhash")>();
    //     auto h_itr = hash_index.find(document_hash);

    //     auto byte_arr = document_hash.extract_as_byte_array();
    //     string readable_hash = document_graph::to_hex((const char *)byte_arr.data(), byte_arr.size());
    //     check (h_itr != hash_index.end(), "Cannot erase document; does not exist: " + readable_hash);

    //     remove_edges (document_hash, false);
    //     hash_index.erase (h_itr);
    // }

    // void document_graph::certify_document(const name &certifier, const checksum256 &hash, const string &notes)
    // {
    //     document_table d_t(contract, contract.value);
    //     auto hash_index = d_t.get_index<name("idhash")>();
    //     auto h_itr = hash_index.find(hash);
    //     check(h_itr != hash_index.end(), "document not found: " + document_graph::to_string(hash));

    //     // this is enforced in the new certificate function too, so might be duplicative
    //     require_auth(certifier);

    //     // TODO: should a certifier be able to sign the same document fork multiple times?
    //     hash_index.modify(h_itr, contract, [&](auto &d) {
    //         d.certificates.push_back(new_certificate(certifier, notes));
    //     });
    // }

    // document_graph::certificate document_graph::new_certificate(const name &certifier, const string &notes)
    // {
    //     require_auth(certifier);
    //     certificate cert = certificate{};
    //     cert.certifier = certifier;
    //     cert.notes = notes;
    //     return cert;
    // }

    // series of functions used to fingerprint each content object
    // TODO: is there a more performant way to achieve the same result?
    // TODO: put this into an interface or cleaner abstraction

    // std::string document_graph::to_string(const document_graph::content &content)
    // {
    //     return  "{" + string(content.label) + "=" + to_string(content.value) + "}";
    // }

    // std::string document_graph::to_string(const document_graph::content_group &content_group)
    // {
    //     string results = "[";
    //     bool is_first = true;

    //     for (const document_graph::content &content : content_group)
    //     {
    //         if (is_first)
    //         {
    //             is_first = false;
    //         }
    //         else
    //         {
    //             results = results + ",";
    //         }
    //         results = results + to_string(content);
    //     }

    //     results = results + "]";
    //     return results;
    // }

    // std::string document_graph::to_string(const vector<document_graph::content_group> &content_groups)
    // {
    //     string results = "[";
    //     bool is_first = true;

    //     for (const document_graph::content_group &content_group : content_groups)
    //     {
    //         if (is_first)
    //         {
    //             is_first = false;
    //         }
    //         else
    //         {
    //             results = results + ",";
    //         }
    //         results = results + to_string(content_group);
    //     }

    //     results = results + "]";
    //     return results;
    // }

    // std::string document_graph::to_hex(const char *d, uint32_t s)
    // {
    //     std::string r;
    //     const char *to_hex = "0123456789abcdef";
    //     auto c = reinterpret_cast<const uint8_t *>(d);
    //     for (auto i = 0; i < s; ++i)
    //         (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
    //     return r;
    // }

    // std::string document_graph::readable_hash (const checksum256 &proposal_hash)
    // {
    //     auto byte_arr = proposal_hash.extract_as_byte_array();
    //     return document_graph::to_hex((const char *)byte_arr.data(), byte_arr.size());
    // }

    // converts a string to a uint64 type
    // uint64_t document_graph::to_uint64 (const string &fingerprint)
    // {
    //     uint64_t id = 0;
    //     checksum256 h = sha256(const_cast<char*>(fingerprint.c_str()), fingerprint.size());
    //     auto hbytes = h.extract_as_byte_array();
    //     for(int i=0; i<4; i++) {
    //         id <<=8;
    //         id |= hbytes[i];
    //     }
    //     return id;
    // }

    // uint64_t document_graph::edge_id(checksum256 from_node, checksum256 to_node, name edge_name)
    // {
    //     return to_uint64(readable_hash(from_node) + readable_hash(to_node) + edge_name.to_string());
    // }

    // uint64_t document_graph::hash(checksum256 from_node, checksum256 to_node)
    // {
    //     return to_uint64(readable_hash(from_node) + readable_hash(to_node));
    // }

    // uint64_t document_graph::hash(checksum256 node, name edge_name)
    // {
    //     return to_uint64(readable_hash(node) + edge_name.to_string());
    // }

} // namespace hypha