#include <document_graph/content.hpp>

namespace hypha
{

    Content::Content(std::string label, FlexValue value) : label{label}, value{value} {}
    Content::Content() {}
    Content::~Content() {}

    const std::string Content::getLabel()
    {
        return label;
    }

    void Content::setLabel(std::string label)
    {
        label = label;
    }

    const Content::FlexValue Content::getValue()
    {
        return value;
    }

    void Content::setValue(Content::FlexValue value)
    {
        value = value;
    }

    const std::string Content::toString()
    {
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
            std::string str_value = Content::toHex((const char *)arr.data(), arr.size());
            str += "[checksum256," + str_value + "]";
        }
        else
        {
            str += "[name," + std::get<eosio::name>(value).to_string() + "]";
        }
        str += "}";
        return str;
    }

    std::string Content::toHex(const char *d, uint32_t s)
    {
        std::string r;
        const char *to_hex = "0123456789abcdef";
        auto c = reinterpret_cast<const uint8_t *>(d);
        for (auto i = 0; i < s; ++i)
            (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
        return r;
    }

} // namespace hypha