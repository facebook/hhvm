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

#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>

#include <folly/Random.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"
#include "hphp/util/exception.h"
#include "hphp/util/logger.h"

namespace HPHP {

namespace {
  static std::string g_entryPoint;
  static std::unordered_map<std::uintptr_t, std::string> g_nativeFuncNames;
} // namespace

void Recorder::onExternalThreadEventProcess(
    const c_ExternalThreadEventWaitHandle* id) {
  if (UNLIKELY(m_enabled)) {
    m_externalThreadEventProcesses.emplace_back(
      std::bit_cast<std::uintptr_t>(id));
  }
}

void Recorder::requestExit() {
  if (UNLIKELY(m_enabled)) {
    try {
      BlobEncoder encoder;
      toArray().serde(encoder);
      const auto data{encoder.take()};
      const auto dir{std::filesystem::canonical(RO::EvalRecordDir)};
      std::filesystem::create_directory(dir);
      const auto file{dir / (std::to_string(folly::Random::rand64()) + ".hhr")};
      std::ofstream ofs{file, std::ios::binary};
      ofs.write(data.data(), data.size());
    } catch (const std::exception& e) {
      Logger::FWarning("Error while recording: {}", e.what());
    } catch (const req::root<Object>& e) {
      Logger::FWarning("Error while recording: {}", e.get()->invokeToString());
    } catch (...) {
      Logger::FWarning("Error while recording: {}", current_exception_name());
    }
    m_enabled = false;
    m_externalThreadEventCreates.clear();
    m_externalThreadEventProcesses.clear();
    m_nativeCalls.clear();
  }
}

void Recorder::requestInit() {
  m_enabled = folly::Random::oneIn64(RO::EvalRecordSampleRate);
  if (UNLIKELY(m_enabled)) {
    HPHP::DebuggerHook::attach<DebuggerHook>();
  }
}

void Recorder::setEntryPoint(const String& entryPoint) {
  g_entryPoint = std::filesystem::absolute(entryPoint.toCppString());
}

struct Recorder::DebuggerHook final : public HPHP::DebuggerHook {
  static DebuggerHook* GetInstance() {
    static DebuggerHook hook;
    return &hook;
  }

  void onRequestInit() override {
    const auto serverGlobal{php_global(StaticString{"_SERVER"}).asCArrRef()};
    for (auto i{serverGlobal.begin()}; i; ++i) {
      Recorder::get().m_serverGlobal.set(i.first(), i.second());
    }
    HPHP::DebuggerHook::detach();
  }
};

struct Recorder::LoggerHook final : public HPHP::LoggerHook {
  void operator()(const char*, const char* msg, const char* ending) override {
    get().m_nativeCalls.back().stdouts.append(std::string{msg} + ending);
  }
};

struct Recorder::StdoutHook final : public ExecutionContext::StdoutHook {
  void operator()(const char* s, int len) override {
    g_context->writeStdout(s, len, true);
    get().m_nativeCalls.back().stdouts.append(std::string(s, len));
  }
};

void Recorder::addNativeFuncName(std::uintptr_t id, const char* name) {
  g_nativeFuncNames[id] = name;
}

Recorder& Recorder::get() {
  return g_context->m_recorder.value();
}

Recorder::StdoutHook* Recorder::getStdoutHook() {
  static StdoutHook stdoutHook;
  return &stdoutHook;
}

template<>
String Recorder::serialize(Variant value) {
  UnlimitSerializationScope _;
  VariableSerializer vs{VariableSerializer::Type::DebuggerSerialize};
  try {
    return vs.serializeValue(value, false);
  } catch (const req::root<Object>&) {
    // TODO Record the error
    return vs.serializeValue(init_null(), false);
  }
}

template<>
String Recorder::serialize(std::nullptr_t) {
  return serialize<Variant>(init_null());
}

template<>
String Recorder::serialize(bool value) {
  return serialize<Variant>(value);
}

template<>
String Recorder::serialize(double value) {
  return serialize<Variant>(value);
}

template<>
String Recorder::serialize(std::int64_t value) {
  return serialize<Variant>(value);
}

template<>
String Recorder::serialize(Array value) {
  return serialize<Variant>(value);
}

template<>
String Recorder::serialize(ArrayArg value) {
  return serialize(Variant{value.get()});
}

template<>
String Recorder::serialize(const Class* value) {
  return serialize<Variant>(const_cast<Class*>(value));
}

template<>
String Recorder::serialize(Object value) {
  return serialize<Variant>(value);
}

template<>
String Recorder::serialize(ObjectArg value) {
  return serialize(Variant{value.get()});
}

template<>
String Recorder::serialize(ObjectData* value) {
  return serialize(Variant{value});
}

template<>
String Recorder::serialize(Resource value) {
  return serialize<Variant>(value);
}

template<>
String Recorder::serialize(String value) {
  return serialize<Variant>(value);
}

template<>
String Recorder::serialize(StringArg value) {
  return serialize(Variant{value.get()});
}

template<>
String Recorder::serialize(TypedValue value) {
  return serialize(Variant::wrap(value));
}

void Recorder::onNativeCallArg(const String& arg) {
  m_nativeCalls.back().args.append(arg);
}

void Recorder::onNativeCallEntry(std::uintptr_t id) {
  static LoggerHook loggerHook;
  m_nativeCalls.emplace_back().id = id;
  g_context->addStdoutHook(getStdoutHook());
  g_context->backupSession();
  g_context->clearUserErrorHandlers();
  Logger::SetThreadHook(&loggerHook);
  m_enabled = false;

  // FIXME Disabling memory threshold until surprise flags are handled
  tl_heap->setMemThresholdCallback(std::numeric_limits<std::size_t>::max());
}

void Recorder::onNativeCallExit() {
  g_context->removeStdoutHook(getStdoutHook());
  g_context->restoreSession();
  Logger::SetThreadHook(nullptr);
  m_enabled = true;
}

void Recorder::onNativeCallReturn(const String& ret) {
  m_nativeCalls.back().ret = ret;
  onNativeCallExit();
}

void Recorder::onNativeCallThrow(std::exception_ptr exc) {
  try {
    std::rethrow_exception(exc);
  } catch (const req::root<Object>& e) {
    m_nativeCalls.back().exc = serialize(Variant{e});
  }
  onNativeCallExit();
}

void Recorder::onNativeCallWaitHandle(const Object& object) {
  object->incRefCount();
  m_nativeCalls.back().waitHandle = std::bit_cast<std::uintptr_t>(object.get());
  const auto kind{std::bit_cast<c_Awaitable*>(object.get())->getKind()};
  if (kind == c_Awaitable::Kind::ExternalThreadEvent) {
    m_externalThreadEventCreates.emplace_back(m_nativeCalls.back().waitHandle);
  }
  onNativeCallExit();
}

Array Recorder::toArray() const {
  DictInit files{g_context->m_evaledFiles.size()};
  for (const auto& [k, _] : g_context->m_evaledFiles) {
    const auto path{std::filesystem::absolute(k->data())};
    std::ostringstream oss;
    oss << std::ifstream{path}.rdbuf();
    files.set(String{path}, oss.str());
  }
  VecInit externalThreadEventOrder{m_externalThreadEventCreates.size()};
  for (const auto create : m_externalThreadEventCreates) {
    auto order{std::numeric_limits<std::uint64_t>::max()};
    for (std::uint64_t i{0}; i < m_externalThreadEventProcesses.size(); i++) {
      if (create == m_externalThreadEventProcesses[i]) {
        order = i;
        break;
      }
    }
    externalThreadEventOrder.append(order);
  }
  const std::size_t numNativeCall{m_nativeCalls.size()};
  VecInit nativeCalls{numNativeCall};
  DictInit nativeFuncIds{numNativeCall};
  for (std::size_t i{0}; i < numNativeCall; i++) {
    auto [id, stdouts, args, ret, exc, wh]{m_nativeCalls.at(i)};
    if (wh) {
      const auto ptr{std::bit_cast<c_Awaitable*>(wh)};
      if (ptr->isSucceeded()) {
        ret = serialize(ptr->getResult());
      } else if (ptr->isFailed()) {
        exc = serialize(ptr->getException());
      }
      wh = static_cast<std::uint8_t>(ptr->getKind()) + 1;
      ptr->decReleaseCheck();
    }
    nativeCalls.append(make_vec_array(id, stdouts, args, ret, exc, wh));
    nativeFuncIds.set(String{g_nativeFuncNames[id]}, id);
  }
  return make_dict_array(
    "header", make_dict_array(
      "compilerId", compilerId(),
      "entryPoint", g_entryPoint,
      "serverGlobal", m_serverGlobal
    ),
    "files", files.toArray(),
    "eteOrder", externalThreadEventOrder.toArray(),
    "nativeCalls", nativeCalls.toArray(),
    "nativeFuncIds", nativeFuncIds.toArray()
  );
}

} // namespace HPHP
