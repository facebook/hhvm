/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cctype>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <folly/Conv.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

#include "mcrouter/lib/carbon/CommonSerializationTraits.h"
#include "mcrouter/lib/carbon/Fields.h"
#include "mcrouter/lib/carbon/Keys.h"
#include "mcrouter/tools/mcpiper/PrettyFormat.h"
#include "mcrouter/tools/mcpiper/StyledString.h"
#include "mcrouter/tools/mcpiper/ValueFormatter.h"

namespace carbon {
namespace detail {

class McPiperVisitor {
 public:
  explicit McPiperVisitor(bool script, size_t indent = 0)
      : indent_(indent), script_(script) {}

  template <class T>
  bool enterMixin(size_t /* id */, folly::StringPiece /* name */, const T&) {
    return true;
  }

  bool leaveMixin() {
    return true;
  }

  template <class T>
  bool visitField(size_t /* id */, folly::StringPiece name, const T& t) {
    if (kExcuseValues.find(name.str()) == kExcuseValues.end()) {
      auto content = serialize(t);
      if (!content.empty()) {
        out_.append(prepareAndRenderHeader(name));
        out_.append(content);
      }
    }
    return true;
  }

  facebook::memcache::StyledString styled() && {
    return std::move(out_);
  }

 private:
  const std::unordered_set<std::string> kExcuseValues = {
      "value",
      "flags",
      "result",
      "key"};
  facebook::memcache::StyledString out_;
  size_t indent_{0};
  const facebook::memcache::PrettyFormat format_{};
  const bool script_{false};
  bool commaNeeded_{false};

  // key
  template <class T>
  facebook::memcache::StyledString serialize(const carbon::Keys<T>& value) {
    facebook::memcache::StyledString out;
    out.append(value.fullKey().str(), format_.dataValueColor);
    return out;
  }

  // string-like
  facebook::memcache::StyledString serialize(char value) {
    facebook::memcache::StyledString out;
    char mark = script_ ? '"' : '\'';
    out.pushBack(mark);
    out.pushBack(value, format_.dataValueColor);
    out.pushBack(mark);
    return out;
  }
  facebook::memcache::StyledString serialize(const std::string& value) {
    return serializeString(value);
  }
  facebook::memcache::StyledString serialize(const folly::IOBuf& buf) {
    auto buffer = buf;
    auto strPiece = folly::StringPiece(buffer.coalesce());
    return serializeString(strPiece);
  }

  // boolean
  facebook::memcache::StyledString serialize(const bool& b) {
    facebook::memcache::StyledString out;
    out.append(b ? "true" : "false", format_.dataValueColor);
    return out;
  }

  // numbers
  template <class T>
  std::enable_if_t<
      std::is_arithmetic<T>::value,
      facebook::memcache::StyledString>
  serialize(const T& value) {
    facebook::memcache::StyledString out;
    out.append(folly::to<std::string>(value), format_.dataValueColor);
    return out;
  }

  // enums
  template <class T>
  std::enable_if_t<std::is_enum<T>::value, facebook::memcache::StyledString>
  serialize(const T& value) {
    facebook::memcache::StyledString out;
    out.append(
        folly::to<std::string>(static_cast<std::underlying_type_t<T>>(value)),
        format_.dataValueColor);
    return out;
  }

  // optional
  template <class T>
  facebook::memcache::StyledString serialize(const folly::Optional<T>& opt) {
    if (opt.hasValue()) {
      return serialize(opt.value());
    }
    return facebook::memcache::StyledString{};
  }

  // optional_field_ref
  template <class T>
  facebook::memcache::StyledString serialize(
      apache::thrift::optional_field_ref<T&> opt) {
    if (opt.has_value()) {
      return serialize(opt.value());
    }
    return facebook::memcache::StyledString{};
  }

  // linear containers
  template <class T>
  std::enable_if_t<
      carbon::detail::IsLinearContainer<T>::value,
      facebook::memcache::StyledString>
  serialize(const T& values) {
    facebook::memcache::StyledString out;

    if (SerializationTraits<T>::size(values) == 0) {
      out.append("[]");
      return out;
    }

    out.pushBack('[');
    incrementIndentation();
    for (auto it = SerializationTraits<T>::begin(values);
         it != SerializationTraits<T>::end(values);
         ++it) {
      out.append(startNewLineAndIndent());
      out.append(serialize(*it));
    }
    decrementIndentation();
    out.append(startNewLineAndIndent());
    out.pushBack(']');

    return out;
  }

  // kv containers
  template <class T>
  std::enable_if_t<
      carbon::detail::IsKVContainer<T>::value,
      facebook::memcache::StyledString>
  serialize(const T& values) {
    facebook::memcache::StyledString out;

    if (SerializationTraits<T>::size(values) == 0) {
      out.append("{}");
      return out;
    }

    out.pushBack('{');
    incrementIndentation();
    for (auto it = SerializationTraits<T>::begin(values);
         it != SerializationTraits<T>::end(values);
         ++it) {
      out.append(startNewLineAndIndent());
      if (script_) {
        out.append(serializeString(folly::to<std::string>(it->first)));
      } else {
        out.append(serialize(it->first));
      }
      out.append(": ");
      out.append(serialize(it->second));
    }
    decrementIndentation();
    out.append(startNewLineAndIndent());
    out.pushBack('}');

    return out;
  }

  // carbon structs and carbon unions
  template <class T>
  std::enable_if_t<
      carbon::IsCarbonStruct<T>::value &&
          !carbon::IsThriftWrapperStruct<T>::value,
      facebook::memcache::StyledString>
  serialize(const T& value) {
    facebook::memcache::StyledString out;

    McPiperVisitor printer(script_, indent_ + 1);
    value.visitFields(printer);
    auto content = std::move(printer).styled();
    if (!content.empty()) {
      out.pushBack('{');

      incrementIndentation();
      out.append(content);
      decrementIndentation();

      out.append(startNewLineAndIndent());
      out.pushBack('}');
    } else {
      out.append("{}");
    }

    return out;
  }

  // thrift structs
  template <class T>
  std::enable_if_t<
      carbon::IsThriftWrapperStruct<T>::value,
      facebook::memcache::StyledString>
  serialize(const T& t) {
    facebook::memcache::StyledString out;
    out.append(serializeString(
        apache::thrift::SimpleJSONSerializer::serialize<std::string>(
            t.getThriftStruct())));
    return out;
  }

  // user type
  template <class T>
  std::enable_if_t<
      detail::IsUserReadWriteDefined<T>::value,
      facebook::memcache::StyledString>
  serialize(const T& /* value */) {
    facebook::memcache::StyledString out;
    out.append(serializeString("<User type>"));
    return out;
  }

  facebook::memcache::StyledString prepareAndRenderHeader(
      folly::StringPiece name) {
    facebook::memcache::StyledString out;
    out.append(startNewLineAndIndent());
    if (script_) {
      out.append("\"");
      out.append(name.str());
      out.append("\": ");
    } else {
      out.append(name.str(), format_.msgAttrColor);
      out.append(": ", format_.msgAttrColor);
    }
    return out;
  }

  facebook::memcache::StyledString serializeString(
      folly::StringPiece str) const {
    facebook::memcache::StyledString out;
    if (script_) {
      out.append("\"");
      if (!std::all_of<folly::StringPiece::const_iterator, int(int)>(
              str.begin(), str.end(), std::isprint)) {
        /* JSON doesn't deal with arbitrary binary data - the input string
           must be valid UTF-8.  So we just hex encode the whole string. */
        out.append(folly::hexlify(str));
      } else {
        out.append(folly::cEscape<std::string>(str));
      }
      out.append("\"");
    } else {
      out.append(folly::backslashify(str), format_.dataValueColor);
    }
    return out;
  }

  static std::string serializeIndentation(size_t indentLevel) {
    constexpr size_t kIndentationUnit = 2;
    return std::string(indentLevel * kIndentationUnit, ' ');
  }

  facebook::memcache::StyledString startNewLineAndIndent() {
    facebook::memcache::StyledString out;
    if (script_ && commaNeeded_) {
      out.pushBack(',');
    }
    out.pushBack('\n');
    out.append(serializeIndentation(indent_));

    commaNeeded_ = true;
    return out;
  }

  void incrementIndentation() {
    ++indent_;
    commaNeeded_ = false;
  }
  void decrementIndentation() {
    --indent_;
    commaNeeded_ = false;
  }
};

} // namespace detail

template <class R>
facebook::memcache::StyledString
print(const R& req, folly::StringPiece /* name */, bool script) {
  detail::McPiperVisitor printer(script, 1 /* indentation */);
  req.visitFields(printer);
  return std::move(printer).styled();
}

} // namespace carbon
