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
//         wink@google.com (Wink Saville) (refactored from wire_format.h)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#ifndef GOOGLE_PROTOBUF_WIRE_FORMAT_LITE_INL_H__
#define GOOGLE_PROTOBUF_WIRE_FORMAT_LITE_INL_H__

#include <algorithm>
#include <string>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arenastring.h>


namespace google {
namespace protobuf {
namespace internal {

// Implementation details of ReadPrimitive.

template <>
inline bool WireFormatLite::ReadPrimitive<int32, WireFormatLite::TYPE_INT32>(
    io::CodedInputStream* input,
    int32* value) {
  uint32 temp;
  if (!input->ReadVarint32(&temp)) return false;
  *value = static_cast<int32>(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<int64, WireFormatLite::TYPE_INT64>(
    io::CodedInputStream* input,
    int64* value) {
  uint64 temp;
  if (!input->ReadVarint64(&temp)) return false;
  *value = static_cast<int64>(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<uint32, WireFormatLite::TYPE_UINT32>(
    io::CodedInputStream* input,
    uint32* value) {
  return input->ReadVarint32(value);
}
template <>
inline bool WireFormatLite::ReadPrimitive<uint64, WireFormatLite::TYPE_UINT64>(
    io::CodedInputStream* input,
    uint64* value) {
  return input->ReadVarint64(value);
}
template <>
inline bool WireFormatLite::ReadPrimitive<int32, WireFormatLite::TYPE_SINT32>(
    io::CodedInputStream* input,
    int32* value) {
  uint32 temp;
  if (!input->ReadVarint32(&temp)) return false;
  *value = ZigZagDecode32(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<int64, WireFormatLite::TYPE_SINT64>(
    io::CodedInputStream* input,
    int64* value) {
  uint64 temp;
  if (!input->ReadVarint64(&temp)) return false;
  *value = ZigZagDecode64(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<uint32, WireFormatLite::TYPE_FIXED32>(
    io::CodedInputStream* input,
    uint32* value) {
  return input->ReadLittleEndian32(value);
}
template <>
inline bool WireFormatLite::ReadPrimitive<uint64, WireFormatLite::TYPE_FIXED64>(
    io::CodedInputStream* input,
    uint64* value) {
  return input->ReadLittleEndian64(value);
}
template <>
inline bool WireFormatLite::ReadPrimitive<int32, WireFormatLite::TYPE_SFIXED32>(
    io::CodedInputStream* input,
    int32* value) {
  uint32 temp;
  if (!input->ReadLittleEndian32(&temp)) return false;
  *value = static_cast<int32>(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<int64, WireFormatLite::TYPE_SFIXED64>(
    io::CodedInputStream* input,
    int64* value) {
  uint64 temp;
  if (!input->ReadLittleEndian64(&temp)) return false;
  *value = static_cast<int64>(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<float, WireFormatLite::TYPE_FLOAT>(
    io::CodedInputStream* input,
    float* value) {
  uint32 temp;
  if (!input->ReadLittleEndian32(&temp)) return false;
  *value = DecodeFloat(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<double, WireFormatLite::TYPE_DOUBLE>(
    io::CodedInputStream* input,
    double* value) {
  uint64 temp;
  if (!input->ReadLittleEndian64(&temp)) return false;
  *value = DecodeDouble(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<bool, WireFormatLite::TYPE_BOOL>(
    io::CodedInputStream* input,
    bool* value) {
  uint64 temp;
  if (!input->ReadVarint64(&temp)) return false;
  *value = temp != 0;
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<int, WireFormatLite::TYPE_ENUM>(
    io::CodedInputStream* input,
    int* value) {
  uint32 temp;
  if (!input->ReadVarint32(&temp)) return false;
  *value = static_cast<int>(temp);
  return true;
}

template <>
inline const uint8* WireFormatLite::ReadPrimitiveFromArray<
  uint32, WireFormatLite::TYPE_FIXED32>(
    const uint8* buffer,
    uint32* value) {
  return io::CodedInputStream::ReadLittleEndian32FromArray(buffer, value);
}
template <>
inline const uint8* WireFormatLite::ReadPrimitiveFromArray<
  uint64, WireFormatLite::TYPE_FIXED64>(
    const uint8* buffer,
    uint64* value) {
  return io::CodedInputStream::ReadLittleEndian64FromArray(buffer, value);
}
template <>
inline const uint8* WireFormatLite::ReadPrimitiveFromArray<
  int32, WireFormatLite::TYPE_SFIXED32>(
    const uint8* buffer,
    int32* value) {
  uint32 temp;
  buffer = io::CodedInputStream::ReadLittleEndian32FromArray(buffer, &temp);
  *value = static_cast<int32>(temp);
  return buffer;
}
template <>
inline const uint8* WireFormatLite::ReadPrimitiveFromArray<
  int64, WireFormatLite::TYPE_SFIXED64>(
    const uint8* buffer,
    int64* value) {
  uint64 temp;
  buffer = io::CodedInputStream::ReadLittleEndian64FromArray(buffer, &temp);
  *value = static_cast<int64>(temp);
  return buffer;
}
template <>
inline const uint8* WireFormatLite::ReadPrimitiveFromArray<
  float, WireFormatLite::TYPE_FLOAT>(
    const uint8* buffer,
    float* value) {
  uint32 temp;
  buffer = io::CodedInputStream::ReadLittleEndian32FromArray(buffer, &temp);
  *value = DecodeFloat(temp);
  return buffer;
}
template <>
inline const uint8* WireFormatLite::ReadPrimitiveFromArray<
  double, WireFormatLite::TYPE_DOUBLE>(
    const uint8* buffer,
    double* value) {
  uint64 temp;
  buffer = io::CodedInputStream::ReadLittleEndian64FromArray(buffer, &temp);
  *value = DecodeDouble(temp);
  return buffer;
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadRepeatedPrimitive(
    int,  // tag_size, unused.
    uint32 tag,
    io::CodedInputStream* input,
    RepeatedField<CType>* values) {
  CType value;
  if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
  values->Add(value);
  int elements_already_reserved = values->Capacity() - values->size();
  while (elements_already_reserved > 0 && input->ExpectTag(tag)) {
    if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
    values->AddAlreadyReserved(value);
    elements_already_reserved--;
  }
  return true;
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadRepeatedFixedSizePrimitive(
    int tag_size,
    uint32 tag,
    io::CodedInputStream* input,
    RepeatedField<CType>* values) {
  GOOGLE_DCHECK_EQ(UInt32Size(tag), static_cast<size_t>(tag_size));
  CType value;
  if (!ReadPrimitive<CType, DeclaredType>(input, &value))
    return false;
  values->Add(value);

  // For fixed size values, repeated values can be read more quickly by
  // reading directly from a raw array.
  //
  // We can get a tight loop by only reading as many elements as can be
  // added to the RepeatedField without having to do any resizing. Additionally,
  // we only try to read as many elements as are available from the current
  // buffer space. Doing so avoids having to perform boundary checks when
  // reading the value: the maximum number of elements that can be read is
  // known outside of the loop.
  const void* void_pointer;
  int size;
  input->GetDirectBufferPointerInline(&void_pointer, &size);
  if (size > 0) {
    const uint8* buffer = reinterpret_cast<const uint8*>(void_pointer);
    // The number of bytes each type occupies on the wire.
    const int per_value_size = tag_size + static_cast<int>(sizeof(value));

    // parentheses around (std::min) prevents macro expansion of min(...)
    int elements_available =
        (std::min)(values->Capacity() - values->size(), size / per_value_size);
    int num_read = 0;
    while (num_read < elements_available &&
           (buffer = io::CodedInputStream::ExpectTagFromArray(
               buffer, tag)) != NULL) {
      buffer = ReadPrimitiveFromArray<CType, DeclaredType>(buffer, &value);
      values->AddAlreadyReserved(value);
      ++num_read;
    }
    const int read_bytes = num_read * per_value_size;
    if (read_bytes > 0) {
      input->Skip(read_bytes);
    }
  }
  return true;
}

// Specializations of ReadRepeatedPrimitive for the fixed size types, which use
// the optimized code path.
#define READ_REPEATED_FIXED_SIZE_PRIMITIVE(CPPTYPE, DECLARED_TYPE)             \
template <>                                                                    \
inline bool WireFormatLite::ReadRepeatedPrimitive<                             \
  CPPTYPE, WireFormatLite::DECLARED_TYPE>(                                     \
    int tag_size,                                                              \
    uint32 tag,                                                                \
    io::CodedInputStream* input,                                               \
    RepeatedField<CPPTYPE>* values) {                                          \
  return ReadRepeatedFixedSizePrimitive<                                       \
    CPPTYPE, WireFormatLite::DECLARED_TYPE>(                                   \
      tag_size, tag, input, values);                                           \
}

READ_REPEATED_FIXED_SIZE_PRIMITIVE(uint32, TYPE_FIXED32)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(uint64, TYPE_FIXED64)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(int32, TYPE_SFIXED32)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(int64, TYPE_SFIXED64)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(float, TYPE_FLOAT)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(double, TYPE_DOUBLE)

#undef READ_REPEATED_FIXED_SIZE_PRIMITIVE

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
bool WireFormatLite::ReadRepeatedPrimitiveNoInline(
    int tag_size,
    uint32 tag,
    io::CodedInputStream* input,
    RepeatedField<CType>* value) {
  return ReadRepeatedPrimitive<CType, DeclaredType>(
      tag_size, tag, input, value);
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadPackedPrimitive(io::CodedInputStream* input,
                                                RepeatedField<CType>* values) {
  int length;
  if (!input->ReadVarintSizeAsInt(&length)) return false;
  io::CodedInputStream::Limit limit = input->PushLimit(length);
  while (input->BytesUntilLimit() > 0) {
    CType value;
    if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
    values->Add(value);
  }
  input->PopLimit(limit);
  return true;
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadPackedFixedSizePrimitive(
    io::CodedInputStream* input, RepeatedField<CType>* values) {
  int length;
  if (!input->ReadVarintSizeAsInt(&length)) return false;
  const int old_entries = values->size();
  const int new_entries = length / static_cast<int>(sizeof(CType));
  const int new_bytes = new_entries * static_cast<int>(sizeof(CType));
  if (new_bytes != length) return false;
  // We would *like* to pre-allocate the buffer to write into (for
  // speed), but *must* avoid performing a very large allocation due
  // to a malicious user-supplied "length" above.  So we have a fast
  // path that pre-allocates when the "length" is less than a bound.
  // We determine the bound by calling BytesUntilTotalBytesLimit() and
  // BytesUntilLimit().  These return -1 to mean "no limit set".
  // There are four cases:
  // TotalBytesLimit  Limit
  // -1               -1     Use slow path.
  // -1               >= 0   Use fast path if length <= Limit.
  // >= 0             -1     Use slow path.
  // >= 0             >= 0   Use fast path if length <= min(both limits).
  int64 bytes_limit = input->BytesUntilTotalBytesLimit();
  if (bytes_limit == -1) {
    bytes_limit = input->BytesUntilLimit();
  } else {
    // parentheses around (std::min) prevents macro expansion of min(...)
    bytes_limit =
        (std::min)(bytes_limit, static_cast<int64>(input->BytesUntilLimit()));
  }
  if (bytes_limit >= new_bytes) {
    // Fast-path that pre-allocates *values to the final size.
#if defined(PROTOBUF_LITTLE_ENDIAN)
    values->Resize(old_entries + new_entries, 0);
    // values->mutable_data() may change after Resize(), so do this after:
    void* dest = reinterpret_cast<void*>(values->mutable_data() + old_entries);
    if (!input->ReadRaw(dest, new_bytes)) {
      values->Truncate(old_entries);
      return false;
    }
#else
    values->Reserve(old_entries + new_entries);
    CType value;
    for (int i = 0; i < new_entries; ++i) {
      if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
      values->AddAlreadyReserved(value);
    }
#endif
  } else {
    // This is the slow-path case where "length" may be too large to
    // safely allocate.  We read as much as we can into *values
    // without pre-allocating "length" bytes.
    CType value;
    for (int i = 0; i < new_entries; ++i) {
      if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
      values->Add(value);
    }
  }
  return true;
}

// Specializations of ReadPackedPrimitive for the fixed size types, which use
// an optimized code path.
#define READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(CPPTYPE, DECLARED_TYPE)      \
template <>                                                                    \
inline bool WireFormatLite::ReadPackedPrimitive<                               \
  CPPTYPE, WireFormatLite::DECLARED_TYPE>(                                     \
    io::CodedInputStream* input,                                               \
    RepeatedField<CPPTYPE>* values) {                                          \
  return ReadPackedFixedSizePrimitive<                                         \
      CPPTYPE, WireFormatLite::DECLARED_TYPE>(input, values);                  \
}

READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(uint32, TYPE_FIXED32)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(uint64, TYPE_FIXED64)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(int32, TYPE_SFIXED32)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(int64, TYPE_SFIXED64)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(float, TYPE_FLOAT)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(double, TYPE_DOUBLE)

#undef READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
bool WireFormatLite::ReadPackedPrimitiveNoInline(io::CodedInputStream* input,
                                                 RepeatedField<CType>* values) {
  return ReadPackedPrimitive<CType, DeclaredType>(input, values);
}


template<typename MessageType>
inline bool WireFormatLite::ReadGroup(
    int field_number, io::CodedInputStream* input,
    MessageType* value) {
  if (!input->IncrementRecursionDepth()) return false;
  if (!value->MergePartialFromCodedStream(input)) return false;
  input->UnsafeDecrementRecursionDepth();
  // Make sure the last thing read was an end tag for this group.
  if (!input->LastTagWas(MakeTag(field_number, WIRETYPE_END_GROUP))) {
    return false;
  }
  return true;
}
template<typename MessageType>
inline bool WireFormatLite::ReadMessage(
    io::CodedInputStream* input, MessageType* value) {
  int length;
  if (!input->ReadVarintSizeAsInt(&length)) return false;
  std::pair<io::CodedInputStream::Limit, int> p =
      input->IncrementRecursionDepthAndPushLimit(length);
  if (p.second < 0 || !value->MergePartialFromCodedStream(input)) return false;
  // Make sure that parsing stopped when the limit was hit, not at an endgroup
  // tag.
  return input->DecrementRecursionDepthAndPopLimit(p.first);
}

// ===================================================================

inline void WireFormatLite::WriteTag(int field_number, WireType type,
                                     io::CodedOutputStream* output) {
  output->WriteTag(MakeTag(field_number, type));
}

inline void WireFormatLite::WriteInt32NoTag(int32 value,
                                            io::CodedOutputStream* output) {
  output->WriteVarint32SignExtended(value);
}
inline void WireFormatLite::WriteInt64NoTag(int64 value,
                                            io::CodedOutputStream* output) {
  output->WriteVarint64(static_cast<uint64>(value));
}
inline void WireFormatLite::WriteUInt32NoTag(uint32 value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint32(value);
}
inline void WireFormatLite::WriteUInt64NoTag(uint64 value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint64(value);
}
inline void WireFormatLite::WriteSInt32NoTag(int32 value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint32(ZigZagEncode32(value));
}
inline void WireFormatLite::WriteSInt64NoTag(int64 value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint64(ZigZagEncode64(value));
}
inline void WireFormatLite::WriteFixed32NoTag(uint32 value,
                                              io::CodedOutputStream* output) {
  output->WriteLittleEndian32(value);
}
inline void WireFormatLite::WriteFixed64NoTag(uint64 value,
                                              io::CodedOutputStream* output) {
  output->WriteLittleEndian64(value);
}
inline void WireFormatLite::WriteSFixed32NoTag(int32 value,
                                               io::CodedOutputStream* output) {
  output->WriteLittleEndian32(static_cast<uint32>(value));
}
inline void WireFormatLite::WriteSFixed64NoTag(int64 value,
                                               io::CodedOutputStream* output) {
  output->WriteLittleEndian64(static_cast<uint64>(value));
}
inline void WireFormatLite::WriteFloatNoTag(float value,
                                            io::CodedOutputStream* output) {
  output->WriteLittleEndian32(EncodeFloat(value));
}
inline void WireFormatLite::WriteDoubleNoTag(double value,
                                             io::CodedOutputStream* output) {
  output->WriteLittleEndian64(EncodeDouble(value));
}
inline void WireFormatLite::WriteBoolNoTag(bool value,
                                           io::CodedOutputStream* output) {
  output->WriteVarint32(value ? 1 : 0);
}
inline void WireFormatLite::WriteEnumNoTag(int value,
                                           io::CodedOutputStream* output) {
  output->WriteVarint32SignExtended(value);
}

// See comment on ReadGroupNoVirtual to understand the need for this template
// parameter name.
template<typename MessageType_WorkAroundCppLookupDefect>
inline void WireFormatLite::WriteGroupNoVirtual(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    io::CodedOutputStream* output) {
  WriteTag(field_number, WIRETYPE_START_GROUP, output);
  value.MessageType_WorkAroundCppLookupDefect::SerializeWithCachedSizes(output);
  WriteTag(field_number, WIRETYPE_END_GROUP, output);
}
template<typename MessageType_WorkAroundCppLookupDefect>
inline void WireFormatLite::WriteMessageNoVirtual(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    io::CodedOutputStream* output) {
  WriteTag(field_number, WIRETYPE_LENGTH_DELIMITED, output);
  output->WriteVarint32(
      value.MessageType_WorkAroundCppLookupDefect::GetCachedSize());
  value.MessageType_WorkAroundCppLookupDefect::SerializeWithCachedSizes(output);
}

// ===================================================================

inline uint8* WireFormatLite::WriteTagToArray(int field_number,
                                              WireType type,
                                              uint8* target) {
  return io::CodedOutputStream::WriteTagToArray(MakeTag(field_number, type),
                                                target);
}

inline uint8* WireFormatLite::WriteInt32NoTagToArray(int32 value,
                                                     uint8* target) {
  return io::CodedOutputStream::WriteVarint32SignExtendedToArray(value, target);
}
inline uint8* WireFormatLite::WriteInt64NoTagToArray(int64 value,
                                                     uint8* target) {
  return io::CodedOutputStream::WriteVarint64ToArray(
      static_cast<uint64>(value), target);
}
inline uint8* WireFormatLite::WriteUInt32NoTagToArray(uint32 value,
                                                      uint8* target) {
  return io::CodedOutputStream::WriteVarint32ToArray(value, target);
}
inline uint8* WireFormatLite::WriteUInt64NoTagToArray(uint64 value,
                                                      uint8* target) {
  return io::CodedOutputStream::WriteVarint64ToArray(value, target);
}
inline uint8* WireFormatLite::WriteSInt32NoTagToArray(int32 value,
                                                      uint8* target) {
  return io::CodedOutputStream::WriteVarint32ToArray(ZigZagEncode32(value),
                                                     target);
}
inline uint8* WireFormatLite::WriteSInt64NoTagToArray(int64 value,
                                                      uint8* target) {
  return io::CodedOutputStream::WriteVarint64ToArray(ZigZagEncode64(value),
                                                     target);
}
inline uint8* WireFormatLite::WriteFixed32NoTagToArray(uint32 value,
                                                       uint8* target) {
  return io::CodedOutputStream::WriteLittleEndian32ToArray(value, target);
}
inline uint8* WireFormatLite::WriteFixed64NoTagToArray(uint64 value,
                                                       uint8* target) {
  return io::CodedOutputStream::WriteLittleEndian64ToArray(value, target);
}
inline uint8* WireFormatLite::WriteSFixed32NoTagToArray(int32 value,
                                                        uint8* target) {
  return io::CodedOutputStream::WriteLittleEndian32ToArray(
      static_cast<uint32>(value), target);
}
inline uint8* WireFormatLite::WriteSFixed64NoTagToArray(int64 value,
                                                        uint8* target) {
  return io::CodedOutputStream::WriteLittleEndian64ToArray(
      static_cast<uint64>(value), target);
}
inline uint8* WireFormatLite::WriteFloatNoTagToArray(float value,
                                                     uint8* target) {
  return io::CodedOutputStream::WriteLittleEndian32ToArray(EncodeFloat(value),
                                                           target);
}
inline uint8* WireFormatLite::WriteDoubleNoTagToArray(double value,
                                                      uint8* target) {
  return io::CodedOutputStream::WriteLittleEndian64ToArray(EncodeDouble(value),
                                                           target);
}
inline uint8* WireFormatLite::WriteBoolNoTagToArray(bool value,
                                                    uint8* target) {
  return io::CodedOutputStream::WriteVarint32ToArray(value ? 1 : 0, target);
}
inline uint8* WireFormatLite::WriteEnumNoTagToArray(int value,
                                                    uint8* target) {
  return io::CodedOutputStream::WriteVarint32SignExtendedToArray(value, target);
}

template<typename T>
inline uint8* WireFormatLite::WritePrimitiveNoTagToArray(
      const RepeatedField<T>& value,
      uint8* (*Writer)(T, uint8*), uint8* target) {
  const int n = value.size();
  GOOGLE_DCHECK_GT(n, 0);

  const T* ii = value.unsafe_data();
  int i = 0;
  do {
    target = Writer(ii[i], target);
  } while (++i < n);

  return target;
}

template<typename T>
inline uint8* WireFormatLite::WriteFixedNoTagToArray(
      const RepeatedField<T>& value,
      uint8* (*Writer)(T, uint8*), uint8* target) {
#if defined(PROTOBUF_LITTLE_ENDIAN)
  (void) Writer;

  const int n = value.size();
  GOOGLE_DCHECK_GT(n, 0);

  const T* ii = value.unsafe_data();
  const int bytes = n * static_cast<int>(sizeof(ii[0]));
  memcpy(target, ii, static_cast<size_t>(bytes));
  return target + bytes;
#else
  return WritePrimitiveNoTagToArray(value, Writer, target);
#endif
}

inline uint8* WireFormatLite::WriteInt32NoTagToArray(
    const RepeatedField< int32>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteInt32NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteInt64NoTagToArray(
    const RepeatedField< int64>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteInt64NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteUInt32NoTagToArray(
    const RepeatedField<uint32>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteUInt32NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteUInt64NoTagToArray(
    const RepeatedField<uint64>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteUInt64NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteSInt32NoTagToArray(
    const RepeatedField< int32>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteSInt32NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteSInt64NoTagToArray(
    const RepeatedField< int64>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteSInt64NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteFixed32NoTagToArray(
    const RepeatedField<uint32>& value, uint8* target) {
  return WriteFixedNoTagToArray(value, WriteFixed32NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteFixed64NoTagToArray(
    const RepeatedField<uint64>& value, uint8* target) {
  return WriteFixedNoTagToArray(value, WriteFixed64NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteSFixed32NoTagToArray(
    const RepeatedField< int32>& value, uint8* target) {
  return WriteFixedNoTagToArray(value, WriteSFixed32NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteSFixed64NoTagToArray(
    const RepeatedField< int64>& value, uint8* target) {
  return WriteFixedNoTagToArray(value, WriteSFixed64NoTagToArray, target);
}
inline uint8* WireFormatLite::WriteFloatNoTagToArray(
    const RepeatedField< float>& value, uint8* target) {
  return WriteFixedNoTagToArray(value, WriteFloatNoTagToArray, target);
}
inline uint8* WireFormatLite::WriteDoubleNoTagToArray(
    const RepeatedField<double>& value, uint8* target) {
  return WriteFixedNoTagToArray(value, WriteDoubleNoTagToArray, target);
}
inline uint8* WireFormatLite::WriteBoolNoTagToArray(
    const RepeatedField<  bool>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteBoolNoTagToArray, target);
}
inline uint8* WireFormatLite::WriteEnumNoTagToArray(
    const RepeatedField<   int>& value, uint8* target) {
  return WritePrimitiveNoTagToArray(value, WriteEnumNoTagToArray, target);
}

inline uint8* WireFormatLite::WriteInt32ToArray(int field_number,
                                                int32 value,
                                                uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteInt32NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteInt64ToArray(int field_number,
                                                int64 value,
                                                uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteInt64NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteUInt32ToArray(int field_number,
                                                 uint32 value,
                                                 uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteUInt32NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteUInt64ToArray(int field_number,
                                                 uint64 value,
                                                 uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteUInt64NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteSInt32ToArray(int field_number,
                                                 int32 value,
                                                 uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteSInt32NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteSInt64ToArray(int field_number,
                                                 int64 value,
                                                 uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteSInt64NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteFixed32ToArray(int field_number,
                                                  uint32 value,
                                                  uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED32, target);
  return WriteFixed32NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteFixed64ToArray(int field_number,
                                                  uint64 value,
                                                  uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED64, target);
  return WriteFixed64NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteSFixed32ToArray(int field_number,
                                                   int32 value,
                                                   uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED32, target);
  return WriteSFixed32NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteSFixed64ToArray(int field_number,
                                                   int64 value,
                                                   uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED64, target);
  return WriteSFixed64NoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteFloatToArray(int field_number,
                                                float value,
                                                uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED32, target);
  return WriteFloatNoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteDoubleToArray(int field_number,
                                                 double value,
                                                 uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED64, target);
  return WriteDoubleNoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteBoolToArray(int field_number,
                                               bool value,
                                               uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteBoolNoTagToArray(value, target);
}
inline uint8* WireFormatLite::WriteEnumToArray(int field_number,
                                               int value,
                                               uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteEnumNoTagToArray(value, target);
}

template<typename T>
inline uint8* WireFormatLite::WritePrimitiveToArray(
    int field_number,
    const RepeatedField<T>& value,
    uint8* (*Writer)(int, T, uint8*), uint8* target) {
  const int n = value.size();
  if (n == 0) {
    return target;
  }

  const T* ii = value.unsafe_data();
  int i = 0;
  do {
    target = Writer(field_number, ii[i], target);
  } while (++i < n);

  return target;
}

inline uint8* WireFormatLite::WriteInt32ToArray(
    int field_number, const RepeatedField< int32>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteInt32ToArray, target);
}
inline uint8* WireFormatLite::WriteInt64ToArray(
    int field_number, const RepeatedField< int64>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteInt64ToArray, target);
}
inline uint8* WireFormatLite::WriteUInt32ToArray(
    int field_number, const RepeatedField<uint32>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteUInt32ToArray, target);
}
inline uint8* WireFormatLite::WriteUInt64ToArray(
    int field_number, const RepeatedField<uint64>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteUInt64ToArray, target);
}
inline uint8* WireFormatLite::WriteSInt32ToArray(
    int field_number, const RepeatedField< int32>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteSInt32ToArray, target);
}
inline uint8* WireFormatLite::WriteSInt64ToArray(
    int field_number, const RepeatedField< int64>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteSInt64ToArray, target);
}
inline uint8* WireFormatLite::WriteFixed32ToArray(
    int field_number, const RepeatedField<uint32>& value, uint8* target) {
  return WritePrimitiveToArray(
      field_number, value, WriteFixed32ToArray, target);
}
inline uint8* WireFormatLite::WriteFixed64ToArray(
    int field_number, const RepeatedField<uint64>& value, uint8* target) {
  return WritePrimitiveToArray(
      field_number, value, WriteFixed64ToArray, target);
}
inline uint8* WireFormatLite::WriteSFixed32ToArray(
    int field_number, const RepeatedField< int32>& value, uint8* target) {
  return WritePrimitiveToArray(
      field_number, value, WriteSFixed32ToArray, target);
}
inline uint8* WireFormatLite::WriteSFixed64ToArray(
    int field_number, const RepeatedField< int64>& value, uint8* target) {
  return WritePrimitiveToArray(
      field_number, value, WriteSFixed64ToArray, target);
}
inline uint8* WireFormatLite::WriteFloatToArray(
    int field_number, const RepeatedField< float>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteFloatToArray, target);
}
inline uint8* WireFormatLite::WriteDoubleToArray(
    int field_number, const RepeatedField<double>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteDoubleToArray, target);
}
inline uint8* WireFormatLite::WriteBoolToArray(
    int field_number, const RepeatedField<  bool>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteBoolToArray, target);
}
inline uint8* WireFormatLite::WriteEnumToArray(
    int field_number, const RepeatedField<   int>& value, uint8* target) {
  return WritePrimitiveToArray(field_number, value, WriteEnumToArray, target);
}
inline uint8* WireFormatLite::WriteStringToArray(int field_number,
                                                 const string& value,
                                                 uint8* target) {
  // String is for UTF-8 text only
  // WARNING:  In wire_format.cc, both strings and bytes are handled by
  //   WriteString() to avoid code duplication.  If the implementations become
  //   different, you will need to update that usage.
  target = WriteTagToArray(field_number, WIRETYPE_LENGTH_DELIMITED, target);
  return io::CodedOutputStream::WriteStringWithSizeToArray(value, target);
}
inline uint8* WireFormatLite::WriteBytesToArray(int field_number,
                                                const string& value,
                                                uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_LENGTH_DELIMITED, target);
  return io::CodedOutputStream::WriteStringWithSizeToArray(value, target);
}


template<typename MessageType>
inline uint8* WireFormatLite::InternalWriteGroupToArray(
    int field_number, const MessageType& value, bool deterministic,
    uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_START_GROUP, target);
  target = value.InternalSerializeWithCachedSizesToArray(deterministic, target);
  return WriteTagToArray(field_number, WIRETYPE_END_GROUP, target);
}
template<typename MessageType>
inline uint8* WireFormatLite::InternalWriteMessageToArray(
    int field_number, const MessageType& value, bool deterministic,
    uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_LENGTH_DELIMITED, target);
  target = io::CodedOutputStream::WriteVarint32ToArray(
    static_cast<uint32>(value.GetCachedSize()), target);
  return value.InternalSerializeWithCachedSizesToArray(deterministic, target);
}

// See comment on ReadGroupNoVirtual to understand the need for this template
// parameter name.
template<typename MessageType_WorkAroundCppLookupDefect>
inline uint8* WireFormatLite::InternalWriteGroupNoVirtualToArray(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    bool deterministic, uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_START_GROUP, target);
  target = value.MessageType_WorkAroundCppLookupDefect::
      InternalSerializeWithCachedSizesToArray(deterministic, target);
  return WriteTagToArray(field_number, WIRETYPE_END_GROUP, target);
}
template<typename MessageType_WorkAroundCppLookupDefect>
inline uint8* WireFormatLite::InternalWriteMessageNoVirtualToArray(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    bool deterministic, uint8* target) {
  target = WriteTagToArray(field_number, WIRETYPE_LENGTH_DELIMITED, target);
  target = io::CodedOutputStream::WriteVarint32ToArray(
        static_cast<uint32>(
            value.MessageType_WorkAroundCppLookupDefect::GetCachedSize()),
        target);
  return value.MessageType_WorkAroundCppLookupDefect::
      InternalSerializeWithCachedSizesToArray(deterministic, target);
}

// ===================================================================

inline size_t WireFormatLite::Int32Size(int32 value) {
  return io::CodedOutputStream::VarintSize32SignExtended(value);
}
inline size_t WireFormatLite::Int64Size(int64 value) {
  return io::CodedOutputStream::VarintSize64(static_cast<uint64>(value));
}
inline size_t WireFormatLite::UInt32Size(uint32 value) {
  return io::CodedOutputStream::VarintSize32(value);
}
inline size_t WireFormatLite::UInt64Size(uint64 value) {
  return io::CodedOutputStream::VarintSize64(value);
}
inline size_t WireFormatLite::SInt32Size(int32 value) {
  return io::CodedOutputStream::VarintSize32(ZigZagEncode32(value));
}
inline size_t WireFormatLite::SInt64Size(int64 value) {
  return io::CodedOutputStream::VarintSize64(ZigZagEncode64(value));
}
inline size_t WireFormatLite::EnumSize(int value) {
  return io::CodedOutputStream::VarintSize32SignExtended(value);
}

inline size_t WireFormatLite::StringSize(const string& value) {
  return LengthDelimitedSize(value.size());
}
inline size_t WireFormatLite::BytesSize(const string& value) {
  return LengthDelimitedSize(value.size());
}


template<typename MessageType>
inline size_t WireFormatLite::GroupSize(const MessageType& value) {
  return value.ByteSizeLong();
}
template<typename MessageType>
inline size_t WireFormatLite::MessageSize(const MessageType& value) {
  return LengthDelimitedSize(value.ByteSizeLong());
}

// See comment on ReadGroupNoVirtual to understand the need for this template
// parameter name.
template<typename MessageType_WorkAroundCppLookupDefect>
inline size_t WireFormatLite::GroupSizeNoVirtual(
    const MessageType_WorkAroundCppLookupDefect& value) {
  return value.MessageType_WorkAroundCppLookupDefect::ByteSizeLong();
}
template<typename MessageType_WorkAroundCppLookupDefect>
inline size_t WireFormatLite::MessageSizeNoVirtual(
    const MessageType_WorkAroundCppLookupDefect& value) {
  return LengthDelimitedSize(
      value.MessageType_WorkAroundCppLookupDefect::ByteSizeLong());
}

inline size_t WireFormatLite::LengthDelimitedSize(size_t length) {
  // The static_cast here prevents an error in certain compiler configurations
  // but is not technically correct--if length is too large to fit in a uint32
  // then it will be silently truncated. We will need to fix this if we ever
  // decide to start supporting serialized messages greater than 2 GiB in size.
  return length + io::CodedOutputStream::VarintSize32(
      static_cast<uint32>(length));
}

}  // namespace internal
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_WIRE_FORMAT_LITE_INL_H__
