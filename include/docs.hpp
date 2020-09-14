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

      struct [[eosio::table]] document
      {
         uint64_t id;
         checksum256 hash;
         name creator;
         vector<document_graph::content_group> content_groups;

         vector<document_graph::certificate> certificates;
         uint64_t primary_key() const { return id; }
         uint64_t by_creator() const { return creator.value; }
         checksum256 by_hash() const { return hash; }

         time_point created_date = current_time_point();
         uint64_t by_created() const { return created_date.sec_since_epoch(); }

         EOSLIB_SERIALIZE(document, (id)(hash)(creator)(content_groups)(certificates)(created_date))
      };

      typedef multi_index<name("documents"), document,
                          indexed_by<name("idhash"), const_mem_fun<document, checksum256, &document::by_hash>>,
                          indexed_by<name("bycreator"), const_mem_fun<document, uint64_t, &document::by_creator>>,
                          indexed_by<name("bycreated"), const_mem_fun<document, uint64_t, &document::by_created>>>
          document_table;

      // Any account/member can creator a new document
      ACTION create(const name &creator, const vector<document_graph::content_group> &content_groups);

      // Transform a legacy object format to the new document format
      ACTION transform(const name &scope, const uint64_t &id);

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

      struct [[eosio::table]] Debug
      {
         uint64_t debug_id;
         string notes;
         time_point created_date = current_time_point();
         uint64_t primary_key() const { return debug_id; }
      };

      typedef multi_index<name("debugs"), Debug> debug_table;

      void debug(const string &notes)
      {
         debug_table d_t(get_self(), get_self().value);
         d_t.emplace(get_self(), [&](auto &d) {
            d.debug_id = d_t.available_primary_key();
            d.notes = notes;
         });
      }

      // scope: proposal, proparchive, role, assignment
      struct [[eosio::table, eosio::contract("docs")]] Object
      {
         uint64_t id;

         // core maps
         map<string, name> names;
         map<string, string> strings;
         map<string, asset> assets;
         map<string, time_point> time_points;
         map<string, uint64_t> ints;
         map<string, transaction> trxs;
         map<string, float> floats;
         uint64_t primary_key() const { return id; }

         // indexes
         uint64_t by_owner() const { return names.at("owner").value; }
         uint64_t by_type() const { return names.at("type").value; }
         uint64_t by_fk() const { return ints.at("fk"); }

         // timestamps
         time_point created_date = current_time_point();
         time_point updated_date = current_time_point();
         uint64_t by_created() const { return created_date.sec_since_epoch(); }
         uint64_t by_updated() const { return updated_date.sec_since_epoch(); }
      };

      typedef multi_index<name("objects"), Object,
                          indexed_by<name("bycreated"), const_mem_fun<Object, uint64_t, &Object::by_created>>, // index 2
                          indexed_by<name("byupdated"), const_mem_fun<Object, uint64_t, &Object::by_updated>>, // 3
                          indexed_by<name("byowner"), const_mem_fun<Object, uint64_t, &Object::by_owner>>,     // 4
                          indexed_by<name("bytype"), const_mem_fun<Object, uint64_t, &Object::by_type>>,       // 5
                          indexed_by<name("byfk"), const_mem_fun<Object, uint64_t, &Object::by_fk>>            // 6
                          >
          object_table;

      // flexvalue can any of these commonly used eosio data types
      // a checksum256 can be a link to another document, akin to an "edge" on a graph
      typedef std::variant<name, string, asset, time_point, int64_t, checksum256> flexvalue;

      // a single labeled flexvalue
      struct content
      {
         string label;
         flexvalue value;

         EOSLIB_SERIALIZE(content, (label)(value))
      };

      typedef vector<content> content_group;

      struct certificate
      {
         name certifier;
         string notes;
         time_point certification_date = current_time_point();

         EOSLIB_SERIALIZE(certificate, (certifier)(notes)(certification_date))
      };
   };
} // namespace hyphaspace