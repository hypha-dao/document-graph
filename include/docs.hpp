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

      DECLARE_DOCUMENT_GRAPH(docs)

      // Any account/member can creator a new document
      ACTION create(eosio::name & creator, ContentGroups & content_groups);
      ACTION createroot (const std::string &notes);

      ACTION getornewget(const name &creator, const ContentGroups &content_groups);
      ACTION getornewnew(const name &creator, const ContentGroups &content_groups);

      ACTION newedge(eosio::name & creator, const checksum256 &from_node, const checksum256 &to_node, const name &edge_name);

      ACTION removeedge(const checksum256 &from_node, const checksum256 &to_node, const name &edge_name);

      ACTION erase(const checksum256 &hash);

      ACTION testgetasset(const checksum256 &hash,
                          const string &groupLabel,
                          const string &contentLabel,
                          const asset &contentValue);
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
      DocumentGraph m_dg = DocumentGraph(get_self());
   };
} // namespace hypha