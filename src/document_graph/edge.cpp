#include <document_graph/document.hpp>
#include <document_graph/edge.hpp>
#include <document_graph/util.hpp>
#include <logger/logger.hpp>

namespace hypha
{
    Edge::Edge() {}
    Edge::Edge(const eosio::name &contract,
               const eosio::name &creator,
               const eosio::checksum256 &from_node,
               const eosio::checksum256 &to_node,
               const eosio::name &edge_name)
        : contract{contract}, creator{creator}, from_node{from_node}, to_node{to_node}, edge_name{edge_name}
    {
        TRACE_FUNCTION()
        emplace();
    }

    Edge::~Edge() {}

    // static
    void Edge::write(const eosio::name &_contract,
                     const eosio::name &_creator,
                     const eosio::checksum256 &_from_node,
                     const eosio::checksum256 &_to_node,
                     const eosio::name &_edge_name)
    {
        edge_table e_t(_contract, _contract.value);

        const int64_t edgeID = concatHash(_from_node, _to_node, _edge_name);

        EOS_CHECK(
          e_t.find(edgeID) == e_t.end(), 
          util::to_str("Edge from: ", _from_node, 
                       " to: ", _to_node, 
                       " with name: ", _edge_name, " already exists")
        );
        
        std::string edge_name_str = _edge_name.to_string();
        EOS_CHECK(!edge_name_str.empty(), "Edge name cannot be empty");
        EOS_CHECK(!isdigit(edge_name_str[0]), "Edge name cannot start with a number");
        EOS_CHECK(edge_name_str.find('.') == std::string::npos, "Edge name cannot contain '.' characters");

        e_t.emplace(_contract, [&](auto &e) {
            e.id = edgeID;
            e.from_node_edge_name_index = concatHash(_from_node, _edge_name);
            e.from_node_to_node_index = concatHash(_from_node, _to_node);
            e.to_node_edge_name_index = concatHash(_to_node, _edge_name);
            e.creator = _creator;
            e.contract = _contract;
            e.from_node = _from_node;
            e.to_node = _to_node;
            e.edge_name = _edge_name;
            e.created_date = eosio::current_time_point();
        });
    }

    // static
    Edge Edge::getOrNew(const eosio::name &_contract,
                        const eosio::name &creator,
                        const eosio::checksum256 &_from_node,
                        const eosio::checksum256 &_to_node,
                        const eosio::name &_edge_name)
    {
        edge_table e_t(_contract, _contract.value);
        auto itr = e_t.find(concatHash(_from_node, _to_node, _edge_name));

        if (itr != e_t.end())
        {
            return *itr;
        }

        return Edge(_contract, creator, _from_node, _to_node, _edge_name);
    }

    // static getter
    Edge Edge::get(const eosio::name &_contract,
                   const eosio::checksum256 &_from_node,
                   const eosio::checksum256 &_to_node,
                   const eosio::name &_edge_name)
    {
        edge_table e_t(_contract, _contract.value);
        auto itr = e_t.find(concatHash(_from_node, _to_node, _edge_name));

        EOS_CHECK(itr != e_t.end(), "edge does not exist: from " + readableHash(_from_node) + " to " + readableHash(_to_node) + " with edge name of " + _edge_name.to_string());

        return *itr;
    }

    // static getter
    Edge Edge::get(const eosio::name &_contract,
                   const eosio::checksum256 &_from_node,
                   const eosio::name &_edge_name)
    {
        edge_table e_t(_contract, _contract.value);
        auto fromEdgeIndex = e_t.get_index<eosio::name("byfromname")>();
        auto index = concatHash(_from_node, _edge_name);
        auto itr = fromEdgeIndex.find(index);

        EOS_CHECK(itr != fromEdgeIndex.end() && itr->from_node_edge_name_index == index, "edge does not exist: from " + readableHash(_from_node) + " with edge name of " + _edge_name.to_string());

        return *itr;
    }

    // static getter
    Edge Edge::getTo(const eosio::name &_contract,
                     const eosio::checksum256 &_to_node,
                     const eosio::name &_edge_name)
    {
        edge_table e_t(_contract, _contract.value);
        auto toEdgeIndex = e_t.get_index<eosio::name("bytoname")>();
        auto index = concatHash(_to_node, _edge_name);
        auto itr = toEdgeIndex.find(index);

        EOS_CHECK(itr != toEdgeIndex.end() && itr->to_node_edge_name_index == index, "edge does not exist: to " + readableHash(_to_node) + " with edge name of " + _edge_name.to_string());

        return *itr;
    }

    // static getter
    std::pair<bool, Edge> Edge::getIfExists(const eosio::name &_contract,
                                            const eosio::checksum256 &_from_node,
                                            const eosio::name &_edge_name)
    {
        edge_table e_t(_contract, _contract.value);
        auto fromEdgeIndex = e_t.get_index<eosio::name("byfromname")>();
        auto index = concatHash(_from_node, _edge_name);
        auto itr = fromEdgeIndex.find(index);

        if (itr != fromEdgeIndex.end() && itr->from_node_edge_name_index == index)
        {
            return std::pair<bool, Edge> (true, *itr);
        }

        return std::pair<bool, Edge>(false, Edge{});
    }

    // static getter
    bool Edge::exists(const eosio::name &_contract,
                      const eosio::checksum256 &_from_node,
                      const eosio::checksum256 &_to_node,
                      const eosio::name &_edge_name)
    {
        edge_table e_t(_contract, _contract.value);
        auto itr = e_t.find(concatHash(_from_node, _to_node, _edge_name));
        if (itr != e_t.end())
            return true;
        return false;
    }

    void Edge::emplace()
    {
        // update indexes prior to save
        id = concatHash(from_node, to_node, edge_name);

        from_node_edge_name_index = concatHash(from_node, edge_name);
        from_node_to_node_index = concatHash(from_node, to_node);
        to_node_edge_name_index = concatHash(to_node, edge_name);

        edge_table e_t(getContract(), getContract().value);

        EOS_CHECK(
          e_t.find(id) == e_t.end(), 
          util::to_str("Edge from: ", from_node, 
                       " to: ", to_node, 
                       " with name: ", edge_name, " already exists")
        );

        e_t.emplace(getContract(), [&](auto &e) {
            e = *this;
            e.created_date = eosio::current_time_point();
        });
    }

    void Edge::erase()
    {
        edge_table e_t(getContract(), getContract().value);
        auto itr = e_t.find(id);

        EOS_CHECK(itr != e_t.end(), "edge does not exist: from " + readableHash(from_node) + " to " + readableHash(to_node) + " with edge name of " + edge_name.to_string());
        e_t.erase(itr);
    }

    uint64_t Edge::primary_key() const { return id; }
    uint64_t Edge::by_from_node_edge_name_index() const { return from_node_edge_name_index; }
    uint64_t Edge::by_from_node_to_node_index() const { return from_node_to_node_index; }
    uint64_t Edge::by_to_node_edge_name_index() const { return to_node_edge_name_index; }
    uint64_t Edge::by_edge_name() const { return edge_name.value; }
    uint64_t Edge::by_created() const { return created_date.sec_since_epoch(); }
    uint64_t Edge::by_creator() const { return creator.value; }

    eosio::checksum256 Edge::by_from() const { return from_node; }
    eosio::checksum256 Edge::by_to() const { return to_node; }
} // namespace hypha