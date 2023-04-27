// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// This file contains miscellaneous helper code used by generated code --
// including lite types -- but which should not be used directly by users.

#ifndef GOOGLE_PROTOBUF_GENERATED_MESSAGE_UTIL_H__
#define GOOGLE_PROTOBUF_GENERATED_MESSAGE_UTIL_H__

#include <assert.h>
#include <atomic>
#include <climits>
#include <string>
#include <vector>

#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>  // Add direct dep on port for pb.cc
#include <google/protobuf/has_bits.h>
#include <google/protobuf/implicit_weak_message.h>
#include <google/protobuf/map_entry_lite.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/wire_format_lite.h>

namespace google {

namespace protobuf {

class Arena;

namespace io { class CodedInputStream; }

namespace internal {


// Annotation for the compiler to emit a deprecation message if a field marked
// with option 'deprecated=true' is used in the code, or for other things in
// generated code which are deprecated.
//
// For internal use in the pb.cc files, deprecation warnings are suppressed
// there.
#undef DEPRECATED_PROTOBUF_FIELD
#define PROTOBUF_DEPRECATED

#define GOOGLE_PROTOBUF_DEPRECATED_ATTR


// Returns the offset of the given field within the given aggregate type.
// This is equivalent to the ANSI C offsetof() macro.  However, according
// to the C++ standard, offsetof() only works on POD types, and GCC
// enforces this requirement with a warning.  In practice, this rule is
// unnecessarily strict; there is probably no compiler or platform on
// which the offsets of the direct fields of a class are non-constant.
// Fields inherited from superclasses *can* have non-constant offsets,
// but that's not what this macro will be used for.
#if defined(__clang__)
// For Clang we use __builtin_offsetof() and suppress the warning,
// to avoid Control Flow Integrity and UBSan vptr sanitizers from
// crashing while trying to validate the invalid reinterpet_casts.
#define GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(TYPE, FIELD)  \
  _Pragma("clang diagnostic push")                                   \
  _Pragma("clang diagnostic ignored \"-Winvalid-offsetof\"")         \
  __builtin_offsetof(TYPE, FIELD)                                    \
  _Pragma("clang diagnostic pop")
#else
// Note that we calculate relative to the pointer value 16 here since if we
// just use zero, GCC complains about dereferencing a NULL pointer.  We
// choose 16 rather than some other number just in case the compiler would
// be confused by an unaligned pointer.
#define GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(TYPE, FIELD)  \
  static_cast< ::google::protobuf::uint32>(                           \
      reinterpret_cast<const char*>(                                 \
          &reinterpret_cast<const TYPE*>(16)->FIELD) -               \
      reinterpret_cast<const char*>(16))
#endif

// Constants for special floating point values.
LIBPROTOBUF_EXPORT double Infinity();
LIBPROTOBUF_EXPORT double NaN();

LIBPROTOBUF_EXPORT void InitProtobufDefaults();

// This used by proto1
inline const std::string& GetEmptyString() {
  InitProtobufDefaults();
  return GetEmptyStringAlreadyInited();
}

// True if IsInitialized() is true for all elements of t.  Type is expected
// to be a RepeatedPtrField<some message type>.  It's useful to have this
// helper here to keep the protobuf compiler from ever having to emit loops in
// IsInitialized() methods.  We want the C++ compiler to inline this or not
// as it sees fit.
template <class Type> bool AllAreInitialized(const Type& t) {
  for (int i = t.size(); --i >= 0; ) {
    if (!t.Get(i).IsInitialized()) return false;
  }
  return true;
}

// "Weak" variant of AllAreInitialized, used to implement implicit weak fields.
// This version operates on MessageLite to avoid introducing a dependency on the
// concrete message type.
template <class T>
bool AllAreInitializedWeak(const ::google::protobuf::RepeatedPtrField<T>& t) {
  for (int i = t.size(); --i >= 0;) {
    if (!reinterpret_cast<const ::google::protobuf::internal::RepeatedPtrFieldBase&>(t)
             .Get<::google::protobuf::internal::ImplicitWeakTypeHandler<T> >(i)
             .IsInitialized()) {
      return false;
    }
  }
  return true;
}

struct LIBPROTOBUF_EXPORT FieldMetadata {
  uint32 offset;  // offset of this field in the struct
  uint32 tag;     // field * 8 + wire_type
  // byte offset * 8 + bit_offset;
  // if the high bit is set then this is the byte offset of the oneof_case
  // for this field.
  uint32 has_offset;
  uint32 type;      // the type of this field.
  const void* ptr;  // auxiliary data

  // From the serializer point of view each fundamental type can occur in
  // 4 different ways. For simplicity we treat all combinations as a cartesion
  // product although not all combinations are allowed.
  enum FieldTypeClass {
    kPresence,
    kNoPresence,
    kRepeated,
    kPacked,
    kOneOf,
    kNumTypeClasses  // must be last enum
  };
  // C++ protobuf has 20 fundamental types, were we added Cord and StringPiece
  // and also distinquish the same types if they have different wire format.
  enum {
    kCordType = 19,
    kStringPieceType = 20,
    kInlinedType = 21,
    kNumTypes = 21,
    kSpecial = kNumTypes * kNumTypeClasses,
  };

  static int CalculateType(int fundamental_type, FieldTypeClass type_class);
};

inline bool IsPresent(const void* base, uint32 hasbit) {
  const uint32* has_bits_array = static_cast<const uint32*>(base);
  return (has_bits_array[hasbit / 32] & (1u << (hasbit & 31))) != 0;
}

inline bool IsOneofPresent(const void* base, uint32 offset, uint32 tag) {
  const uint32* oneof =
      reinterpret_cast<const uint32*>(static_cast<const uint8*>(base) + offset);
  return *oneof == tag >> 3;
}

typedef void (*SpecialSerializer)(const uint8* base, uint32 offset, uint32 tag,
                                  uint32 has_offset,
                                  ::google::protobuf::io::CodedOutputStream* output);

LIBPROTOBUF_EXPORT void ExtensionSerializer(const uint8* base, uint32 offset, uint32 tag,
                         uint32 has_offset,
                         ::google::protobuf::io::CodedOutputStream* output);
LIBPROTOBUF_EXPORT void UnknownFieldSerializerLite(const uint8* base, uint32 offset, uint32 tag,
                                uint32 has_offset,
                                ::google::protobuf::io::CodedOutputStream* output);

struct SerializationTable {
  int num_fields;
  const FieldMetadata* field_table;
};

LIBPROTOBUF_EXPORT void SerializeInternal(const uint8* base, const FieldMetadata* table,
                       int num_fields, ::google::protobuf::io::CodedOutputStream* output);

inline void TableSerialize(const ::google::protobuf::MessageLite& msg,
                           const SerializationTable* table,
                           ::google::protobuf::io::CodedOutputStream* output) {
  const FieldMetadata* field_table = table->field_table;
  int num_fields = table->num_fields - 1;
  const uint8* base = reinterpret_cast<const uint8*>(&msg);
  // TODO(gerbens) This skips the first test if we could use the fast
  // array serialization path, we should make this
  // int cached_size =
  //    *reinterpret_cast<const int32*>(base + field_table->offset);
  // SerializeWithCachedSize(msg, field_table + 1, num_fields, cached_size, ...)
  // But we keep conformance with the old way for now.
  SerializeInternal(base, field_table + 1, num_fields, output);
}

uint8* SerializeInternalToArray(const uint8* base, const FieldMetadata* table,
                                int num_fields, bool is_deterministic,
                                uint8* buffer);

inline uint8* TableSerializeToArray(const ::google::protobuf::MessageLite& msg,
                                    const SerializationTable* table,
                                    bool is_deterministic, uint8* buffer) {
  const uint8* base = reinterpret_cast<const uint8*>(&msg);
  const FieldMetadata* field_table = table->field_table + 1;
  int num_fields = table->num_fields - 1;
  return SerializeInternalToArray(base, field_table, num_fields,
                                  is_deterministic, buffer);
}

template <typename T>
struct CompareHelper {
  bool operator()(const T& a, const T& b) { return a < b; }
};

template <>
struct CompareHelper<ArenaStringPtr> {
  bool operator()(const ArenaStringPtr& a, const ArenaStringPtr& b) {
    return a.Get() < b.Get();
  }
};

struct CompareMapKey {
  template <typename T>
  bool operator()(const MapEntryHelper<T>& a, const MapEntryHelper<T>& b) {
    return Compare(a.key_, b.key_);
  }
  template <typename T>
  bool Compare(const T& a, const T& b) {
    return CompareHelper<T>()(a, b);
  }
};

template <typename MapFieldType, const SerializationTable* table>
void MapFieldSerializer(const uint8* base, uint32 offset, uint32 tag,
                        uint32 has_offset,
                        ::google::protobuf::io::CodedOutputStream* output) {
  typedef MapEntryHelper<typename MapFieldType::EntryTypeTrait> Entry;
  typedef typename MapFieldType::MapType::const_iterator Iter;

  const MapFieldType& map_field =
      *reinterpret_cast<const MapFieldType*>(base + offset);
  const SerializationTable* t =
      table +
      has_offset;  // has_offset is overloaded for maps to mean table offset
  if (!output->IsSerializationDeterministic()) {
    for (Iter it = map_field.GetMap().begin(); it != map_field.GetMap().end();
         ++it) {
      Entry map_entry(*it);
      output->WriteVarint32(tag);
      output->WriteVarint32(map_entry._cached_size_);
      SerializeInternal(reinterpret_cast<const uint8*>(&map_entry),
                        t->field_table, t->num_fields, output);
    }
  } else {
    std::vector<Entry> v;
    for (Iter it = map_field.GetMap().begin(); it != map_field.GetMap().end();
         ++it) {
      v.push_back(Entry(*it));
    }
    std::sort(v.begin(), v.end(), CompareMapKey());
    for (int i = 0; i < v.size(); i++) {
      output->WriteVarint32(tag);
      output->WriteVarint32(v[i]._cached_size_);
      SerializeInternal(reinterpret_cast<const uint8*>(&v[i]), t->field_table,
                        t->num_fields, output);
    }
  }
}

LIBPROTOBUF_EXPORT MessageLite* DuplicateIfNonNullInternal(MessageLite* message);
LIBPROTOBUF_EXPORT MessageLite* GetOwnedMessageInternal(Arena* message_arena,
                                     MessageLite* submessage,
                                     Arena* submessage_arena);

template <typename T>
T* DuplicateIfNonNull(T* message) {
  // The casts must be reinterpret_cast<> because T might be a forward-declared
  // type that the compiler doesn't know is related to MessageLite.
  return reinterpret_cast<T*>(
      DuplicateIfNonNullInternal(reinterpret_cast<MessageLite*>(message)));
}

template <typename T>
T* GetOwnedMessage(Arena* message_arena, T* submessage,
                   Arena* submessage_arena) {
  // The casts must be reinterpret_cast<> because T might be a forward-declared
  // type that the compiler doesn't know is related to MessageLite.
  return reinterpret_cast<T*>(GetOwnedMessageInternal(
      message_arena, reinterpret_cast<MessageLite*>(submessage),
      submessage_arena));
}

// Hide atomic from the public header and allow easy change to regular int
// on platforms where the atomic might have a perf impact.
class LIBPROTOBUF_EXPORT CachedSize {
 public:
  int Get() const { return size_.load(std::memory_order_relaxed); }
  void Set(int size) { size_.store(size, std::memory_order_relaxed); }
 private:
  std::atomic<int> size_{0};
};

// SCCInfo represents information of a strongly connected component of
// mutual dependent messages.
struct LIBPROTOBUF_EXPORT SCCInfoBase {
  // We use 0 for the Initialized state, because test eax,eax, jnz is smaller
  // and is subject to macro fusion.
  enum {
    kInitialized = 0,  // final state
    kRunning = 1,
    kUninitialized = -1,  // initial state
  };
#ifndef _MSC_VER
  std::atomic<int> visit_status;
#else
  // MSVC doesnt make std::atomic constant initialized. This union trick
  // makes it so.
  union {
    int visit_status_to_make_linker_init;
    std::atomic<int> visit_status;
  };
#endif
  int num_deps;
  void (*init_func)();
  // This is followed by an array  of num_deps
  // const SCCInfoBase* deps[];
};

template <int N>
struct SCCInfo {
  SCCInfoBase base;
  // Semantically this is const SCCInfo<T>* which is is a templated type.
  // The obvious inheriting from SCCInfoBase mucks with struct initialization.
  // Attempts showed the compiler was generating dynamic initialization code.
  // Zero length arrays produce warnings with MSVC.
  SCCInfoBase* deps[N ? N : 1];
};

LIBPROTOBUF_EXPORT void InitSCCImpl(SCCInfoBase* scc);

inline void InitSCC(SCCInfoBase* scc) {
  auto status = scc->visit_status.load(std::memory_order_acquire);
  if (GOOGLE_PREDICT_FALSE(status != SCCInfoBase::kInitialized)) InitSCCImpl(scc);
}

LIBPROTOBUF_EXPORT void DestroyMessage(const void* message);
LIBPROTOBUF_EXPORT void DestroyString(const void* s);
// Destroy (not delete) the message
inline void OnShutdownDestroyMessage(const void* ptr) {
  OnShutdownRun(DestroyMessage, ptr);
}
// Destroy the string (call string destructor)
inline void OnShutdownDestroyString(const std::string* ptr) {
  OnShutdownRun(DestroyString, ptr);
}

}  // namespace internal
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_GENERATED_MESSAGE_UTIL_H__
