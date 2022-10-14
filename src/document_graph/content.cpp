#include <document_graph/content.hpp>
#include <document_graph/util.hpp>

namespace hypha
{

    Content::Content(std::string label, FlexValue value) : label{label}, value{value} {}
    Content::Content() {}
    Content::~Content() {}

    const bool Content::isEmpty () const
    {
        if (std::holds_alternative<std::monostate>(value)) {
            return true;
        }
        return false;
    }

    const std::string Content::toString() const
    {
        if (isEmpty()) return "";
        
        std::string str = "{" + std::string(label) + "=";
        if (std::holds_alternative<std::int64_t>(value))
        {
            str += "[int64," + std::to_string(std::get<std::int64_t>(value)) + "]";
        }
        else if (std::holds_alternative<eosio::asset>(value))
        {
            str += "[asset," + std::get<eosio::asset>(value).to_string() + "]";
        }
        else if (std::holds_alternative<eosio::time_point>(value))
        {
            str += "[time_point," + std::to_string(std::get<eosio::time_point>(value).sec_since_epoch()) + "]";
        }
        else if (std::holds_alternative<std::string>(value))
        {
            str += "[string," + std::get<std::string>(value) + "]";
        }
        else if (std::holds_alternative<eosio::checksum256>(value))
        {
            eosio::checksum256 cs_value = std::get<eosio::checksum256>(value);
            auto arr = cs_value.extract_as_byte_array();
            std::string str_value = toHex((const char *)arr.data(), arr.size());
            str += "[checksum256," + str_value + "]";
        }
        else
        {
            str += "[name," + std::get<eosio::name>(value).to_string() + "]";
        }
        str += "}";
        return str;
    }

    template<class T>
    T& Content::getAs()
    {
        EOS_CHECK(
            std::holds_alternative<T>(value),
            to_str(
                "Content value for label [", label, "] is not of expected type.", 
                " Expected: ", getType<T>(), " but has: ", getIndexType()
            )
        );
        return std::get<T>(value);
    }

    template<class T>
    const T& Content::getAs() const
    {
        EOS_CHECK(
            std::holds_alternative<T>(value),
            to_str(
                "Content value for label [", label, "] is not of expected type.", 
                " Expected: ", getType<T>(), " but has: ", getIndexType()
            )
        );
        return std::get<T>(value);
    }

    template eosio::name& Content::getAs<eosio::name>();
    template eosio::asset& Content::getAs<eosio::asset>();
    template eosio::time_point& Content::getAs<eosio::time_point>();
    template eosio::checksum256& Content::getAs<eosio::checksum256>();
    template std::string& Content::getAs<std::string>();
    template std::int64_t& Content::getAs<std::int64_t>();

    template const eosio::name& Content::getAs<eosio::name>() const;
    template const eosio::asset& Content::getAs<eosio::asset>() const;
    template const eosio::time_point& Content::getAs<eosio::time_point>() const;
    template const eosio::checksum256& Content::getAs<eosio::checksum256>() const;
    template const std::string& Content::getAs<std::string>() const;
    template const std::int64_t& Content::getAs<std::int64_t>() const;
} // namespace hypha