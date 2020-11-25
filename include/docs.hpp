#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

#include <document_graph/content_group.hpp>
#include <document_graph/document_graph.hpp>

using namespace eosio;

namespace hypha
{
   CONTRACT docs : public contract
   {
   public:
      docs(name self, name code, datastream<const char *> ds);
      ~docs();

      // DECLARE_DOCUMENT(docs)
      //typedef std::variant<name, string, asset, time_point, int64_t, checksum256> flexvalue;
      // Any account/member can creator a new document
      ACTION create(eosio::name &creator, std::vector<ContentGroup> &content_groups);
      // ACTION getorcreate(const name &creator, const vector<document_graph::content_group> &content_groups);

      ACTION newedge(eosio::name &creator, const checksum256 &from_node, const checksum256 &to_node, const name &edge_name);

      ACTION removeedge(const checksum256 &from_node, const checksum256 &to_node, const name &edge_name, const bool strict);
      // ACTION remedgesft(const checksum256 &from_node, const checksum256 &to_node, const bool strict);
      // ACTION remedgesfn(const checksum256 &from_node, const name &edge_name, const bool strict);
      // // ACTION remedgestn(const checksum256 &to_node, const name &edge_name, const bool strict);

      // ACTION testedgeidx (const checksum256 &from_node, const name &edge_name, const bool strict);

      // // Fork creates a new document (node in a graph) from an existing document.
      // // The forked content should contain only new or updated entries to avoid data duplication. (lazily enforced?)
      // ACTION fork(const checksum256 &hash, const name &creator, const vector<document_graph::content_group> &content_groups);

      // Creates a 'certificate' on a specific fork.
      // A certificate can be customized based on the document, but it represents
      // the signatures, with notes/timestamp, and of course auth is enforced
      // ACTION certify(const name &certifier, const checksum256 &hash, const std::string &notes);

      // // debug only: deletes all docs
      // ACTION reset();

   private:
      DocumentGraph m_dg = DocumentGraph (get_self());
   };
} // namespace hypha