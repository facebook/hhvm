/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/record-replay.h"

#include <exception>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <folly/dynamic.h>
#include <folly/json.h>
#include <sys/stat.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP::rr {

namespace {
  static std::unordered_map<NativeFunction, std::string> g_nativeFuncNames;

  static const std::unordered_set<std::string> g_noRecordingPrefixes{
    "DateTime->",
    "HH\\ImmMap->",
    "HH\\ImmSet->",
    "HH\\ImmVector->",
    "HH\\Map->",
    "HH\\Set->",
    "HH\\Vector->",
    "NativeTrimmableMap->",
    "NativeTrimmableMap2->",
    "NativeTrimmableMap3->",
  };

  static const std::unordered_set<std::string> g_noRecordingSuffixes{
    "->__sleep",
    "->__wakeup",
  };
} // namespace

void addNativeFuncName(NativeFunction ptr, std::string_view name) {
  g_nativeFuncNames[ptr] = name;
}

std::string_view getNativeFuncName(NativeFunction ptr) {
  return g_nativeFuncNames.at(ptr);
}

NativeFunction getNativeFuncPtr(std::string_view name) {
  for (const auto& [ptr, n] : g_nativeFuncNames) {
    if (n == name) {
      return ptr;
    }
  }
  return nullptr;
}

bool shouldRecordReplay(NativeFunction ptr) {
  const auto name{g_nativeFuncNames[ptr]};
  for (const auto& prefix : g_noRecordingPrefixes) {
    if (name.starts_with(prefix)) {
      return false;
    }
  }
  for (const auto& suffix : g_noRecordingSuffixes) {
    if (name.ends_with(suffix)) {
      return false;
    }
  }
  VMRegAnchor _;
  auto func{vmfp()->func()};
  if (func->nativeFuncPtr() != ptr) {
    func = Func::load(StringData::Make(name));
  }
  assert(func != nullptr);
  const auto attrs{func->attrs()};
  return !((attrs & AttrNoRecording) | (attrs & AttrIsFoldable));
}

template<>
String serialize(Variant value) {
  TmpAssign _1{RO::NoticeFrequency, 0L};
  TmpAssign _2{RO::WarningFrequency, 0L};
  VariableSerializer vs{VariableSerializer::Type::DebuggerSerialize};
  try {
    return vs.serializeValue(value, true);
  } catch (...) {
    return vs.serializeValue(init_null(), true); // TODO Record the error
  }
}

template<>
String serialize(bool value) {
  return serialize<Variant>(value);
}

template<>
String serialize(const char* value) {
  return serialize<Variant>(value);
}

template<>
String serialize(double value) {
  return serialize<Variant>(value);
}

template<>
String serialize(int value) {
  return serialize<Variant>(value);
}

template<>
String serialize(folly::dynamic value) {
  return serialize<Variant>(folly::toJson(value));
}

template<>
String serialize(req::ptr<Directory> value) {
  return serialize<Variant>(value ? Resource{value} : init_null());
}

template<>
String serialize(req::ptr<File> value) {
  if (value && value->seekable()) {
    const auto pos{value->tell()};
    value->rewind();
    String contents;
    try {
      contents = value->read();
    } catch (const StringBufferLimitException&) {}
    value->seek(pos);
    if (!contents.isNull()) {
      return serialize<Variant>(contents);
    }
  }
  return serialize<Variant>(init_null());
}

template<>
String serialize(req::ptr<StreamContext> value) {
  return serialize<Variant>(value ? Resource{value} : init_null());
}

template<>
String serialize(std::exception_ptr value) {
  try {
    std::rethrow_exception(value);
  } catch (const req::root<Object>& e) {
    return serialize<Variant>(e);
  } catch (const FatalErrorException& e) {
    return serialize<Variant>(make_vec_array(
      e.getMessage(),
      e.getBacktrace(),
      e.isRecoverable(),
      e.isSilent()
    ));
  } catch (...) {
    return serialize<Variant>(init_null());
  }
}

template<>
String serialize(std::int64_t value) {
  return serialize<Variant>(value);
}

template<>
String serialize(std::nullptr_t) {
  return serialize<Variant>(init_null());
}

template<>
String serialize(std::string_view value) {
  return serialize<Variant>(value.data());
}

template<>
String serialize(struct stat* value) {
  const auto ptr{std::bit_cast<const char*>(value)};
  const String str{StringData::Make(ptr, sizeof(*value), CopyStringMode{})};
  return serialize<Variant>(str);
}

template<>
String serialize(Array value) {
  return serialize<Variant>(value);
}

template<>
String serialize(ArrayArg value) {
  return serialize(Variant{value.get()});
}

template<>
String serialize(const Class* value) {
  return serialize<Variant>(const_cast<Class*>(value));
}

template<>
String serialize(ObjectData* value) {
  if (value != nullptr && value->instanceof("Generator")) {
    return serialize(init_null());
  } else {
    return serialize(Variant{value});
  }
}

template<>
String serialize(Object value) {
  return serialize<ObjectData*>(value.get());
}

template<>
String serialize(ObjectArg value) {
  return serialize<ObjectData*>(value.get());
}

template<>
String serialize(Optional<std::filesystem::path> value) {
  return serialize<Variant>(value ? value->string() : init_null());
}

template<>
String serialize(Optional<std::string> value) {
  return serialize<Variant>(value ? *value : init_null());
}

template<>
String serialize(Optional<AutoloadMap::FileResult> value) {
  return serialize<Variant>(value ? Variant{value->path} : init_null());
}

template<>
String serialize(Resource value) {
  return serialize<Variant>(value);
}

template<>
String serialize(String value) {
  return serialize<Variant>(value);
}

template<>
String serialize(StringArg value) {
  return serialize(Variant{value.get()});
}

template<>
String serialize(TypedValue value) {
  return serialize(Variant::wrap(value));
}

template<>
Variant unserialize(const String& recordedValue) {
  TmpAssign _1{RO::NoticeFrequency, 0L};
  TmpAssign _2{RO::WarningFrequency, 0L};
  TmpAssign _3{RO::EvalCheckPropTypeHints, 0};
  return VariableUnserializer{
    recordedValue.data(),
    static_cast<std::size_t>(recordedValue.size()),
    VariableUnserializer::Type::DebuggerSerialize
  }.unserialize();
}

template<>
bool unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  always_assert_flog(variant.isBoolean(), "{}", recordedValue);
  return variant.asBooleanVal();
}

template<>
double unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  always_assert_flog(variant.isDouble(), "{}", recordedValue);
  return variant.asDoubleVal();
}

template<>
int unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  always_assert_flog(variant.isInteger(), "{}", recordedValue);
  return variant.asInt64Val();
}

template<>
req::ptr<Directory> unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isNull()) {
    return nullptr;
  } else {
    always_assert_flog(variant.isResource(), "{}", recordedValue);
    return cast<Directory>(variant.asCResRef());
  }
}

template<>
req::ptr<File> unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isNull()) {
    return nullptr;
  } else {
    always_assert_flog(variant.isString(), "{}", recordedValue);
    const auto contents{variant.asCStrRef()};
    return req::make<MemFile>(contents.c_str(), contents.size());
  }
}

template<>
std::exception_ptr unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isObject()) {
    return std::make_exception_ptr(req::root<Object>{variant.asCObjRef()});
  } else if (variant.isArray()) {
    const auto array{variant.asCArrRef()};
    always_assert_flog(array.size() == 4, "{}", array.size());
    auto exc{FatalErrorException{
      array[0].asCStrRef().toCppString(),
      array[1].asCArrRef(),
      array[2].asBooleanVal(),
    }};
    exc.setSilent(array[3].asBooleanVal());
    return std::make_exception_ptr(exc);
  } else {
    always_assert_flog(variant.isNull(), "{}", recordedValue);
    return std::make_exception_ptr(std::exception{});
  }
}

template<>
std::int64_t unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  always_assert_flog(variant.isInteger(), "{}", recordedValue);
  return variant.asInt64Val();
}

template<>
struct stat* unserialize(const String& recordedValue) {
  const auto left{recordedValue.find('"')};
  always_assert(left != String::npos);
  const auto right{recordedValue.rfind('"')};
  always_assert(right != String::npos);
  const auto size{right - left - 1};
  always_assert_flog(size == sizeof(struct stat), "{}", size);
  return std::bit_cast<struct stat*>(recordedValue.data() + left + 1);
}

template<>
void unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  always_assert_flog(variant.isNull(), "{}", recordedValue);
}

template<>
Array unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  always_assert_flog(variant.isArray(), "{}", recordedValue);
  return variant.asCArrRef();
}

template<>
Object unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isNull()) {
    return Object{};
  } else {
    always_assert_flog(variant.isObject(), "{}", recordedValue);
    return variant.asCObjRef();
  }
}

template<>
Optional<std::filesystem::path> unserialize(
    const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isNull()) {
    return std::nullopt;
  } else {
    always_assert_flog(variant.isString(), "{}", recordedValue);
    return variant.asCStrRef().toCppString();
  }
}

template<>
Optional<std::string> unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isNull()) {
    return std::nullopt;
  } else {
    always_assert_flog(variant.isString(), "{}", recordedValue);
    return variant.asCStrRef().toCppString();
  }
}

template<>
Optional<AutoloadMap::FileResult> unserialize(
    const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isNull()) {
    return std::nullopt;
  } else {
    return AutoloadMap::FileResult{variant.asCStrRef()};
  }
}

template<>
Resource unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  always_assert_flog(variant.isResource(), "{}", recordedValue);
  return variant.asCResRef();
}

template<>
String unserialize(const String& recordedValue) {
  const auto variant{unserialize<Variant>(recordedValue)};
  if (variant.isNull()) {
    return String{};
  } else {
    always_assert_flog(variant.isString(), "{}", recordedValue);
    return variant.asCStrRef();
  }
}

template<>
TypedValue unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).detach();
}

} // namespace HPHP
