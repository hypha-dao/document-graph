#include <document.hpp>

using namespace hyphaspace;

document::document(name self, name code, datastream<const char *> ds) : contract(self, code, ds) {}
document::~document() {}

// void document::create(const name &owner, const map<string, vector<flexvalue>> &values)
void document::create(const name &owner, const content &content)
{
   doc_table d_t (get_self(), get_self().value);
   d_t.emplace(get_self(), [&](auto& d) {
      d.id              = d_t.available_primary_key();
      d.owner           = get_self();
      d.content          = content;
      
      string string_data = std::to_string(d.id) + d.owner.to_string() + document::to_string(content);
      //string string_data = "junk";
      d.hash = eosio::sha256(const_cast<char *>(string_data.c_str()), sizeof(string_data));

      // write a 'free' created receipt to the blockchain history logs
      action(
         permission_level{get_self(), name("active")},
         get_self(), name("created"),
         std::make_tuple(d.hash, d.id, d.owner, d.content))
		.send();
   });   
}

void document::edit (const checksum256 &hash, const name &owner, const content &content ) 
{
   doc_table d_t (get_self(), get_self().value);
   auto hash_index = d_t.get_index<name("idhash")>();
   auto h_itr = hash_index.find(hash);
   check (h_itr != hash_index.end(), "Document not found: " + document::to_string(hash));

   d_t.emplace(get_self(), [&](auto &d) {
      d.id                 = d_t.available_primary_key();
      d.owner              = get_self();
      d.values             = values;
      d.values["parent"]   = std::vector<flexvalue> {hash};
      
      string string_data = std::to_string(d.id) + d.owner.to_string() + document::to_string(values);
      d.hash = eosio::sha256(const_cast<char *>(string_data.c_str()), sizeof(string_data));

      // write a 'free' created receipt to the blockchain history logs
      action(
         permission_level{get_self(), name("active")},
         get_self(), name("created"),
         std::make_tuple(d.hash, d.id, d.owner, d.values))
		.send();
   });
}

void document::created(const checksum256 &hash, const uint64_t &id, const name &owner, const content &content) {
   require_auth (get_self());
}
