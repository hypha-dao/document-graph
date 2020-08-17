#include <document.hpp>

using namespace hyphaspace;

document::document(name self, name code, datastream<const char *> ds) : contract(self, code, ds) {}
document::~document() {}

void document::create(const name &creator, const content &content)
{
   require_auth (creator);

   // TODO: error out if content is empty
   // TODO: ensure that the creator is authorized/member

   doc_table d_t (get_self(), get_self().value);
   d_t.emplace(get_self(), [&](auto& d) {
      d.id              = d_t.available_primary_key();
      d.creator         = creator;
      d.content         = content;
      
      // fingerprint the content object 
      string string_data = document::to_string(content); 
      checksum256 content_hash = eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());

      auto hash_index = d_t.get_index<name("idhash")>();
      auto h_itr = hash_index.find(content_hash);

      auto byte_arr = content_hash.extract_as_byte_array();
      string readable_hash = to_hex( (const char*)byte_arr.data(), byte_arr.size()) ;

      // if this content exists already, error out and send back the hash of the existing document
      if (h_itr != hash_index.end()) {
         check (false, "document exists already: " + readable_hash);
      }

      // write a 'free' created receipt to the blockchain history logs
      action(
         permission_level{get_self(), name("active")},
         get_self(), name("created"),
         std::make_tuple(content_hash, d.id, d.creator, d.content))
		.send();

      d.hash = content_hash;
   });   
}

void document::fork (const checksum256 &hash, const name &creator, const content &content ) 
{
   require_auth (creator);

   doc_table d_t (get_self(), get_self().value);
   auto hash_index = d_t.get_index<name("idhash")>();
   auto h_itr = hash_index.find(hash);
   check (h_itr != hash_index.end(), "document not found: " + document::to_string(hash));

   // TODO: error out if content is empty
   // TODO: error out if content does not provide new/updated data to the forked object

   d_t.emplace(get_self(), [&](auto &d) {
      d.id                 = d_t.available_primary_key();
      d.creator            = get_self();

      d.content            = content;
      d.content["parent"]  = std::vector<flexvalue> {hash};
      
      string string_data = document::to_string(content);
      d.hash = eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());

      // write a 'free' created receipt to the blockchain history logs
      action(
         permission_level{get_self(), name("active")},
         get_self(), name("created"),
         std::make_tuple(d.hash, d.id, d.creator, d.content))
		.send();
   });
}

void document::created(const checksum256 &hash, const uint64_t &id, const name &creator, const content &content) {
   // only the contract can announce this
   require_auth (get_self());
}

void document::certify (const name &certifier, const checksum256 &hash, const string &notes) 
{
   doc_table d_t (get_self(), get_self().value);
   auto hash_index = d_t.get_index<name("idhash")>();
   auto h_itr = hash_index.find(hash);
   check (h_itr != hash_index.end(), "document not found: " + document::to_string(hash));

   // this is enforced in the new certificate function too, so might be duplicative
   require_auth (certifier);

   // TODO: should a certifier be able to sign the same document fork multiple times?
   hash_index.modify(h_itr, get_self(), [&](auto &d) {
     d.certificates.push_back(new_certificate(certifier, notes));
   });
}

std::string document::to_string(document::flexvalue value)
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
      string str_value = to_hex( (const char*)arr.data(), arr.size()) ;
      return "[checksum256," + str_value + "]";
   }
   else 
   {
      return "[name," + std::get<name>(value).to_string() + "]";
   }
}

std::string document::to_string(vector<flexvalue> values) 
{
   string results = "[";
   bool is_first = true;
   for (std::vector<flexvalue>::iterator it = values.begin() ; it != values.end(); ++it)
   {  
      if (is_first) {
         is_first = false;
      } else {
         results = results + ",";
      }

      results = results + to_string(*it);
   }
   results = results + "]";
   return results;
}

std::string document::to_string(const content &content)
{
   string results = "[";
   bool is_first = true;

   std::map<string, vector<flexvalue>>::const_iterator value_itr;
   for (value_itr = content.begin(); value_itr != content.end(); ++value_itr)
   {      
      if (is_first) {
         is_first = false;
      } else {
         results = results + ",";
      }
      string to_append = string(value_itr->first + "=" + to_string(value_itr->second));

      results = results + to_append;
   }
   results = results + "]";
   return results;
}

document::certificate document::new_certificate (const name &certifier, const string &notes) {
   require_auth (certifier);
   certificate cert = certificate{};
   cert.certifier = certifier;
   cert.notes = notes;
   return cert;
}

// void document::writehash (const string &string_data) 
// {
//    checksum256 content_hash = eosio::sha256(const_cast<char *>(string_data.c_str()), string_data.length());
//    hash_table h_t (get_self(), get_self().value);
//    h_t.emplace (get_self(), [&](auto &h) {
//       h.hash_id = h_t.available_primary_key();
//       h.data_to_hash = string_data;
//       h.hash = content_hash;
//    });
// }

// void document::testhash () 
// {
//    string string_data_1 = "[[string,happy birthday sir]]";
//    checksum256 content_hash_1 = eosio::sha256(const_cast<char *>(string_data_1.c_str()), sizeof(string_data_1));

//    string string_data_2 = "[[string,happy birthday maam]]";
//    checksum256 content_hash_2 = eosio::sha256(const_cast<char *>(string_data_2.c_str()), sizeof(string_data_2));

//    hash_table h_t (get_self(), get_self().value);
//    h_t.emplace (get_self(), [&](auto &h) {
//       h.hash_id = h_t.available_primary_key();
//       h.data_to_hash = string_data_1;
//       h.hash = content_hash_1;
//    });

//    h_t.emplace (get_self(), [&](auto &h) {
//       h.hash_id = h_t.available_primary_key();
//       h.data_to_hash = string_data_2;
//       h.hash = content_hash_2;
//    });
// }


void document::reset() 
{
   require_auth (get_self());
   doc_table d_t (get_self(), get_self().value);
   auto d_itr = d_t.begin();
   while (d_itr != d_t.end()) {
      d_itr = d_t.erase (d_itr);
   }

   // hash_table h_t (get_self(), get_self().value);
   // auto h_itr = h_t.begin();
   // while (h_itr != h_t.end()) {
   //    h_itr = h_t.erase (h_itr);
   // }
}