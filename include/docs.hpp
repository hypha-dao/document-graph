#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

#include <document_graph.hpp>

using namespace eosio;
using namespace std;

namespace hyphaspace
{
   CONTRACT docs : public contract
   {
   public:
      docs(name self, name code, datastream<const char *> ds);
      ~docs();

      typedef std::variant<name, string, asset, time_point, int64_t, checksum256> flexvalue;

      typedef multi_index<name("documents"), document_graph::document,
                          indexed_by<name("idhash"), const_mem_fun<document_graph::document, checksum256, &document_graph::document::by_hash>>,
                          indexed_by<name("bycreator"), const_mem_fun<document_graph::document, uint64_t, &document_graph::document::by_creator>>,
                          indexed_by<name("bycreated"), const_mem_fun<document_graph::document, uint64_t, &document_graph::document::by_created>>>
          document_table;

      typedef multi_index<name("edges"), document_graph::edge,
                          indexed_by<name("fromnode"), const_mem_fun<document_graph::edge, checksum256, &document_graph::edge::by_from>>,
                          indexed_by<name("tonode"), const_mem_fun<document_graph::edge, checksum256, &document_graph::edge::by_to>>,
                          indexed_by<name("edgename"), const_mem_fun<document_graph::edge, uint64_t, &document_graph::edge::by_edge_name>>,
                          indexed_by<name("bycreated"), const_mem_fun<document_graph::edge, uint64_t, &document_graph::edge::by_created>>,
                          indexed_by<name("byfromname"), const_mem_fun<document_graph::edge, uint64_t, &document_graph::edge::by_from_node_edge_name_index>>,
                          indexed_by<name("byfromto"), const_mem_fun<document_graph::edge, uint64_t, &document_graph::edge::by_from_node_to_node_index>>,
                          indexed_by<name("bytoname"), const_mem_fun<document_graph::edge, uint64_t, &document_graph::edge::by_to_node_edge_name_index>>>
          edge_table;

      // Any account/member can creator a new document
      ACTION create(const name &creator, const vector<document_graph::content_group> &content_groups);
      ACTION getorcreate(const name &creator, const vector<document_graph::content_group> &content_groups);

      ACTION newedge(const checksum256 &from_node, const checksum256 &to_node, const name &edge_name);

      ACTION removeedge(const checksum256 &from_node, const checksum256 &to_node, const name &edge_name, const bool strict);
      ACTION remedgesft(const checksum256 &from_node, const checksum256 &to_node, const bool strict);
      ACTION remedgesfn(const checksum256 &from_node, const name &edge_name, const bool strict);
      // ACTION remedgestn(const checksum256 &to_node, const name &edge_name, const bool strict);

      ACTION testedgeidx (const checksum256 &from_node, const name &edge_name, const bool strict);

      // Fork creates a new document (node in a graph) from an existing document.
      // The forked content should contain only new or updated entries to avoid data duplication. (lazily enforced?)
      ACTION fork(const checksum256 &hash, const name &creator, const vector<document_graph::content_group> &content_groups);

      // Creates a 'certificate' on a specific fork.
      // A certificate can be customized based on the document, but it represents
      // the signatures, with notes/timestamp, and of course auth is enforced
      ACTION certify(const name &certifier, const checksum256 &hash, const string &notes);

      // This is the broadcast event that a new document was created.
      // It includes the hash that was calculated during the create action.
      // This could be the key for a QR code to pull up an entire document, as an example
      // ACTION created(const checksum256 &hash, const uint64_t &id, const name &creator, const content &content);
      ACTION created(const name &creator, const checksum256 &hash);

      // debug only: deletes all docs
      ACTION reset();

   private:
      document_graph _document_graph = document_graph(get_self());
   };
} // namespace hyphaspace