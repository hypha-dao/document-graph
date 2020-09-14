#include <docs.hpp>

using namespace hyphaspace;

docs::docs(name self, name code, datastream<const char *> ds) : contract(self, code, ds) {}
docs::~docs() {}

void docs::create(const name &creator, const vector<document_graph::content_group> &content_groups)
{
   _document_graph.create_document(creator, content_groups);
}

void docs::fork (const checksum256 &hash, const name &creator, const vector<document_graph::content_group> &content_groups )
{
   _document_graph.fork_document(hash, creator, content_groups);
}

void docs::created(const name &creator, const checksum256 &hash)
{
   // only the contract can announce this
   require_auth(get_self());
}

void docs::certify(const name &certifier, const checksum256 &hash, const string &notes)
{
   _document_graph.certify_document(certifier, hash, notes);
}

void docs::transform (const name &scope, const uint64_t &id) 
{
   object_table o_t(name("dao.hypha"), scope.value);
	auto o_itr = o_t.find(id);
	check(o_itr != o_t.end(), "Scope: " + scope.to_string() + "; Object ID: " + std::to_string(id) + " does not exist.");

   vector<document_graph::content_group> content_groups;
   document_graph::content_group cg {};
   
   std::map<string, name>::const_iterator name_itr;
   for (name_itr = o_itr->names.begin(); name_itr != o_itr->names.end(); ++name_itr) {
      document_graph::content c {};
      c.label = name_itr->first;
      c.value = name_itr->second;
      cg.push_back(c);
   }

   std::map<string, asset>::const_iterator asset_itr;
   for (asset_itr = o_itr->assets.begin(); asset_itr != o_itr->assets.end(); ++asset_itr) {
      document_graph::content c {};
      c.label = asset_itr->first;
      c.value = asset_itr->second;
      cg.push_back(c);
   }

   std::map<string, string>::const_iterator string_itr;
   for (string_itr = o_itr->strings.begin(); string_itr != o_itr->strings.end(); ++string_itr) {
      document_graph::content c {};
      c.label = string_itr->first;
      c.value = string_itr->second;
      cg.push_back(c);
   }

   std::map<string, uint64_t>::const_iterator int_itr;
   for (int_itr = o_itr->ints.begin(); int_itr != o_itr->ints.end(); ++int_itr) {
      document_graph::content c {};
      c.label = int_itr->first;
      c.value = int_itr->second;
      cg.push_back(c);
   }

   std::map<string, time_point>::const_iterator time_point_itr;
   for (time_point_itr = o_itr->time_points.begin(); time_point_itr != o_itr->time_points.end(); ++time_point_itr) {
      document_graph::content c {};
      c.label = time_point_itr->first;
      c.value = time_point_itr->second;
      cg.push_back(c);
   }

   content_groups.push_back(cg);
   create (get_self(), content_groups);
}

void docs::reset()
{
   require_auth(get_self());
   document_table d_t(get_self(), get_self().value);
   auto d_itr = d_t.begin();
   while (d_itr != d_t.end())
   {
      d_itr = d_t.erase(d_itr);
   }
}