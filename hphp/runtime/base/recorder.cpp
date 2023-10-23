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

#include <deque>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <folly/dynamic.h>
#include <folly/Random.h>
#include <sys/stat.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/replayer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"
#include "hphp/util/exception.h"
#include "hphp/util/logger.h"
#include "hphp/util/optional.h"

namespace HPHP {

using namespace rr;

namespace {
  static struct DefaultWriter final : public Recorder::Writer {
    void write(const std::vector<char>& recording) override {
      const auto dir{std::filesystem::canonical(RO::EvalRecordDir)};
      std::filesystem::create_directory(dir);
      const auto name{std::to_string(folly::Random::rand64()) + ".hhr"};
      std::ofstream ofs{dir / name, std::ios::binary};
      ofs.write(recording.data(), recording.size());
    }
  } g_defaultWriter;

  static std::string g_entryPoint;
  static std::string g_hdf;
  static std::string g_ini;
  static Recorder::Writer* g_writer{&g_defaultWriter};
} // namespace

void Recorder::onGetFactsForRequest(HPHP::FactsStore*& map) {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    unitCacheClearSync();
    recorder->m_parentFactsStore = map;
    map = getFactsStore();
  }
}

void Recorder::onHasReceived(bool received) {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    auto& event{recorder->m_nativeEvents.emplace_back()};
    event.type = NativeEvent::Type::HAS_RECEIVED;
    event.value.append(received);
  }
}

void Recorder::onProcessSleepEvents(std::int64_t now) {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    auto& event{recorder->m_nativeEvents.emplace_back()};
    event.type = NativeEvent::Type::PROCESS_SLEEP_EVENTS;
    event.value.append(now);
  }
}

void Recorder::onReceiveSomeUntil(c_ExternalThreadEventWaitHandle* received) {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    auto& event{recorder->m_nativeEvents.emplace_back()};
    event.type = NativeEvent::Type::RECEIVE_SOME_UNTIL;
    for (auto wh{received}; wh != nullptr; wh = wh->getNextToProcess()) {
      event.value.append(recorder->m_threads[wh]);
    }
  }
}

void Recorder::onRuntimeOptionLoad(const IniSettingMap& ini, const Hdf& hdf,
                                   const std::string& cmd) {
  g_entryPoint = !cmd.empty() ? std::filesystem::absolute(cmd) : "";
  g_hdf = hdf.toString();
  g_ini = serialize(ini.toArray()).toCppString();
}

void Recorder::onTryReceiveSome(c_ExternalThreadEventWaitHandle* received) {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    auto& event{recorder->m_nativeEvents.emplace_back()};
    event.type = NativeEvent::Type::TRY_RECEIVE_SOME;
    for (auto wh{received}; wh != nullptr; wh = wh->getNextToProcess()) {
      event.value.append(recorder->m_threads[wh]);
    }
  }
}

void Recorder::onVisitEntitiesToInvalidate() {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    auto& event{recorder->m_nativeEvents.emplace_back()};
    event.type = NativeEvent::Type::VISIT_ENTITIES_TO_INVALIDATE;
  }
}

void Recorder::onVisitEntitiesToInvalidateFast() {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    auto& event{recorder->m_nativeEvents.emplace_back()};
    event.type = NativeEvent::Type::VISIT_ENTITIES_TO_INVALIDATE_FAST;
  }
}

void Recorder::onVisitEntity(const std::string& entity) {
  if (const auto recorder{get()}; UNLIKELY(recorder && recorder->m_enabled)) {
    recorder->m_nativeEvents.back().value.append(String{entity});
  }
}

void Recorder::requestExit() {
  if (UNLIKELY(m_enabled)) {
    resolveWaitHandles();
    Stream::setThreadLocalFileHandler(m_parentStreamWrapper);
    try {
      BlobEncoder encoder;
      toArray().serde(encoder);
      g_writer->write(encoder.take());
    } catch (...) {
      // TODO Record failure to record
    }
    m_enabled = false;
    m_factsStore.clear();
    m_nativeCalls.clear();
    m_nativeEvents.clear();
    m_nextThreadCreationOrder = 0;
    m_parentFactsStore = nullptr;
    m_parentStreamWrapper = nullptr;
    m_pendingWaitHandleToNativeCall.clear();
    m_serverGlobal.clear();
    m_streamWrapper.clear();
    m_threads.clear();
  }
}

void Recorder::requestInit() {
  switch (HPHP::Treadmill::sessionKind()) {
    case HPHP::Treadmill::SessionKind::CLISession:
    case HPHP::Treadmill::SessionKind::HttpRequest:
      if (folly::Random::oneIn64(RO::EvalRecordSampleRate)) {
        const auto& url{RO::EvalRecordSampleUrl};
        m_enabled = url.empty() || url == g_context->getRequestUrl();
      }
      break;
    default:
      break;
  }
  if (UNLIKELY(m_enabled)) {
    m_factsStore = Array::CreateDict();
    m_parentStreamWrapper = Stream::getThreadLocalFileHandler();
    m_streamWrapper = Array::CreateDict();
    Stream::setThreadLocalFileHandler(getStreamWrapper());
    HPHP::DebuggerHook::attach<DebuggerHook>();
  }
}

void Recorder::setWriter(Writer* writer) {
  g_writer = writer != nullptr ? writer : &g_defaultWriter;
}

struct Recorder::DebuggerHook final : public HPHP::DebuggerHook {
  static DebuggerHook* GetInstance() {
    static DebuggerHook hook;
    return &hook;
  }

  void onRequestInit() override {
    const auto serverGlobal{php_global(StaticString{"_SERVER"}).asCArrRef()};
    for (auto i{serverGlobal.begin()}; i; ++i) {
      Recorder::get()->m_serverGlobal.set(i.first(), i.second());
    }
    HPHP::DebuggerHook::detach();
  }
};

struct Recorder::FactsStore final : public HPHP::FactsStore {
  template<typename R, typename C, typename... A>
  static auto wrap(R(C::*m)(A...), const char* name) {
    return [=](A... args) -> R {
      const auto ret{(get()->m_parentFactsStore->*m)(std::forward<A>(args)...)};
      const auto key{serialize(make_vec_array(name, serialize(args)...))};
      get()->m_factsStore.set(key, serialize(ret));
      return ret;
    };
  }

  void close() override {
    get()->m_parentFactsStore->close();
  }

  void ensureUpdated() override {
    get()->m_parentFactsStore->ensureUpdated();
  }

  Array getBaseTypes(const String& derivedType, const Variant& filters)
      override {
    static constexpr auto m{&HPHP::FactsStore::getBaseTypes};
    static const auto wrapper{wrap(m, "getBaseTypes")};
    return wrapper(derivedType, filters);
  }

  Optional<FileResult> getConstantFile(const String& constantName) override {
    using M = Optional<FileResult>(AutoloadMap::*)(const String&);
    static constexpr M m{&HPHP::FactsStore::getConstantFile};
    static const auto wrapper{wrap(m, "getConstantFile1")};
    return wrapper(constantName);
  }

  Optional<std::filesystem::path> getConstantFile(std::string_view name)
      override {
    using M = Optional<std::filesystem::path>(AutoloadMap::*)(std::string_view);
    static constexpr M m{&HPHP::FactsStore::getConstantFile};
    static const auto wrapper{wrap(m, "getConstantFile2")};
    return wrapper(name);
  }

  Array getDerivedTypes(const String& baseType, const Variant& filters)
      override {
    static constexpr auto m{&HPHP::FactsStore::getDerivedTypes};
    static const auto wrapper{wrap(m, "getDerivedTypes")};
    return wrapper(baseType, filters);
  }

  Array getFileAttrArgs(const String& file, const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getFileAttrArgs};
    static const auto wrapper{wrap(m, "getFileAttrArgs")};
    return wrapper(file, attr);
  }

  Array getFileAttributes(const String& file) override {
    static constexpr auto m{&HPHP::FactsStore::getFileAttributes};
    static const auto wrapper{wrap(m, "getFileAttributes")};
    return wrapper(file);
  }

  Array getFileConstants(const String& path) override {
    static constexpr auto m{&HPHP::FactsStore::getFileConstants};
    static const auto wrapper{wrap(m, "getFileConstants")};
    return wrapper(path);
  }

  Array getFileFunctions(const String& path) override {
    static constexpr auto m{&HPHP::FactsStore::getFileFunctions};
    static const auto wrapper{wrap(m, "getFileFunctions")};
    return wrapper(path);
  }

  Array getFileModules(const String& path) override {
    static constexpr auto m{&HPHP::FactsStore::getFileModules};
    static const auto wrapper{wrap(m, "getFileModules")};
    return wrapper(path);
  }

  Array getFilesWithAttribute(const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getFilesWithAttribute};
    static const auto wrapper{wrap(m, "getFilesWithAttribute")};
    return wrapper(attr);
  }

  Array getFilesWithAttributeAndAnyValue(const String& attr,
      const folly::dynamic& value) override {
    static constexpr auto m{
      &HPHP::FactsStore::getFilesWithAttributeAndAnyValue};
    static const auto wrapper{wrap(m, "getFilesWithAttributeAndAnyValue")};
    return wrapper(attr, value);
  }

  Array getFileTypeAliases(const String& path) override {
    static constexpr auto m{&HPHP::FactsStore::getFileTypeAliases};
    static const auto wrapper{wrap(m, "getFileTypeAliases")};
    return wrapper(path);
  }

  Array getFileTypes(const String& path) override {
    static constexpr auto m{&HPHP::FactsStore::getFileTypes};
    static const auto wrapper{wrap(m, "getFileTypes")};
    return wrapper(path);
  }

  Optional<FileResult> getFunctionFile(const String& functionName) override {
    using M = Optional<FileResult>(AutoloadMap::*)(const String&);
    static constexpr M m{&HPHP::FactsStore::getFunctionFile};
    static const auto wrapper{wrap(m, "getFunctionFile1")};
    return wrapper(functionName);
  }

  Optional<std::filesystem::path> getFunctionFile(std::string_view name)
      override {
    using M = Optional<std::filesystem::path>(AutoloadMap::*)(std::string_view);
    static constexpr M m{&HPHP::FactsStore::getFunctionFile};
    static const auto wrapper{wrap(m, "getFunctionFile2")};
    return wrapper(name);
  }

  Variant getKind(const String& type) override {
    static constexpr auto m{&HPHP::FactsStore::getKind};
    static const auto wrapper{wrap(m, "getKind")};
    return wrapper(type);
  }

  Array getMethodAttrArgs(const String& type, const String& method,
      const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getMethodAttrArgs};
    static const auto wrapper{wrap(m, "getMethodAttrArgs")};
    return wrapper(type, method, attr);
  }

  Array getMethodAttributes(const String& type, const String& method)
      override {
    static constexpr auto m{&HPHP::FactsStore::getMethodAttributes};
    static const auto wrapper{wrap(m, "getMethodAttributes")};
    return wrapper(type, method);
  }

  Array getMethodsWithAttribute(const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getMethodsWithAttribute};
    static const auto wrapper{wrap(m, "getMethodsWithAttribute")};
    return wrapper(attr);
  }

  Optional<FileResult> getModuleFile(const String& moduleName) override {
    using M = Optional<FileResult>(AutoloadMap::*)(const String&);
    static constexpr M m{&HPHP::FactsStore::getModuleFile};
    static const auto wrapper{wrap(m, "getModuleFile1")};
    return wrapper(moduleName);
  }

  Optional<std::filesystem::path> getModuleFile(std::string_view name)
      override {
    using M = Optional<std::filesystem::path>(AutoloadMap::*)(std::string_view);
    static constexpr M m{&HPHP::FactsStore::getModuleFile};
    static const auto wrapper{wrap(m, "getModuleFile2")};
    return wrapper(name);
  }

  Holder getNativeHolder() noexcept override {
    return Holder{this, nullptr};
  }

  Array getTypeAliasAttrArgs(const String& type, const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getTypeAliasAttrArgs};
    static const auto wrapper{wrap(m, "getTypeAliasAttrArgs")};
    return wrapper(type, attr);
  }

  Array getTypeAliasAttributes(const String& typeAlias) override {
    static constexpr auto m{&HPHP::FactsStore::getTypeAliasAttributes};
    static const auto wrapper{wrap(m, "getTypeAliasAttributes")};
    return wrapper(typeAlias);
  }

  Array getTypeAliasesWithAttribute(const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getTypeAliasesWithAttribute};
    static const auto wrapper{wrap(m, "getTypeAliasesWithAttribute")};
    return wrapper(attr);
  }

  Optional<FileResult> getTypeAliasFile(const String& aliasName) override {
    using M = Optional<FileResult>(AutoloadMap::*)(const String&);
    static constexpr M m{&HPHP::FactsStore::getTypeAliasFile};
    static const auto wrapper{wrap(m, "getTypeAliasFile1")};
    return wrapper(aliasName);
  }

  Optional<std::filesystem::path> getTypeAliasFile(std::string_view name)
      override {
    using M = Optional<std::filesystem::path>(AutoloadMap::*)(std::string_view);
    static constexpr M m{&HPHP::FactsStore::getTypeAliasFile};
    static const auto wrapper{wrap(m, "getTypeAliasFile2")};
    return wrapper(name);
  }

  Array getTypeAttrArgs(const String& type, const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getTypeAttrArgs};
    static const auto wrapper{wrap(m, "getTypeAttrArgs")};
    return wrapper(type, attr);
  }

  Array getTypeAttributes(const String& type) override {
    static constexpr auto m{&HPHP::FactsStore::getTypeAttributes};
    static const auto wrapper{wrap(m, "getTypeAttributes")};
    return wrapper(type);
  }

  Optional<FileResult> getTypeFile(const String& typeName) override {
    using M = Optional<FileResult>(AutoloadMap::*)(const String&);
    static constexpr M m{&HPHP::FactsStore::getTypeFile};
    static const auto wrapper{wrap(m, "getTypeFile1")};
    return wrapper(typeName);
  }

  Optional<std::filesystem::path> getTypeFile(std::string_view name) override {
    using M = Optional<std::filesystem::path>(AutoloadMap::*)(std::string_view);
    static constexpr M m{&HPHP::FactsStore::getTypeFile};
    static const auto wrapper{wrap(m, "getTypeFile2")};
    return wrapper(name);
  }

  Variant getTypeName(const String& type) override {
    static constexpr auto m{&HPHP::FactsStore::getTypeName};
    static const auto wrapper{wrap(m, "getTypeName")};
    return wrapper(type);
  }

  Optional<FileResult> getTypeOrTypeAliasFile(const String& typeName) override {
    using M = Optional<FileResult>(AutoloadMap::*)(const String&);
    static constexpr M m{&HPHP::FactsStore::getTypeOrTypeAliasFile};
    static const auto wrapper{wrap(m, "getTypeOrTypeAliasFile1")};
    return wrapper(typeName);
  }

  Optional<std::filesystem::path> getTypeOrTypeAliasFile(std::string_view name)
      override {
    using M = Optional<std::filesystem::path>(AutoloadMap::*)(std::string_view);
    static constexpr M m{&HPHP::FactsStore::getTypeOrTypeAliasFile};
    static const auto wrapper{wrap(m, "getTypeOrTypeAliasFile2")};
    return wrapper(name);
  }

  Array getTypesWithAttribute(const String& attr) override {
    static constexpr auto m{&HPHP::FactsStore::getTypesWithAttribute};
    static const auto wrapper{wrap(m, "getTypesWithAttribute")};
    return wrapper(attr);
  }

  bool isTypeAbstract(const String& type) override {
    static constexpr auto m{&HPHP::FactsStore::isTypeAbstract};
    static const auto wrapper{wrap(m, "isTypeAbstract")};
    return wrapper(type);
  }

  bool isTypeFinal(const String& type) override {
    static constexpr auto m{&HPHP::FactsStore::isTypeFinal};
    static const auto wrapper{wrap(m, "isTypeFinal")};
    return wrapper(type);
  }
};

struct Recorder::LoggerHook final : public HPHP::LoggerHook {
  void operator()(const char*, const char* msg, const char* ending) override {
    get()->m_nativeCalls.back().stdouts.append(std::string{msg} + ending);
  }
};

struct Recorder::StdoutHook final : public ExecutionContext::StdoutHook {
  void operator()(const char* s, int len) override {
    if (g_context->numStdoutHooks() == 1) {
      g_context->writeStdout(s, len, true);
      get()->m_nativeCalls.back().stdouts.append(std::string(s, len));
    }
  }
};

struct Recorder::StreamWrapper final : public Stream::Wrapper {
  template<typename R, typename C, typename P, typename... A>
  static auto wrap(R(C::*m)(P, A...), const char* name) {
    return [=](P path, A... args) -> R {
      Stream::setThreadLocalFileHandler(get()->m_parentStreamWrapper);
      const auto wrapper{Stream::getWrapperFromURI(path)};
      Stream::setThreadLocalFileHandler(getStreamWrapper());
      const auto ret{(wrapper->*m)(path, std::forward<A>(args)...)};
      const auto key{serialize(make_vec_array(name, path, serialize(args)...))};
      get()->m_streamWrapper.set(key, serialize(ret));
      return ret;
    };
  }

  template<typename R, typename C, typename P>
  static auto wrap(R(C::*m)(P, struct stat*), const char* name) {
    return [=](P path, struct stat* buf) -> R {
      Stream::setThreadLocalFileHandler(get()->m_parentStreamWrapper);
      const auto wrapper{Stream::getWrapperFromURI(path)};
      Stream::setThreadLocalFileHandler(getStreamWrapper());
      const auto ret{(wrapper->*m)(path, buf)};
      const auto key{serialize(make_vec_array(name, path))};
      get()->m_streamWrapper.set(key, serialize(
        make_vec_array(ret, serialize(buf))));
      return ret;
    };
  }

  int access(const String& path, int mode) override {
    static constexpr auto m{&Stream::Wrapper::access};
    static const auto wrapper{wrap(m, "access")};
    return wrapper(path, mode);
  }

  Optional<std::string> getxattr(const char* path, const char* xattr) override {
    static constexpr auto m{&Stream::Wrapper::getxattr};
    static const auto wrapper{wrap(m, "getxattr")};
    return wrapper(path, xattr);
  }

  bool isNormalFileStream() const override {
    return true;
  }

  int lstat(const String& path, struct stat* buf) override {
    static constexpr auto m{&Stream::Wrapper::lstat};
    static const auto wrapper{wrap(m, "lstat")};
    return wrapper(path, buf);
  }

  int mkdir(const String& path, int mode, int options) override {
    static constexpr auto m{&Stream::Wrapper::mkdir};
    static const auto wrapper{wrap(m, "mkdir")};
    return wrapper(path, mode, options);
  }

  req::ptr<File> open(const String& path, const String& mode, int options,
      const req::ptr<StreamContext>& context) override {
    static constexpr auto m{&Stream::Wrapper::open};
    static const auto wrapper{wrap(m, "open")};
    return wrapper(path, mode, options, context);
  }

  req::ptr<Directory> opendir(const String& path) override {
    static constexpr auto m{&Stream::Wrapper::opendir};
    static const auto wrapper{wrap(m, "opendir")};
    return wrapper(path);
  }

  String realpath(const String& path) override {
    static constexpr auto m{&Stream::Wrapper::realpath};
    static const auto wrapper{wrap(m, "realpath")};
    return wrapper(path);
  }

  int rename(const String& oldname, const String& newname) override {
    static constexpr auto m{&Stream::Wrapper::rename};
    static const auto wrapper{wrap(m, "rename")};
    return wrapper(oldname, newname);
  }

  int rmdir(const String& path, int options) override {
    static constexpr auto m{&Stream::Wrapper::rmdir};
    static const auto wrapper{wrap(m, "rmdir")};
    return wrapper(path, options);
  }

  int stat(const String& path, struct stat* buf) override {
    static constexpr auto m{&Stream::Wrapper::stat};
    static const auto wrapper{wrap(m, "stat")};
    return wrapper(path, buf);
  }

  int unlink(const String& path) override {
    static constexpr auto m{&Stream::Wrapper::unlink};
    static const auto wrapper{wrap(m, "unlink")};
    return wrapper(path);
  }
};

Recorder* Recorder::get() {
  return g_context->m_recorder ? &g_context->m_recorder.value() : nullptr;
}

FactsStore* Recorder::getFactsStore() {
  static FactsStore factsStore;
  return &factsStore;
}

Recorder::StdoutHook* Recorder::getStdoutHook() {
  static StdoutHook stdoutHook;
  return &stdoutHook;
}

Stream::Wrapper* Recorder::getStreamWrapper() {
  static StreamWrapper streamWrapper;
  return &streamWrapper;
}

void Recorder::onNativeCallArg(const String& arg) {
  m_nativeCalls.back().args.append(arg);
}

void Recorder::onNativeCallEntry(NativeFunction ptr) {
  static LoggerHook loggerHook;
  m_nativeCalls.emplace_back().ptr = ptr;
  g_context->addStdoutHook(getStdoutHook());
  Logger::SetThreadHook(&loggerHook);
  m_enabled = false;

  // TODO Disabling memory threshold until surprise flags are handled
  tl_heap->setMemThresholdCallback(std::numeric_limits<std::size_t>::max());
}

void Recorder::onNativeCallExit() {
  g_context->removeStdoutHook(getStdoutHook());
  Logger::SetThreadHook(nullptr);
  m_enabled = true;
}

void Recorder::onNativeCallReturn(const String& ret) {
  m_nativeCalls.back().ret = ret;
  onNativeCallExit();
}

void Recorder::onNativeCallThrow(std::exception_ptr exc) {
  m_nativeCalls.back().exc = serialize(exc);
  onNativeCallExit();
}

void Recorder::onNativeCallWaitHandle(c_Awaitable* wh) {
  resolveWaitHandles();
  m_nativeCalls.back().wh = static_cast<std::uint8_t>(wh->getKind()) + 1;
  switch (wh->getKind()) {
    case c_Awaitable::Kind::ExternalThreadEvent:
      m_threads[wh->asExternalThreadEvent()] = m_nextThreadCreationOrder++;
      [[fallthrough]];
    case c_Awaitable::Kind::Static:
      m_pendingWaitHandleToNativeCall[wh] = m_nativeCalls.size() - 1;
      wh->incRefCount();
      break;
    case c_Awaitable::Kind::Sleep:
      m_nativeCalls.back().ret = serialize(
        wh->asSleep()->getWakeTime().time_since_epoch().count());
      break;
    default:
      break;
  }
  onNativeCallExit();
}

void Recorder::resolveWaitHandles() {
  for (const auto [wh, i] : m_pendingWaitHandleToNativeCall) {
    if (wh->isFinished()) {
      if (wh->isSucceeded()) {
        m_nativeCalls[i].ret = serialize(wh->getResult());
      } else {
        m_nativeCalls[i].exc = serialize(wh->getException());
      }
      m_pendingWaitHandleToNativeCall.erase(wh);
      wh->decReleaseCheck();
    }
  }
}

Array Recorder::toArray() const {
  const std::size_t numNativeCall{m_nativeCalls.size()};
  VecInit nativeCalls{numNativeCall};
  DictInit nativeFuncIds{numNativeCall};
  for (std::size_t i{0}; i < numNativeCall; i++) {
    const auto [ptr, stdouts, args, ret, exc, wh]{m_nativeCalls.at(i)};
    const auto id{std::bit_cast<std::int64_t>(ptr)};
    nativeCalls.append(make_vec_array(id, stdouts, args, ret, exc, wh));
    nativeFuncIds.set(String{getNativeFuncName(ptr)}, id);
  }
  VecInit nativeEvents{m_nativeEvents.size()};
  for (const auto& [type, value] : m_nativeEvents) {
    nativeEvents.append(make_vec_array(static_cast<std::int64_t>(type), value));
  }
  return make_dict_array(
    "header", make_dict_array(
      "compilerId", compilerId(),
      "entryPoint", g_entryPoint,
      "hdf", g_hdf,
      "ini", unserialize<Array>(g_ini),
      "serverGlobal", m_serverGlobal
    ),
    "factsStore", m_factsStore,
    "nativeCalls", nativeCalls.toArray(),
    "nativeEvents", nativeEvents.toArray(),
    "nativeFuncIds", nativeFuncIds.toArray(),
    "streamWrapper", m_streamWrapper
  );
}

} // namespace HPHP
