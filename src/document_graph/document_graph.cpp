
#include <document_graph/util.hpp>
#include <document_graph/document_graph.hpp>
#include <document_graph/document.hpp>
// #include <document_graph/content.hpp>

namespace hypha
{
    // Document DocumentGraph::createDocument(eosio::name &creator, std::vector<ContentGroup> &contentGroups) {

        
    //     return document;
    // }

    // Document DocumentGraph::createDocument(eosio::name &creator, ContentGroup &contentGroup) 
    // {
    //     std::vector<ContentGroup> contentGroups;
    //     contentGroups.push_back(contentGroup);
    //     return createDocument (creator, contentGroups);
    // }

    // Document DocumentGraph::createDocument(eosio::name &creator, Content &content) 
    // {
    //     ContentGroup contentGroup;
    //     contentGroup.push_back (content);
    //     return createDocument (creator, contentGroup);
    // }

    // Document DocumentGraph::createDocument(eosio::name &creator, const std::string &label, const Content::FlexValue &value)
    // {
    //     Content contentItem (label, value);
    //     return createDocument(creator, contentItem);
    // }

    // Document DocumentGraph::getOrCreate(const eosio::name &creator, const std::vector<ContentGroup> &contentGroups)
    // {
    //     require_auth(creator);

    //     // TODO: error out if content is empty
    //     // TODO: ensure that the creator is authorized/member
    //     Document::document_table d_t(m_contract, m_contract.value);
    //     Document document(d_t.available_primary_key(), creator, contentGroups);

    //     auto hash_index = d_t.get_index<eosio::name("idhash")>();
    //     auto h_itr = hash_index.find(document.getHash());

    //     // if this content exists already, return the document
    //     if (h_itr != hash_index.end() && h_itr->hash == document.getHash()) {
    //         return *h_itr;
    //     }

    //     d_t.emplace(m_contract, [&](auto &d) {
    //         d = std::move(document);
    //         d.created_date = eosio::current_time_point();
    //     });

    //     return document;
    // }

    // Document DocumentGraph::getOrCreate(const eosio::name &creator, const ContentGroup &contentGroup)
    // {
    //      std::vector<ContentGroup> contentGroups;
    //     contentGroups.push_back(contentGroup);
    //     return getOrCreate (creator, contentGroups);
    // }

    // Document DocumentGraph::getOrCreate(const eosio::name &creator, const Content &content) 
    // {
    //     ContentGroup contentGroup;
    //     contentGroup.push_back (content);
    //     return getOrCreate (creator, contentGroup);
    // }

    // Document DocumentGraph::getOrCreate(const eosio::name &creator, const std::string &label, const Content::FlexValue &value)
    // {
    //     Content contentItem (label, value);
    //     return getOrCreate(creator, contentItem);
    // }

    Edge DocumentGraph::createEdge (eosio::name &creator, const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode, const eosio::name &edgeName)
    {
        require_auth(creator);

        DocumentGraph::edge_table e_t(m_contract, m_contract.value);
        Edge edge(creator, fromNode, toNode, edgeName);

        e_t.emplace(m_contract, [&](auto &e) {
            e = std::move(edge);
            e.created_date = eosio::current_time_point();
        });

        return edge;
    }

    std::vector<Edge> DocumentGraph::getEdges (const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode)
    {
        std::vector<Edge> edges;

        // this index uniquely identifies all edges that share this fromNode and toNode
        uint64_t index = concatHash(fromNode, toNode);
        edge_table e_t (m_contract, m_contract.value);
        auto from_name_index = e_t.get_index<eosio::name("byfromto")>();
        auto itr = from_name_index.find (index);

        while (itr != from_name_index.end() && itr->from_node_to_node_index == index) {
            edges.push_back (*itr);
            itr++;
        } 

        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesOrFail (const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode)
    {
        std::vector<Edge> edges = getEdges (fromNode, toNode);
        eosio::check (edges.size() > 0, "no edges exist: from " + readableHash(fromNode) + " to " + readableHash(toNode));
        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesFrom (const eosio::checksum256 &fromNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges;

        // this index uniquely identifies all edges that share this fromNode and edgeName
        uint64_t index = concatHash(fromNode, edgeName);
        edge_table e_t (m_contract, m_contract.value);
        auto from_name_index = e_t.get_index<eosio::name("byfromname")>();
        auto itr = from_name_index.find (index);

        while (itr != from_name_index.end() && itr->from_node_edge_name_index == index) {
            edges.push_back (*itr);
            itr++;
        } 

        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesFromOrFail (const eosio::checksum256 &fromNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges = getEdgesFrom (fromNode, edgeName);
        eosio::check (edges.size() > 0, "no edges exist: from " + readableHash(fromNode) + " with name " + edgeName.to_string());
        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesTo (const eosio::checksum256 &toNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges;

        // this index uniquely identifies all edges that share this toNode and edgeName
        uint64_t index = concatHash(toNode, edgeName);
        edge_table e_t (m_contract, m_contract.value);
        auto from_name_index = e_t.get_index<eosio::name("bytoname")>();
        auto itr = from_name_index.find (index);

        while (itr != from_name_index.end() && itr->to_node_edge_name_index == index) {
            edges.push_back (*itr);
            itr++;
        } 

        return edges;
    }

    std::vector<Edge> DocumentGraph::getEdgesToOrFail (const eosio::checksum256 &toNode, const eosio::name &edgeName)
    {
        std::vector<Edge> edges = getEdgesTo (toNode, edgeName);
        eosio::check (edges.size() > 0, "no edges exist: to " + readableHash(toNode) + " with name " + edgeName.to_string());
        return edges;
    }

    void DocumentGraph::removeEdge (const eosio::checksum256 &fromNode, const eosio::checksum256 &toNode, const eosio::name &edgeName)
    {       
        edge_table e_t (m_contract, m_contract.value);
        auto itr = e_t.find (concatHash (fromNode, toNode, edgeName));

        eosio::check (itr == e_t.end(), "edge does not exist: from " + readableHash(fromNode) 
                + " to " + readableHash(toNode) + " with edge name of " + edgeName.to_string());
        e_t.erase (itr);
    }

    void DocumentGraph::removeEdges (const eosio::checksum256 &node)
    {
        edge_table e_t (m_contract, m_contract.value);
        
        auto from_node_index = e_t.get_index<eosio::name("fromnode")>();
        auto from_itr = from_node_index.find (node);

        while (from_itr != from_node_index.end() && from_itr->from_node == node) {
            from_itr = from_node_index.erase (from_itr);
        } 

        auto to_node_index = e_t.get_index<eosio::name("tonode")>();
        auto to_itr = to_node_index.find (node);

        while (to_itr != to_node_index.end() && to_itr->to_node == node) {
            to_itr = to_node_index.erase (to_itr);
        } 
    }

    // unsure how erasing documents should be handled, re: approvals
    // for now, permissions should be handled in the contract action rather than this class
    void DocumentGraph::eraseDocument(const eosio::checksum256 &documentHash)
    {
        Document::document_table d_t(m_contract, m_contract.value);
        auto hash_index = d_t.get_index<eosio::name("idhash")>();
        auto h_itr = hash_index.find(documentHash);

        eosio::check (h_itr != hash_index.end(), "Cannot erase document; does not exist: " + readableHash(documentHash));

        removeEdges (documentHash);
        hash_index.erase (h_itr);
    }

    
    // document_graph::certificate document_graph::new_certificate(const name &certifier, const string &notes)
    // {
    //     require_auth(certifier);
    //     certificate cert = certificate{};
    //     cert.certifier = certifier;
    //     cert.notes = notes;
    //     return cert;
    // }

} // namespace hypha