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

#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

#include <folly/Random.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/build-info.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"

namespace HPHP {

void Recorder::onSessionInit() {
  if (UNLIKELY(RuntimeOption::EvalRecordReplay)) {
    m_enabled = folly::Random::oneIn64(RuntimeOption::EvalRecordSampleRate);
    m_nativeCalls.clear();
  }
}

void Recorder::onSessionExit() {
  if (UNLIKELY(m_enabled)) {
    const auto dir{FileUtil::expandUser(RuntimeOption::EvalRecordDir) + '/'};
    FileUtil::mkdir(dir);
    const auto file{dir + std::to_string(folly::Random::rand64()) + ".hhvm"};
    VariableSerializer vs{VariableSerializer::Type::Serialize};
    std::ofstream{file} << vs.serializeValue(toVariant(), false).data();
    m_enabled = false;
    m_nativeCalls.clear();
  }
}

void Recorder::setEntryPoint(std::string entryPoint) {
  getRecorder().m_entryPoint = entryPoint;
}

Recorder::NativeFuncNames& Recorder::getNativeFuncNames() {
  static NativeFuncNames nativeFuncNames;
  return nativeFuncNames;
}

Recorder& Recorder::getRecorder() {
  return RI().m_recorder;
}

Variant Recorder::toVariant() const {
  DictInit object{3};
  DictInit header{11};
  header.set(StringData::Make("build_id"), buildId().toString());
  header.set(StringData::Make("compiler_id"), compilerId().toString());
  DictInit env{static_cast<std::size_t>(g_context->getEnvs().size())};
  for (auto i{g_context->getEnvs().begin()}; i; ++i) {
    env.set(i.first().getStringData(), i.second().getStringData()->data());
  }
  header.set(StringData::Make("entryPoint"),
    RuntimeOption::ServerMode ? g_context->getRequestUrl() : m_entryPoint);
  header.set(StringData::Make("env"), env.toVariant());
  header.set(StringData::Make("hostname"), Process::GetHostName());
  header.set(StringData::Make("repo_schema_id"), repoSchemaId().toString());
  header.set(StringData::Make("request_id"), Logger::GetRequestId());
  header.set(StringData::Make("server"), RuntimeOption::ServerMode);
  header.set(StringData::Make("thread_id"), Process::GetThreadId());
  header.set(StringData::Make("thread_pid"), Process::GetThreadPid());
  header.set(StringData::Make("time_seconds"), TimeStamp::CurrentSecond());
  object.set(StringData::Make("header"), header.toVariant());
  DictInit files{g_context->m_evaledFiles.size()};
  for (const auto& [k, _] : g_context->m_evaledFiles) {
    const auto path{k->data()};
    std::ostringstream oss;
    oss << std::ifstream{path}.rdbuf();
    files.set(StringData::Make(path), oss.str());
  }
  object.set(StringData::Make("files"), files.toVariant());
  DictInit nativeCalls{m_nativeCalls.size()};
  for (const auto& [id, calls] : m_nativeCalls) {
    VecInit nativeNameCalls{calls.size()};
    for (const auto& [ret, args] : calls) {
      DictInit nativeCall{2};
      nativeCall.set(StringData::Make("ret"), ret);
      VecInit callArgs{args.size()};
      for (const auto& arg : args) {
        callArgs.append(arg);
      }
      nativeCall.set(StringData::Make("args"), callArgs.toVariant());
      nativeNameCalls.append(nativeCall.toVariant());
    }
    const auto name{StringData::Make(getNativeFuncNames()[id])};
    nativeCalls.set(name, nativeNameCalls.toVariant());
  }
  object.set(StringData::Make("nativeCalls"), nativeCalls.toVariant());
  return object.toVariant();
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
Variant Recorder::toVariant(std::nullptr_t) {
  return {};
}

template<>
Variant Recorder::toVariant(Array value) {
  return value;
}

template<>
Variant Recorder::toVariant(ArrayArg value) {
  return Variant{value.get()};
}

template<>
Variant Recorder::toVariant(Class const* value) {
  return const_cast<Class*>(value);
}

template<>
Variant Recorder::toVariant(Object value) {
  return value;
}

template<>
Variant Recorder::toVariant(ObjectArg value) {
  return Variant{value.get()};
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
Variant Recorder::toVariant(StringArg value) {
  return Variant{value.get()};
}

template<>
Variant Recorder::toVariant(StringData value) {
  return value.data();
}

template<>
Variant Recorder::toVariant(TypedValue value) {
  return Variant::wrap(value);
}

template<>
Variant Recorder::toVariant(Variant value) {
  return Variant{std::move(value), Variant::TVDup{}};
}

} // namespace HPHP
