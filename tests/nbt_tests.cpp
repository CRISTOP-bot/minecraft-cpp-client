#include "mcpp/nbt.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {
using Bytes = std::vector<std::uint8_t>;
using mcpp::nbt::Type;

void u8(Bytes& out, std::uint8_t value) { out.push_back(value); }
void u16(Bytes& out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>(value >> 8U));
    out.push_back(static_cast<std::uint8_t>(value));
}
void u32(Bytes& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>(value >> 24U));
    out.push_back(static_cast<std::uint8_t>(value >> 16U));
    out.push_back(static_cast<std::uint8_t>(value >> 8U));
    out.push_back(static_cast<std::uint8_t>(value));
}
void str(Bytes& out, const std::string& value) {
    u16(out, static_cast<std::uint16_t>(value.size()));
    out.insert(out.end(), value.begin(), value.end());
}
void named(Bytes& out, Type type, const std::string& name) {
    u8(out, static_cast<std::uint8_t>(type));
    str(out, name);
}

void rootIntIsReadBigEndian() {
    Bytes bytes;
    named(bytes, Type::Int, "answer");
    u32(bytes, 0x01020304U);
    const auto doc = mcpp::nbt::parse(bytes);
    assert(doc.name == "answer");
    assert(doc.root.type == Type::Int);
    assert(std::get<std::int32_t>(doc.root.value) == 0x01020304);
}

void compoundListAndStringAreParsed() {
    Bytes bytes;
    named(bytes, Type::Compound, "root");
    named(bytes, Type::String, "greeting");
    str(bytes, "hello");
    named(bytes, Type::List, "numbers");
    u8(bytes, static_cast<std::uint8_t>(Type::Int));
    u32(bytes, 2);
    u32(bytes, 7);
    u32(bytes, 42);
    u8(bytes, static_cast<std::uint8_t>(Type::End));

    const auto doc = mcpp::nbt::parse(bytes);
    const auto& root = std::get<mcpp::nbt::Compound>(doc.root.value);
    assert(std::get<std::string>(root.at("greeting").value) == "hello");
    const auto& numbers = std::get<mcpp::nbt::List>(root.at("numbers").value);
    assert(numbers.size() == 2);
    assert(std::get<std::int32_t>(numbers[0].value) == 7);
    assert(std::get<std::int32_t>(numbers[1].value) == 42);
}

template <typename Fn>
void expectsParseError(Fn&& fn) {
    bool raised = false;
    try { fn(); }
    catch (const mcpp::nbt::ParseError& error) {
        raised = true;
        assert(std::string(error.what()).size() > 0);
    }
    assert(raised);
}

void rejectsMalformedAndOversizedInputs() {
    expectsParseError([] { (void)mcpp::nbt::parse(Bytes{0}); });
    expectsParseError([] { (void)mcpp::nbt::parse(Bytes{99, 0, 0}); });
    expectsParseError([] { (void)mcpp::nbt::parse(Bytes{3, 0, 0, 0}); });
    expectsParseError([] { (void)mcpp::nbt::parse(Bytes{7, 0, 0, 0xff, 0xff, 0xff, 0xff}); });

    Bytes good;
    named(good, Type::Int, "x");
    u32(good, 1);
    mcpp::nbt::Limits limits;
    limits.maxInputBytes = 2;
    expectsParseError([&] { (void)mcpp::nbt::parse(good, limits); });
}

void rejectsNonEmptyEndListsAndTrailingBytes() {
    Bytes badList;
    named(badList, Type::List, "list");
    u8(badList, static_cast<std::uint8_t>(Type::End));
    u32(badList, 1);
    expectsParseError([&] { (void)mcpp::nbt::parse(badList); });

    Bytes trailing;
    named(trailing, Type::Int, "x");
    u32(trailing, 3);
    trailing.push_back(0);
    expectsParseError([&] { (void)mcpp::nbt::parse(trailing); });
}
}

int main() {
    rootIntIsReadBigEndian();
    compoundListAndStringAreParsed();
    rejectsMalformedAndOversizedInputs();
    rejectsNonEmptyEndListsAndTrailingBytes();
    std::cout << "NBT parser tests passed\n";
}
