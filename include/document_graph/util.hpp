#pragma once

#include <string>
#include <eosio/crypto.hpp>
#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/symbol.hpp>

#include "content.hpp"

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
  //class Content;
  using eosio::name;
  using eosio::asset;
  using eosio::time_point;
  using eosio::symbol_code;

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
      std::string to_str_h(const T& arg);

      std::string to_str_h(const char* arr);

      using strc = const char*;

      // template<std::size_t N>
      // inline std::string to_str_h(const char(&x)[N]) { return x; }

      extern template std::string to_str_h<name>(const name&);
      extern template std::string to_str_h<time_point>(const time_point&);
      extern template std::string to_str_h<symbol_code>(const symbol_code&);
      extern template std::string to_str_h<asset>(const asset&);
      extern template std::string to_str_h<std::string>(const std::string&);
      extern template std::string to_str_h<int64_t>(const int64_t&);
      extern template std::string to_str_h<uint64_t>(const uint64_t&);
      extern template std::string to_str_h<uint8_t>(const uint8_t&);
      extern template std::string to_str_h<uint32_t>(const uint32_t&);
      extern template std::string to_str_h<int32_t>(const int32_t&);
      extern template std::string to_str_h<Content>(const Content&);
    }
    
    // //Helper function to convert 1+ X type variables to string
    // template<class T, class...Args>
    // std::string to_str(const T& first, const Args&... others)
    // {
    //   return (detail::to_str_h(first) + ... + detail::to_str_h(others));
    // }

    #define __NARGT__(...)  __NARGT_I_(__VA_ARGS__,__RSEQT_N())
    #define __NARGT_I_(...) __ARGT_N(__VA_ARGS__)
    #define __ARGT_N(\
          _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, N, ...) N
        
    #define __RSEQT_N()\
    14,13,12,11,10,9,8,7,6,5,4,3,2,1

    #define CONCATENATET(arg1, arg2)   CONCATENATE1T(arg1, arg2)
    #define CONCATENATE1T(arg1, arg2)  CONCATENATE2T(arg1, arg2)
    #define CONCATENATE2T(arg1, arg2)  arg1##arg2

    #define to_str_(N, ...) CONCATENATET(to_str, N)(__VA_ARGS__)
    #define to_str(...) to_str_(__NARGT__(__VA_ARGS__), __VA_ARGS__)

    #define to_str1(arg1) (hypha::util::detail::to_str_h(arg1))
    #define to_str2(arg1, arg2) (hypha::util::detail::to_str_h(arg1) + to_str1(arg2))
    #define to_str3(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str2(__VA_ARGS__))
    #define to_str4(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str3(__VA_ARGS__))
    #define to_str5(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str4(__VA_ARGS__))
    #define to_str6(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str5(__VA_ARGS__))
    #define to_str7(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str6(__VA_ARGS__))
    #define to_str8(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str7(__VA_ARGS__))
    #define to_str9(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str8(__VA_ARGS__))
    #define to_str10(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str9(__VA_ARGS__))
    #define to_str11(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str10(__VA_ARGS__))
    #define to_str12(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str11(__VA_ARGS__))
    #define to_str13(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str12(__VA_ARGS__))
    #define to_str14(arg1, ...) (hypha::util::detail::to_str_h(arg1) + to_str13(__VA_ARGS__))

    #define _s(a) &a[0]

    template <typename... Rest>
    uint64_t hashCombine(Rest&&... rest)
    {
        uint64_t seed = 0;
        detail::_hashCombine(seed, std::forward<Rest>(rest)...);
        return seed;
    }
  }
} // namespace hypha