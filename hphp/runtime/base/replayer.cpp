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

#include "hphp/runtime/base/replayer.h"

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/assertions.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"

namespace HPHP {

namespace {
  static std::unordered_map<std::uintptr_t, std::string> nativeFuncNames;
} // namespace

String Replayer::file(const String& path) {
  return m_files[path].asCStrRef();
}

String Replayer::init(const String& path) {
  std::ifstream ifs{path.data(), std::ios::binary | std::ios::ate};
  always_assert(!ifs.fail());
  std::vector<char> data(static_cast<std::size_t>(ifs.tellg()));
  ifs.seekg(0).read(data.data(), data.size());
  BlobDecoder decoder{data.data(), data.size()};
  Array recording;
  recording.serde(decoder);
  m_files = recording[String{"files"}].asCArrRef();
  m_nativeCalls = recording[String{"nativeCalls"}].asCArrRef();
  const auto header{recording[String{"header"}].asCArrRef()};
  always_assert(header[String{"compilerId"}].asCStrRef() == compilerId());
  return header[String{"entryPoint"}].asCStrRef();
}

void Replayer::addNativeFuncName(std::uintptr_t id, const char* name) {
  nativeFuncNames[id] = name;
}

template<>
void Replayer::fromVariant(const Variant& value) {
  always_assert(value.isNull());
}

template<>
bool Replayer::fromVariant(const Variant& value) {
  return value.asBooleanVal();
}

template<>
double Replayer::fromVariant(const Variant& value) {
  return value.asDoubleVal();
}

template<>
std::int64_t Replayer::fromVariant(const Variant& value) {
  return value.asInt64Val();
}

template<>
Array Replayer::fromVariant(const Variant& value) {
  return value.asCArrRef();
}

template<>
Object Replayer::fromVariant(const Variant& value) {
  return value.asCObjRef();
}

template<>
Resource Replayer::fromVariant(const Variant& value) {
  return value.asCResRef();
}

template<>
String Replayer::fromVariant(const Variant& value) {
  return value.asCStrRef();
}

template<>
TypedValue Replayer::fromVariant(const Variant& value) {
  return *value.asTypedValue();
}

template<>
Variant Replayer::fromVariant(const Variant& value) {
  return value;
}

Variant Replayer::popNativeArg(std::uintptr_t id) {
  static auto& replayer{g_context->m_replayer};
  always_assert(nativeFuncNames.count(id) > 0);
  const String name{nativeFuncNames[id]};
  always_assert(replayer.m_nativeCalls.exists(name));
  auto& calls{replayer.m_nativeCalls[name].asArrRef()};
  always_assert(!calls.empty());
  auto& call{calls[0].asArrRef()};
  always_assert(call.size() == 2);
  auto& args{call[1].asArrRef()};
  always_assert(!args.empty());
  return args.pop();
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, bool) {
  always_assert(popNativeArg(id).isBoolean());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, double) {
  always_assert(popNativeArg(id).isDouble());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, std::int64_t) {
  always_assert(popNativeArg(id).isInteger());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Array) {
  always_assert(popNativeArg(id).isArray());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, const Class*) {
  always_assert(popNativeArg(id).isClass());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Native::ArrayArg) {
  always_assert(popNativeArg(id).isArray());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Native::ObjectArg) {
  always_assert(popNativeArg(id).isObject());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Native::StringArg) {
  always_assert(popNativeArg(id).isString());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Object) {
  always_assert(popNativeArg(id).isObject());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Resource) {
  always_assert(popNativeArg(id).isResource());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, String) {
  always_assert(popNativeArg(id).isString());
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, TypedValue) {
  popNativeArg(id);
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Variant) {
  popNativeArg(id);
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, bool& arg) {
  arg = fromVariant<bool>(popNativeArg(id));
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, double& arg) {
  arg = fromVariant<double>(popNativeArg(id));
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, std::int64_t& arg) {
  arg = fromVariant<std::int64_t>(popNativeArg(id));
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Array& arg) {
  arg = fromVariant<Array>(popNativeArg(id));
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Native::ArrayArg& arg) {
  arg->decRefAndRelease();
  arg.m_px = fromVariant<Array>(popNativeArg(id)).get();
  arg.m_px->incRefCount();
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Native::ObjectArg& arg) {
  arg->decRefAndRelease();
  arg.m_px = fromVariant<Object>(popNativeArg(id)).get();
  arg->incRefCount();
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Native::StringArg& arg) {
  arg->decRefAndRelease();
  arg.m_px = fromVariant<String>(popNativeArg(id)).get();
  arg->incRefCount();
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Object& arg) {
  arg = fromVariant<Object>(popNativeArg(id));
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, ObjectData* arg) {
  const auto objectData{fromVariant<Object>(popNativeArg(id)).get()};
  arg->decRefAndRelease();
  arg->initHeader(*objectData, RefCount::OneReference);
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, String& arg) {
  arg = fromVariant<String>(popNativeArg(id));
}

template<>
void Replayer::popNativeArg(std::uintptr_t id, Variant& arg) {
  arg = popNativeArg(id);
}

Variant Replayer::popNativeCall(std::uintptr_t id) {
  static auto& replayer{g_context->m_replayer};
  always_assert(nativeFuncNames.count(id) > 0);
  const String name{nativeFuncNames[id]};
  always_assert(replayer.m_nativeCalls.exists(name));
  auto& calls{replayer.m_nativeCalls[name].asArrRef()};
  always_assert(!calls.empty());
  auto call{calls.pop().asArrRef()};
  always_assert(call.size() == 2);
  auto args{call[1].asArrRef()};
  always_assert(args.empty());
  return call[0];
}

} // namespace HPHP
