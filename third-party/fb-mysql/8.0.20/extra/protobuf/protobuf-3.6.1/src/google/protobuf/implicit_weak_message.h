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

#ifndef GOOGLE_PROTOBUF_IMPLICIT_WEAK_MESSAGE_H__
#define GOOGLE_PROTOBUF_IMPLICIT_WEAK_MESSAGE_H__

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/message_lite.h>

// This file is logically internal-only and should only be used by protobuf
// generated code.

namespace google {
namespace protobuf {
namespace internal {

// An implementation of MessageLite that treats all data as unknown. This type
// acts as a placeholder for an implicit weak field in the case where the true
// message type does not get linked into the binary.
class LIBPROTOBUF_EXPORT ImplicitWeakMessage : public MessageLite {
 public:
  ImplicitWeakMessage() : arena_(NULL) {}
  explicit ImplicitWeakMessage(Arena* arena) : arena_(arena) {}

  static const ImplicitWeakMessage* default_instance();

  string GetTypeName() const { return ""; }

  MessageLite* New() const { return new ImplicitWeakMessage; }
  MessageLite* New(Arena* arena) const {
    return Arena::CreateMessage<ImplicitWeakMessage>(arena);
  }

  Arena* GetArena() const { return arena_; }

  void Clear() { data_.clear(); }

  bool IsInitialized() const { return true; }

  void CheckTypeAndMergeFrom(const MessageLite& other) {
    data_.append(static_cast<const ImplicitWeakMessage&>(other).data_);
  }

  bool MergePartialFromCodedStream(io::CodedInputStream* input);

  size_t ByteSizeLong() const { return data_.size(); }

  void SerializeWithCachedSizes(io::CodedOutputStream* output) const {
    output->WriteString(data_);
  }

  int GetCachedSize() const { return static_cast<int>(data_.size()); }

  typedef void InternalArenaConstructable_;

 private:
  Arena* const arena_;
  string data_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ImplicitWeakMessage);
};

// A type handler for use with implicit weak repeated message fields.
template <typename ImplicitWeakType>
class ImplicitWeakTypeHandler {
 public:
  typedef ImplicitWeakType Type;
  typedef ::google::protobuf::MessageLite WeakType;
  static const bool Moveable = false;

  // With implicit weak fields, we need separate NewFromPrototype and
  // NewFromPrototypeWeak functions. The former is used when we want to create a
  // strong dependency on the message type, and it just delegates to the
  // GenericTypeHandler. The latter avoids creating a strong dependency, by
  // simply calling MessageLite::New.
  static inline ::google::protobuf::MessageLite* NewFromPrototype(
      const ::google::protobuf::MessageLite* prototype, ::google::protobuf::Arena* arena = NULL) {
    return prototype->New(arena);
  }

  static inline void Delete(::google::protobuf::MessageLite* value, Arena* arena) {
    if (arena == NULL) {
      delete value;
    }
  }
  static inline ::google::protobuf::Arena* GetArena(::google::protobuf::MessageLite* value) {
    return value->GetArena();
  }
  static inline void* GetMaybeArenaPointer(::google::protobuf::MessageLite* value) {
    return value->GetArena();
  }
  static inline void Clear(::google::protobuf::MessageLite* value) {
    value->Clear();
  }
  static void Merge(const ::google::protobuf::MessageLite& from,
                    ::google::protobuf::MessageLite* to) {
    to->CheckTypeAndMergeFrom(from);
  }
  static inline size_t SpaceUsedLong(const Type& value) {
    return value.SpaceUsedLong();
  }
};

}  // namespace internal
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_IMPLICIT_WEAK_MESSAGE_H__
