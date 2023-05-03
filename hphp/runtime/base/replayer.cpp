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
#include <vector>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/object-iterator.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/assertions.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"

namespace HPHP {

Replayer::~Replayer() {
  always_assert(m_nativeCalls.empty());
}

String Replayer::file(const String& path) const {
  return m_files.at(path.toCppString());
}

Replayer& Replayer::get() {
  static Replayer replayer;
  return replayer;
}

String Replayer::init(const String& path) {
  std::ifstream ifs{path.data(), std::ios::binary | std::ios::ate};
  always_assert(!ifs.fail());
  std::vector<char> data(static_cast<std::size_t>(ifs.tellg()));
  ifs.seekg(0).read(data.data(), data.size());
  BlobDecoder decoder{data.data(), data.size()};
  const ArrayData* ptr;
  BlobEncoderHelper<decltype(ptr)>::serde(decoder, ptr, false);
  const Array recording{const_cast<ArrayData*>(ptr)};
  const auto header{recording[String{"header"}].asCArrRef()};
  always_assert(header[String{"compilerId"}].asCStrRef() == compilerId());
  for (auto i{recording[String{"files"}].asCArrRef().begin()}; i; ++i) {
    m_files[i.first().asCStrRef().data()] = i.second().asCStrRef().data();
  }
  std::unordered_map<std::uintptr_t, std::uintptr_t> nativeFuncRecordToReplayId;
  for (auto i{recording[String("nativeFuncIds")].asCArrRef().begin()}; i; ++i) {
    const auto id{m_nativeFuncNames[i.first().asCStrRef().data()]};
    nativeFuncRecordToReplayId[i.second().asInt64Val()] = id;
  }
  for (auto i{recording[String{"nativeCalls"}].asCArrRef().begin()}; i; ++i) {
    const auto call{i.second().asCArrRef()};
    always_assert(call.size() == 5);
    const auto id{nativeFuncRecordToReplayId[call[0].asInt64Val()]};
    m_nativeCalls.push_back(NativeCall{
      id,
      call[1].asCArrRef(),
      call[2].asCArrRef(),
      call[3].asCStrRef(),
      call[4].asCStrRef(),
    });
  }
  return header[String{"entryPoint"}].toString();
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Variant& arg) {
  always_assert(recordedArg.equal(Recorder::serialize<Variant>(arg)));
}

template<>
void Replayer::nativeArg(const String& recordedArg, bool arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, double arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, std::int64_t arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Array& arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Class* arg) {
  nativeArg<const Variant&>(recordedArg, const_cast<Class*>(arg));
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Object& arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const Resource& arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, const String& arg) {
  nativeArg<const Variant&>(recordedArg, arg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, TypedValue arg) {
  nativeArg<const Variant&>(recordedArg, Variant::wrap(arg));
}

template<> bool Replayer::unserialize(const String&);

template<>
void Replayer::nativeArg(const String& recordedArg, bool& arg) {
  arg = unserialize<bool>(recordedArg);
}

template<> double Replayer::unserialize(const String&);

template<>
void Replayer::nativeArg(const String& recordedArg, double& arg) {
  arg = unserialize<double>(recordedArg);
}

template<> std::int64_t Replayer::unserialize(const String&);

template<>
void Replayer::nativeArg(const String& recordedArg, std::int64_t& arg) {
  arg = unserialize<std::int64_t>(recordedArg);
}

template<> Array Replayer::unserialize(const String&);

template<>
void Replayer::nativeArg(const String& recordedArg, Array& arg) {
  arg = unserialize<Array>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, ArrayArg arg) {
  arg.m_px = unserialize<Array>(recordedArg).detach();
}

template<> Object Replayer::unserialize(const String&);

template<>
void Replayer::nativeArg(const String& recordedArg, Object& arg) {
  arg = unserialize<Object>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, ObjectArg arg) {
  arg.m_px = unserialize<Object>(recordedArg).detach();
}

template<>
void Replayer::nativeArg(const String& recordedArg, ObjectData* arg) {
  const auto object{unserialize<Object>(recordedArg)};
  always_assert(object->getVMClass() == arg->getVMClass());
  if (arg->isCollection()) {
    auto array{collections::toArray(arg)};
    IterateKV(
      collections::toArray(object.get()).get(),
      [&array](const TypedValue& k, const TypedValue& v) {
        array.set(k, v);
      }
    );
  } else {
    IteratePropMemOrder(
      object.get(),
      [arg](Slot slot, const Class::Prop&, tv_rval recordedValue) {
        tvSet(recordedValue.tv(), arg->propLvalAtOffset(slot));
        return false;
      },
      [arg](TypedValue key, TypedValue recordedValue) {
        tvSet(recordedValue, arg->makeDynProp(tvCastToStringData(key)));
        return false;
      }
    );
  }
}

template<> String Replayer::unserialize(const String&);

template<>
void Replayer::nativeArg(const String& recordedArg, String& arg) {
  arg = unserialize<String>(recordedArg);
}

template<>
void Replayer::nativeArg(const String& recordedArg, StringArg arg) {
  arg.m_px = unserialize<String>(recordedArg).detach();
}

template<> Variant Replayer::unserialize(const String&);

template<>
void Replayer::nativeArg(const String& recordedArg, Variant& arg) {
  arg = unserialize<Variant>(recordedArg);
}

std::tuple<Array, String, String> Replayer::popNativeCall(std::uintptr_t id) {
  always_assert(!m_nativeCalls.empty());
  const auto [recordedId, stdouts, args, ret, exc]{m_nativeCalls.front()};
  m_nativeCalls.pop_front();
  always_assert(recordedId == id);
  for (auto i{stdouts.begin()}; i; ++i) {
    g_context->write(i.second().asCStrRef());
  }
  return std::make_tuple(args, ret, exc);
}

template<>
Variant Replayer::unserialize(const String& recordedValue) {
  TmpAssign _{RO::EvalCheckPropTypeHints, 0};
  return VariableUnserializer{
    recordedValue.data(),
    static_cast<std::size_t>(recordedValue.size()),
    VariableUnserializer::Type::DebuggerSerialize
  }.unserialize();
}

template<>
void Replayer::unserialize(const String& recordedValue) {
  always_assert(unserialize<Variant>(recordedValue).isNull());
}

template<>
bool Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).asBooleanVal();
}

template<>
double Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).asDoubleVal();
}

template<>
std::int64_t Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).asInt64Val();
}

template<>
Array Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).asCArrRef();
}

template<>
Object Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).asCObjRef();
}

template<>
Resource Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).asCResRef();
}

template<>
String Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).asCStrRef();
}

template<>
TypedValue Replayer::unserialize(const String& recordedValue) {
  return unserialize<Variant>(recordedValue).detach();
}

} // namespace HPHP
