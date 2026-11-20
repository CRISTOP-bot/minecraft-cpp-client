#include "mcpp/nbt.hpp"

#include <bit>
#include <limits>
#include <sstream>
#include <type_traits>

namespace mcpp::nbt {
namespace {

class Reader final {
public:
    Reader(const std::vector<std::uint8_t>& bytes, Limits limits)
        : m_bytes(bytes), m_limits(limits) {
        if (bytes.size() > limits.maxInputBytes)
            fail("input exceeds configured byte limit");
    }

    Document document() {
        const Type type = readType();
        if (type == Type::End)
            fail("root tag cannot be TAG_End");
        const std::string name = readString();
        Tag root = readPayload(type, 0);
        if (m_offset != m_bytes.size())
            fail("trailing bytes after root tag");
        return {name, std::move(root)};
    }

private:
    [[noreturn]] void fail(const std::string& message) const {
        throw ParseError(m_offset, message);
    }

    std::uint8_t readU8() {
        if (m_offset >= m_bytes.size())
            fail("unexpected end of input");
        return m_bytes[m_offset++];
    }

    template <typename UInt>
    UInt readUnsigned() {
        static_assert(std::is_unsigned_v<UInt>);
        UInt value = 0;
        for (std::size_t i = 0; i < sizeof(UInt); ++i)
            value = static_cast<UInt>((value << 8U) | static_cast<UInt>(readU8()));
        return value;
    }

    std::int8_t readI8() { return std::bit_cast<std::int8_t>(readU8()); }
    std::int16_t readI16() { return std::bit_cast<std::int16_t>(readUnsigned<std::uint16_t>()); }
    std::int32_t readI32() { return std::bit_cast<std::int32_t>(readUnsigned<std::uint32_t>()); }
    std::int64_t readI64() { return std::bit_cast<std::int64_t>(readUnsigned<std::uint64_t>()); }
    float readF32() { return std::bit_cast<float>(readUnsigned<std::uint32_t>()); }
    double readF64() { return std::bit_cast<double>(readUnsigned<std::uint64_t>()); }

    Type readType() {
        const std::uint8_t id = readU8();
        if (id > static_cast<std::uint8_t>(Type::LongArray))
            fail("unknown NBT tag type");
        return static_cast<Type>(id);
    }

    std::size_t readLength() {
        const std::int32_t length = readI32();
        if (length < 0)
            fail("negative NBT collection length");
        const auto result = static_cast<std::size_t>(length);
        if (result > m_limits.maxElements - m_elements)
            fail("NBT collection exceeds configured element limit");
        m_elements += result;
        return result;
    }

    std::string readString() {
        const std::size_t length = readUnsigned<std::uint16_t>();
        if (length > m_bytes.size() - m_offset)
            fail("truncated NBT string");
        std::string result;
        result.reserve(length);
        for (std::size_t i = 0; i < length; ++i)
            result.push_back(static_cast<char>(m_bytes[m_offset++]));
        return result;
    }

    Tag readPayload(Type type, std::size_t depth) {
        if (depth > m_limits.maxDepth)
            fail("NBT nesting exceeds configured depth limit");
        if (++m_elements > m_limits.maxElements)
            fail("NBT document exceeds configured element limit");

        switch (type) {
        case Type::End:
            fail("TAG_End is only valid as a compound terminator");
        case Type::Byte: return {type, readI8()};
        case Type::Short: return {type, readI16()};
        case Type::Int: return {type, readI32()};
        case Type::Long: return {type, readI64()};
        case Type::Float: return {type, readF32()};
        case Type::Double: return {type, readF64()};
        case Type::ByteArray: {
            const std::size_t count = readLength();
            if (count > m_bytes.size() - m_offset)
                fail("truncated TAG_Byte_Array");
            ByteArray values;
            values.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
                values.push_back(readI8());
            return {type, std::move(values)};
        }
        case Type::String:
            return {type, readString()};
        case Type::List: {
            const Type itemType = readType();
            const std::size_t count = readLength();
            if (itemType == Type::End && count != 0)
                fail("non-empty TAG_List cannot have TAG_End elements");
            List values;
            values.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
                values.push_back(readPayload(itemType, depth + 1));
            return {type, std::move(values)};
        }
        case Type::Compound: {
            Compound values;
            for (;;) {
                const Type childType = readType();
                if (childType == Type::End)
                    break;
                std::string name = readString();
                values.insert_or_assign(std::move(name), readPayload(childType, depth + 1));
            }
            return {type, std::move(values)};
        }
        case Type::IntArray: {
            const std::size_t count = readLength();
            if (count > (m_bytes.size() - m_offset) / sizeof(std::int32_t))
                fail("truncated TAG_Int_Array");
            IntArray values;
            values.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
                values.push_back(readI32());
            return {type, std::move(values)};
        }
        case Type::LongArray: {
            const std::size_t count = readLength();
            if (count > (m_bytes.size() - m_offset) / sizeof(std::int64_t))
                fail("truncated TAG_Long_Array");
            LongArray values;
            values.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
                values.push_back(readI64());
            return {type, std::move(values)};
        }
        }
        fail("unsupported NBT tag type");
    }

    const std::vector<std::uint8_t>& m_bytes;
    Limits m_limits;
    std::size_t m_offset{0};
    std::size_t m_elements{0};
};

} // namespace

ParseError::ParseError(std::size_t offset, const std::string& message)
    : std::runtime_error([&] {
          std::ostringstream out;
          out << "NBT parse error at byte " << offset << ": " << message;
          return out.str();
      }()),
      m_offset(offset) {}

Document parse(const std::vector<std::uint8_t>& bytes, Limits limits) {
    return Reader(bytes, limits).document();
}

} // namespace mcpp::nbt
