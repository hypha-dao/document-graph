#include <eosio/crypto.hpp>
#include <eosio/name.hpp>

#include <document_graph/util.hpp>

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

    const std::string readableHash(const eosio::checksum256 &hash)
    {
        auto byte_arr = hash.extract_as_byte_array();
        return toHex((const char *)byte_arr.data(), byte_arr.size());
    }
} // namespace hypha