#pragma once

#include <variant>
#include <type_traits>

#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/crypto.hpp>
#include <logger/logger.hpp>

namespace hypha
{
    struct Content
    {
        typedef std::variant<std::monostate, eosio::name, std::string, eosio::asset, eosio::time_point,
                             std::int64_t, eosio::checksum256>
            FlexValue;
    private:

        template<class T>
        constexpr auto getType() const {
            if constexpr (std::is_same_v<T, eosio::name>) 
                return "name";
            else if constexpr (std::is_same_v<T, std::string>)
                return "string";
            else if constexpr (std::is_same_v<T, std::int64_t>)
                return "int64";
            else if constexpr (std::is_same_v<T, eosio::checksum256>)
                return "checksum";
            else if constexpr (std::is_same_v<T, eosio::asset>)
                return "asset";
            else if constexpr (std::is_same_v<T, eosio::time_point>)
                return "time_point";
            else 
                return "unkown type";
        }

        auto getIndexType() const {
            size_t index = value.index();
            if (index == 0) 
                return "monostate";
            else if (index == 1) 
                return "name";
            else if (index == 2)
                return "string";
            else if (index == 3)
                return "asset";
            else if (index == 4)
                return "time_point";
            else if (index == 5)
                return "int64";
            else if (index == 6)
                return "checksum";
            else 
                return "unkown index";
        }
    public:
        Content();
        Content(std::string label, FlexValue value);
        ~Content();

        const bool isEmpty() const;

        const std::string toString() const;

        // NOTE: not using m_ notation because this changes serialization format
        std::string label;
        FlexValue value;

        //Can return reference to stored type
        template <class T>
        T& getAs();

        template <class T>
        const T& getAs() const;

        inline bool operator==(const Content& other) 
        {
          return label == other.label && value == other.value;
        }

        EOSLIB_SERIALIZE(Content, (label)(value))
    };

    //Specify which types are valid for casting
    extern template eosio::name& Content::getAs<eosio::name>();
    extern template eosio::asset& Content::getAs<eosio::asset>();
    extern template eosio::time_point& Content::getAs<eosio::time_point>();
    extern template eosio::checksum256& Content::getAs<eosio::checksum256>();
    extern template std::string& Content::getAs<std::string>();
    extern template std::int64_t& Content::getAs<std::int64_t>();
    
    extern template const eosio::name& Content::getAs<eosio::name>() const;
    extern template const eosio::asset& Content::getAs<eosio::asset>() const;
    extern template const eosio::time_point& Content::getAs<eosio::time_point>() const;
    extern template const eosio::checksum256& Content::getAs<eosio::checksum256>() const;
    extern template const std::string& Content::getAs<std::string>() const;
    extern template const std::int64_t& Content::getAs<std::int64_t>() const;
    

} // namespace hypha