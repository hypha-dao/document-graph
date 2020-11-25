#include <docs.hpp>

namespace hypha {

   docs::docs(name self, name code, datastream<const char *> ds) : contract(self, code, ds) {}
   docs::~docs() {}

   void docs::create(name &creator, std::vector<ContentGroup> &content_groups)
   {
      Document document (get_self(), creator, content_groups);
      document.emplace ();
   }

// void docs::getorcreate(const name &creator, const vector<document_graph::content_group> &content_groups)
// {
//    _document_graph.get_or_create(creator, content_groups);
// }

   void docs::newedge (name &creator, const checksum256 &from_node, const checksum256 &to_node, const name &edge_name)
   {
      m_dg.createEdge(creator, from_node, to_node, edge_name);
   }

   void docs::removeedge (const checksum256 &from_node, const checksum256 &to_node, const name &edge_name, const bool strict)
   {
      m_dg.removeEdge(from_node, to_node, edge_name);
   }

// void docs::testedgeidx (const checksum256 &from_node, const name &edge_name, const bool strict) 
// {
//    vector<document_graph::edge> edges = _document_graph.get_edges_from_name(from_node, edge_name, strict);
//    for ( document_graph::edge e : edges) 
//    {
//       _document_graph.remove_edge(e.from_node, e.to_node, e.edge_name, true);
//    }
// }

// void docs::fork (const checksum256 &hash, const name &creator, const vector<document_graph::content_group> &content_groups )
// {
//    _document_graph.fork_document(hash, creator, content_groups);
// }

   // void docs::certify(const name &certifier, const checksum256 &hash, const std::string &notes)
   // {
   //    Document document (get_self(), hash);
   //    document.certify(certifier, notes);
   // }

// void docs::reset()
// {
//    require_auth(get_self());
//    document_table d_t(get_self(), get_self().value);
//    auto d_itr = d_t.begin();
//    while (d_itr != d_t.end())
//    {
//       d_itr = d_t.erase(d_itr);
//    }

//    edge_table e_t(get_self(), get_self().value);
//    auto e_itr = e_t.begin();
//    while (e_itr != e_t.end())
//    {
//       e_itr = e_t.erase(e_itr);
//    }
// }
}