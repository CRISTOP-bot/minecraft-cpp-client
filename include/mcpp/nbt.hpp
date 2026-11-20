#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace mcpp::nbt {

enum class Type : std::uint8_t {
    End = 0, Byte = 1, Short = 2, Int = 3, Long = 4, Float = 5, Double = 6,
    ByteArray = 7, String = 8, List = 9, Compound = 10, IntArray = 11, LongArray = 12
};

struct Tag;
using List = std::vector<Tag>;
using Compound = std::map<std::string, Tag, std::less<>>;
using ByteArray = std::vector<std::int8_t>;
using IntArray = std::vector<std::int32_t>;
using LongArray = std::vector<std::int64_t>;
using Value = std::variant<std::monostate, std::int8_t, std::int16_t, std::int32_t,
                           std::int64_t, float, double, ByteArray, std::string,
                           List, Compound, IntArray, LongArray>;

struct Tag final {
    Type type{Type::End};
    Value value{};
};

struct Document final {
    std::string name;
    Tag root;
};

struct Limits final {
    std::size_t maxInputBytes{64U * 1024U * 1024U};
    std::size_t maxElements{1U * 1024U * 1024U};
    std::size_t maxDepth{64};
};

class ParseError final : public std::runtime_error {
public:
    ParseError(std::size_t offset, const std::string& message);
    [[nodiscard]] std::size_t offset() const noexcept { return m_offset; }
private:
    std::size_t m_offset;
};

/// Parse one uncompressed, big-endian Java Edition NBT document.
/// Throws ParseError for invalid input or configured safety-limit violations.
[[nodiscard]] Document parse(const std::vector<std::uint8_t>& bytes,
                             Limits limits = {});

} // namespace mcpp::nbt
