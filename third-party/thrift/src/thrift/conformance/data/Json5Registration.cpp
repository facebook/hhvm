/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/conformance/data/Json5Registration.h>

#include <type_traits>

#include <boost/mp11.hpp>

#include <thrift/conformance/cpp2/AnyStructSerializer.h>
#include <thrift/conformance/cpp2/internal/AnyStructSerializer.h>
#include <thrift/conformance/data/internal/TestGenerator.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/detail/Json5ProtocolReader.h>
#include <thrift/lib/cpp2/protocol/detail/Json5ProtocolWriter.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/test/testset/Testset.h>

namespace mp11 = boost::mp11;
using apache::thrift::test::testset::FieldModifier;

namespace apache::thrift::conformance::detail {

// Uses the unconditional `detail` reader/writer headers rather than the
// THRIFT_HAS_JSON5_PROTOCOL-gated Json5Protocol.h facade, which the xplat
// generator build never sees.
template <>
struct ProtocolHelper<StandardProtocol::Json5> {
  using reader = apache::thrift::json5::detail::Json5ProtocolReader;
  using writer = apache::thrift::json5::detail::Json5ProtocolWriter;
};

} // namespace apache::thrift::conformance::detail

namespace apache::thrift::conformance::data {
namespace {

template <class T>
void registerJson5(AnyRegistry& registry) {
  registry.registerSerializer<T>(
      &getAnyStandardSerializer<T, StandardProtocol::Json5>());
}

template <typename ElementTag>
void registerVariants(AnyRegistry& registry) {
  using namespace apache::thrift::test::testset::detail;
  registerJson5<typename struct_ByFieldType<ElementTag, mod_set<>>::type>(
      registry);
  registerJson5<typename struct_ByFieldType<
      ElementTag,
      mod_set<FieldModifier::Optional>>::type>(registry);
  registerJson5<typename struct_ByFieldType<
      ElementTag,
      mod_set<FieldModifier::Terse>>::type>(registry);
  registerJson5<typename union_ByFieldType<ElementTag, mod_set<>>::type>(
      registry);
}

} // namespace

void registerRoundTripJson5Serializers(AnyRegistry& registry) {
  registerJson5<protocol::Value>(registry);

  // Must match the type set iterated by addRoundTripToSuite, or generated cases
  // will have no matching serializer.
  mp11::mp_for_each<detail::PrimaryTypeTags>([&](auto element_tag) {
    using ElementTag = decltype(element_tag);
    registerVariants<ElementTag>(registry);
    if constexpr (!std::is_same_v<ElementTag, type::bool_t>) {
      registerVariants<type::list<ElementTag>>(registry);
    }
    mp11::mp_for_each<detail::KeyTypeTags>([&](auto key_tag) {
      using KeyTag = decltype(key_tag);
      registerVariants<type::map<KeyTag, ElementTag>>(registry);
    });
  });
  mp11::mp_for_each<detail::KeyTypeTags>([&](auto key_tag) {
    using KeyTag = decltype(key_tag);
    registerVariants<type::set<KeyTag>>(registry);
  });
}

} // namespace apache::thrift::conformance::data
