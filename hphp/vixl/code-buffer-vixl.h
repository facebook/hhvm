// Copyright 2017, VIXL authors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef VIXL_CODE_BUFFER_H
#define VIXL_CODE_BUFFER_H

#include <cstring>

#include "globals-vixl.h"
#include "utils-vixl.h"

namespace vixl {

class CodeBuffer {
 public:
  static const size_t kDefaultCapacity = 4 * KBytes;

  explicit CodeBuffer(size_t capacity = kDefaultCapacity);
  CodeBuffer(byte* buffer, size_t capacity);
  ~CodeBuffer() VIXL_NEGATIVE_TESTING_ALLOW_EXCEPTION;

  void Reset();

  // Make the buffer executable or writable. These states are mutually
  // exclusive.
  // Note that these require page-aligned memory blocks, which we can only
  // guarantee with VIXL_CODE_BUFFER_MMAP.
  void SetExecutable();
  void SetWritable();

  ptrdiff_t GetOffsetFrom(ptrdiff_t offset) const {
    ptrdiff_t cursor_offset = cursor_ - buffer_;
    VIXL_ASSERT((offset >= 0) && (offset <= cursor_offset));
    return cursor_offset - offset;
  }
  VIXL_DEPRECATED("GetOffsetFrom",
                  ptrdiff_t OffsetFrom(ptrdiff_t offset) const) {
    return GetOffsetFrom(offset);
  }

  ptrdiff_t GetCursorOffset() const { return GetOffsetFrom(0); }
  VIXL_DEPRECATED("GetCursorOffset", ptrdiff_t CursorOffset() const) {
    return GetCursorOffset();
  }

  void Rewind(ptrdiff_t offset) {
    byte* rewound_cursor = buffer_ + offset;
    VIXL_ASSERT((buffer_ <= rewound_cursor) && (rewound_cursor <= cursor_));
    cursor_ = rewound_cursor;
  }

  template <typename T>
  T GetOffsetAddress(ptrdiff_t offset) const {
    VIXL_STATIC_ASSERT(sizeof(T) >= sizeof(uintptr_t));
    VIXL_ASSERT((offset >= 0) && (offset <= (cursor_ - buffer_)));
    return reinterpret_cast<T>(buffer_ + offset);
  }

  // Return the address of the start or end of the emitted code.
  template <typename T>
  T GetStartAddress() const {
    VIXL_STATIC_ASSERT(sizeof(T) >= sizeof(uintptr_t));
    return GetOffsetAddress<T>(0);
  }
  template <typename T>
  T GetEndAddress() const {
    VIXL_STATIC_ASSERT(sizeof(T) >= sizeof(uintptr_t));
    return GetOffsetAddress<T>(GetSizeInBytes());
  }

  size_t GetRemainingBytes() const {
    VIXL_ASSERT((cursor_ >= buffer_) && (cursor_ <= (buffer_ + capacity_)));
    return (buffer_ + capacity_) - cursor_;
  }
  VIXL_DEPRECATED("GetRemainingBytes", size_t RemainingBytes() const) {
    return GetRemainingBytes();
  }

  size_t GetSizeInBytes() const {
    VIXL_ASSERT((cursor_ >= buffer_) && (cursor_ <= (buffer_ + capacity_)));
    return cursor_ - buffer_;
  }

  // A code buffer can emit:
  //  * 8, 16, 32 or 64-bit data: constant.
  //  * 16 or 32-bit data: instruction.
  //  * string: debug info.
  void Emit8(uint8_t data) { Emit(data); }

  void Emit16(uint16_t data) { Emit(data); }

  void Emit32(uint32_t data) { Emit(data); }

  void Emit64(uint64_t data) { Emit(data); }

  void EmitString(const char* string);

  void EmitData(const void* data, size_t size);

  template <typename T>
  void Emit(T value) {
    VIXL_ASSERT(HasSpaceFor(sizeof(value)));
    dirty_ = true;
    byte* c = cursor_;
    memcpy(c, &value, sizeof(value));
    cursor_ = c + sizeof(value);
  }

  void UpdateData(size_t offset, const void* data, size_t size);

  // Align to 32bit.
  void Align();

  // Ensure there is enough space for and emit 'n' zero bytes.
  void EmitZeroedBytes(int n);

  bool Is16bitAligned() const { return IsAligned<2>(cursor_); }

  bool Is32bitAligned() const { return IsAligned<4>(cursor_); }

  size_t GetCapacity() const { return capacity_; }
  VIXL_DEPRECATED("GetCapacity", size_t capacity() const) {
    return GetCapacity();
  }

  bool IsManaged() const { return managed_; }

  void Grow(size_t new_capacity);

  bool IsDirty() const { return dirty_; }

  void SetClean() { dirty_ = false; }

  bool HasSpaceFor(size_t amount) const {
    return GetRemainingBytes() >= amount;
  }

  void EnsureSpaceFor(size_t amount, bool* has_grown) {
    bool is_full = !HasSpaceFor(amount);
    if (is_full) Grow(capacity_ * 2 + amount);
    VIXL_ASSERT(has_grown != NULL);
    *has_grown = is_full;
  }
  void EnsureSpaceFor(size_t amount) {
    bool placeholder;
    EnsureSpaceFor(amount, &placeholder);
  }

 private:
  // Backing store of the buffer.
  byte* buffer_;
  // If true the backing store is allocated and deallocated by the buffer. The
  // backing store can then grow on demand. If false the backing store is
  // provided by the user and cannot be resized internally.
  bool managed_;
  // Pointer to the next location to be written.
  byte* cursor_;
  // True if there has been any write since the buffer was created or cleaned.
  bool dirty_;
  // Capacity in bytes of the backing store.
  size_t capacity_;
};

}  // namespace vixl

#endif  // VIXL_CODE_BUFFER_H
