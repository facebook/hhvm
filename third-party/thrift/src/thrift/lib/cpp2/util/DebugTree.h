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

#include <folly/ExceptionString.h>
#include <folly/logging/xlog.h>
#include <thrift/common/detail/string.h>
#include <thrift/common/tree_printer.h>
#include <thrift/lib/cpp2/op/PatchTraits.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/thrift/detail/DynamicPatch.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_types.h>

namespace apache::thrift::detail {

using TypeRef = apache::thrift::syntax_graph::TypeRef;
using scope = apache::thrift::tree_printer::scope;

// TypeRef needs to be optional so that we can handle the case if we don't have
// TypeRef information, e.g., if field id does not exist in the structure.
using OptionalTypeRef = std::optional<TypeRef>;

// Strongly Typed URI
struct Uri {
  explicit Uri(std::string uri) : uri(std::move(uri)) {}
  std::string uri;
};

// Find TypeRef from URI.
// This is designed to pretty-print Thrift.Any when we have URI and we need to
// get the field name from field id.
class TypeFinder {
 public:
  template <class T>
  TypeFinder& add() {
    return add(SchemaRegistry::get().getDefinitionNode<T>().program());
  }
  TypeFinder& add(const syntax_graph::ProgramNode& node);
  OptionalTypeRef findType(const Uri& uri) const;
  OptionalTypeRef findTypeInAny(const type::Type& type) const;

 private:
  std::unordered_map<std::string, TypeRef> uriToType_;
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
//   debugTree(value, TypeFinder, TypeRef);
//
// If schema does not match the data (e.g., out-dated), we will still try to
// return what we have (e.g., if we can't get field name, we might still return
// field id).
//
// The result is printable, so that users can write
//
//   LOG(INFO) << debugTree(value, typeFinder, typeRef);
//
template <class T>
scope debugTree(
    const T& t, const TypeFinder& finder, const OptionalTypeRef& ref) {
  return DebugTree<T>{}(t, finder, ref);
}

template <class T>
scope debugTree(const T& t, const TypeFinder& finder, const Uri& uri) {
  return debugTree(t, finder, finder.findType(uri));
}

template <class T>
scope debugTree(const T& t, const TypeFinder& finder, const type::Type& type) {
  return debugTree(t, finder, finder.findTypeInAny(type));
}

template <class T>
scope debugTree(const T& t, const TypeFinder& finder) {
  return debugTree(t, finder, OptionalTypeRef{});
}

template <class T, class... Args>
scope debugTree(
    const T& t, const syntax_graph::SyntaxGraph& graph, Args&&... args) {
  TypeFinder finder;
  for (const auto& p : graph.programs()) {
    finder.add(*p);
  }
  return debugTree(t, finder, std::forward<Args>(args)...);
}

template <class T>
scope debugTree(const T& t) {
  DebugTree<T> impl;
  if constexpr (__FBTHRIFT_IS_VALID(impl, impl(t))) {
    return impl(t);
  }
  return impl(t, TypeFinder{}, OptionalTypeRef{});
}

template <>
struct DebugTree<bool> {
  scope operator()(bool b, const TypeFinder&, const OptionalTypeRef&) {
    return scope::make_root("{}", b ? "true" : "false");
  }
};

template <class T>
struct DebugTree<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
  scope operator()(T v, const TypeFinder&, const OptionalTypeRef&) {
    return scope::make_root("{}", v);
  }
};

template <>
struct DebugTree<std::string> {
  scope operator()(
      const std::string& buf, const TypeFinder&, const OptionalTypeRef&);
};

template <>
struct DebugTree<folly::IOBuf> {
  scope operator()(
      const folly::IOBuf& buf, const TypeFinder&, const OptionalTypeRef&);
};

template <>
struct DebugTree<protocol::ValueList> {
  scope operator()(
      const protocol::ValueList&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::ValueSet> {
  scope operator()(
      const protocol::ValueSet&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::ValueMap> {
  scope operator()(
      const protocol::ValueMap&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::Value> {
  scope operator()(
      const protocol::Value&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::Object> {
  scope operator()(
      const protocol::Object&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<type::AnyStruct> {
  scope operator()(
      const type::AnyStruct&, const TypeFinder&, const OptionalTypeRef&);
};

template <>
struct DebugTree<op::BoolPatch> {
  scope operator()(
      const op::BoolPatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::BytePatch> {
  scope operator()(
      const op::BytePatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::I16Patch> {
  scope operator()(
      const op::I16Patch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::I32Patch> {
  scope operator()(
      const op::I32Patch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::I64Patch> {
  scope operator()(
      const op::I64Patch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::FloatPatch> {
  scope operator()(
      const op::FloatPatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::DoublePatch> {
  scope operator()(
      const op::DoublePatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::StringPatch> {
  scope operator()(
      const op::StringPatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<op::BinaryPatch> {
  scope operator()(
      const op::BinaryPatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicListPatch> {
  scope operator()(
      const protocol::DynamicListPatch&,
      const TypeFinder&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicSetPatch> {
  scope operator()(
      const protocol::DynamicSetPatch&,
      const TypeFinder&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicMapPatch> {
  scope operator()(
      const protocol::DynamicMapPatch&,
      const TypeFinder&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicStructPatch> {
  scope operator()(
      const protocol::DynamicStructPatch&,
      const TypeFinder&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicUnionPatch> {
  scope operator()(
      const protocol::DynamicUnionPatch&,
      const TypeFinder&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicUnknownPatch> {
  scope operator()(
      const protocol::DynamicUnknownPatch&,
      const TypeFinder&,
      const OptionalTypeRef&);
};
template <>
struct DebugTree<op::AnyPatch> {
  scope operator()(
      const op::AnyPatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <>
struct DebugTree<protocol::DynamicPatch> {
  scope operator()(
      const protocol::DynamicPatch&, const TypeFinder&, const OptionalTypeRef&);
};
template <class T>
struct DebugTree<T, std::enable_if_t<op::is_patch_v<T>>> {
  scope operator()(const T& t, const TypeFinder& finder, OptionalTypeRef ref) {
    auto patch = protocol::DynamicPatch::fromObject(t.toObject());
    if (!ref) {
      ref = finder.findType(Uri{apache::thrift::uri<typename T::value_type>()});
    }
    return debugTree(patch, finder, ref);
  }
  scope operator()(const T& t) {
    return debugTree(t, TypeFinder{}.add<typename T::value_type>());
  }
};
template <class T>
struct DebugTree<T, std::enable_if_t<is_thrift_class_v<T>>> {
  scope operator()(const T& t, const TypeFinder& finder, OptionalTypeRef ref) {
    auto value = protocol::asValueStruct<type::infer_tag<T>>(t);
    if (!ref) {
      ref = finder.findType(Uri{apache::thrift::uri<T>()});
    }
    return debugTree(value, finder, ref);
  }
  scope operator()(const T& t) { return debugTree(t, TypeFinder{}.add<T>()); }
};

} // namespace apache::thrift::detail

namespace apache::thrift::util {

template <class... Args>
tree_printer::scope debugTree(Args&&... args) try {
  return apache::thrift::detail::debugTree(std::forward<Args>(args)...);
} catch (std::exception& e) {
  // We practice defensive programming to swallow any exceptions so that we
  // don't crash user's code during logging.
  auto msg = folly::exceptionStr(e);
  XLOG(DFATAL) << msg;
  return tree_printer::scope::make_root(
      "[Failed to print, error: {}]", apache::thrift::detail::escape(msg));
}
} // namespace apache::thrift::util
