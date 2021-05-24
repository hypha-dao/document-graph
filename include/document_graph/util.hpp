#pragma once

#include <string>
#include <eosio/crypto.hpp>
#include <eosio/name.hpp>

namespace hypha
{

    const std::string toHex(const char *d, std::uint32_t s);
    const std::string readableHash(const eosio::checksum256 &hash);
    const std::uint64_t toUint64(const std::string &fingerprint);
    const std::uint64_t concatHash(const eosio::checksum256 sha1, const eosio::checksum256 sha2, const eosio::name label);
    const std::uint64_t concatHash(const eosio::checksum256 sha1, const eosio::checksum256 sha2);
    const std::uint64_t concatHash(const eosio::checksum256 sha, const eosio::name label);

  namespace util
  {    
    namespace detail 
    {

      template<class T>
      struct supports_to_string
      {
        template<class U>
        static auto can_pass_to_string(const U* arg) -> decltype(std::to_string(*arg), char(0))
        {}

        static std::array<char, 2> can_pass_to_string(...) { }

        static constexpr bool value = (sizeof(can_pass_to_string((T*)0)) == 1);
      };

      template<class T>
      struct supports_call_to_string
      {
        template<class U>
        static auto can_pass_to_string(const U* arg) -> decltype(arg->to_string(), char(0))
        {}

        static std::array<char, 2> can_pass_to_string(...) { }

        static constexpr bool value = (sizeof(can_pass_to_string((T*)0)) == 1);
      };
      
      template<class T>
      std::string to_str_h(const T& arg)
      {
        if constexpr (supports_to_string<T>::value) {
          return std::to_string(arg);
        }
        else if constexpr (supports_call_to_string<T>::value) {
          return arg.to_string();
        }
        else if constexpr (std::is_same_v<T, eosio::checksum256>) {
          return readableHash(arg);
        }
        else if constexpr (std::is_same_v<T, class ContentGroup>) {

          std::string s;

          s = "ContentGroup {\n";

          for (auto& content : arg) {
            s += "\tContent " + content.toString() + "\n";
          }
          s += "}\n";

          return s;
        }
        else {
          return arg;
        }
      }
    }   
    
    //Helper function to convert 1+ X type variables to string
    template<class T, class...Args>
    std::string to_str(const T& first, const Args&... others)
    {
      return (detail::to_str_h(first) + ... + detail::to_str_h(others));
    }

  }

} // namespace hypha