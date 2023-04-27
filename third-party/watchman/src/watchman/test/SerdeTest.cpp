/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Serde.h"
#include <folly/portability/GTest.h>

namespace {
using namespace watchman;

template <typename T>
struct HasField : public serde::Object {
  T field;

  template <typename X>
  void map(X& x) {
    x("field", field);
  }
};

TEST(SerdeTest, map_boolean) {
  HasField<bool> s;
  s.field = false;

  auto o = serde::encode(s);
  EXPECT_EQ(JSON_OBJECT, o.type());
  EXPECT_EQ(1, o.object().size());
  auto field = o.object().at("field");
  EXPECT_EQ(false, field.asBool());

  s.field = true;
  o = serde::encode(s);
  field = o.object().at("field");
  EXPECT_EQ(true, field.asBool());
}

template <typename T>
void roundtrip(const T& value) {
  HasField<T> s;
  s.field = value;

  auto result = serde::decode<HasField<T>>(serde::encode(s));
  EXPECT_EQ(result.field, value);
}

TEST(SerdeTest, integer_roundtrip) {
  roundtrip<bool>(false);
  roundtrip<bool>(true);

  roundtrip<char>(100);
  roundtrip<signed char>(100);
  roundtrip<unsigned char>(100);
  roundtrip<signed short>(100);
  roundtrip<unsigned short>(100);
  roundtrip<signed int>(100);
  roundtrip<unsigned int>(100);
  roundtrip<signed long>(100);
  roundtrip<unsigned long>(100);
  roundtrip<signed long long>(100);
  roundtrip<unsigned long long>(100);

  roundtrip<int8_t>(100);
  roundtrip<uint8_t>(100);
  roundtrip<int16_t>(100);
  roundtrip<uint16_t>(100);
  roundtrip<int32_t>(100);
  roundtrip<uint32_t>(100);
  roundtrip<int64_t>(100);
  roundtrip<uint64_t>(100);
}

TEST(SerdeTest, array_roundtrip) {
  roundtrip<std::vector<int>>(std::vector<int>{10, 20, -30});
}

TEST(SerdeTest, map_roundtrip) {
  roundtrip<std::map<w_string, bool>>(
      std::map<w_string, bool>{{"foo", false}, {"bar", true}});
}

TEST(SerdeTest, missing_keys_are_okay) {
  {
    auto decoded = serde::decode<HasField<int>>(json_object());
    EXPECT_EQ(0, decoded.field);
  }

  {
    auto decoded = serde::decode<HasField<std::string>>(json_object());
    EXPECT_EQ("", decoded.field);
  }
}

struct RequiredField : serde::Object {
  std::string str;

  template <typename X>
  void map(X& x) {
    x.required("str", str);
  }
};

TEST(SerdeTest, required_keys_are_necessary) {
  EXPECT_THROW(serde::decode<RequiredField>(json_object()), serde::MissingKey);

  auto decoded = serde::decode<RequiredField>(
      json_object({{"str", typed_string_to_json("value")}}));
  EXPECT_EQ("value", decoded.str);

  auto encoded = serde::encode(decoded);
  EXPECT_EQ(JSON_OBJECT, encoded.type());
  EXPECT_EQ(1, encoded.object().size());
  auto str = encoded.object().at("str");
  EXPECT_EQ(JSON_STRING, str.type());
  EXPECT_EQ("value", str.asString());
}

struct Base : serde::Object {
  std::string version;

  template <typename X>
  void map(X& x) {
    x("version", version);
  }
};

struct Derived : Base {
  bool yes;
  int count;

  template <typename X>
  void map(X& x) {
    Base::map(x);
    x("yes", yes);
    x("count", count);
  }
};

TEST(SerdeTest, base_class) {
  Derived d;
  d.version = "ver 15";
  d.yes = true;
  d.count = -45;

  auto o = serde::encode(d);
  auto& encoded = o.object();
  EXPECT_EQ(3, encoded.size());

  auto version = encoded.at("version");
  EXPECT_EQ(JSON_STRING, version.type());
  EXPECT_EQ("ver 15", version.asString());

  auto yes = encoded.at("yes");
  EXPECT_EQ(JSON_TRUE, yes.type());

  auto count = encoded.at("count");
  EXPECT_EQ(JSON_INTEGER, count.type());
}

struct OptionalMembers : serde::Object {
  std::optional<bool> set;
  std::optional<bool> unset;

  template <typename X>
  void map(X& x) {
    x("set", set);
    x.skip_if_default("unset", unset);
  }
};

TEST(SerdeTest, optional_field) {
  OptionalMembers members;

  {
    auto o = serde::encode(members);
    auto& encoded = o.object();
    EXPECT_EQ(1, encoded.size());
    EXPECT_EQ(JSON_NULL, encoded.at("set").type());

    OptionalMembers decoded = serde::decode<OptionalMembers>(o);
    EXPECT_EQ(members.set, decoded.set);
    EXPECT_EQ(members.unset, decoded.unset);
  }

  {
    members.set = true;
    members.unset = false;

    auto o = serde::encode(members);
    auto& encoded = o.object();
    EXPECT_EQ(2, encoded.size());
    EXPECT_EQ(JSON_TRUE, encoded.at("set").type());
    EXPECT_EQ(JSON_FALSE, encoded.at("unset").type());

    OptionalMembers decoded = serde::decode<OptionalMembers>(o);
    EXPECT_EQ(members.set, decoded.set);
    EXPECT_EQ(members.unset, decoded.unset);
  }
}

using TupleArray = serde::Array<1, int, bool>;

TEST(SerdeTest, arrays_as_tuples) {
  TupleArray ta;

  {
    // Encoding a tuple only emits the required and non-default elements.
    auto o = serde::encode(ta);
    EXPECT_EQ(JSON_ARRAY, o.type());
    auto& array = o.array();
    EXPECT_EQ(1, array.size());

    auto parsed = serde::decode<TupleArray>(o);
    EXPECT_EQ(0, std::get<0>(parsed));
    EXPECT_EQ(false, std::get<1>(parsed));
  }

  std::get<0>(ta) = 10;
  std::get<1>(ta) = true;

  {
    // Non-default elements are all emitted.
    auto o = serde::encode(ta);
    EXPECT_EQ(JSON_ARRAY, o.type());
    auto& array = o.array();
    EXPECT_EQ(2, array.size());

    auto parsed = serde::decode<TupleArray>(o);
    EXPECT_EQ(10, std::get<0>(parsed));
    EXPECT_EQ(true, std::get<1>(parsed));
  }
}

TEST(SerdeTest, longer_arrays_are_not_rejected) {
  auto j = json_array(
      {json_integer(10), json_false(), typed_string_to_json("hello world")});
  auto ta = serde::decode<TupleArray>(j);
  EXPECT_EQ(10, std::get<0>(ta));
  EXPECT_EQ(false, std::get<1>(ta));
}

struct SkippedThings : serde::Object {
  int x = 10;
  int y = 5;

  template <typename X>
  void map(X& m) {
    m.skip_if("x", x, [](const auto& x) { return x == 10; });
    m.skip_if("y", y, [](const auto& y) { return y == 5; });
  }
};

using MultipleOptionals = serde::Array<0, int, int, int>;

TEST(SerdeTest, non_defaults_after_defaults_are_encoded) {
  auto json = serde::encode(MultipleOptionals{{0, 0, 10}});
  EXPECT_EQ(JSON_ARRAY, json.type());
  auto& arr = json.array();
  EXPECT_EQ(3, arr.size());
  EXPECT_EQ(0, arr[0].asInt());
  EXPECT_EQ(0, arr[1].asInt());
  EXPECT_EQ(10, arr[2].asInt());

  json = serde::encode(MultipleOptionals{{0, 0, 0}});
  EXPECT_EQ(JSON_ARRAY, json.type());
  EXPECT_EQ(0, json.array().size());
}

TEST(SerdeTest, skip_if) {
  SkippedThings s;

  {
    auto o = serde::encode(s);
    EXPECT_EQ(JSON_OBJECT, o.type());
    EXPECT_EQ(0, o.object().size());

    auto decoded = serde::decode<SkippedThings>(o);
    EXPECT_EQ(0, decoded.x);
    EXPECT_EQ(0, decoded.y);
  }

  s.x = 0;
  s.y = 0;

  {
    auto o = serde::encode(s);
    EXPECT_EQ(JSON_OBJECT, o.type());
    auto& map = o.object();
    EXPECT_EQ(0, map.at("x").asInt());
    EXPECT_EQ(0, map.at("y").asInt());

    auto decoded = serde::decode<SkippedThings>(o);
    EXPECT_EQ(0, decoded.x);
    EXPECT_EQ(0, decoded.y);
  }

  s.x = 100;
  s.y = 5;

  {
    auto o = serde::encode(s);
    EXPECT_EQ(JSON_OBJECT, o.type());
    auto& map = o.object();
    EXPECT_EQ(1, map.size());
    EXPECT_EQ(100, map.at("x").asInt());

    auto decoded = serde::decode<SkippedThings>(o);
    EXPECT_EQ(100, decoded.x);
    // This is a bit strange, but unset keys in a JSON object always produce
    // default-initialized fields. Therefore, this is 0, not 5.
    EXPECT_EQ(0, decoded.y);
  }
}

} // namespace
