/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredFieldsDecoder.h>
#include <proxygen/lib/http/structuredheaders/StructuredFieldsEncoder.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersDecoder.h>

#include <folly/json.h>
#include <folly/portability/GTest.h>
#include <folly/testing/TestUtil.h>

#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace proxygen { namespace {

namespace SF = StructuredFields;

constexpr std::string_view kHttpwgFixturePath =
    "proxygen/lib/http/structuredheaders/test/structured-field-tests/";

std::string readFixture(std::string_view fileName) {
  const auto resourcePath = folly::test::find_resource(
      std::string(kHttpwgFixturePath) + std::string(fileName));
  std::ifstream input(resourcePath.string());
  if (!input) {
    throw std::runtime_error("failed to open RFC 9651 fixture " +
                             std::string(fileName));
  }

  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string combineFieldLines(const folly::dynamic& lines) {
  std::string combined;
  for (const auto& line : lines) {
    if (!combined.empty()) {
      combined.append(", ");
    }
    combined.append(line.asString());
  }
  return combined;
}

bool optionalBool(const folly::dynamic& record, const char* key) {
  const auto* value = record.get_ptr(key);
  return value != nullptr && value->asBool();
}

std::string expectedCanonicalValue(const folly::dynamic& record) {
  if (const auto* canonical = record.get_ptr("canonical")) {
    return combineFieldLines(*canonical);
  }
  return combineFieldLines(record["raw"]);
}

int base32Value(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A';
  }
  if (c >= '2' && c <= '7') {
    return c - '2' + 26;
  }
  throw std::runtime_error("invalid base32 fixture byte");
}

std::string decodeBase32FixtureValue(std::string_view input) {
  std::string decoded;
  uint32_t buffer = 0;
  uint8_t bits = 0;
  bool paddingSeen = false;
  for (char c : input) {
    if (c == '=') {
      paddingSeen = true;
      continue;
    }
    if (paddingSeen) {
      throw std::runtime_error("base32 fixture has data after padding");
    }

    buffer = (buffer << 5) | static_cast<uint32_t>(base32Value(c));
    bits += 5;
    if (bits >= 8) {
      bits -= 8;
      decoded.push_back(static_cast<char>((buffer >> bits) & 0xFF));
    }
  }
  return decoded;
}

int64_t roundToThousandths(double value) {
  const long double scaled = static_cast<long double>(value) * 1000.0L;
  const bool negative = scaled < 0;
  const long double absScaled = negative ? -scaled : scaled;
  const long double floorValue = std::floor(absScaled);
  const long double fraction = absScaled - floorValue;
  auto rounded = static_cast<uint64_t>(floorValue);

  constexpr long double kTieEpsilon = 1e-9L;
  if (fraction > 0.5L + kTieEpsilon ||
      (std::abs(fraction - 0.5L) <= kTieEpsilon && rounded % 2 != 0)) {
    ++rounded;
  }

  return negative ? -static_cast<int64_t>(rounded)
                  : static_cast<int64_t>(rounded);
}

SF::BareItem bareItemFromJson(const folly::dynamic& value) {
  if (value.isBool()) {
    return SF::BareItem::boolean(value.asBool());
  }
  if (value.isInt()) {
    return SF::BareItem::integer(value.asInt());
  }
  if (value.isDouble()) {
    return SF::BareItem::decimal(
        SF::Decimal{roundToThousandths(value.asDouble())});
  }
  if (value.isString()) {
    return SF::BareItem::string(value.asString());
  }
  if (value.isObject()) {
    const auto type = value["__type"].asString();
    if (type == "token") {
      return SF::BareItem::token(value["value"].asString());
    }
    if (type == "binary") {
      return SF::BareItem::byteSequence(
          decodeBase32FixtureValue(value["value"].asString()));
    }
    if (type == "date") {
      return SF::BareItem::date(value["value"].asInt());
    }
    if (type == "displaystring") {
      return SF::BareItem::displayString(value["value"].asString());
    }
  }
  throw std::runtime_error("unsupported RFC 9651 fixture bare item");
}

SF::Parameters parametersFromJson(const folly::dynamic& value) {
  SF::Parameters parameters;
  for (const auto& parameter : value) {
    parameters.set(parameter[0].asString(), bareItemFromJson(parameter[1]));
  }
  return parameters;
}

SF::Item itemFromJson(const folly::dynamic& value) {
  return SF::Item{.bareItem = bareItemFromJson(value[0]),
                  .parameters = parametersFromJson(value[1])};
}

bool isInnerListFixtureValue(const folly::dynamic& value) {
  return value.isArray() && value.size() == 2 && value[0].isArray() &&
         (value[0].empty() || value[0][0].isArray());
}

SF::InnerList innerListFromJson(const folly::dynamic& value) {
  SF::InnerList innerList;
  for (const auto& item : value[0]) {
    innerList.items.push_back(itemFromJson(item));
  }
  innerList.parameters = parametersFromJson(value[1]);
  return innerList;
}

SF::ListMember listMemberFromJson(const folly::dynamic& value) {
  if (isInnerListFixtureValue(value)) {
    return innerListFromJson(value);
  }
  return itemFromJson(value);
}

SF::List listFromJson(const folly::dynamic& value) {
  SF::List list;
  for (const auto& member : value) {
    list.push_back(listMemberFromJson(member));
  }
  return list;
}

SF::Dictionary dictionaryFromJson(const folly::dynamic& value) {
  SF::Dictionary dictionary;
  for (const auto& member : value) {
    dictionary.set(member[0].asString(), listMemberFromJson(member[1]));
  }
  return dictionary;
}

struct ParseAndSerializeResult {
  SF::DecodeError decodeError{SF::DecodeError::OK};
  SF::EncodeError encodeError{SF::EncodeError::OK};
  std::string serialized;
};

ParseAndSerializeResult parseAndSerialize(std::string_view headerType,
                                          std::string_view input) {
  ParseAndSerializeResult result;
  StructuredFieldsDecoder decoder(input);
  StructuredFieldsEncoder encoder;

  if (headerType == "item") {
    SF::Item item;
    result.decodeError = decoder.decodeItem(item);
    if (result.decodeError == SF::DecodeError::OK) {
      result.encodeError = encoder.encodeItem(item);
    }
  } else if (headerType == "list") {
    SF::List list;
    result.decodeError = decoder.decodeList(list);
    if (result.decodeError == SF::DecodeError::OK) {
      result.encodeError = encoder.encodeList(list);
    }
  } else if (headerType == "dictionary") {
    SF::Dictionary dictionary;
    result.decodeError = decoder.decodeDictionary(dictionary);
    if (result.decodeError == SF::DecodeError::OK) {
      result.encodeError = encoder.encodeDictionary(dictionary);
    }
  } else {
    throw std::runtime_error("unsupported RFC 9651 header_type " +
                             std::string(headerType));
  }

  result.serialized = encoder.get();
  return result;
}

struct SerializeFixtureResult {
  SF::EncodeError encodeError{SF::EncodeError::OK};
  std::string serialized;
};

SerializeFixtureResult serializeFixtureExpected(
    std::string_view headerType, const folly::dynamic& expected) {
  SerializeFixtureResult result;
  StructuredFieldsEncoder encoder;

  if (headerType == "item") {
    result.encodeError = encoder.encodeItem(itemFromJson(expected));
  } else if (headerType == "list") {
    result.encodeError = encoder.encodeList(listFromJson(expected));
  } else if (headerType == "dictionary") {
    result.encodeError = encoder.encodeDictionary(dictionaryFromJson(expected));
  } else {
    throw std::runtime_error("unsupported RFC 9651 header_type " +
                             std::string(headerType));
  }

  result.serialized = encoder.get();
  return result;
}

template <typename T>
void expectBareItemValue(const SF::BareItem& item,
                         SF::BareItem::Type type,
                         const T& expected) {
  EXPECT_EQ(item.type(), type);
  const auto* value = item.get<T>();
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, expected);
}

TEST(StructuredFieldsRfc9651Test, DecodeItem_Rfc9651BareItems_ParsesTypes) {
  // RFC 9651, Sections 3.3 and 4.2.3.
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("999999999999999");
    EXPECT_EQ(decoder.decodeItem(item), SF::DecodeError::OK);
    expectBareItemValue<int64_t>(
        item.bareItem, SF::BareItem::Type::INTEGER, 999999999999999);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("4.500");
    EXPECT_EQ(decoder.decodeItem(item), SF::DecodeError::OK);
    expectBareItemValue<SF::Decimal>(
        item.bareItem, SF::BareItem::Type::DECIMAL, SF::Decimal{4500});
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("foo123/456:bar");
    EXPECT_EQ(decoder.decodeItem(item), SF::DecodeError::OK);
    expectBareItemValue<std::string>(
        item.bareItem, SF::BareItem::Type::TOKEN, "foo123/456:bar");
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder(":aGVsbG8=:");
    EXPECT_EQ(decoder.decodeItem(item), SF::DecodeError::OK);
    expectBareItemValue<std::string>(
        item.bareItem, SF::BareItem::Type::BYTE_SEQUENCE, "hello");
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("@1659578233");
    EXPECT_EQ(decoder.decodeItem(item), SF::DecodeError::OK);
    expectBareItemValue<int64_t>(
        item.bareItem, SF::BareItem::Type::DATE, 1659578233);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder(
        "%\"This is intended for display to %c3%bcsers.\"");
    EXPECT_EQ(decoder.decodeItem(item), SF::DecodeError::OK);
    expectBareItemValue<std::string>(
        item.bareItem,
        SF::BareItem::Type::DISPLAY_STRING,
        std::string("This is intended for display to \xC3\xBC"
                    "sers."));
  }
}

TEST(StructuredFieldsRfc9651Test, DecodeItem_InvalidBareItems_FailsStrictly) {
  // RFC 9651, Sections 1.1 and 4.2. Cases are drawn from
  // httpwg/structured-field-tests item.json, number.json, binary.json, and
  // display-string.json.
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("1000000000000000");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("4.5000");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("@1.0");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("%\"bad %C3%BC\"");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder("*aGVsbG8=*");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder(":=aGVsbG8=:");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder(":a=GVsbG8=:");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
  {
    SF::Item item;
    StructuredFieldsDecoder decoder(":_-Ah:");
    EXPECT_NE(decoder.decodeItem(item), SF::DecodeError::OK);
  }
}

TEST(StructuredFieldsRfc9651Test,
     HttpwgStructuredFieldTests_ParseAndCanonicalize) {
  // These fixtures are imported from httpwg/structured-field-tests for RFC
  // 9651. For valid fields, RFC 9651 Section 4 parsing must round-trip through
  // Section 4 serialization to the fixture's canonical value.
  const std::vector<std::string> fixtureFiles = {
      "binary.json",
      "boolean.json",
      "date.json",
      "dictionary.json",
      "display-string.json",
      "examples.json",
      "item.json",
      "key-generated.json",
      "large-generated.json",
      "list.json",
      "listlist.json",
      "number-generated.json",
      "number.json",
      "param-dict.json",
      "param-list.json",
      "param-listlist.json",
      "string-generated.json",
      "string.json",
      "token-generated.json",
      "token.json",
  };

  for (const auto& fixtureFile : fixtureFiles) {
    const auto testRecords = folly::parseJson(readFixture(fixtureFile));
    for (const auto& record : testRecords) {
      SCOPED_TRACE(fixtureFile + ": " + record["name"].asString());
      const auto input = combineFieldLines(record["raw"]);
      const auto result =
          parseAndSerialize(record["header_type"].asString(), input);

      if (optionalBool(record, "must_fail")) {
        EXPECT_NE(result.decodeError, SF::DecodeError::OK);
        continue;
      }

      if (optionalBool(record, "can_fail") &&
          result.decodeError != SF::DecodeError::OK) {
        continue;
      }

      EXPECT_EQ(result.decodeError, SF::DecodeError::OK);
      EXPECT_EQ(result.encodeError, SF::EncodeError::OK);
      EXPECT_EQ(result.serialized, expectedCanonicalValue(record));
    }
  }
}

TEST(StructuredFieldsRfc9651Test, HttpwgStructuredFieldTests_Serialize) {
  // These are the RFC 9651 serializer-only fixtures from
  // httpwg/structured-field-tests/serialisation-tests. Decimal JSON values are
  // rounded into this API's fixed-thousandths Decimal representation before
  // serialization.
  const std::vector<std::string> fixtureFiles = {
      "serialisation-tests/key-generated.json",
      "serialisation-tests/number.json",
      "serialisation-tests/string-generated.json",
      "serialisation-tests/token-generated.json",
  };

  for (const auto& fixtureFile : fixtureFiles) {
    const auto testRecords = folly::parseJson(readFixture(fixtureFile));
    for (const auto& record : testRecords) {
      SCOPED_TRACE(fixtureFile + ": " + record["name"].asString());
      const auto result = serializeFixtureExpected(
          record["header_type"].asString(), record["expected"]);

      if (optionalBool(record, "must_fail")) {
        EXPECT_NE(result.encodeError, SF::EncodeError::OK);
        continue;
      }

      EXPECT_EQ(result.encodeError, SF::EncodeError::OK);
      EXPECT_EQ(result.serialized, combineFieldLines(record["canonical"]));
    }
  }
}

TEST(StructuredFieldsRfc9651Test, EncodeItem_Rfc9651BareItems_Canonicalizes) {
  // RFC 9651, Section 4.1.3.
  {
    SF::Item item{.bareItem = SF::BareItem::decimal(SF::Decimal{-400}),
                  .parameters = {}};
    StructuredFieldsEncoder encoder;
    EXPECT_EQ(encoder.encodeItem(item), SF::EncodeError::OK);
    EXPECT_EQ(encoder.get(), "-0.4");
  }
  {
    SF::Item item{.bareItem = SF::BareItem::byteSequence("hello"),
                  .parameters = {}};
    StructuredFieldsEncoder encoder;
    EXPECT_EQ(encoder.encodeItem(item), SF::EncodeError::OK);
    EXPECT_EQ(encoder.get(), ":aGVsbG8=:");
  }
  {
    SF::Item item{.bareItem = SF::BareItem::displayString(
                      std::string("This is intended for display to \xC3\xBC"
                                  "sers.")),
                  .parameters = {}};
    StructuredFieldsEncoder encoder;
    EXPECT_EQ(encoder.encodeItem(item), SF::EncodeError::OK);
    EXPECT_EQ(encoder.get(),
              "%\"This is intended for display to %c3%bcsers.\"");
  }
}

TEST(StructuredFieldsRfc9651Test, DecodeItem_Parameters_AreOrderedLastWins) {
  // RFC 9651, Sections 3.1.2 and 4.2.3.2.
  SF::Item item;
  StructuredFieldsDecoder decoder("5;foo=bar;flag;b=?0;foo=\"last\"");
  EXPECT_EQ(decoder.decodeItem(item), SF::DecodeError::OK);

  EXPECT_EQ(item.parameters.size(), 3);
  EXPECT_EQ(item.parameters.entries()[0].key, "foo");
  EXPECT_EQ(item.parameters.entries()[1].key, "flag");
  EXPECT_EQ(item.parameters.entries()[2].key, "b");

  ASSERT_NE(item.parameters.get("foo"), nullptr);
  expectBareItemValue<std::string>(
      *item.parameters.get("foo"), SF::BareItem::Type::STRING, "last");

  ASSERT_NE(item.parameters.get("flag"), nullptr);
  expectBareItemValue<bool>(
      *item.parameters.get("flag"), SF::BareItem::Type::BOOLEAN, true);

  ASSERT_NE(item.parameters.get("b"), nullptr);
  expectBareItemValue<bool>(
      *item.parameters.get("b"), SF::BareItem::Type::BOOLEAN, false);

  StructuredFieldsEncoder encoder;
  EXPECT_EQ(encoder.encodeItem(item), SF::EncodeError::OK);
  EXPECT_EQ(encoder.get(), "5;foo=\"last\";flag;b=?0");
}

TEST(StructuredFieldsRfc9651Test, DecodeList_InnerLists_ParseAndRoundTrip) {
  // RFC 9651, Sections 3.1.1, 4.1.1, and 4.2.1.
  const std::string input = "(\"foo\";a=1;b=2);lvl=5, (bar baz);lvl=1, ()";
  SF::List list;
  StructuredFieldsDecoder decoder(input);
  EXPECT_EQ(decoder.decodeList(list), SF::DecodeError::OK);

  ASSERT_EQ(list.size(), 3);
  ASSERT_TRUE(std::holds_alternative<SF::InnerList>(list[0]));
  const auto& first = std::get<SF::InnerList>(list[0]);
  ASSERT_EQ(first.items.size(), 1);
  expectBareItemValue<std::string>(
      first.items[0].bareItem, SF::BareItem::Type::STRING, "foo");
  ASSERT_NE(first.items[0].parameters.get("a"), nullptr);
  expectBareItemValue<int64_t>(
      *first.items[0].parameters.get("a"), SF::BareItem::Type::INTEGER, 1);
  ASSERT_NE(first.parameters.get("lvl"), nullptr);
  expectBareItemValue<int64_t>(
      *first.parameters.get("lvl"), SF::BareItem::Type::INTEGER, 5);

  ASSERT_TRUE(std::holds_alternative<SF::InnerList>(list[2]));
  EXPECT_TRUE(std::get<SF::InnerList>(list[2]).items.empty());

  StructuredFieldsEncoder encoder;
  EXPECT_EQ(encoder.encodeList(list), SF::EncodeError::OK);
  EXPECT_EQ(encoder.get(), input);
}

TEST(StructuredFieldsRfc9651Test,
     DecodeDictionary_Rfc9651Semantics_RoundTrips) {
  // RFC 9651, Sections 3.2, 4.1.2, and 4.2.2.
  // Includes the duplicate-key case from httpwg/structured-field-tests
  // dictionary.json.
  const std::string input = "a=?0, b, c;foo=bar, a=5, d=(1 2);valid";
  SF::Dictionary dictionary;
  StructuredFieldsDecoder decoder(input);
  EXPECT_EQ(decoder.decodeDictionary(dictionary), SF::DecodeError::OK);

  EXPECT_EQ(dictionary.size(), 4);
  EXPECT_EQ(dictionary.entries()[0].key, "a");
  ASSERT_NE(dictionary.get("a"), nullptr);
  ASSERT_TRUE(std::holds_alternative<SF::Item>(*dictionary.get("a")));
  const auto& a = std::get<SF::Item>(*dictionary.get("a"));
  expectBareItemValue<int64_t>(a.bareItem, SF::BareItem::Type::INTEGER, 5);

  ASSERT_NE(dictionary.get("b"), nullptr);
  ASSERT_TRUE(std::holds_alternative<SF::Item>(*dictionary.get("b")));
  const auto& b = std::get<SF::Item>(*dictionary.get("b"));
  expectBareItemValue<bool>(b.bareItem, SF::BareItem::Type::BOOLEAN, true);

  ASSERT_NE(dictionary.get("d"), nullptr);
  ASSERT_TRUE(std::holds_alternative<SF::InnerList>(*dictionary.get("d")));
  const auto& d = std::get<SF::InnerList>(*dictionary.get("d"));
  ASSERT_NE(d.parameters.get("valid"), nullptr);
  expectBareItemValue<bool>(
      *d.parameters.get("valid"), SF::BareItem::Type::BOOLEAN, true);

  StructuredFieldsEncoder encoder;
  EXPECT_EQ(encoder.encodeDictionary(dictionary), SF::EncodeError::OK);
  EXPECT_EQ(encoder.get(), "a=5, b, c;foo=bar, d=(1 2);valid");
}

TEST(StructuredFieldsRfc9651Test, EmptyContainers_Rfc9651_OmitFieldValue) {
  // RFC 9651, Sections 3.1, 3.2, and 4.1. Cases are drawn from
  // httpwg/structured-field-tests list.json and dictionary.json.
  {
    SF::List list;
    StructuredFieldsDecoder decoder("");
    EXPECT_EQ(decoder.decodeList(list), SF::DecodeError::OK);
    EXPECT_TRUE(list.empty());

    StructuredFieldsEncoder encoder;
    EXPECT_EQ(encoder.encodeList(list), SF::EncodeError::OK);
    EXPECT_EQ(encoder.get(), "");
  }
  {
    SF::Dictionary dictionary;
    StructuredFieldsDecoder decoder("");
    EXPECT_EQ(decoder.decodeDictionary(dictionary), SF::DecodeError::OK);
    EXPECT_TRUE(dictionary.empty());

    StructuredFieldsEncoder encoder;
    EXPECT_EQ(encoder.encodeDictionary(dictionary), SF::EncodeError::OK);
    EXPECT_EQ(encoder.get(), "");
  }
}

TEST(StructuredFieldsRfc9651Test, DecodeWebBotAuth_CurrentFixtures_Parse) {
  // Web Bot Auth draft Appendix A.2.2, using RFC 9651 Structured Fields.
  {
    const std::string input = "agent2=\"https://signature-agent.test\"";
    SF::Dictionary signatureAgent;
    StructuredFieldsDecoder decoder(input);
    EXPECT_EQ(decoder.decodeDictionary(signatureAgent), SF::DecodeError::OK);
    ASSERT_NE(signatureAgent.get("agent2"), nullptr);
    ASSERT_TRUE(
        std::holds_alternative<SF::Item>(*signatureAgent.get("agent2")));
    const auto& agent = std::get<SF::Item>(*signatureAgent.get("agent2"));
    expectBareItemValue<std::string>(agent.bareItem,
                                     SF::BareItem::Type::STRING,
                                     "https://signature-agent.test");
  }

  {
    const std::string input =
        "sig2=(\"@authority\" \"signature-agent\";key=\"agent2\")"
        ";created=1735689600"
        ";keyid=\"poqkLGiymh_W0uP6PZFw-dvez3QJT5SolqXBCW38r0U\""
        ";alg=\"ed25519\""
        ";expires=4889289600"
        ";nonce=\"XeP72svPKNiGEg3aDE7WJuTpN69H08oMFqC8NLFy1MptpENAT3WZTYwK+"
        "MYdsFMlaqHCJGo9ZAhqer1NWY9Epg==\""
        ";tag=\"web-bot-auth\"";
    SF::Dictionary signatureInput;
    StructuredFieldsDecoder decoder(input);
    EXPECT_EQ(decoder.decodeDictionary(signatureInput), SF::DecodeError::OK);
    ASSERT_NE(signatureInput.get("sig2"), nullptr);
    ASSERT_TRUE(
        std::holds_alternative<SF::InnerList>(*signatureInput.get("sig2")));
    const auto& sig2 = std::get<SF::InnerList>(*signatureInput.get("sig2"));
    ASSERT_EQ(sig2.items.size(), 2);
    expectBareItemValue<std::string>(
        sig2.items[0].bareItem, SF::BareItem::Type::STRING, "@authority");
    expectBareItemValue<std::string>(
        sig2.items[1].bareItem, SF::BareItem::Type::STRING, "signature-agent");
    ASSERT_NE(sig2.items[1].parameters.get("key"), nullptr);
    expectBareItemValue<std::string>(*sig2.items[1].parameters.get("key"),
                                     SF::BareItem::Type::STRING,
                                     "agent2");
    ASSERT_NE(sig2.parameters.get("tag"), nullptr);
    expectBareItemValue<std::string>(*sig2.parameters.get("tag"),
                                     SF::BareItem::Type::STRING,
                                     "web-bot-auth");

    StructuredFieldsEncoder encoder;
    EXPECT_EQ(encoder.encodeDictionary(signatureInput), SF::EncodeError::OK);
    EXPECT_EQ(encoder.get(), input);
  }

  {
    const std::string input =
        "sig2=:"
        "DGiW2ErlQh0hc8wY2FQdbnFd6CEmonyY8nlvECIJFaUSYYNvNvSsGyP99BUGtq51gA4ouX"
        "lkUwjnta084bpjCg==:";
    SF::Dictionary signature;
    StructuredFieldsDecoder decoder(input);
    EXPECT_EQ(decoder.decodeDictionary(signature), SF::DecodeError::OK);
    ASSERT_NE(signature.get("sig2"), nullptr);
    ASSERT_TRUE(std::holds_alternative<SF::Item>(*signature.get("sig2")));
    const auto& item = std::get<SF::Item>(*signature.get("sig2"));
    EXPECT_EQ(item.bareItem.type(), SF::BareItem::Type::BYTE_SEQUENCE);

    StructuredFieldsEncoder encoder;
    EXPECT_EQ(encoder.encodeDictionary(signature), SF::EncodeError::OK);
    EXPECT_EQ(encoder.get(), input);
  }
}

TEST(StructuredFieldsRfc9651Test, DecodeWebBotAuth_LegacyAgentFixture_Parses) {
  // Web Bot Auth Appendix A.2.3 keeps this legacy sf-string example.
  SF::Item signatureAgent;
  StructuredFieldsDecoder decoder("\"https://signature-agent.test\"");
  EXPECT_EQ(decoder.decodeItem(signatureAgent), SF::DecodeError::OK);
  expectBareItemValue<std::string>(signatureAgent.bareItem,
                                   SF::BareItem::Type::STRING,
                                   "https://signature-agent.test");
}

TEST(StructuredFieldsRfc9651Test, LegacyStructuredHeadersApi_StillParsesDraft) {
  {
    StructuredHeaderItem item;
    const std::string input = "*Zm9vZA==*";
    StructuredHeadersDecoder decoder(input);
    EXPECT_EQ(decoder.decodeItem(item), StructuredHeaders::DecodeError::OK);
    EXPECT_EQ(item.tag, StructuredHeaderItem::Type::BINARYCONTENT);
    EXPECT_EQ(item, std::string("food"));
  }
  {
    ParameterisedList list;
    const std::string input = "abc_123;a=1;b=2";
    StructuredHeadersDecoder decoder(input);
    EXPECT_EQ(decoder.decodeParameterisedList(list),
              StructuredHeaders::DecodeError::OK);
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].identifier, "abc_123");
    EXPECT_EQ(list[0].parameterMap["a"], int64_t(1));
    EXPECT_EQ(list[0].parameterMap["b"], int64_t(2));
  }
}

}} // namespace proxygen
