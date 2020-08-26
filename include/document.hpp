#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

using namespace eosio;
using namespace std;

namespace hyphaspace
{
   CONTRACT document : public contract
   {
   public:
      document(name self, name code, datastream<const char *> ds);
      ~document();

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

      // Any account/member can creator a new document
      ACTION create(const name &creator, const vector<content_group> &content_groups);

      // Fork creates a new document (node in a graph) from an existing document.
      // The forked content should contain only new or updated entries to avoid data duplication. (lazily enforced?)
      // ACTION fork(const checksum256 &hash, const name &creator, const contents &contents);

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
      struct certificate
      {
         name certifier;
         string notes;
         time_point certification_date = current_time_point();

         EOSLIB_SERIALIZE(certificate, (certifier)(notes)(certification_date))
      };

      struct [[eosio::table]] doc
      {
         uint64_t id;
         checksum256 hash;
         name creator;
         vector<content_group> content_groups;

         vector<certificate> certificates;
         uint64_t primary_key() const { return id; }
         uint64_t by_creator() const { return creator.value; }
         checksum256 by_hash() const { return hash; }

         time_point created_date = current_time_point();
         uint64_t by_created() const { return created_date.sec_since_epoch(); }

         EOSLIB_SERIALIZE(doc, (id)(hash)(creator)(content_groups)(certificates)(created_date))
      };

      typedef multi_index<name("documents"), doc,
                          indexed_by<name("idhash"), const_mem_fun<doc, checksum256, &doc::by_hash>>,
                          indexed_by<name("bycreator"), const_mem_fun<doc, uint64_t, &doc::by_creator>>,
                          indexed_by<name("bycreated"), const_mem_fun<doc, uint64_t, &doc::by_created>>> 
          doc_table;

      struct [[eosio::table]] Debug
      {
         uint64_t debug_id;
         string notes;
         time_point created_date = current_time_point();
         uint64_t primary_key() const { return debug_id; }
      };

      typedef multi_index<name("debugs"), Debug> debug_table;

      // creates a new certificate, require auth of certifier
      document::certificate new_certificate(const name &certifier, const string &notes);

      // series of functions used to fingerprint each content object
      // TODO: is there a more performant way to achieve the same result?
      // TODO: put this into an interface or cleaner abstraction
      std::string to_string(const vector<content_group> &content_groups);
      std::string to_string(const content_group &content_group);
      std::string to_string(const content &content);
      std::string to_string(const document::flexvalue &value);

      void debug(const string &notes)
      {
         debug_table d_t(get_self(), get_self().value);
         d_t.emplace(get_self(), [&](auto &d) {
            d.debug_id = d_t.available_primary_key();
            d.notes = notes;
         });
      }

      // this is required to send the hash back to the user in error messages as human readable
      std::string to_hex(const char *d, uint32_t s)
      {
         std::string r;
         const char *to_hex = "0123456789abcdef";
         auto c = reinterpret_cast<const uint8_t *>(d);
         for (auto i = 0; i < s; ++i)
            (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
         return r;
      }
   };
} // namespace hyphaspace