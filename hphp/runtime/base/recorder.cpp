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

#include "hphp/runtime/base/recorder.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <folly/Random.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"

namespace HPHP {

namespace {
  static std::unordered_map<std::uintptr_t, std::string> nativeFuncNames;
} // namespace

void Recorder::requestExit() {
  if (UNLIKELY(m_enabled)) {
    const auto dir{FileUtil::expandUser(RO::EvalRecordDir) + '/'};
    FileUtil::mkdir(dir);
    const auto file{dir + std::to_string(folly::Random::rand64()) + ".hhvm"};
    BlobEncoder encoder;
    toArray().serde(encoder);
    const auto data{encoder.take()};
    std::ofstream ofs{file, std::ios::binary};
    ofs.write(data.data(), data.size());
    m_enabled = false;
    m_nativeCalls.clear();
  }
}

void Recorder::requestInit() {
  m_enabled = folly::Random::oneIn64(RO::EvalRecordSampleRate);
}

void Recorder::setEntryPoint(const String& entryPoint) {
  m_entryPoint = entryPoint;
}

void Recorder::addNativeArg(std::uintptr_t id, Variant arg) {
  static auto& recorder{g_context->m_recorder};
  recorder.m_nativeCalls[id].back().second.append(arg);
}

void Recorder::addNativeCall(std::uintptr_t id, Variant ret) {
  static auto& recorder{g_context->m_recorder};
  auto& call{recorder.m_nativeCalls[id].emplace_back()};
  call.first = ret;
  call.second = Array::CreateVec();
}

void Recorder::addNativeFuncName(std::uintptr_t id, const char* name) {
  nativeFuncNames[id] = name;
}

bool Recorder::isEnabled() {
  return g_context->m_recorder.m_enabled;
}

Array Recorder::toArray() const {
  DictInit files{g_context->m_evaledFiles.size()};
  for (const auto& [k, _] : g_context->m_evaledFiles) {
    const auto path{k->data()};
    std::ostringstream oss;
    oss << std::ifstream{path}.rdbuf();
    files.set(StringData::Make(path), oss.str());
  }
  DictInit nativeCalls{m_nativeCalls.size()};
  for (const auto& [id, calls] : m_nativeCalls) {
    VecInit nameCalls{calls.size()};
    for (const auto&[ret, args] : calls) {
      nameCalls.append(make_vec_array(ret, args));
    }
    nativeCalls.set(String{nativeFuncNames[id]}, nameCalls.toVariant());
  }
  return make_dict_array(
    "header", make_dict_array(
      "compilerId", compilerId(),
      "entryPoint", m_entryPoint
    ),
    "files", files.toArray(),
    "nativeCalls", nativeCalls.toArray()
  );
}

template<>
Variant Recorder::toVariant(std::nullptr_t) {
  return init_null();
}

template<>
Variant Recorder::toVariant(bool value) {
  return value;
}

template<>
Variant Recorder::toVariant(double value) {
  return value;
}

template<>
Variant Recorder::toVariant(std::int64_t value) {
  return value;
}

template<>
Variant Recorder::toVariant(Array value) {
  return value;
}

template<>
Variant Recorder::toVariant(const Class* value) {
  return const_cast<Class*>(value);
}

template<>
Variant Recorder::toVariant(Native::ArrayArg value) {
  return Variant{value.m_px};
}

template<>
Variant Recorder::toVariant(Native::ObjectArg value) {
  return Variant{value.m_px};
}

template<>
Variant Recorder::toVariant(Native::StringArg value) {
  return Variant{value.m_px};
}

template<>
Variant Recorder::toVariant(Object value) {
  return value;
}

template<>
Variant Recorder::toVariant(ObjectData* value) {
  return Variant{value};
}

template<>
Variant Recorder::toVariant(Resource value) {
  return value;
}

template<>
Variant Recorder::toVariant(String value) {
  return value;
}

template<>
Variant Recorder::toVariant(TypedValue value) {
  return Variant::wrap(value);
}

template<>
Variant Recorder::toVariant(Variant value) {
  return value;
}

} // namespace HPHP
