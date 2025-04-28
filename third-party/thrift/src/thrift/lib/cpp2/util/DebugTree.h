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

#pragma once

#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/patch/DynamicPatch.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/shared/tree_printer.h>

namespace apache::thrift::detail {

using SyntaxGraph = apache::thrift::schema::SyntaxGraph;
using TypeRef = apache::thrift::schema::TypeRef;
using scope = apache::thrift::tree_printer::scope;

// TypeRef needs to be optional so that we can handle the case if we don't have
// TypeRef information, e.g., if field id does not exist in the structure.
using OptionalTypeRef = std::optional<apache::thrift::schema::TypeRef>;

// Strongly Typed URI
struct Uri {
  explicit Uri(std::string uri) : uri(std::move(uri)) {}
  std::string uri;
};

// A wrapper of syntax graph with that indexes some extra data.
class SGWrapper {
 public:
  /* implicit */ SGWrapper(const SyntaxGraph& graph) : graph_(graph) {}
  decltype(auto) programs() const { return graph_.programs(); }
  OptionalTypeRef findType(const Uri& uri) const;
  OptionalTypeRef findTypeInAny(const type::Type& type) const;

 private:
  const SyntaxGraph& graph_;

  std::unordered_map<std::string, TypeRef> genUriToTypeRef() const;
  std::unordered_map<std::string, TypeRef> uriToType_ = genUriToTypeRef();
};

// We can specialize this class to support pretty-printing custom type
template <class T, class = void>
struct DebugTree {
  static_assert(folly::always_false<T>);
};

// Similar to the existing `debugString(...)`, debug tree converts thrift data
// structure to a tree-like structure for debugging.
//
// The default interface is
//
//   debugTree(value, SyntaxGraph, TypeRef);
//
// If schema does not match the data (e.g., out-dated), we will still try to
// return what we have (e.g., if we can't get field name, we might still return
// field id).
//
// The result is printable, so that users can write
//
//   LOG(INFO) << debugTree(value, syntaxGraph, typeRef);
//
template <class T>
scope debugTree(
    const T& t, const SGWrapper& graph, const OptionalTypeRef& ref) {
  return DebugTree<T>{}(t, graph, ref);
}

template <class T>
scope debugTree(const T& t, const SGWrapper& graph, const Uri& uri) {
  return debugTree(t, graph, graph.findType(uri));
}

template <class T>
scope debugTree(const T& t, const SGWrapper& graph, const type::Type& type) {
  return debugTree(t, graph, graph.findTypeInAny(type));
}

template <class T>
scope debugTree(const T& t, const SGWrapper& graph) {
  return debugTree(t, graph, OptionalTypeRef{});
}

template <>
struct DebugTree<bool> {
  scope operator()(bool b, const SGWrapper&, const OptionalTypeRef&) {
    return scope::make_root("{}", b ? "true" : "false");
  }
};

template <class T>
struct DebugTree<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
  scope operator()(T v, const SGWrapper&, const OptionalTypeRef&) {
    return scope::make_root("{}", v);
  }
};

template <>
struct DebugTree<std::string> {
  scope operator()(
      const std::string& buf, const SGWrapper&, const OptionalTypeRef&);
};

template <>
struct DebugTree<folly::IOBuf> {
  scope operator()(
      const folly::IOBuf& buf, const SGWrapper&, const OptionalTypeRef&);
};

template <>
struct DebugTree<protocol::ValueList> {
  scope operator()(
      const protocol::ValueList&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::ValueSet> {
  scope operator()(
      const protocol::ValueSet&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::ValueMap> {
  scope operator()(
      const protocol::ValueMap&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::Value> {
  scope operator()(
      const protocol::Value&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::Object> {
  scope operator()(
      const protocol::Object&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<type::AnyStruct> {
  scope operator()(
      const type::AnyStruct&, const SGWrapper&, const OptionalTypeRef&);
};

template <>
struct DebugTree<op::BoolPatch> {
  scope operator()(
      const op::BoolPatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::BytePatch> {
  scope operator()(
      const op::BytePatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::I16Patch> {
  scope operator()(
      const op::I16Patch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::I32Patch> {
  scope operator()(
      const op::I32Patch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::I64Patch> {
  scope operator()(
      const op::I64Patch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::FloatPatch> {
  scope operator()(
      const op::FloatPatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::DoublePatch> {
  scope operator()(
      const op::DoublePatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::StringPatch> {
  scope operator()(
      const op::StringPatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::BinaryPatch> {
  scope operator()(
      const op::BinaryPatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicListPatch> {
  scope operator()(
      const protocol::DynamicListPatch&,
      const SGWrapper&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicSetPatch> {
  scope operator()(
      const protocol::DynamicSetPatch&,
      const SGWrapper&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicMapPatch> {
  scope operator()(
      const protocol::DynamicMapPatch&,
      const SGWrapper&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicStructPatch> {
  scope operator()(
      const protocol::DynamicStructPatch&,
      const SGWrapper&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicUnionPatch> {
  scope operator()(
      const protocol::DynamicUnionPatch&,
      const SGWrapper&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicUnknownPatch> {
  scope operator()(
      const protocol::DynamicUnknownPatch&,
      const SGWrapper&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<op::AnyPatch> {
  scope operator()(
      const op::AnyPatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicPatch> {
  scope operator()(
      const protocol::DynamicPatch&, const SGWrapper&, const OptionalTypeRef&);
};
template <class T>
struct DebugTree<T, std::enable_if_t<op::is_patch_v<T>>> {
  scope operator()(const T& t, const SGWrapper& graph, OptionalTypeRef ref) {
    auto patch = protocol::DynamicPatch::fromObject(t.toObject());
    if (!ref) {
      ref = graph.findType(Uri{apache::thrift::uri<typename T::value_type>()});
    }
    return debugTree(patch, graph, ref);
  }
};

} // namespace apache::thrift::detail

namespace apache::thrift::util {
using apache::thrift::detail::debugTree;
}
