#include <document_graph/util.hpp>

#include "document_graph/content.hpp"

namespace hypha
{
    const std::string toHex(const char *d, std::uint32_t s)
    {
        std::string r;
        const char *to_hex = "0123456789abcdef";
        auto c = reinterpret_cast<const uint8_t *>(d);
        for (auto i = 0; i < s; ++i)
            (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
        return r;
    }

    namespace util::detail {

    std::string to_str_h(const char* arr) { return arr; }

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
      
      template std::string to_str_h<name>(const name&);
      template std::string to_str_h<time_point>(const time_point&);
      template std::string to_str_h<symbol_code>(const symbol_code&);
      template std::string to_str_h<asset>(const asset&);
      template std::string to_str_h<std::string>(const std::string&);
      template std::string to_str_h<int64_t>(const int64_t&);
      template std::string to_str_h<uint64_t>(const uint64_t&);
      template std::string to_str_h<uint8_t>(const uint8_t&);
      template std::string to_str_h<uint32_t>(const uint32_t&);
      template std::string to_str_h<int32_t>(const int32_t&);
      template std::string to_str_h<hypha::Content>(const hypha::Content&);      
    }
} // namespace hypha