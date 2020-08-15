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

      typedef std::string key;
      typedef std::variant<name, string, asset, time_point, int64_t, checksum256> flexvalue;
      typedef vector<flexvalue> value;
      typedef map<key, value> content;

      ACTION create(const name &owner, const content &content);
      ACTION created(const checksum256 &hash, const uint64_t &id, const name &owner, const content &content);

      // ACTION create(const name &owner, const map<string, vector<flexvalue>> &values);
      // ACTION created(const checksum256 &hash, const uint64_t &id, const name &owner, const map<string, vector<flexvalue>> &values);

      ACTION edit (const checksum256 &hash, const name &owner, const content &content );
      ACTION certify (const checksum256 &hash);

   private:

      struct [[eosio::table]] doc
      {
         checksum256 hash;
         uint64_t id;
         name owner;
         // map<string, vector<flexvalue>> values;
         content content;

         uint64_t primary_key() const { return id; }
         uint64_t by_owner() const { return owner.value; }
         checksum256 by_hash() const { return hash; }

         // timestamps
         time_point created_date = current_time_point();
         time_point updated_date = current_time_point();
         uint64_t by_created() const { return created_date.sec_since_epoch(); }
         uint64_t by_updated() const { return updated_date.sec_since_epoch(); }
      };

      typedef multi_index<name("documents"), doc,
                          indexed_by<name("idhash"), const_mem_fun<doc, checksum256, &doc::by_hash>>,
                          indexed_by<name("byowner"), const_mem_fun<doc, uint64_t, &doc::by_owner>>,    
                          indexed_by<name("bycreated"), const_mem_fun<doc, uint64_t, &doc::by_created>>, 
                          indexed_by<name("byupdated"), const_mem_fun<doc, uint64_t, &doc::by_updated>>> 
          doc_table;

      std::string to_string(document::flexvalue value)
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
            string str_value = string( (const char*)arr.data(), arr.size()) ;
            return "[checksum256," + str_value + "]";
         }
         else 
         {
            return "[name," + std::get<name>(value).to_string() + "]";
         }
      }

      std::string to_string(vector<flexvalue> values) 
      {
         string results = "[";
         for (std::vector<flexvalue>::iterator it = values.begin() ; it != values.end(); ++it)
         {            
            results = results + to_string(*it);
            if (it != values.end()) {
               results = results + ",";
            }
         }
         results =+ "]";
         return results;
      }

      std::string to_string(const content &content)
      {
         string results = "[";
         std::map<string, vector<flexvalue>>::const_iterator value_itr;
         for (value_itr = content.begin(); value_itr != content.end(); ++value_itr)
         {
            results = results + "[" + value_itr->first + "=" + to_string(value_itr->second);
           
            if (value_itr != content.end())
            {
               results = results + ",";
            }
         }
         results =+ "]";
         return results;
      }
   };
} // namespace hyphaspace