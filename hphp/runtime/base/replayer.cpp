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
#include <sstream>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/variable-unserializer.h"

namespace HPHP {

String Replayer::file(std::string path) {
  return m_files[String{path}].asStrRef();
}

std::string Replayer::init(std::string path) {
  std::ostringstream oss;
  oss << std::ifstream{path}.rdbuf();
  const auto str{oss.str()};
  VariableUnserializer vu{str.data(), str.size(),
    VariableUnserializer::Type::Serialize};
  auto recording{vu.unserialize().asArrRef()};
  m_files = recording[String{"files"}].asArrRef();
  m_header = recording[String{"header"}].asArrRef();
  m_nativeCalls = recording[String{"nativeCalls"}].asArrRef();
  return m_header[String{"entryPoint"}].asStrRef().toCppString();
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

std::pair<Array, Variant> Replayer::getNativeCall(NativeFuncId id) {
  static const auto& nativeCalls{g_context->m_replayer.m_nativeCalls};
  Variant call;
  nativeCalls[String{getNativeFuncNames()[id]}].asArrRef()->popMove(call);
  const auto& arr{call.asArrRef()};
  return {arr[String{"args"}].asArrRef(), arr[String{"ret"}]};
}

Replayer::NativeFuncNames& Replayer::getNativeFuncNames() {
  static NativeFuncNames nativeFuncNames;
  return nativeFuncNames;
}

} // namespace HPHP
