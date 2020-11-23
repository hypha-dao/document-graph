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
        typedef std::variant<eosio::name, std::string, eosio::asset, eosio::time_point,
                             std::int64_t, eosio::checksum256>
            FlexValue;

    public:
        Content();
        Content(std::string label, FlexValue value);
        ~Content();

        const FlexValue getValue();
        void setValue(FlexValue value);

        const std::string getLabel();
        void setLabel(std::string label);

        const std::string toString();

        // NOTE: not using m_ notation because this changes serialization format
        std::string label;
        FlexValue value;

    private:
        std::string toHex(const char *d, uint32_t s);

        EOSLIB_SERIALIZE(Content, (label)(value))
    };

} // namespace hypha