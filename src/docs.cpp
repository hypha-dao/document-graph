#include <docs.hpp>

namespace hypha
{

   docs::docs(name self, name code, datastream<const char *> ds) : contract(self, code, ds) {}
   docs::~docs() {}

   void docs::create(name &creator, ContentGroups &content_groups)
   {
      Document document(get_self(), creator, content_groups);
      document.emplace();
   }

   void docs::getornewget(const name &creator, const ContentGroups &content_groups)
   {
      Document document = Document::getOrNew(get_self(), creator, content_groups);
      eosio::check(document.getCreated().sec_since_epoch() > 0, "created new instead of reading from existing");
   }

   void docs::getornewnew(const name &creator, const ContentGroups &content_groups)
   {
      Document document = Document::getOrNew(get_self(), creator, content_groups);
      eosio::check(document.getCreated().sec_since_epoch() == 0, "read from existing instead of creating new");
      document.emplace();
      eosio::check(document.getCreated().sec_since_epoch() > 0, "created_date not populated when saved");
   }

   void docs::newedge(name &creator, const checksum256 &from_node, const checksum256 &to_node, const name &edge_name)
   {
      Edge edge(get_self(), creator, from_node, to_node, edge_name);
      edge.emplace();
   }

   void docs::removeedge(const checksum256 &from_node, const checksum256 &to_node, const name &edge_name)
   {
      Edge edge = Edge::get(get_self(), from_node, to_node, edge_name);
      edge.erase();
   }

   void docs::erase(const checksum256 &hash)
   {
      DocumentGraph dg(get_self());
      dg.eraseDocument(hash);
   }

   void docs::testgetasset(const checksum256 &hash,
                           const string &groupLabel,
                           const string &contentLabel,
                           const asset &contentValue)
   {
      Document document(get_self(), hash);
      asset readValue = ContentWrapper::getContent(document.getContentGroups(), groupLabel, contentLabel).getAs<eosio::asset>();
      eosio::check(readValue == contentValue, "read value does not equal content value. read value: " +
                                                  readValue.to_string() + " expected value: " + contentValue.to_string());
   }

   void docs::createroot(const std::string &notes)
   {
      require_auth(get_self());

      Document rootDoc(get_self(), get_self(), Content("root_node", get_self()));
      rootDoc.emplace();

      // //Create the settings document as well and add an edge to it
      // ContentGroups settingCgs{{Content(CONTENT_GROUP_LABEL, common::SETTINGS),
      //                           Content(common::ROOT_NODE, readableHash(rootDoc.getHash()))}};

      // Document settingsDoc(get_self(), get_self(), std::move(settingCgs));
      // settingsDoc.emplace();

      // Edge rootToSettings(get_self(), get_self(), rootDoc.getHash(), settingsDoc.getHash(), common::SETTINGS_EDGE);
      // rootToSettings.emplace();
   }

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
} // namespace hypha