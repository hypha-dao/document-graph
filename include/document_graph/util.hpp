#pragma once
#include <string>
#include <eosio/crypto.hpp>

namespace hypha {

    static const std::string toHex (const char *d, std::uint32_t s);
    static const std::string readableHash (const eosio::checksum256 &hash);
    static const std::uint64_t toUint64 (const std::string &fingerprint);
}