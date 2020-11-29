#pragma once
#include <variant>
#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/crypto.hpp>

namespace hypha
{
    struct Content
    {
        typedef std::variant<std::monostate, eosio::name, std::string, eosio::asset, eosio::time_point,
                             std::int64_t, eosio::checksum256>
            FlexValue;

    public:
        Content();
        Content(std::string label, FlexValue value);
        ~Content();

        const bool isEmpty();

        const std::string toString();

        // NOTE: not using m_ notation because this changes serialization format
        std::string label;
        FlexValue value;

        template <class T>
        T getAs()
        {
            eosio::check(std::holds_alternative<T>(value), "Content value is not of expected type");
            return std::get<T>(value);
        }

        EOSLIB_SERIALIZE(Content, (label)(value))
    };

} // namespace hypha