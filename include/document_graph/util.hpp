#pragma once

#include <string>
#include <eosio/crypto.hpp>
#include <eosio/name.hpp>

namespace std {
  template <> struct hash<eosio::name>
  {
    size_t operator()(const eosio::name& x) const
    {
      return std::hash<uint64_t>{}(x.value);
    }
  };
}

namespace hypha
{

  const std::string toHex(const char *d, std::uint32_t s);
  const std::uint64_t toUint64(const std::string &fingerprint);
 
  namespace util
  {
    namespace detail 
    {
       
      template <typename T, typename... Rest>
      void _hashCombine(uint64_t& seed, const T& v, Rest&&... rest)
      {
          seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
          (_hashCombine(seed, rest), ...);
      }

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
      struct supports_call_to_string_v2
      {
        template<class U>
        static auto can_pass_to_string(const U* arg) -> decltype(arg->toString(), char(0))
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
        else if constexpr (supports_call_to_string_v2<T>::value) {
          return arg.toString();
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


    template <typename... Rest>
    uint64_t hashCombine(Rest&&... rest)
    {
        uint64_t seed = 0;
        detail::_hashCombine(seed, std::forward<Rest>(rest)...);
        return seed;
    }
  }
} // namespace hypha