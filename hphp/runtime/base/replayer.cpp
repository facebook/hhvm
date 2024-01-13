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

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <optional>
#include <vector>

#include <folly/dynamic.h>
#include <folly/json.h>
#include <sys/stat.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/object-iterator.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/assertions.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/optional.h"

namespace HPHP {

using namespace rr;

namespace {
  static Replayer replayer;
} // namespace

Replayer::Replayer() :
  m_debuggerHook{nullptr},
  m_entryPoint{},
  m_factsStore{},
  m_globals{},
  m_inNativeCall{false},
  m_nativeCalls{},
  m_nativeEvents{},
  m_nativeFuncIds{},
  m_nextThreadCreationOrder{0},
  m_streamWrapper{},
  m_threads{},
  m_unitSns{} {}

std::string Replayer::getEntryPoint() {
  if (!replayer.m_entryPoint.empty()) {
    return replayer.m_entryPoint;
  } else {
    const auto server{replayer.m_globals[String{"_SERVER"}].asCArrRef()};
    return server[String{"SCRIPT_FILENAME"}].asCStrRef().toCppString();
  }
}

FactsStore* Replayer::onGetFactsForRequest() {
  return get().m_factsStore.empty() ? nullptr : getFactsStore();
}

bool Replayer::onHasReceived() {
  const auto event{std::move(replayer.m_nativeEvents.front())};
  replayer.m_nativeEvents.pop_front();
  always_assert(event.type == NativeEvent::Type::HAS_RECEIVED);
  always_assert(event.value.size() == 1);
  return event.value[0].asBooleanVal();
}

std::int64_t Replayer::onParse(const String& filename) {
  always_assert_flog(replayer.m_unitSns.exists(filename), "{}", filename);
  return replayer.m_unitSns[filename].asInt64Val();
}

std::int64_t Replayer::onProcessSleepEvents() {
  const auto event{std::move(replayer.m_nativeEvents.front())};
  replayer.m_nativeEvents.pop_front();
  always_assert(event.type == NativeEvent::Type::PROCESS_SLEEP_EVENTS);
  always_assert(event.value.size() == 1);
  return event.value[0].asInt64Val();
}

c_ExternalThreadEventWaitHandle* Replayer::onReceiveSomeUntil() {
  const auto event{std::move(replayer.m_nativeEvents.front())};
  replayer.m_nativeEvents.pop_front();
  always_assert(event.type == NativeEvent::Type::RECEIVE_SOME_UNTIL);
  c_ExternalThreadEventWaitHandle* received{nullptr};
  for (std::int64_t i{event.value.size()}; i-- > 0;) {
    const auto order{event.value[i].asInt64Val()};
    const auto ptr{replayer.m_threads.at(order)};
    const auto wh{ptr->asExternalThreadEvent()};
    wh->setNextToProcess(received);
    received = wh;
  }
  return received;
}

void Replayer::onRuntimeOptionLoad(IniSettingMap& ini, Hdf& hdf,
                                   const std::string& path) {
  std::ifstream ifs{path, std::ios::binary | std::ios::ate};
  always_assert_flog(!ifs.fail(), "{}", path);
  std::vector<char> data(static_cast<std::size_t>(ifs.tellg()));
  ifs.seekg(0).read(data.data(), data.size());
  ifs.close();
  std::string_view sv{data.data(), data.size()};
  const auto size{std::stoull(std::string{sv.substr(sv.rfind('\0') + 1)})};
  BlobDecoder decoder{data.data(), size};
  const ArrayData* ptr;
  BlobEncoderHelper<decltype(ptr)>::serde(decoder, ptr, true);
  const Array recording{const_cast<ArrayData*>(ptr)};
  const auto compilerId{recording[String{"compilerId"}].asCStrRef()};
  // TODO Enable compiler ID assertion after replay completes successfully.
  // always_assert_flog(compilerId == HPHP::compilerId(), "{}", compilerId);
  replayer.m_entryPoint = recording[String{"entryPoint"}].asCStrRef().c_str();
  replayer.m_factsStore = recording[String("factsStore")].asCArrRef();
  replayer.m_globals = recording[String{"globals"}].asCArrRef();
  Hdf newHdf;
  newHdf.fromString(recording[String{"hdf"}].asCStrRef().c_str());
  IniSettingMap newIni{recording[String{"ini"}].asCArrRef()};
  std::unordered_map<std::uintptr_t, NativeFunction> nativeFuncRecordToReplayId;
  for (auto i{recording[String("nativeFuncIds")].asCArrRef().begin()}; i; ++i) {
    const auto id{std::bit_cast<NativeFunction>(i.second().asInt64Val())};
    const auto name{i.first().asCStrRef().toCppString()};
    replayer.m_nativeFuncIds[id] = name;
  }
  for (auto i{recording[String{"nativeCalls"}].asCArrRef().begin()}; i; ++i) {
    const auto call{i.second().asCArrRef()};
    always_assert_flog(call.size() == 8, "{}", call.size());
    replayer.m_nativeCalls.push_back(NativeCall{
      std::bit_cast<NativeFunction>(call[0].asInt64Val()),
      call[1].asInt64Val(),
      call[2].asCArrRef(),
      call[3].asCArrRef(),
      call[4].asCArrRef(),
      call[5].asCStrRef(),
      call[6].asCStrRef(),
      static_cast<std::uint8_t>(call[7].asInt64Val()),
    });
  }
  for (auto i{recording[String{"nativeEvents"}].asCArrRef().begin()}; i; ++i) {
    const auto event{i.second().asCArrRef()};
    always_assert_flog(event.size() == 2, "{}", event.size());
    replayer.m_nativeEvents.push_back(NativeEvent{
      static_cast<NativeEvent::Type>(event[0].asInt64Val()),
      event[1].asCArrRef(),
    });
  }
  replayer.m_streamWrapper = recording[String("streamWrapper")].asCArrRef();
  replayer.m_unitSns = recording[String("unitSns")].asCArrRef();
  RuntimeOption::Load(newIni, newHdf, {}, {}, nullptr, replayer.m_entryPoint);
  RO::EvalRecordSampleRate = 0;
  RO::EvalReplay = true;
  RO::EvalUnitPrefetcherMaxThreads = 0;
  hdf = newHdf;
  ini = newIni;
  if (RO::RepoAuthoritative) {
    RO::RepoPath = std::tmpnam(nullptr);
    std::ofstream ofs{RO::RepoPath, std::ios::binary};
    ofs.write(&data[size], sv.rfind('\0') - size);
  }
}

c_ExternalThreadEventWaitHandle* Replayer::onTryReceiveSome() {
  const auto event{std::move(replayer.m_nativeEvents.front())};
  replayer.m_nativeEvents.pop_front();
  always_assert(event.type == NativeEvent::Type::TRY_RECEIVE_SOME);
  c_ExternalThreadEventWaitHandle* received{nullptr};
  for (std::int64_t i{event.value.size()}; i-- > 0;) {
    const auto order{event.value[i].asInt64Val()};
    const auto ptr{replayer.m_threads.at(order)};
    const auto wh{ptr->asExternalThreadEvent()};
    wh->setNextToProcess(received);
    received = wh;
  }
  return received;
}

void Replayer::onVisitEntitiesToInvalidate(const Variant& visitor) {
  const auto event{std::move(replayer.m_nativeEvents.front())};
  replayer.m_nativeEvents.pop_front();
  always_assert(event.type == NativeEvent::Type::VISIT_ENTITIES_TO_INVALIDATE);
  for (auto i{event.value.begin()}; i; ++i) {
    vm_call_user_func(visitor, make_vec_array(i.second().asCStrRef()));
  }
}

void Replayer::onVisitEntitiesToInvalidateFast(const Variant& visitor) {
  const auto event{std::move(replayer.m_nativeEvents.front())};
  replayer.m_nativeEvents.pop_front();
  always_assert(event.type ==
    NativeEvent::Type::VISIT_ENTITIES_TO_INVALIDATE_FAST);
  for (auto i{event.value.begin()}; i; ++i) {
    vm_call_user_func(visitor, make_vec_array(i.second().asCStrRef()));
  }
}

void Replayer::requestExit() {
  Stream::setThreadLocalFileHandler(nullptr);
  always_assert_flog(replayer.m_nativeCalls.empty(), "{}",
    replayer.m_nativeCalls.size());
  always_assert_flog(replayer.m_nativeEvents.empty(), "{}",
    replayer.m_nativeEvents.size());
}

void Replayer::requestInit() {
  Stream::setThreadLocalFileHandler(getStreamWrapper());
  always_assert(HPHP::DebuggerHook::attach<DebuggerHook>());
}

void Replayer::setDebuggerHook(HPHP::DebuggerHook* debuggerHook) {
  replayer.m_debuggerHook = debuggerHook;
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Variant& arg) {
  String str;
  try {
    str = serialize(arg);
  } catch (const req::root<Object>&) {
    return; // TODO Handle serialization errors
  }
  always_assert_flog(recordedArg.equal(str), "{} != {}", recordedArg, str);
}

template<>
void Replayer::nativeArg(const String& recordedArg, bool arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, bool& arg) {
  arg = unserialize<bool>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const char* arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, double arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, double& arg) {
  arg = unserialize<double>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const folly::dynamic& arg) {
  nativeArg<const Variant&>(recordedArg, folly::toJson(arg));
}

template<>
void Replayer::nativeArg(const String& recordedArg, int arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg,
    const req::ptr<StreamContext>& arg) {
  nativeArg<const Variant&>(recordedArg, OptResource{arg});
}

template<>
void Replayer::nativeArg(const String& recordedArg, std::int64_t arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, std::int64_t& arg) {
  arg = unserialize<std::int64_t>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, std::string_view arg) {
  nativeArg<const Variant&>(recordedArg, arg.data());
}

template<>
void Replayer::nativeArg(const String& recordedArg, struct stat* arg) {
  *arg = *unserialize<struct stat*>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, Array& arg) {
  arg = unserialize<Array>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Array& arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, ArrayArg arg) {
  nativeArg<const Variant&>(recordedArg, Variant{arg.m_px});
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Class* arg) {
  nativeArg<const Variant&>(recordedArg, const_cast<Class*>(arg));
}

namespace {
  void updateObject(ObjectData* from, ObjectData* to) {
    IteratePropMemOrder(
      from,
      [from, to](Slot slot, const Class::Prop&, tv_rval value) {
        const auto obj{value.val().pobj};
        auto val{to->propLvalAtOffset(slot)};
        if (value.type() == DataType::Object && !obj->isCollection()) {
          updateObject(obj, val.val().pobj);
        } else if (value.type() == KindOfString && val.type() == KindOfObject) {
          const auto fromCls{value.val().pstr};
          const auto toCls{val.val().pobj->getClassName().get()};
          always_assert_flog(fromCls->same(toCls), "{} != {}", fromCls, toCls);
          tvSet(val.tv(), from->propLvalAtOffset(slot));
        } else {
          tvSet(value.tv(), val);
        }
        return false;
      },
      [to](TypedValue key, TypedValue value) {
        tvSet(value, to->makeDynProp(tvCastToStringData(key)));
        return false;
      }
    );
  }
} // namespace

template<>
void Replayer::nativeArg(const String& recordedArg, ObjectData* arg) {
  const auto variant{unserialize<Variant>(recordedArg)};
  if (!variant.isObject()) {
    return always_assert(serialize(arg) == recordedArg);
  }
  const auto object{variant.asCObjRef()};
  const auto cls{object->getVMClass()};
  const auto argCls{arg->getVMClass()};
  always_assert_flog(cls == argCls, "{} != {}", cls->name(), argCls->name());
  if (arg->isCollection()) {
    while (collections::getSize(arg) > 0) {
      collections::pop(arg);
    }
    IterateKV(
      collections::asArray(object.get()),
      [arg](const TypedValue& key, const TypedValue& value) {
        switch (arg->collectionType()) {
          case CollectionType::Map:
            collections::set(arg, &key, &value);
            break;
          case CollectionType::Set:
            collections::append(arg, const_cast<TypedValue*>(&key));
            break;
          case CollectionType::Vector:
            collections::append(arg, const_cast<TypedValue*>(&value));
            break;
          case CollectionType::ImmMap:
          case CollectionType::ImmSet:
          case CollectionType::ImmVector:
          case CollectionType::Pair:
            always_assert(tvEqual(collections::at(arg, &key).tv(), value));
            break;
        }
        return false;
      }
    );
  } else {
    updateObject(object.get(), arg);
  }
  always_assert(object->equal(*arg));
}

template<>
void Replayer::nativeArg(const String& recordedArg, Object& arg) {
  nativeArg<ObjectData*>(recordedArg, arg.get());
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Object& arg) {
  nativeArg<ObjectData*>(recordedArg, arg.get());
}

template<>
void Replayer::nativeArg(const String& recordedArg, ObjectArg arg) {
  nativeArg<ObjectData*>(recordedArg, arg.m_px);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const OptResource& arg) {
  const_cast<OptResource&>(arg) = unserialize<OptResource>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, String& arg) {
  arg = unserialize<String>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const String& arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, StringArg arg) {
  nativeArg<const Variant&>(recordedArg, Variant{arg.m_px});
}

template<>
void Replayer::nativeArg(const String& recordedArg, TypedValue arg) {
  nativeArg<const Variant&>(recordedArg, Variant::wrap(arg));
}

template<>
void Replayer::nativeArg(const String& recordedArg, Variant& arg) {
  arg = unserialize<Variant>(recordedArg);
}

struct Replayer::DebuggerHook final : public HPHP::DebuggerHook {
  static DebuggerHook* GetInstance() {
    static DebuggerHook hook;
    return &hook;
  }

  void onRequestInit() override {
    const auto globals{replayer.m_globals};
    for (auto i{replayer.m_globals.begin()}; i; ++i) {
      php_global_set(StaticString{i.first().asCStrRef().c_str()}, i.second());
    }
    HPHP::DebuggerHook::detach();
    if (auto debuggerHook{replayer.m_debuggerHook}; debuggerHook != nullptr) {
      HPHP::DebuggerHook::s_activeHook = debuggerHook;
      HPHP::DebuggerHook::s_numAttached++;
      RI().m_debuggerHook = debuggerHook;
      RI().m_reqInjectionData.setDebuggerAttached(true);
      RI().m_reqInjectionData.setDebuggerIntr(true);
      debuggerHook->onRequestInit();
    }
  }
};

struct Replayer::Exception final : public ExtendedException {
  Exception(const std::string& message, ArrayData* backtrace) :
    ExtendedException(message, backtrace) {}
};

struct Replayer::ExternalThreadEvent final : public AsioExternalThreadEvent {
  static c_ExternalThreadEventWaitHandle* create(const Object& exception) {
    return (new ExternalThreadEvent{exception})->getWaitHandle();
  }

  static c_ExternalThreadEventWaitHandle* create(const TypedValue& result) {
    return (new ExternalThreadEvent{result})->getWaitHandle();
  }

  void unserialize(TypedValue& result) override {
    markAsFinished();
    if (exception) {
      // NOLINTNEXTLINE(facebook-hte-ThrowNonStdExceptionIssue)
      throw req::root<Object>(exception);
    } else {
      result = this->result;
    }
  }

 private:
  explicit ExternalThreadEvent(const Object& exc) : exception{exc} {}
  explicit ExternalThreadEvent(const TypedValue& res) : result{res} {}

  Object exception;
  TypedValue result;
};

struct Replayer::FactsStore final : public HPHP::FactsStore {
  template<typename R, typename C, typename... A>
  static auto wrap(R(C::*)(A...), const char* name) {
    return [=](A... args) -> R {
      const auto key{serialize(make_vec_array(name, serialize(args)...))};
      always_assert_flog(replayer.m_factsStore.exists(key), "{}", key);
      return unserialize<R>(replayer.m_factsStore[key].asCStrRef());
    };
  }

  void close() override {}

  void ensureUpdated() override {}

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
    static const auto wrapper{
      wrap(m, "getFilesWithAttributeAndAnyValue")};
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

struct Replayer::StreamWrapper final : public Stream::Wrapper {
  template<typename R, typename C, typename P, typename... A>
  static auto wrap(R(C::*)(P, A...), const char* name) {
    return [=](P path, A... args) -> R {
      const auto key{serialize(make_vec_array(name, path, serialize(args)...))};
      always_assert_flog(replayer.m_streamWrapper.exists(key), "{}", key);
      return unserialize<R>(replayer.m_streamWrapper[key].asCStrRef());
    };
  }

  template<typename R, typename C, typename P>
  static auto wrap(R(C::*)(P, struct stat*), const char* name) {
    return [=](P path, struct stat* buf) -> R {
      const auto key{serialize(make_vec_array(name, path))};
      always_assert_flog(replayer.m_streamWrapper.exists(key), "{}", key);
      const auto array{unserialize<Array>(
        replayer.m_streamWrapper[key].asCStrRef())};
      always_assert_flog(array.size() == 2, "{}", array.size());
      nativeArg(array[1].asCStrRef(), buf);
      return array[0].asInt64Val();
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

Replayer& Replayer::get() {
  return replayer;
}

FactsStore* Replayer::getFactsStore() {
  static FactsStore factsStore;
  return &factsStore;
}

Stream::Wrapper* Replayer::getStreamWrapper() {
  static StreamWrapper streamWrapper;
  return &streamWrapper;
}

Object Replayer::makeWaitHandle(const NativeCall& call) {
  Object exc;
  TypedValue ret{immutable_uninit_base};
  if (!call.exc.empty()) {
    exc = unserialize<Object>(call.exc);
  } else if (!call.ret.empty()) {
    ret = unserialize<TypedValue>(call.ret);
  }
  const auto kind{call.wh - 1};
  switch (static_cast<c_Awaitable::Kind>(kind)) {
    case c_Awaitable::Kind::ExternalThreadEvent: {
      c_ExternalThreadEventWaitHandle* wh;
      if (exc) {
        wh = ExternalThreadEvent::create(exc);
      } else {
        wh = ExternalThreadEvent::create(ret);
      }
      m_threads[m_nextThreadCreationOrder++] = wh;
      return Object{wh};
    }
    case c_Awaitable::Kind::Sleep: {
      always_assert(ret.m_type == DataType::Int64);
      auto wh{req::make<c_SleepWaitHandle>()};
      wh->initialize(0);
      using TimePoint = AsioSession::TimePoint;
      wh->m_waketime = TimePoint{TimePoint::duration{ret.m_data.num}};
      return Object{wh};
    }
    case c_Awaitable::Kind::Static:
      if (exc) {
        return Object{c_StaticWaitHandle::CreateFailed(exc.detach())};
      } else {
        return Object{c_StaticWaitHandle::CreateSucceeded(ret)};
      }
    default:
      always_assert_flog(false, "Unhandled WH kind: {}", kind);
  }
}

NativeCall Replayer::popNativeCall(NativeFunction ptr) {
  const auto replayName{getNativeFuncName(ptr)};
  always_assert_flog(!m_nativeCalls.empty(), "{}", replayName);
  const auto call{std::move(m_nativeCalls.front())};
  m_nativeCalls.pop_front();
  const auto& recordName{m_nativeFuncIds.at(call.ptr)};
  const auto recordPtr{getNativeFuncPtr(recordName)};
  always_assert_flog(recordPtr == ptr, "{} != {}", recordName, replayName);
  stackLimitAndSurprise() ^= call.flags & kSurpriseFlagMask;
  if (const auto signum{call.flags & ~kSurpriseFlagMask}; UNLIKELY(signum)) {
    RID().sendSignal(signum);
  }
  for (auto i{call.stdouts.begin()}; i; ++i) {
    g_context->write(i.second().asCStrRef());
  }
  for (auto i{call.errs.begin()}; i; ++i) {
    const auto err{unserialize<Array>(i.second().asCStrRef())};
    always_assert_flog(err.size() == 4, "{}", err.size());
    const auto message{err[0].asCStrRef().toCppString()};
    const auto errnum{err[2].asInt64Val()};
    const auto swallowExceptions{err[3].asBooleanVal()};
    m_inNativeCall = false;
    if (const auto backtrace{err[1]}; backtrace.isArray()) {
      const Exception e{message, backtrace.asCArrRef().get()};
      g_context->callUserErrorHandler(e, errnum, swallowExceptions);
    } else {
      const HPHP::Exception e{message};
      g_context->callUserErrorHandler(e, errnum, swallowExceptions);
    }
    m_inNativeCall = true;
  }
  return call;
}

} // namespace HPHP
