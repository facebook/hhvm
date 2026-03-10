// Copyright 2015, VIXL authors
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

#ifndef VIXL_AARCH64_SIMULATOR_AARCH64_H_
#define VIXL_AARCH64_SIMULATOR_AARCH64_H_

#include <memory>
#include <mutex>
#include <random>
#include <unordered_map>
#include <vector>

#include "../cpu-features.h"
#include "../globals-vixl.h"
#include "../utils-vixl.h"

#include "abi-aarch64.h"
#include "cpu-features-auditor-aarch64.h"
#include "debugger-aarch64.h"
#include "disasm-aarch64.h"
#include "instructions-aarch64.h"
#include "simulator-constants-aarch64.h"

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64

// These are only used for the ABI feature, and depend on checks performed for
// it.
#ifdef VIXL_HAS_ABI_SUPPORT
#include <tuple>
#if __cplusplus >= 201402L
// Required for `std::index_sequence`
#include <utility>
#endif
#endif

// The hosts that Simulator running on may not have these flags defined.
#ifndef PROT_BTI
#define PROT_BTI 0x10
#endif
#ifndef PROT_MTE
#define PROT_MTE 0x20
#endif

namespace vixl {
namespace aarch64 {

class Simulator;
struct RuntimeCallStructHelper;

enum class MemoryAccessResult { Success = 0, Failure = 1 };

// Try to access a piece of memory at the given address. Accessing that memory
// might raise a signal which, if handled by a custom signal handler, should
// setup the native and simulated context in order to continue. Return whether
// the memory access failed (i.e: raised a signal) or succeeded.
MemoryAccessResult TryMemoryAccess(uintptr_t address, uintptr_t access_size);

#ifdef VIXL_ENABLE_IMPLICIT_CHECKS
// Access a byte of memory from the address at the given offset. If the memory
// could be accessed then return MemoryAccessResult::Success. If the memory
// could not be accessed, and therefore raised a signal, setup the simulated
// context and return MemoryAccessResult::Failure.
//
// If a signal is raised then it is expected that the signal handler will place
// MemoryAccessResult::Failure in the native return register and the address of
// _vixl_internal_AccessMemory_continue into the native instruction pointer.
extern "C" MemoryAccessResult _vixl_internal_ReadMemory(uintptr_t address,
                                                        uintptr_t offset);
extern "C" uintptr_t _vixl_internal_AccessMemory_continue();
#endif  // VIXL_ENABLE_IMPLICIT_CHECKS

class SimStack {
 public:
  SimStack() {}
  explicit SimStack(size_t size) : usable_size_(size) {}

  // Guard against accesses above the stack base. This could occur, for example,
  // if the first simulated function tries to read stack arguments that haven't
  // been properly initialised in the Simulator's stack.
  void SetBaseGuardSize(size_t size) { base_guard_size_ = size; }

  // Guard against stack overflows. The size should be large enough to detect
  // the largest stride made (by `MacroAssembler::Claim()` or equivalent) whilst
  // initialising stack objects.
  void SetLimitGuardSize(size_t size) { limit_guard_size_ = size; }

  // The minimum usable size of the stack.
  // Equal to "stack base" - "stack limit", in AAPCS64 terminology.
  void SetUsableSize(size_t size) { usable_size_ = size; }

  // Set the minimum alignment for the stack parameters.
  void AlignToBytesLog2(int align_log2) { align_log2_ = align_log2; }

  class Allocated {
   public:
    // Using AAPCS64 terminology, highest addresses at the top:
    //
    //  data_.get() + alloc_size ->
    //                              |
    //                              | Base guard
    //                 GetBase() -> |                  |
    //                                |                |
    //                                |                | AAPCS64-legal
    //                                | Usable stack   | values of 'sp'.
    //                                |                |
    //                                |                |
    //                GetLimit() -> |
    //                              | Limit guard
    //               data_.get() -> |
    //
    // The Simulator detects (and forbids) accesses to either guard region.

    char* GetBase() const { return base_; }
    char* GetLimit() const { return limit_; }

    template <typename T>
    bool IsAccessInGuardRegion(const T* base, size_t size) const {
      VIXL_ASSERT(size > 0);
      // Inclusive bounds.
      const char* start = reinterpret_cast<const char*>(base);
      const char* end = start + size - 1;
      const char* data_start = data_.get();
      const char* data_end = data_start + alloc_size_ - 1;
      bool in_base_guard = (start <= data_end) && (end >= base_);
      bool in_limit_guard = (start <= limit_) && (end >= data_start);
      return in_base_guard || in_limit_guard;
    }

   private:
    std::unique_ptr<char[]> data_;
    char* limit_;
    char* base_;
    size_t alloc_size_;

    friend class SimStack;
  };

  // Allocate the stack, locking the parameters.
  Allocated Allocate() {
    size_t align_to = uint64_t{1} << align_log2_;
    size_t l = AlignUp(limit_guard_size_, align_to);
    size_t u = AlignUp(usable_size_, align_to);
    size_t b = AlignUp(base_guard_size_, align_to);
    size_t size = l + u + b;

    Allocated a;
    size_t alloc_size = (align_to - 1) + size;
    a.data_ = std::make_unique<char[]>(alloc_size);
    void* data = a.data_.get();
    auto data_aligned =
        reinterpret_cast<char*>(std::align(align_to, size, data, alloc_size));
    a.limit_ = data_aligned + l - 1;
    a.base_ = data_aligned + l + u;
    a.alloc_size_ = alloc_size;
    return a;
  }

 private:
  size_t base_guard_size_ = 256;
  size_t limit_guard_size_ = 4 * 1024;
  size_t usable_size_ = 8 * 1024;
  size_t align_log2_ = 4;

  static const size_t kDefaultBaseGuardSize = 256;
  static const size_t kDefaultLimitGuardSize = 4 * 1024;
  static const size_t kDefaultUsableSize = 8 * 1024;
};

// Armv8.5 MTE helpers.
inline int GetAllocationTagFromAddress(uint64_t address) {
  return static_cast<int>(ExtractUnsignedBitfield64(59, 56, address));
}

template <typename T>
T AddressUntag(T address) {
  // Cast the address using a C-style cast. A reinterpret_cast would be
  // appropriate, but it can't cast one integral type to another.
  uint64_t bits = (uint64_t)address;
  return (T)(bits & ~kAddressTagMask);
}

// A callback function, called when a function has been intercepted if a
// BranchInterception entry exists in branch_interceptions. The address of
// the intercepted function is passed to the callback. For usage see
// BranchInterception.
using InterceptionCallback = std::function<void(uint64_t)>;

class MetaDataDepot {
 public:
  class MetaDataMTE {
   public:
    explicit MetaDataMTE(int tag) : tag_(tag) {}

    int GetTag() const { return tag_; }
    void SetTag(int tag) {
      VIXL_ASSERT(IsUint4(tag));
      tag_ = tag;
    }

    static bool IsActive() { return is_active; }
    static void SetActive(bool value) { is_active = value; }

   private:
    static bool is_active;
    int16_t tag_;

    friend class MetaDataDepot;
  };

  // Generate a key for metadata recording from a untagged address.
  template <typename T>
  uint64_t GenerateMTEkey(T address) const {
    // Cast the address using a C-style cast. A reinterpret_cast would be
    // appropriate, but it can't cast one integral type to another.
    return (uint64_t)(AddressUntag(address)) >> kMTETagGranuleInBytesLog2;
  }

  template <typename R, typename T>
  R GetAttribute(T map, uint64_t key) {
    auto pair = map->find(key);
    R value = (pair == map->end()) ? nullptr : &pair->second;
    return value;
  }

  template <typename T>
  int GetMTETag(T address, Instruction const* pc = nullptr) {
    uint64_t key = GenerateMTEkey(address);
    MetaDataMTE* m = GetAttribute<MetaDataMTE*>(&metadata_mte_, key);

    if (!m) {
      std::stringstream sstream;
      sstream << std::hex << "MTE ERROR : instruction at 0x"
              << reinterpret_cast<uint64_t>(pc)
              << " touched a unallocated memory location 0x"
              << (uint64_t)(address) << ".\n";
      VIXL_ABORT_WITH_MSG(sstream.str().c_str());
    }

    return m->GetTag();
  }

  template <typename T>
  void SetMTETag(T address, int tag, Instruction const* pc = nullptr) {
    VIXL_ASSERT(IsAligned((uintptr_t)address, kMTETagGranuleInBytes));
    uint64_t key = GenerateMTEkey(address);
    MetaDataMTE* m = GetAttribute<MetaDataMTE*>(&metadata_mte_, key);

    if (!m) {
      metadata_mte_.insert({key, MetaDataMTE(tag)});
    } else {
      // Overwrite
      if (m->GetTag() == tag) {
        std::stringstream sstream;
        sstream << std::hex << "MTE WARNING : instruction at 0x"
                << reinterpret_cast<uint64_t>(pc)
                << ", the same tag is assigned to the address 0x"
                << (uint64_t)(address) << ".\n";
        VIXL_WARNING(sstream.str().c_str());
      }
      m->SetTag(tag);
    }
  }

  template <typename T>
  size_t CleanMTETag(T address) {
    VIXL_ASSERT(
        IsAligned(reinterpret_cast<uintptr_t>(address), kMTETagGranuleInBytes));
    uint64_t key = GenerateMTEkey(address);
    return metadata_mte_.erase(key);
  }

  size_t GetTotalCountMTE() { return metadata_mte_.size(); }

  // A pure virtual struct that allows the templated BranchInterception struct
  // to be stored. For more information see BranchInterception.
  struct BranchInterceptionAbstract {
    virtual ~BranchInterceptionAbstract() {}
    // Call the callback_ if one exists, otherwise do a RuntimeCall.
    virtual void operator()(Simulator* simulator) const = 0;
  };

  // An entry denoting a function to intercept when branched to during
  // simulator execution. When a function is intercepted the callback will be
  // called if one exists otherwise the function will be passed to
  // RuntimeCall.
  template <typename R, typename... P>
  struct BranchInterception : public BranchInterceptionAbstract {
    BranchInterception(R (*function)(P...),
                       InterceptionCallback callback = nullptr)
        : function_(function), callback_(callback) {}

    void operator()(Simulator* simulator) const VIXL_OVERRIDE;

   private:
    // Pointer to the function that will be intercepted.
    R (*function_)(P...);

    // Function to be called instead of function_
    InterceptionCallback callback_;
  };

  // Register a new BranchInterception object. If 'function' is branched to
  // (e.g: "blr function") in the future; instead, if provided, 'callback' will
  // be called otherwise a runtime call will be performed on 'function'.
  //
  // For example: this can be used to always perform runtime calls on
  // non-AArch64 functions without using the macroassembler.
  //
  // Note: only unconditional branches to registers are currently supported to
  // be intercepted, e.g: "br"/"blr".
  //
  // TODO: support intercepting other branch types.
  template <typename R, typename... P>
  void RegisterBranchInterception(R (*function)(P...),
                                  InterceptionCallback callback = nullptr) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(function);
    std::unique_ptr<BranchInterceptionAbstract> intercept =
        std::make_unique<BranchInterception<R, P...>>(function, callback);
    branch_interceptions_.insert(std::make_pair(addr, std::move(intercept)));
  }

  // Search for branch interceptions to the branch_target address; If one is
  // found return it otherwise return nullptr.
  BranchInterceptionAbstract* FindBranchInterception(uint64_t branch_target) {
    // Check for interceptions to the target address, if one is found, call it.
    auto search = branch_interceptions_.find(branch_target);
    if (search != branch_interceptions_.end()) {
      return search->second.get();
    } else {
      return nullptr;
    }
  }

  void ResetState() { branch_interceptions_.clear(); }

 private:
  // Tag recording of each allocated memory in the tag-granule.
  std::unordered_map<uint64_t, class MetaDataMTE> metadata_mte_;

  // Store a map of addresses to be intercepted and their corresponding branch
  // interception object, see 'BranchInterception'.
  std::unordered_map<uintptr_t, std::unique_ptr<BranchInterceptionAbstract>>
      branch_interceptions_;
};


// Representation of memory, with typed getters and setters for access.
class Memory {
 public:
  explicit Memory(SimStack::Allocated stack) : stack_(std::move(stack)) {
    metadata_depot_ = nullptr;
  }

  const SimStack::Allocated& GetStack() { return stack_; }

  template <typename A>
  bool IsMTETagsMatched(A address, Instruction const* pc = nullptr) const {
    if (MetaDataDepot::MetaDataMTE::IsActive()) {
      // Cast the address using a C-style cast. A reinterpret_cast would be
      // appropriate, but it can't cast one integral type to another.
      uint64_t addr = (uint64_t)address;
      int pointer_tag = GetAllocationTagFromAddress(addr);
      int memory_tag = metadata_depot_->GetMTETag(AddressUntag(addr), pc);
      return pointer_tag == memory_tag;
    }
    return true;
  }

  template <typename T, typename A>
  std::optional<T> Read(A address, Instruction const* pc = nullptr) const {
    T value;
    VIXL_STATIC_ASSERT((sizeof(value) == 1) || (sizeof(value) == 2) ||
                       (sizeof(value) == 4) || (sizeof(value) == 8) ||
                       (sizeof(value) == 16));
    auto base = reinterpret_cast<const char*>(AddressUntag(address));
    if (stack_.IsAccessInGuardRegion(base, sizeof(value))) {
      VIXL_ABORT_WITH_MSG("Attempt to read from stack guard region");
    }
    if (!IsMTETagsMatched(address, pc)) {
      VIXL_ABORT_WITH_MSG("Tag mismatch.");
    }
    if (TryMemoryAccess(reinterpret_cast<uintptr_t>(base), sizeof(value)) ==
        MemoryAccessResult::Failure) {
      return std::nullopt;
    }
    memcpy(&value, base, sizeof(value));
    return value;
  }

  template <typename T, typename A>
  bool Write(A address, T value, Instruction const* pc = nullptr) const {
    VIXL_STATIC_ASSERT((sizeof(value) == 1) || (sizeof(value) == 2) ||
                       (sizeof(value) == 4) || (sizeof(value) == 8) ||
                       (sizeof(value) == 16));
    auto base = reinterpret_cast<char*>(AddressUntag(address));
    if (stack_.IsAccessInGuardRegion(base, sizeof(value))) {
      VIXL_ABORT_WITH_MSG("Attempt to write to stack guard region");
    }
    if (!IsMTETagsMatched(address, pc)) {
      VIXL_ABORT_WITH_MSG("Tag mismatch.");
    }
    if (TryMemoryAccess(reinterpret_cast<uintptr_t>(base), sizeof(value)) ==
        MemoryAccessResult::Failure) {
      return false;
    }
    memcpy(base, &value, sizeof(value));
    return true;
  }

  template <typename A>
  std::optional<uint64_t> ReadUint(int size_in_bytes, A address) const {
    switch (size_in_bytes) {
      case 1:
        return Read<uint8_t>(address);
      case 2:
        return Read<uint16_t>(address);
      case 4:
        return Read<uint32_t>(address);
      case 8:
        return Read<uint64_t>(address);
    }
    VIXL_UNREACHABLE();
    return 0;
  }

  template <typename A>
  std::optional<int64_t> ReadInt(int size_in_bytes, A address) const {
    switch (size_in_bytes) {
      case 1:
        return Read<int8_t>(address);
      case 2:
        return Read<int16_t>(address);
      case 4:
        return Read<int32_t>(address);
      case 8:
        return Read<int64_t>(address);
    }
    VIXL_UNREACHABLE();
    return 0;
  }

  template <typename A>
  bool Write(int size_in_bytes, A address, uint64_t value) const {
    switch (size_in_bytes) {
      case 1:
        return Write(address, static_cast<uint8_t>(value));
      case 2:
        return Write(address, static_cast<uint16_t>(value));
      case 4:
        return Write(address, static_cast<uint32_t>(value));
      case 8:
        return Write(address, value);
    }
    VIXL_UNREACHABLE();
    return false;
  }

  void AppendMetaData(MetaDataDepot* metadata_depot) {
    VIXL_ASSERT(metadata_depot != nullptr);
    VIXL_ASSERT(metadata_depot_ == nullptr);
    metadata_depot_ = metadata_depot;
  }

 private:
  SimStack::Allocated stack_;
  MetaDataDepot* metadata_depot_;
};

// Represent a register (r0-r31, v0-v31, z0-z31, p0-p15).
template <unsigned kMaxSizeInBits>
class SimRegisterBase {
 public:
  static const unsigned kMaxSizeInBytes = kMaxSizeInBits / kBitsPerByte;
  VIXL_STATIC_ASSERT((kMaxSizeInBytes * kBitsPerByte) == kMaxSizeInBits);

  SimRegisterBase() : size_in_bytes_(kMaxSizeInBytes) { Clear(); }

  unsigned GetSizeInBits() const { return size_in_bytes_ * kBitsPerByte; }
  unsigned GetSizeInBytes() const { return size_in_bytes_; }

  void SetSizeInBytes(unsigned size_in_bytes) {
    VIXL_ASSERT(size_in_bytes <= kMaxSizeInBytes);
    size_in_bytes_ = size_in_bytes;
  }
  void SetSizeInBits(unsigned size_in_bits) {
    VIXL_ASSERT(size_in_bits <= kMaxSizeInBits);
    VIXL_ASSERT((size_in_bits % kBitsPerByte) == 0);
    SetSizeInBytes(size_in_bits / kBitsPerByte);
  }

  // Write the specified value. The value is zero-extended if necessary.
  template <typename T>
  void Write(T new_value) {
    // All AArch64 registers are zero-extending.
    if (sizeof(new_value) < GetSizeInBytes()) Clear();
    WriteLane(new_value, 0);
    NotifyRegisterWrite();
  }
  template <typename T>
  VIXL_DEPRECATED("Write", void Set(T new_value)) {
    Write(new_value);
  }

  void Clear() {
    memset(value_, 0, kMaxSizeInBytes);
    NotifyRegisterWrite();
  }

  // Insert a typed value into a register, leaving the rest of the register
  // unchanged. The lane parameter indicates where in the register the value
  // should be inserted, in the range [ 0, sizeof(value_) / sizeof(T) ), where
  // 0 represents the least significant bits.
  template <typename T>
  void Insert(int lane, T new_value) {
    WriteLane(new_value, lane);
    NotifyRegisterWrite();
  }

  // Get the value as the specified type. The value is truncated if necessary.
  template <typename T>
  T Get() const {
    return GetLane<T>(0);
  }

  // Get the lane value as the specified type. The value is truncated if
  // necessary.
  template <typename T>
  T GetLane(int lane) const {
    T result;
    ReadLane(&result, lane);
    return result;
  }
  template <typename T>
  VIXL_DEPRECATED("GetLane", T Get(int lane) const) {
    return GetLane(lane);
  }

  // Get the value of a specific bit, indexed from the least-significant bit of
  // lane 0.
  bool GetBit(int bit) const {
    int bit_in_byte = bit % (sizeof(value_[0]) * kBitsPerByte);
    int byte = bit / (sizeof(value_[0]) * kBitsPerByte);
    return ((value_[byte] >> bit_in_byte) & 1) != 0;
  }

  // Return a pointer to the raw, underlying byte array.
  const uint8_t* GetBytes() const { return value_; }

  // TODO: Make this return a map of updated bytes, so that we can highlight
  // updated lanes for load-and-insert. (That never happens for scalar code, but
  // NEON has some instructions that can update individual lanes.)
  bool WrittenSinceLastLog() const { return written_since_last_log_; }

  void NotifyRegisterLogged() { written_since_last_log_ = false; }

 protected:
  uint8_t value_[kMaxSizeInBytes];

  unsigned size_in_bytes_;

  // Helpers to aid with register tracing.
  bool written_since_last_log_;

  void NotifyRegisterWrite() { written_since_last_log_ = true; }

 private:
  template <typename T>
  void ReadLane(T* dst, int lane) const {
    VIXL_ASSERT(lane >= 0);
    VIXL_ASSERT((sizeof(*dst) + (lane * sizeof(*dst))) <= GetSizeInBytes());
    memcpy(dst, &value_[lane * sizeof(*dst)], sizeof(*dst));
  }

  template <typename T>
  void WriteLane(T src, int lane) {
    VIXL_ASSERT(lane >= 0);
    VIXL_ASSERT((sizeof(src) + (lane * sizeof(src))) <= GetSizeInBytes());
    memcpy(&value_[lane * sizeof(src)], &src, sizeof(src));
  }

  // The default ReadLane and WriteLane methods assume what we are copying is
  // "trivially copyable" by using memcpy. We have to provide alternative
  // implementations for SimFloat16 which cannot be copied this way.

  void ReadLane(vixl::internal::SimFloat16* dst, int lane) const {
    uint16_t rawbits;
    ReadLane(&rawbits, lane);
    *dst = RawbitsToFloat16(rawbits);
  }

  void WriteLane(vixl::internal::SimFloat16 src, int lane) {
    WriteLane(Float16ToRawbits(src), lane);
  }
};

typedef SimRegisterBase<kXRegSize> SimRegister;      // r0-r31
typedef SimRegisterBase<kPRegMaxSize> SimPRegister;  // p0-p15
// FFR has the same format as a predicate register.
typedef SimPRegister SimFFRRegister;

// v0-v31 and z0-z31
class SimVRegister : public SimRegisterBase<kZRegMaxSize> {
 public:
  SimVRegister() : SimRegisterBase<kZRegMaxSize>(), accessed_as_z_(false) {}

  void NotifyAccessAsZ() { accessed_as_z_ = true; }

  void NotifyRegisterLogged() {
    SimRegisterBase<kZRegMaxSize>::NotifyRegisterLogged();
    accessed_as_z_ = false;
  }

  bool AccessedAsZSinceLastLog() const { return accessed_as_z_; }

 private:
  bool accessed_as_z_;
};

// Representation of a SVE predicate register.
class LogicPRegister {
 public:
  inline LogicPRegister(
      SimPRegister& other)  // NOLINT(runtime/references)(runtime/explicit)
      : register_(other) {}

  // Set a conveniently-sized block to 16 bits as the minimum predicate length
  // is 16 bits and allow to be increased to multiples of 16 bits.
  typedef uint16_t ChunkType;

  // Assign a bit into the end positon of the specified lane.
  // The bit is zero-extended if necessary.
  void SetActive(VectorFormat vform, int lane_index, bool value) {
    int psize = LaneSizeInBytesFromFormat(vform);
    int bit_index = lane_index * psize;
    int byte_index = bit_index / kBitsPerByte;
    int bit_offset = bit_index % kBitsPerByte;
    uint8_t byte = register_.GetLane<uint8_t>(byte_index);
    register_.Insert(byte_index, ZeroExtend(byte, bit_offset, psize, value));
  }

  bool IsActive(VectorFormat vform, int lane_index) const {
    int psize = LaneSizeInBytesFromFormat(vform);
    int bit_index = lane_index * psize;
    int byte_index = bit_index / kBitsPerByte;
    int bit_offset = bit_index % kBitsPerByte;
    uint8_t byte = register_.GetLane<uint8_t>(byte_index);
    return ExtractBit(byte, bit_offset);
  }

  // The accessors for bulk processing.
  int GetChunkCount() const {
    VIXL_ASSERT((register_.GetSizeInBytes() % sizeof(ChunkType)) == 0);
    return register_.GetSizeInBytes() / sizeof(ChunkType);
  }

  ChunkType GetChunk(int lane) const { return GetActiveMask<ChunkType>(lane); }

  void SetChunk(int lane, ChunkType new_value) {
    SetActiveMask(lane, new_value);
  }

  void SetAllBits() {
    int chunk_size = sizeof(ChunkType) * kBitsPerByte;
    ChunkType bits = static_cast<ChunkType>(GetUintMask(chunk_size));
    for (int lane = 0;
         lane < (static_cast<int>(register_.GetSizeInBits() / chunk_size));
         lane++) {
      SetChunk(lane, bits);
    }
  }

  template <typename T>
  T GetActiveMask(int lane) const {
    return register_.GetLane<T>(lane);
  }

  template <typename T>
  void SetActiveMask(int lane, T new_value) {
    register_.Insert<T>(lane, new_value);
  }

  void Clear() { register_.Clear(); }

  bool Aliases(const LogicPRegister& other) const {
    return &register_ == &other.register_;
  }

 private:
  // The bit assignment is zero-extended to fill the size of predicate element.
  uint8_t ZeroExtend(uint8_t byte, int index, int psize, bool value) {
    VIXL_ASSERT(index >= 0);
    VIXL_ASSERT(index + psize <= kBitsPerByte);
    int bits = value ? 1 : 0;
    switch (psize) {
      case 1:
        AssignBit(byte, index, bits);
        break;
      case 2:
        AssignBits(byte, index, 0x03, bits);
        break;
      case 4:
        AssignBits(byte, index, 0x0f, bits);
        break;
      case 8:
        AssignBits(byte, index, 0xff, bits);
        break;
      default:
        VIXL_UNREACHABLE();
        return 0;
    }
    return byte;
  }

  SimPRegister& register_;
};

using vixl_uint128_t = std::pair<uint64_t, uint64_t>;

// Representation of a vector register, with typed getters and setters for lanes
// and additional information to represent lane state.
class LogicVRegister {
 public:
  inline LogicVRegister(
      SimVRegister& other)  // NOLINT(runtime/references)(runtime/explicit)
      : register_(other) {
    for (size_t i = 0; i < ArrayLength(saturated_); i++) {
      saturated_[i] = kNotSaturated;
    }
    for (size_t i = 0; i < ArrayLength(round_); i++) {
      round_[i] = 0;
    }
  }

  int64_t Int(VectorFormat vform, int index) const {
    if (IsSVEFormat(vform)) register_.NotifyAccessAsZ();
    int64_t element;
    switch (LaneSizeInBitsFromFormat(vform)) {
      case 8:
        element = register_.GetLane<int8_t>(index);
        break;
      case 16:
        element = register_.GetLane<int16_t>(index);
        break;
      case 32:
        element = register_.GetLane<int32_t>(index);
        break;
      case 64:
        element = register_.GetLane<int64_t>(index);
        break;
      default:
        VIXL_UNREACHABLE();
        return 0;
    }
    return element;
  }

  uint64_t Uint(VectorFormat vform, int index) const {
    if (IsSVEFormat(vform)) register_.NotifyAccessAsZ();
    uint64_t element;
    switch (LaneSizeInBitsFromFormat(vform)) {
      case 8:
        element = register_.GetLane<uint8_t>(index);
        break;
      case 16:
        element = register_.GetLane<uint16_t>(index);
        break;
      case 32:
        element = register_.GetLane<uint32_t>(index);
        break;
      case 64:
        element = register_.GetLane<uint64_t>(index);
        break;
      default:
        VIXL_UNREACHABLE();
        return 0;
    }
    return element;
  }

  int UintArray(VectorFormat vform, uint64_t* dst) const {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      dst[i] = Uint(vform, i);
    }
    return LaneCountFromFormat(vform);
  }

  uint64_t UintLeftJustified(VectorFormat vform, int index) const {
    return Uint(vform, index) << (64 - LaneSizeInBitsFromFormat(vform));
  }

  int64_t IntLeftJustified(VectorFormat vform, int index) const {
    uint64_t value = UintLeftJustified(vform, index);
    int64_t result;
    memcpy(&result, &value, sizeof(result));
    return result;
  }

  void SetInt(VectorFormat vform, int index, int64_t value) const {
    if (IsSVEFormat(vform)) register_.NotifyAccessAsZ();
    switch (LaneSizeInBitsFromFormat(vform)) {
      case 8:
        register_.Insert(index, static_cast<int8_t>(value));
        break;
      case 16:
        register_.Insert(index, static_cast<int16_t>(value));
        break;
      case 32:
        register_.Insert(index, static_cast<int32_t>(value));
        break;
      case 64:
        register_.Insert(index, static_cast<int64_t>(value));
        break;
      default:
        VIXL_UNREACHABLE();
        return;
    }
  }

  void SetIntArray(VectorFormat vform, const int64_t* src) const {
    ClearForWrite(vform);
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      SetInt(vform, i, src[i]);
    }
  }

  void SetUint(VectorFormat vform, int index, uint64_t value) const {
    if (IsSVEFormat(vform)) register_.NotifyAccessAsZ();
    switch (LaneSizeInBitsFromFormat(vform)) {
      case 8:
        register_.Insert(index, static_cast<uint8_t>(value));
        break;
      case 16:
        register_.Insert(index, static_cast<uint16_t>(value));
        break;
      case 32:
        register_.Insert(index, static_cast<uint32_t>(value));
        break;
      case 64:
        register_.Insert(index, static_cast<uint64_t>(value));
        break;
      default:
        VIXL_UNREACHABLE();
        return;
    }
  }

  void SetUint(VectorFormat vform, int index, vixl_uint128_t value) const {
    if (LaneSizeInBitsFromFormat(vform) <= 64) {
      SetUint(vform, index, value.second);
      return;
    }
    VIXL_ASSERT((vform == kFormat1Q) || (vform == kFormatVnQ));
    SetUint(kFormatVnD, 2 * index, value.second);
    SetUint(kFormatVnD, 2 * index + 1, value.first);
  }

  void SetUintArray(VectorFormat vform, const uint64_t* src) const {
    ClearForWrite(vform);
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      SetUint(vform, i, src[i]);
    }
  }

  template <typename T>
  T Float(int index) const {
    return register_.GetLane<T>(index);
  }

  template <typename T>
  void SetFloat(int index, T value) const {
    register_.Insert(index, value);
  }

  template <typename T>
  void SetFloat(VectorFormat vform, int index, T value) const {
    if (IsSVEFormat(vform)) register_.NotifyAccessAsZ();
    register_.Insert(index, value);
  }

  void Clear() { register_.Clear(); }

  // When setting a result in a register larger than the result itself, the top
  // bits of the register must be cleared.
  void ClearForWrite(VectorFormat vform) const {
    // SVE destinations write whole registers, so we have nothing to clear.
    if (IsSVEFormat(vform)) return;

    unsigned size = RegisterSizeInBytesFromFormat(vform);
    for (unsigned i = size; i < register_.GetSizeInBytes(); i++) {
      SetUint(kFormat16B, i, 0);
    }
  }

  // Saturation state for each lane of a vector.
  enum Saturation {
    kNotSaturated = 0,
    kSignedSatPositive = 1 << 0,
    kSignedSatNegative = 1 << 1,
    kSignedSatMask = kSignedSatPositive | kSignedSatNegative,
    kSignedSatUndefined = kSignedSatMask,
    kUnsignedSatPositive = 1 << 2,
    kUnsignedSatNegative = 1 << 3,
    kUnsignedSatMask = kUnsignedSatPositive | kUnsignedSatNegative,
    kUnsignedSatUndefined = kUnsignedSatMask
  };

  // Getters for saturation state.
  Saturation GetSignedSaturation(int index) {
    return static_cast<Saturation>(saturated_[index] & kSignedSatMask);
  }

  Saturation GetUnsignedSaturation(int index) {
    return static_cast<Saturation>(saturated_[index] & kUnsignedSatMask);
  }

  // Setters for saturation state.
  void ClearSat(int index) { saturated_[index] = kNotSaturated; }

  void SetSignedSat(int index, bool positive) {
    SetSatFlag(index, positive ? kSignedSatPositive : kSignedSatNegative);
  }

  void SetUnsignedSat(int index, bool positive) {
    SetSatFlag(index, positive ? kUnsignedSatPositive : kUnsignedSatNegative);
  }

  void SetSatFlag(int index, Saturation sat) {
    saturated_[index] = static_cast<Saturation>(saturated_[index] | sat);
    VIXL_ASSERT((sat & kUnsignedSatMask) != kUnsignedSatUndefined);
    VIXL_ASSERT((sat & kSignedSatMask) != kSignedSatUndefined);
  }

  // Saturate lanes of a vector based on saturation state.
  LogicVRegister& SignedSaturate(VectorFormat vform) {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      Saturation sat = GetSignedSaturation(i);
      if (sat == kSignedSatPositive) {
        SetInt(vform, i, MaxIntFromFormat(vform));
      } else if (sat == kSignedSatNegative) {
        SetInt(vform, i, MinIntFromFormat(vform));
      }
    }
    return *this;
  }

  LogicVRegister& UnsignedSaturate(VectorFormat vform) {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      Saturation sat = GetUnsignedSaturation(i);
      if (sat == kUnsignedSatPositive) {
        SetUint(vform, i, MaxUintFromFormat(vform));
      } else if (sat == kUnsignedSatNegative) {
        SetUint(vform, i, 0);
      }
    }
    return *this;
  }

  // Getter for rounding state.
  bool GetRounding(int index) { return round_[index]; }

  // Setter for rounding state.
  void SetRounding(int index, bool round) { round_[index] = round; }

  // Round lanes of a vector based on rounding state.
  LogicVRegister& Round(VectorFormat vform) {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      SetUint(vform, i, Uint(vform, i) + (GetRounding(i) ? 1 : 0));
    }
    return *this;
  }

  // Unsigned halve lanes of a vector, and use the saturation state to set the
  // top bit.
  LogicVRegister& Uhalve(VectorFormat vform) {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      uint64_t val = Uint(vform, i);
      SetRounding(i, (val & 1) == 1);
      val >>= 1;
      if (GetUnsignedSaturation(i) != kNotSaturated) {
        // If the operation causes unsigned saturation, the bit shifted into the
        // most significant bit must be set.
        val |= (MaxUintFromFormat(vform) >> 1) + 1;
      }
      SetInt(vform, i, val);
    }
    return *this;
  }

  // Signed halve lanes of a vector, and use the carry state to set the top bit.
  LogicVRegister& Halve(VectorFormat vform) {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      int64_t val = Int(vform, i);
      SetRounding(i, (val & 1) == 1);
      val = ExtractSignedBitfield64(63, 1, val);  // >>= 1
      if (GetSignedSaturation(i) == kNotSaturated) {
        SetInt(vform, i, val);
      } else {
        // If the operation causes signed saturation, the sign bit must be
        // inverted.
        uint64_t uval = static_cast<uint64_t>(val);
        SetUint(vform, i, uval ^ ((MaxUintFromFormat(vform) >> 1) + 1));
      }
    }
    return *this;
  }

  int LaneCountFromFormat(VectorFormat vform) const {
    if (IsSVEFormat(vform)) {
      return register_.GetSizeInBits() / LaneSizeInBitsFromFormat(vform);
    } else {
      return vixl::aarch64::LaneCountFromFormat(vform);
    }
  }

 private:
  SimVRegister& register_;

  // Allocate one saturation state entry per lane; largest register is type Q,
  // and lanes can be a minimum of one byte wide.
  Saturation saturated_[kZRegMaxSizeInBytes];

  // Allocate one rounding state entry per lane.
  bool round_[kZRegMaxSizeInBytes];
};

// Represent an SVE addressing mode and abstract per-lane address generation to
// make iteration easy.
//
// Contiguous accesses are described with a simple base address, the memory
// occupied by each lane (`SetMsizeInBytesLog2()`) and the number of elements in
// each struct (`SetRegCount()`).
//
// Scatter-gather accesses also require a SimVRegister and information about how
// to extract lanes from it.
class LogicSVEAddressVector {
 public:
  // scalar-plus-scalar
  // scalar-plus-immediate
  explicit LogicSVEAddressVector(uint64_t base)
      : base_(base),
        msize_in_bytes_log2_(kUnknownMsizeInBytesLog2),
        reg_count_(1),
        vector_(NULL),
        vector_form_(kFormatUndefined),
        vector_mod_(NO_SVE_OFFSET_MODIFIER),
        vector_shift_(0) {}

  // scalar-plus-vector
  // vector-plus-immediate
  //    `base` should be the constant used for each element. That is, the value
  //    of `xn`, or `#<imm>`.
  //    `vector` should be the SimVRegister with offsets for each element. The
  //    vector format must be specified; SVE scatter/gather accesses typically
  //    support both 32-bit and 64-bit addressing.
  //
  //    `mod` and `shift` correspond to the modifiers applied to each element in
  //    scalar-plus-vector forms, such as those used for unpacking and
  //    sign-extension. They are not used for vector-plus-immediate.
  LogicSVEAddressVector(uint64_t base,
                        const SimVRegister* vector,
                        VectorFormat vform,
                        SVEOffsetModifier mod = NO_SVE_OFFSET_MODIFIER,
                        int shift = 0)
      : base_(base),
        msize_in_bytes_log2_(kUnknownMsizeInBytesLog2),
        reg_count_(1),
        vector_(vector),
        vector_form_(vform),
        vector_mod_(mod),
        vector_shift_(shift) {}

  // Set `msize` -- the memory occupied by each lane -- for address
  // calculations.
  void SetMsizeInBytesLog2(int msize_in_bytes_log2) {
    VIXL_ASSERT(msize_in_bytes_log2 >= static_cast<int>(kBRegSizeInBytesLog2));
    VIXL_ASSERT(msize_in_bytes_log2 <= static_cast<int>(kDRegSizeInBytesLog2));
    msize_in_bytes_log2_ = msize_in_bytes_log2;
  }

  bool HasMsize() const {
    return msize_in_bytes_log2_ != kUnknownMsizeInBytesLog2;
  }

  int GetMsizeInBytesLog2() const {
    VIXL_ASSERT(HasMsize());
    return msize_in_bytes_log2_;
  }
  int GetMsizeInBitsLog2() const {
    return GetMsizeInBytesLog2() + kBitsPerByteLog2;
  }

  int GetMsizeInBytes() const { return 1 << GetMsizeInBytesLog2(); }
  int GetMsizeInBits() const { return 1 << GetMsizeInBitsLog2(); }

  void SetRegCount(int reg_count) {
    VIXL_ASSERT(reg_count >= 1);  // E.g. ld1/st1
    VIXL_ASSERT(reg_count <= 4);  // E.g. ld4/st4
    reg_count_ = reg_count;
  }

  int GetRegCount() const { return reg_count_; }

  // Full per-element address calculation for structured accesses.
  //
  // Note that the register number argument (`reg`) is zero-based.
  uint64_t GetElementAddress(int lane, int reg) const {
    VIXL_ASSERT(reg < GetRegCount());
    // Individual structures are always contiguous in memory, so this
    // implementation works for both contiguous and scatter-gather addressing.
    return GetStructAddress(lane) + (reg * GetMsizeInBytes());
  }

  // Full per-struct address calculation for structured accesses.
  uint64_t GetStructAddress(int lane) const;

  bool IsContiguous() const { return vector_ == NULL; }
  bool IsScatterGather() const { return !IsContiguous(); }

 private:
  uint64_t base_;
  int msize_in_bytes_log2_;
  int reg_count_;

  const SimVRegister* vector_;
  VectorFormat vector_form_;
  SVEOffsetModifier vector_mod_;
  int vector_shift_;

  static const int kUnknownMsizeInBytesLog2 = -1;
};

// The proper way to initialize a simulated system register (such as NZCV) is as
// follows:
//  SimSystemRegister nzcv = SimSystemRegister::DefaultValueFor(NZCV);
class SimSystemRegister {
 public:
  // The default constructor represents a register which has no writable bits.
  // It is not possible to set its value to anything other than 0.
  SimSystemRegister() : value_(0), write_ignore_mask_(0xffffffff) {}

  uint32_t GetRawValue() const { return value_; }
  VIXL_DEPRECATED("GetRawValue", uint32_t RawValue() const) {
    return GetRawValue();
  }

  void SetRawValue(uint32_t new_value) {
    value_ = (value_ & write_ignore_mask_) | (new_value & ~write_ignore_mask_);
  }

  uint32_t ExtractBits(int msb, int lsb) const {
    return ExtractUnsignedBitfield32(msb, lsb, value_);
  }
  VIXL_DEPRECATED("ExtractBits", uint32_t Bits(int msb, int lsb) const) {
    return ExtractBits(msb, lsb);
  }

  int32_t ExtractSignedBits(int msb, int lsb) const {
    return ExtractSignedBitfield32(msb, lsb, value_);
  }
  VIXL_DEPRECATED("ExtractSignedBits",
                  int32_t SignedBits(int msb, int lsb) const) {
    return ExtractSignedBits(msb, lsb);
  }

  void SetBits(int msb, int lsb, uint32_t bits);

  // Default system register values.
  static SimSystemRegister DefaultValueFor(SystemRegister id);

#define DEFINE_GETTER(Name, HighBit, LowBit, Func)                            \
  uint32_t Get##Name() const { return this->Func(HighBit, LowBit); }          \
  VIXL_DEPRECATED("Get" #Name, uint32_t Name() const) { return Get##Name(); } \
  void Set##Name(uint32_t bits) { SetBits(HighBit, LowBit, bits); }
#define DEFINE_WRITE_IGNORE_MASK(Name, Mask) \
  static const uint32_t Name##WriteIgnoreMask = ~static_cast<uint32_t>(Mask);

  SYSTEM_REGISTER_FIELDS_LIST(DEFINE_GETTER, DEFINE_WRITE_IGNORE_MASK)

#undef DEFINE_ZERO_BITS
#undef DEFINE_GETTER

 protected:
  // Most system registers only implement a few of the bits in the word. Other
  // bits are "read-as-zero, write-ignored". The write_ignore_mask argument
  // describes the bits which are not modifiable.
  SimSystemRegister(uint32_t value, uint32_t write_ignore_mask)
      : value_(value), write_ignore_mask_(write_ignore_mask) {}

  uint32_t value_;
  uint32_t write_ignore_mask_;
};


class SimExclusiveLocalMonitor {
 public:
  SimExclusiveLocalMonitor() : kSkipClearProbability(8), seed_(0x87654321) {
    Clear();
  }

  // Clear the exclusive monitor (like clrex).
  void Clear() {
    address_ = 0;
    size_ = 0;
  }

  // Clear the exclusive monitor most of the time.
  void MaybeClear() {
    if ((seed_ % kSkipClearProbability) != 0) {
      Clear();
    }

    // Advance seed_ using a simple linear congruential generator.
    seed_ = (seed_ * 48271) % 2147483647;
  }

  // Mark the address range for exclusive access (like load-exclusive).
  void MarkExclusive(uint64_t address, size_t size) {
    address_ = address;
    size_ = size;
  }

  // Return true if the address range is marked (like store-exclusive).
  // This helper doesn't implicitly clear the monitor.
  bool IsExclusive(uint64_t address, size_t size) {
    VIXL_ASSERT(size > 0);
    // Be pedantic: Require both the address and the size to match.
    return (size == size_) && (address == address_);
  }

 private:
  uint64_t address_;
  size_t size_;

  const int kSkipClearProbability;
  uint32_t seed_;
};


// We can't accurate simulate the global monitor since it depends on external
// influences. Instead, this implementation occasionally causes accesses to
// fail, according to kPassProbability.
class SimExclusiveGlobalMonitor {
 public:
  SimExclusiveGlobalMonitor() : kPassProbability(8), seed_(0x87654321) {}

  bool IsExclusive(uint64_t address, size_t size) {
    USE(address, size);

    bool pass = (seed_ % kPassProbability) != 0;
    // Advance seed_ using a simple linear congruential generator.
    seed_ = (seed_ * 48271) % 2147483647;
    return pass;
  }

 private:
  const int kPassProbability;
  uint32_t seed_;
};

class Debugger;

template <uint32_t mode>
uint64_t CryptoOp(uint64_t x, uint64_t y, uint64_t z);

class Simulator : public DecoderVisitor {
 public:
  explicit Simulator(Decoder* decoder,
                     FILE* stream = stdout,
                     SimStack::Allocated stack = SimStack().Allocate());
  ~Simulator();

  void ResetState();

  // Run the simulator.
  virtual void Run();
  void RunFrom(const Instruction* first);


#if defined(VIXL_HAS_ABI_SUPPORT) && __cplusplus >= 201103L && \
    (defined(_MSC_VER) || defined(__clang__) || GCC_VERSION_OR_NEWER(4, 9, 1))
  // Templated `RunFrom` version taking care of passing arguments and returning
  // the result value.
  // This allows code like:
  //    int32_t res = simulator.RunFrom<int32_t, int32_t>(GenerateCode(),
  //                                                      0x123);
  // It requires VIXL's ABI features, and C++11 or greater.
  // Also, the initialisation of tuples is incorrect in GCC before 4.9.1:
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51253
  template <typename R, typename... P>
  R RunFrom(const Instruction* code, P... arguments) {
    return RunFromStructHelper<R, P...>::Wrapper(this, code, arguments...);
  }

  template <typename R, typename... P>
  struct RunFromStructHelper {
    static R Wrapper(Simulator* simulator,
                     const Instruction* code,
                     P... arguments) {
      ABI abi;
      std::tuple<P...> unused_tuple{
          // TODO: We currently do not support arguments passed on the stack. We
          // could do so by using `WriteGenericOperand()` here, but may need to
          // add features to handle situations where the stack is or is not set
          // up.
          (simulator->WriteCPURegister(abi.GetNextParameterGenericOperand<P>()
                                           .GetCPURegister(),
                                       arguments),
           arguments)...};
      simulator->RunFrom(code);
      return simulator->ReadGenericOperand<R>(abi.GetReturnGenericOperand<R>());
    }
  };

  // Partial specialization when the return type is `void`.
  template <typename... P>
  struct RunFromStructHelper<void, P...> {
    static void Wrapper(Simulator* simulator,
                        const Instruction* code,
                        P... arguments) {
      ABI abi;
      std::tuple<P...> unused_tuple{
          // TODO: We currently do not support arguments passed on the stack. We
          // could do so by using `WriteGenericOperand()` here, but may need to
          // add features to handle situations where the stack is or is not set
          // up.
          (simulator->WriteCPURegister(abi.GetNextParameterGenericOperand<P>()
                                           .GetCPURegister(),
                                       arguments),
           arguments)...};
      simulator->RunFrom(code);
    }
  };
#endif

  // Execution ends when the PC hits this address.
  static const Instruction* kEndOfSimAddress;

  // Simulation helpers.
  bool IsSimulationFinished() const { return pc_ == kEndOfSimAddress; }

  const Instruction* ReadPc() const { return pc_; }
  VIXL_DEPRECATED("ReadPc", const Instruction* pc() const) { return ReadPc(); }

  enum BranchLogMode { LogBranches, NoBranchLog };

  void WritePc(const Instruction* new_pc,
               BranchLogMode log_mode = LogBranches) {
    if (log_mode == LogBranches) LogTakenBranch(new_pc);
    pc_ = AddressUntag(new_pc);
    pc_modified_ = true;
  }
  VIXL_DEPRECATED("WritePc", void set_pc(const Instruction* new_pc)) {
    return WritePc(new_pc);
  }

  void IncrementPc() {
    if (!pc_modified_) {
      pc_ = pc_->GetNextInstruction();
    }
  }
  VIXL_DEPRECATED("IncrementPc", void increment_pc()) { IncrementPc(); }

  BType ReadBType() const { return btype_; }
  void WriteNextBType(BType btype) { next_btype_ = btype; }
  void UpdateBType() {
    btype_ = next_btype_;
    next_btype_ = DefaultBType;
  }

  // Helper function to determine BType for branches.
  BType GetBTypeFromInstruction(const Instruction* instr) const;

  bool PcIsInGuardedPage() const { return guard_pages_; }
  void SetGuardedPages(bool guard_pages) { guard_pages_ = guard_pages; }

  const Instruction* GetLastExecutedInstruction() const { return last_instr_; }

  void ExecuteInstruction() {
    // The program counter should always be aligned.
    VIXL_ASSERT(IsWordAligned(pc_));
    pc_modified_ = false;

    // On guarded pages, if BType is not zero, take an exception on any
    // instruction other than BTI, PACI[AB]SP, HLT or BRK.
    if (PcIsInGuardedPage() && (ReadBType() != DefaultBType)) {
      if (pc_->IsPAuth()) {
        Instr i = pc_->Mask(SystemPAuthMask);
        if ((i != PACIASP) && (i != PACIBSP)) {
          VIXL_ABORT_WITH_MSG(
              "Executing non-BTI instruction with wrong BType.");
        }
      } else if (!pc_->IsBti() && !pc_->IsException()) {
        VIXL_ABORT_WITH_MSG("Executing non-BTI instruction with wrong BType.");
      }
    }

    bool last_instr_was_movprfx =
        (form_hash_ == "movprfx_z_z"_h) || (form_hash_ == "movprfx_z_p_z"_h);

    // decoder_->Decode(...) triggers at least the following visitors:
    //  1. The CPUFeaturesAuditor (`cpu_features_auditor_`).
    //  2. The PrintDisassembler (`print_disasm_`), if enabled.
    //  3. The Simulator (`this`).
    // User can add additional visitors at any point, but the Simulator requires
    // that the ordering above is preserved.
    decoder_->Decode(pc_);

    if (last_instr_was_movprfx) {
      VIXL_ASSERT(last_instr_ != NULL);
      VIXL_CHECK(pc_->CanTakeSVEMovprfx(form_hash_, last_instr_));
    }

    last_instr_ = ReadPc();
    IncrementPc();
    LogAllWrittenRegisters();
    UpdateBType();

    VIXL_CHECK(cpu_features_auditor_.InstructionIsAvailable());
  }

  virtual void Visit(Metadata* metadata,
                     const Instruction* instr) VIXL_OVERRIDE;

#define DECLARE(A) virtual void Visit##A(const Instruction* instr);
  VISITOR_LIST_THAT_RETURN(DECLARE)
#undef DECLARE
#define DECLARE(A) \
  VIXL_NO_RETURN virtual void Visit##A(const Instruction* instr);
  VISITOR_LIST_THAT_DONT_RETURN(DECLARE)
#undef DECLARE

  void Simulate_PdT_PgZ_ZnT_ZmT(const Instruction* instr);
  void Simulate_PdT_Xn_Xm(const Instruction* instr);
  void Simulate_ZdB_Zn1B_Zn2B_imm(const Instruction* instr);
  void Simulate_ZdB_ZnB_ZmB(const Instruction* instr);
  void Simulate_ZdD_ZnD_ZmD_imm(const Instruction* instr);
  void Simulate_ZdH_PgM_ZnS(const Instruction* instr);
  void Simulate_ZdH_ZnH_ZmH_imm(const Instruction* instr);
  void Simulate_ZdS_PgM_ZnD(const Instruction* instr);
  void Simulate_ZdS_PgM_ZnS(const Instruction* instr);
  void Simulate_ZdS_ZnS_ZmS_imm(const Instruction* instr);
  void Simulate_ZdT_PgM_ZnT(const Instruction* instr);
  void Simulate_ZdT_PgZ_ZnT_ZmT(const Instruction* instr);
  void Simulate_ZdT_ZnT_ZmT(const Instruction* instr);
  void Simulate_ZdT_ZnT_ZmTb(const Instruction* instr);
  void Simulate_ZdT_ZnT_const(const Instruction* instr);
  void Simulate_ZdaD_ZnS_ZmS_imm(const Instruction* instr);
  void Simulate_ZdaH_ZnH_ZmH_imm_const(const Instruction* instr);
  void Simulate_ZdaS_ZnH_ZmH(const Instruction* instr);
  void Simulate_ZdaS_ZnH_ZmH_imm(const Instruction* instr);
  void Simulate_ZdaS_ZnS_ZmS_imm_const(const Instruction* instr);
  void Simulate_ZdaT_PgM_ZnTb(const Instruction* instr);
  void Simulate_ZdaT_ZnT_ZmT(const Instruction* instr);
  void Simulate_ZdaT_ZnT_const(const Instruction* instr);
  void Simulate_ZdaT_ZnTb_ZmTb(const Instruction* instr);
  void Simulate_ZdnT_PgM_ZdnT_ZmT(const Instruction* instr);
  void Simulate_ZdnT_PgM_ZdnT_const(const Instruction* instr);
  void Simulate_ZdnT_ZdnT_ZmT_const(const Instruction* instr);
  void Simulate_ZtD_PgZ_ZnD_Xm(const Instruction* instr);
  void Simulate_ZtD_Pg_ZnD_Xm(const Instruction* instr);
  void Simulate_ZtS_PgZ_ZnS_Xm(const Instruction* instr);
  void Simulate_ZtS_Pg_ZnS_Xm(const Instruction* instr);

  void SimulateSVEHalvingAddSub(const Instruction* instr);
  void SimulateSVESaturatingArithmetic(const Instruction* instr);
  void SimulateSVEIntArithPair(const Instruction* instr);
  void SimulateSVENarrow(const Instruction* instr);
  void SimulateSVEInterleavedArithLong(const Instruction* instr);
  void SimulateSVEShiftLeftImm(const Instruction* instr);
  void SimulateSVEAddSubCarry(const Instruction* instr);
  void SimulateSVEAddSubHigh(const Instruction* instr);
  void SimulateSVEIntMulLongVec(const Instruction* instr);
  void SimulateSVESaturatingIntMulLongIdx(const Instruction* instr);
  void SimulateSVEExclusiveOrRotate(const Instruction* instr);
  void SimulateSVEBitwiseTernary(const Instruction* instr);
  void SimulateSVEComplexDotProduct(const Instruction* instr);
  void SimulateSVEMulIndex(const Instruction* instr);
  void SimulateSVEMlaMlsIndex(const Instruction* instr);
  void SimulateSVEComplexIntMulAdd(const Instruction* instr);
  void SimulateSVESaturatingMulAddHigh(const Instruction* instr);
  void SimulateSVESaturatingMulHighIndex(const Instruction* instr);
  void SimulateSVEFPConvertLong(const Instruction* instr);
  void SimulateSVEPmull128(const Instruction* instr);
  void SimulateMatrixMul(const Instruction* instr);
  void SimulateSVEFPMatrixMul(const Instruction* instr);
  void SimulateNEONMulByElementLong(const Instruction* instr);
  void SimulateNEONFPMulByElement(const Instruction* instr);
  void SimulateNEONFPMulByElementLong(const Instruction* instr);
  void SimulateNEONComplexMulByElement(const Instruction* instr);
  void SimulateNEONDotProdByElement(const Instruction* instr);
  void SimulateNEONSHA3(const Instruction* instr);
  void SimulateMTEAddSubTag(const Instruction* instr);
  void SimulateMTETagMaskInsert(const Instruction* instr);
  void SimulateMTESubPointer(const Instruction* instr);
  void SimulateMTELoadTag(const Instruction* instr);
  void SimulateMTEStoreTag(const Instruction* instr);
  void SimulateMTEStoreTagPair(const Instruction* instr);
  void Simulate_XdSP_XnSP_Xm(const Instruction* instr);
  void SimulateCpy(const Instruction* instr);
  void SimulateCpyFP(const Instruction* instr);
  void SimulateCpyP(const Instruction* instr);
  void SimulateCpyM(const Instruction* instr);
  void SimulateCpyE(const Instruction* instr);
  void SimulateSetP(const Instruction* instr);
  void SimulateSetM(const Instruction* instr);
  void SimulateSetE(const Instruction* instr);
  void SimulateSetGP(const Instruction* instr);
  void SimulateSetGM(const Instruction* instr);
  void SimulateSignedMinMax(const Instruction* instr);
  void SimulateUnsignedMinMax(const Instruction* instr);
  void SimulateSHA512(const Instruction* instr);

  void VisitCryptoSM3(const Instruction* instr);
  void VisitCryptoSM4(const Instruction* instr);

  // Integer register accessors.

  // Basic accessor: Read the register as the specified type.
  template <typename T>
  T ReadRegister(unsigned code, Reg31Mode r31mode = Reg31IsZeroRegister) const {
    VIXL_ASSERT(
        code < kNumberOfRegisters ||
        ((r31mode == Reg31IsZeroRegister) && (code == kSPRegInternalCode)));
    if ((code == 31) && (r31mode == Reg31IsZeroRegister)) {
      T result;
      memset(&result, 0, sizeof(result));
      return result;
    }
    if ((r31mode == Reg31IsZeroRegister) && (code == kSPRegInternalCode)) {
      code = 31;
    }
    return registers_[code].Get<T>();
  }
  template <typename T>
  VIXL_DEPRECATED("ReadRegister",
                  T reg(unsigned code, Reg31Mode r31mode = Reg31IsZeroRegister)
                      const) {
    return ReadRegister<T>(code, r31mode);
  }

  // Common specialized accessors for the ReadRegister() template.
  int32_t ReadWRegister(unsigned code,
                        Reg31Mode r31mode = Reg31IsZeroRegister) const {
    return ReadRegister<int32_t>(code, r31mode);
  }
  VIXL_DEPRECATED("ReadWRegister",
                  int32_t wreg(unsigned code,
                               Reg31Mode r31mode = Reg31IsZeroRegister) const) {
    return ReadWRegister(code, r31mode);
  }

  int64_t ReadXRegister(unsigned code,
                        Reg31Mode r31mode = Reg31IsZeroRegister) const {
    return ReadRegister<int64_t>(code, r31mode);
  }
  VIXL_DEPRECATED("ReadXRegister",
                  int64_t xreg(unsigned code,
                               Reg31Mode r31mode = Reg31IsZeroRegister) const) {
    return ReadXRegister(code, r31mode);
  }

  SimPRegister& ReadPRegister(unsigned code) {
    VIXL_ASSERT(code < kNumberOfPRegisters);
    return pregisters_[code];
  }

  SimFFRRegister& ReadFFR() { return ffr_register_; }

  // As above, with parameterized size and return type. The value is
  // either zero-extended or truncated to fit, as required.
  template <typename T>
  T ReadRegister(unsigned size,
                 unsigned code,
                 Reg31Mode r31mode = Reg31IsZeroRegister) const {
    uint64_t raw;
    switch (size) {
      case kWRegSize:
        raw = ReadRegister<uint32_t>(code, r31mode);
        break;
      case kXRegSize:
        raw = ReadRegister<uint64_t>(code, r31mode);
        break;
      default:
        VIXL_UNREACHABLE();
        return 0;
    }

    T result;
    VIXL_STATIC_ASSERT(sizeof(result) <= sizeof(raw));
    // Copy the result and truncate to fit. This assumes a little-endian host.
    memcpy(&result, &raw, sizeof(result));
    return result;
  }
  template <typename T>
  VIXL_DEPRECATED("ReadRegister",
                  T reg(unsigned size,
                        unsigned code,
                        Reg31Mode r31mode = Reg31IsZeroRegister) const) {
    return ReadRegister<T>(size, code, r31mode);
  }

  // Use int64_t by default if T is not specified.
  int64_t ReadRegister(unsigned size,
                       unsigned code,
                       Reg31Mode r31mode = Reg31IsZeroRegister) const {
    return ReadRegister<int64_t>(size, code, r31mode);
  }
  VIXL_DEPRECATED("ReadRegister",
                  int64_t reg(unsigned size,
                              unsigned code,
                              Reg31Mode r31mode = Reg31IsZeroRegister) const) {
    return ReadRegister(size, code, r31mode);
  }

  enum RegLogMode { LogRegWrites, NoRegLog };

  // Write 'value' into an integer register. The value is zero-extended. This
  // behaviour matches AArch64 register writes.
  //
  // SP may be specified in one of two ways:
  //  - (code == kSPRegInternalCode) && (r31mode == Reg31IsZeroRegister)
  //  - (code == 31) && (r31mode == Reg31IsStackPointer)
  template <typename T>
  void WriteRegister(unsigned code,
                     T value,
                     RegLogMode log_mode = LogRegWrites,
                     Reg31Mode r31mode = Reg31IsZeroRegister) {
    if (sizeof(T) < kWRegSizeInBytes) {
      // We use a C-style cast on purpose here.
      // Since we do not have access to 'constepxr if', the casts in this `if`
      // must be valid even if we know the code will never be executed, in
      // particular when `T` is a pointer type.
      int64_t tmp_64bit = (int64_t)value;
      int32_t tmp_32bit = static_cast<int32_t>(tmp_64bit);
      WriteRegister<int32_t>(code, tmp_32bit, log_mode, r31mode);
      return;
    }

    VIXL_ASSERT((sizeof(T) == kWRegSizeInBytes) ||
                (sizeof(T) == kXRegSizeInBytes));
    VIXL_ASSERT(
        (code < kNumberOfRegisters) ||
        ((r31mode == Reg31IsZeroRegister) && (code == kSPRegInternalCode)));

    if (code == 31) {
      if (r31mode == Reg31IsZeroRegister) {
        // Discard writes to the zero register.
        return;
      } else {
        code = kSPRegInternalCode;
      }
    }

    // registers_[31] is the stack pointer.
    VIXL_STATIC_ASSERT((kSPRegInternalCode % kNumberOfRegisters) == 31);
    registers_[code % kNumberOfRegisters].Write(value);

    if (log_mode == LogRegWrites) {
      LogRegister(code, GetPrintRegisterFormatForSize(sizeof(T)));
    }
  }
  template <typename T>
  VIXL_DEPRECATED("WriteRegister",
                  void set_reg(unsigned code,
                               T value,
                               RegLogMode log_mode = LogRegWrites,
                               Reg31Mode r31mode = Reg31IsZeroRegister)) {
    WriteRegister<T>(code, value, log_mode, r31mode);
  }

  // Common specialized accessors for the set_reg() template.
  void WriteWRegister(unsigned code,
                      int32_t value,
                      RegLogMode log_mode = LogRegWrites,
                      Reg31Mode r31mode = Reg31IsZeroRegister) {
    WriteRegister(code, value, log_mode, r31mode);
  }
  VIXL_DEPRECATED("WriteWRegister",
                  void set_wreg(unsigned code,
                                int32_t value,
                                RegLogMode log_mode = LogRegWrites,
                                Reg31Mode r31mode = Reg31IsZeroRegister)) {
    WriteWRegister(code, value, log_mode, r31mode);
  }

  void WriteXRegister(unsigned code,
                      int64_t value,
                      RegLogMode log_mode = LogRegWrites,
                      Reg31Mode r31mode = Reg31IsZeroRegister) {
    WriteRegister(code, value, log_mode, r31mode);
  }
  VIXL_DEPRECATED("WriteXRegister",
                  void set_xreg(unsigned code,
                                int64_t value,
                                RegLogMode log_mode = LogRegWrites,
                                Reg31Mode r31mode = Reg31IsZeroRegister)) {
    WriteXRegister(code, value, log_mode, r31mode);
  }

  // As above, with parameterized size and type. The value is either
  // zero-extended or truncated to fit, as required.
  template <typename T>
  void WriteRegister(unsigned size,
                     unsigned code,
                     T value,
                     RegLogMode log_mode = LogRegWrites,
                     Reg31Mode r31mode = Reg31IsZeroRegister) {
    // Zero-extend the input.
    uint64_t raw = 0;
    VIXL_STATIC_ASSERT(sizeof(value) <= sizeof(raw));
    memcpy(&raw, &value, sizeof(value));

    // Write (and possibly truncate) the value.
    switch (size) {
      case kWRegSize:
        WriteRegister(code, static_cast<uint32_t>(raw), log_mode, r31mode);
        break;
      case kXRegSize:
        WriteRegister(code, raw, log_mode, r31mode);
        break;
      default:
        VIXL_UNREACHABLE();
        return;
    }
  }
  template <typename T>
  VIXL_DEPRECATED("WriteRegister",
                  void set_reg(unsigned size,
                               unsigned code,
                               T value,
                               RegLogMode log_mode = LogRegWrites,
                               Reg31Mode r31mode = Reg31IsZeroRegister)) {
    WriteRegister(size, code, value, log_mode, r31mode);
  }

  // Common specialized accessors for the set_reg() template.

  // Commonly-used special cases.
  template <typename T>
  void WriteLr(T value) {
    WriteRegister(kLinkRegCode, value);
  }
  template <typename T>
  VIXL_DEPRECATED("WriteLr", void set_lr(T value)) {
    WriteLr(value);
  }

  template <typename T>
  void WriteSp(T value) {
    WriteRegister(31, value, LogRegWrites, Reg31IsStackPointer);
  }
  template <typename T>
  VIXL_DEPRECATED("WriteSp", void set_sp(T value)) {
    WriteSp(value);
  }

  // Vector register accessors.
  // These are equivalent to the integer register accessors, but for vector
  // registers.

  // A structure for representing a 128-bit Q register.
  struct qreg_t {
    uint8_t val[kQRegSizeInBytes];
  };

  // A structure for representing a SVE Z register.
  struct zreg_t {
    uint8_t val[kZRegMaxSizeInBytes];
  };

  // Basic accessor: read the register as the specified type.
  template <typename T>
  T ReadVRegister(unsigned code) const {
    VIXL_STATIC_ASSERT(
        (sizeof(T) == kBRegSizeInBytes) || (sizeof(T) == kHRegSizeInBytes) ||
        (sizeof(T) == kSRegSizeInBytes) || (sizeof(T) == kDRegSizeInBytes) ||
        (sizeof(T) == kQRegSizeInBytes));
    VIXL_ASSERT(code < kNumberOfVRegisters);

    return vregisters_[code].Get<T>();
  }
  template <typename T>
  VIXL_DEPRECATED("ReadVRegister", T vreg(unsigned code) const) {
    return ReadVRegister<T>(code);
  }

  // Common specialized accessors for the vreg() template.
  int8_t ReadBRegister(unsigned code) const {
    return ReadVRegister<int8_t>(code);
  }
  VIXL_DEPRECATED("ReadBRegister", int8_t breg(unsigned code) const) {
    return ReadBRegister(code);
  }

  vixl::internal::SimFloat16 ReadHRegister(unsigned code) const {
    return RawbitsToFloat16(ReadHRegisterBits(code));
  }
  VIXL_DEPRECATED("ReadHRegister", int16_t hreg(unsigned code) const) {
    return Float16ToRawbits(ReadHRegister(code));
  }

  uint16_t ReadHRegisterBits(unsigned code) const {
    return ReadVRegister<uint16_t>(code);
  }

  float ReadSRegister(unsigned code) const {
    return ReadVRegister<float>(code);
  }
  VIXL_DEPRECATED("ReadSRegister", float sreg(unsigned code) const) {
    return ReadSRegister(code);
  }

  uint32_t ReadSRegisterBits(unsigned code) const {
    return ReadVRegister<uint32_t>(code);
  }
  VIXL_DEPRECATED("ReadSRegisterBits",
                  uint32_t sreg_bits(unsigned code) const) {
    return ReadSRegisterBits(code);
  }

  double ReadDRegister(unsigned code) const {
    return ReadVRegister<double>(code);
  }
  VIXL_DEPRECATED("ReadDRegister", double dreg(unsigned code) const) {
    return ReadDRegister(code);
  }

  uint64_t ReadDRegisterBits(unsigned code) const {
    return ReadVRegister<uint64_t>(code);
  }
  VIXL_DEPRECATED("ReadDRegisterBits",
                  uint64_t dreg_bits(unsigned code) const) {
    return ReadDRegisterBits(code);
  }

  qreg_t ReadQRegister(unsigned code) const {
    return ReadVRegister<qreg_t>(code);
  }
  VIXL_DEPRECATED("ReadQRegister", qreg_t qreg(unsigned code) const) {
    return ReadQRegister(code);
  }

  // As above, with parameterized size and return type. The value is
  // either zero-extended or truncated to fit, as required.
  template <typename T>
  T ReadVRegister(unsigned size, unsigned code) const {
    uint64_t raw = 0;
    T result;

    switch (size) {
      case kSRegSize:
        raw = ReadVRegister<uint32_t>(code);
        break;
      case kDRegSize:
        raw = ReadVRegister<uint64_t>(code);
        break;
      default:
        VIXL_UNREACHABLE();
        break;
    }

    VIXL_STATIC_ASSERT(sizeof(result) <= sizeof(raw));
    // Copy the result and truncate to fit. This assumes a little-endian host.
    memcpy(&result, &raw, sizeof(result));
    return result;
  }
  template <typename T>
  VIXL_DEPRECATED("ReadVRegister", T vreg(unsigned size, unsigned code) const) {
    return ReadVRegister<T>(size, code);
  }

  SimVRegister& ReadVRegister(unsigned code) { return vregisters_[code]; }
  VIXL_DEPRECATED("ReadVRegister", SimVRegister& vreg(unsigned code)) {
    return ReadVRegister(code);
  }

  // Basic accessor: Write the specified value.
  template <typename T>
  void WriteVRegister(unsigned code,
                      T value,
                      RegLogMode log_mode = LogRegWrites) {
    VIXL_STATIC_ASSERT((sizeof(value) == kBRegSizeInBytes) ||
                       (sizeof(value) == kHRegSizeInBytes) ||
                       (sizeof(value) == kSRegSizeInBytes) ||
                       (sizeof(value) == kDRegSizeInBytes) ||
                       (sizeof(value) == kQRegSizeInBytes) ||
                       (sizeof(value) == kZRegMaxSizeInBytes));
    VIXL_ASSERT(code < kNumberOfVRegisters);
    vregisters_[code].Write(value);

    if (log_mode == LogRegWrites) {
      LogVRegister(code, GetPrintRegisterFormat(value));
    }
  }
  template <typename T>
  VIXL_DEPRECATED("WriteVRegister",
                  void set_vreg(unsigned code,
                                T value,
                                RegLogMode log_mode = LogRegWrites)) {
    WriteVRegister(code, value, log_mode);
  }

  // Common specialized accessors for the WriteVRegister() template.
  void WriteBRegister(unsigned code,
                      int8_t value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }
  VIXL_DEPRECATED("WriteBRegister",
                  void set_breg(unsigned code,
                                int8_t value,
                                RegLogMode log_mode = LogRegWrites)) {
    return WriteBRegister(code, value, log_mode);
  }

  void WriteHRegister(unsigned code,
                      vixl::internal::SimFloat16 value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, Float16ToRawbits(value), log_mode);
  }

  void WriteHRegister(unsigned code,
                      int16_t value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }
  VIXL_DEPRECATED("WriteHRegister",
                  void set_hreg(unsigned code,
                                int16_t value,
                                RegLogMode log_mode = LogRegWrites)) {
    return WriteHRegister(code, value, log_mode);
  }

  void WriteSRegister(unsigned code,
                      float value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }
  VIXL_DEPRECATED("WriteSRegister",
                  void set_sreg(unsigned code,
                                float value,
                                RegLogMode log_mode = LogRegWrites)) {
    WriteSRegister(code, value, log_mode);
  }

  void WriteSRegisterBits(unsigned code,
                          uint32_t value,
                          RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }
  VIXL_DEPRECATED("WriteSRegisterBits",
                  void set_sreg_bits(unsigned code,
                                     uint32_t value,
                                     RegLogMode log_mode = LogRegWrites)) {
    WriteSRegisterBits(code, value, log_mode);
  }

  void WriteDRegister(unsigned code,
                      double value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }
  VIXL_DEPRECATED("WriteDRegister",
                  void set_dreg(unsigned code,
                                double value,
                                RegLogMode log_mode = LogRegWrites)) {
    WriteDRegister(code, value, log_mode);
  }

  void WriteDRegisterBits(unsigned code,
                          uint64_t value,
                          RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }
  VIXL_DEPRECATED("WriteDRegisterBits",
                  void set_dreg_bits(unsigned code,
                                     uint64_t value,
                                     RegLogMode log_mode = LogRegWrites)) {
    WriteDRegisterBits(code, value, log_mode);
  }

  void WriteQRegister(unsigned code,
                      qreg_t value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }
  VIXL_DEPRECATED("WriteQRegister",
                  void set_qreg(unsigned code,
                                qreg_t value,
                                RegLogMode log_mode = LogRegWrites)) {
    WriteQRegister(code, value, log_mode);
  }

  void WriteZRegister(unsigned code,
                      zreg_t value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister(code, value, log_mode);
  }

  template <typename T>
  T ReadRegister(Register reg) const {
    return ReadRegister<T>(reg.GetCode(), Reg31IsZeroRegister);
  }

  template <typename T>
  void WriteRegister(Register reg,
                     T value,
                     RegLogMode log_mode = LogRegWrites) {
    WriteRegister<T>(reg.GetCode(), value, log_mode, Reg31IsZeroRegister);
  }

  template <typename T>
  T ReadVRegister(VRegister vreg) const {
    return ReadVRegister<T>(vreg.GetCode());
  }

  template <typename T>
  void WriteVRegister(VRegister vreg,
                      T value,
                      RegLogMode log_mode = LogRegWrites) {
    WriteVRegister<T>(vreg.GetCode(), value, log_mode);
  }

  template <typename T>
  T ReadCPURegister(CPURegister reg) const {
    if (reg.IsVRegister()) {
      return ReadVRegister<T>(VRegister(reg));
    } else {
      return ReadRegister<T>(Register(reg));
    }
  }

  template <typename T>
  void WriteCPURegister(CPURegister reg,
                        T value,
                        RegLogMode log_mode = LogRegWrites) {
    if (reg.IsVRegister()) {
      WriteVRegister<T>(VRegister(reg), value, log_mode);
    } else {
      WriteRegister<T>(Register(reg), value, log_mode);
    }
  }

  template <typename T, typename A>
  std::optional<T> MemRead(A address) const {
    Instruction const* pc = ReadPc();
    return memory_.Read<T>(address, pc);
  }

  template <typename T, typename A>
  bool MemWrite(A address, T value) const {
    Instruction const* pc = ReadPc();
    return memory_.Write(address, value, pc);
  }

  template <typename A>
  std::optional<uint64_t> MemReadUint(int size_in_bytes, A address) const {
    return memory_.ReadUint(size_in_bytes, address);
  }

  template <typename A>
  std::optional<int64_t> MemReadInt(int size_in_bytes, A address) const {
    return memory_.ReadInt(size_in_bytes, address);
  }

  template <typename A>
  bool MemWrite(int size_in_bytes, A address, uint64_t value) const {
    return memory_.Write(size_in_bytes, address, value);
  }

  bool LoadLane(LogicVRegister dst,
                VectorFormat vform,
                int index,
                uint64_t addr) const {
    unsigned msize_in_bytes = LaneSizeInBytesFromFormat(vform);
    return LoadUintToLane(dst, vform, msize_in_bytes, index, addr);
  }

  bool LoadUintToLane(LogicVRegister dst,
                      VectorFormat vform,
                      unsigned msize_in_bytes,
                      int index,
                      uint64_t addr) const {
    VIXL_DEFINE_OR_RETURN_FALSE(value, MemReadUint(msize_in_bytes, addr));
    dst.SetUint(vform, index, value);
    return true;
  }

  bool LoadIntToLane(LogicVRegister dst,
                     VectorFormat vform,
                     unsigned msize_in_bytes,
                     int index,
                     uint64_t addr) const {
    VIXL_DEFINE_OR_RETURN_FALSE(value, MemReadInt(msize_in_bytes, addr));
    dst.SetInt(vform, index, value);
    return true;
  }

  bool StoreLane(const LogicVRegister& src,
                 VectorFormat vform,
                 int index,
                 uint64_t addr) const {
    unsigned msize_in_bytes = LaneSizeInBytesFromFormat(vform);
    return MemWrite(msize_in_bytes, addr, src.Uint(vform, index));
  }

  uint64_t ComputeMemOperandAddress(const MemOperand& mem_op) const;

  template <typename T>
  T ReadGenericOperand(GenericOperand operand) const {
    if (operand.IsCPURegister()) {
      return ReadCPURegister<T>(operand.GetCPURegister());
    } else {
      VIXL_ASSERT(operand.IsMemOperand());
      auto res = MemRead<T>(ComputeMemOperandAddress(operand.GetMemOperand()));
      VIXL_ASSERT(res);
      return *res;
    }
  }

  template <typename T>
  bool WriteGenericOperand(GenericOperand operand,
                           T value,
                           RegLogMode log_mode = LogRegWrites) {
    if (operand.IsCPURegister()) {
      // Outside SIMD, registers are 64-bit or a subset of a 64-bit register. If
      // the width of the value to write is smaller than 64 bits, the unused
      // bits may contain unrelated values that the code following this write
      // needs to handle gracefully.
      // Here we fill the unused bits with a predefined pattern to catch issues
      // early.
      VIXL_ASSERT(operand.GetCPURegister().GetSizeInBits() <= 64);
      uint64_t raw = 0xdeadda1adeadda1a;
      memcpy(&raw, &value, sizeof(value));
      WriteCPURegister(operand.GetCPURegister(), raw, log_mode);
    } else {
      VIXL_ASSERT(operand.IsMemOperand());
      return MemWrite(ComputeMemOperandAddress(operand.GetMemOperand()), value);
    }
    return true;
  }

  bool ReadN() const { return nzcv_.GetN() != 0; }
  VIXL_DEPRECATED("ReadN", bool N() const) { return ReadN(); }

  bool ReadZ() const { return nzcv_.GetZ() != 0; }
  VIXL_DEPRECATED("ReadZ", bool Z() const) { return ReadZ(); }

  bool ReadC() const { return nzcv_.GetC() != 0; }
  VIXL_DEPRECATED("ReadC", bool C() const) { return ReadC(); }

  bool ReadV() const { return nzcv_.GetV() != 0; }
  VIXL_DEPRECATED("ReadV", bool V() const) { return ReadV(); }

  SimSystemRegister& ReadNzcv() { return nzcv_; }
  VIXL_DEPRECATED("ReadNzcv", SimSystemRegister& nzcv()) { return ReadNzcv(); }

  // TODO: Find a way to make the fpcr_ members return the proper types, so
  // these accessors are not necessary.
  FPRounding ReadRMode() const {
    return static_cast<FPRounding>(fpcr_.GetRMode());
  }
  VIXL_DEPRECATED("ReadRMode", FPRounding RMode()) { return ReadRMode(); }

  UseDefaultNaN ReadDN() const {
    return fpcr_.GetDN() != 0 ? kUseDefaultNaN : kIgnoreDefaultNaN;
  }

  VIXL_DEPRECATED("ReadDN", bool DN()) {
    return ReadDN() == kUseDefaultNaN ? true : false;
  }

  SimSystemRegister& ReadFpcr() { return fpcr_; }
  VIXL_DEPRECATED("ReadFpcr", SimSystemRegister& fpcr()) { return ReadFpcr(); }

  // Specify relevant register formats for Print(V)Register and related helpers.
  enum PrintRegisterFormat {
    // The lane size.
    kPrintRegLaneSizeB = 0 << 0,
    kPrintRegLaneSizeH = 1 << 0,
    kPrintRegLaneSizeS = 2 << 0,
    kPrintRegLaneSizeW = kPrintRegLaneSizeS,
    kPrintRegLaneSizeD = 3 << 0,
    kPrintRegLaneSizeX = kPrintRegLaneSizeD,
    kPrintRegLaneSizeQ = 4 << 0,
    kPrintRegLaneSizeUnknown = 5 << 0,

    kPrintRegLaneSizeOffset = 0,
    kPrintRegLaneSizeMask = 7 << 0,

    // The overall register size.
    kPrintRegAsScalar = 0,
    kPrintRegAsDVector = 1 << 3,
    kPrintRegAsQVector = 2 << 3,
    kPrintRegAsSVEVector = 3 << 3,

    kPrintRegAsVectorMask = 3 << 3,

    // Indicate floating-point format lanes. (This flag is only supported for
    // S-, H-, and D-sized lanes.)
    kPrintRegAsFP = 1 << 5,

    // With this flag, print helpers won't check that the upper bits are zero.
    // This also forces the register name to be printed with the `reg<msb:0>`
    // format.
    //
    // The flag is supported with any PrintRegisterFormat other than those with
    // kPrintRegAsSVEVector.
    kPrintRegPartial = 1 << 6,

// Supported combinations.
// These exist so that they can be referred to by name, but also because C++
// does not allow enum types to hold values that aren't explicitly
// enumerated, and we want to be able to combine the above flags.

// Scalar formats.
#define VIXL_DECL_PRINT_REG_SCALAR(size)                           \
  kPrint##size##Reg = kPrintRegLaneSize##size | kPrintRegAsScalar, \
  kPrint##size##RegPartial = kPrintRegLaneSize##size | kPrintRegPartial
#define VIXL_DECL_PRINT_REG_SCALAR_FP(size)                  \
  VIXL_DECL_PRINT_REG_SCALAR(size)                           \
  , kPrint##size##RegFP = kPrint##size##Reg | kPrintRegAsFP, \
    kPrint##size##RegPartialFP = kPrint##size##RegPartial | kPrintRegAsFP
    VIXL_DECL_PRINT_REG_SCALAR(W),
    VIXL_DECL_PRINT_REG_SCALAR(X),
    VIXL_DECL_PRINT_REG_SCALAR_FP(H),
    VIXL_DECL_PRINT_REG_SCALAR_FP(S),
    VIXL_DECL_PRINT_REG_SCALAR_FP(D),
    VIXL_DECL_PRINT_REG_SCALAR(Q),
#undef VIXL_DECL_PRINT_REG_SCALAR
#undef VIXL_DECL_PRINT_REG_SCALAR_FP

#define VIXL_DECL_PRINT_REG_NEON(count, type, size)                     \
  kPrintReg##count##type = kPrintRegLaneSize##type | kPrintRegAs##size, \
  kPrintReg##count##type##Partial = kPrintReg##count##type | kPrintRegPartial
#define VIXL_DECL_PRINT_REG_NEON_FP(count, type, size)                   \
  VIXL_DECL_PRINT_REG_NEON(count, type, size)                            \
  , kPrintReg##count##type##FP = kPrintReg##count##type | kPrintRegAsFP, \
    kPrintReg##count##type##PartialFP =                                  \
        kPrintReg##count##type##Partial | kPrintRegAsFP
    VIXL_DECL_PRINT_REG_NEON(1, B, Scalar),
    VIXL_DECL_PRINT_REG_NEON(8, B, DVector),
    VIXL_DECL_PRINT_REG_NEON(16, B, QVector),
    VIXL_DECL_PRINT_REG_NEON_FP(1, H, Scalar),
    VIXL_DECL_PRINT_REG_NEON_FP(4, H, DVector),
    VIXL_DECL_PRINT_REG_NEON_FP(8, H, QVector),
    VIXL_DECL_PRINT_REG_NEON_FP(1, S, Scalar),
    VIXL_DECL_PRINT_REG_NEON_FP(2, S, DVector),
    VIXL_DECL_PRINT_REG_NEON_FP(4, S, QVector),
    VIXL_DECL_PRINT_REG_NEON_FP(1, D, Scalar),
    VIXL_DECL_PRINT_REG_NEON_FP(2, D, QVector),
    VIXL_DECL_PRINT_REG_NEON(1, Q, Scalar),
#undef VIXL_DECL_PRINT_REG_NEON
#undef VIXL_DECL_PRINT_REG_NEON_FP

#define VIXL_DECL_PRINT_REG_SVE(type)                                 \
  kPrintRegVn##type = kPrintRegLaneSize##type | kPrintRegAsSVEVector, \
  kPrintRegVn##type##Partial = kPrintRegVn##type | kPrintRegPartial
#define VIXL_DECL_PRINT_REG_SVE_FP(type)                       \
  VIXL_DECL_PRINT_REG_SVE(type)                                \
  , kPrintRegVn##type##FP = kPrintRegVn##type | kPrintRegAsFP, \
    kPrintRegVn##type##PartialFP = kPrintRegVn##type##Partial | kPrintRegAsFP
    VIXL_DECL_PRINT_REG_SVE(B),
    VIXL_DECL_PRINT_REG_SVE_FP(H),
    VIXL_DECL_PRINT_REG_SVE_FP(S),
    VIXL_DECL_PRINT_REG_SVE_FP(D),
    VIXL_DECL_PRINT_REG_SVE(Q)
#undef VIXL_DECL_PRINT_REG_SVE
#undef VIXL_DECL_PRINT_REG_SVE_FP
  };

  // Return `format` with the kPrintRegPartial flag set.
  PrintRegisterFormat GetPrintRegPartial(PrintRegisterFormat format) {
    // Every PrintRegisterFormat has a kPrintRegPartial counterpart, so the
    // result of this cast will always be well-defined.
    return static_cast<PrintRegisterFormat>(format | kPrintRegPartial);
  }

  // For SVE formats, return the format of a Q register part of it.
  PrintRegisterFormat GetPrintRegAsQChunkOfSVE(PrintRegisterFormat format) {
    VIXL_ASSERT((format & kPrintRegAsVectorMask) == kPrintRegAsSVEVector);
    // Keep the FP and lane size fields.
    int q_format = format & (kPrintRegLaneSizeMask | kPrintRegAsFP);
    // The resulting format must always be partial, because we're not formatting
    // the whole Z register.
    q_format |= (kPrintRegAsQVector | kPrintRegPartial);

    // This cast is always safe because NEON QVector formats support every
    // combination of FP and lane size that SVE formats do.
    return static_cast<PrintRegisterFormat>(q_format);
  }

  unsigned GetPrintRegLaneSizeInBytesLog2(PrintRegisterFormat format) {
    VIXL_ASSERT((format & kPrintRegLaneSizeMask) != kPrintRegLaneSizeUnknown);
    return (format & kPrintRegLaneSizeMask) >> kPrintRegLaneSizeOffset;
  }

  unsigned GetPrintRegLaneSizeInBytes(PrintRegisterFormat format) {
    return 1 << GetPrintRegLaneSizeInBytesLog2(format);
  }

  unsigned GetPrintRegSizeInBytesLog2(PrintRegisterFormat format) {
    switch (format & kPrintRegAsVectorMask) {
      case kPrintRegAsScalar:
        return GetPrintRegLaneSizeInBytesLog2(format);
      case kPrintRegAsDVector:
        return kDRegSizeInBytesLog2;
      case kPrintRegAsQVector:
        return kQRegSizeInBytesLog2;
      default:
      case kPrintRegAsSVEVector:
        // We print SVE vectors in Q-sized chunks. These need special handling,
        // and it's probably an error to call this function in that case.
        VIXL_UNREACHABLE();
        return kQRegSizeInBytesLog2;
    }
  }

  unsigned GetPrintRegSizeInBytes(PrintRegisterFormat format) {
    return 1 << GetPrintRegSizeInBytesLog2(format);
  }

  unsigned GetPrintRegSizeInBitsLog2(PrintRegisterFormat format) {
    return GetPrintRegSizeInBytesLog2(format) + kBitsPerByteLog2;
  }

  unsigned GetPrintRegSizeInBits(PrintRegisterFormat format) {
    return 1 << GetPrintRegSizeInBitsLog2(format);
  }

  const char* GetPartialRegSuffix(PrintRegisterFormat format) {
    switch (GetPrintRegSizeInBitsLog2(format)) {
      case kBRegSizeLog2:
        return "<7:0>";
      case kHRegSizeLog2:
        return "<15:0>";
      case kSRegSizeLog2:
        return "<31:0>";
      case kDRegSizeLog2:
        return "<63:0>";
      case kQRegSizeLog2:
        return "<127:0>";
    }
    VIXL_UNREACHABLE();
    return "<UNKNOWN>";
  }

  unsigned GetPrintRegLaneCount(PrintRegisterFormat format) {
    unsigned reg_size_log2 = GetPrintRegSizeInBytesLog2(format);
    unsigned lane_size_log2 = GetPrintRegLaneSizeInBytesLog2(format);
    VIXL_ASSERT(reg_size_log2 >= lane_size_log2);
    return 1 << (reg_size_log2 - lane_size_log2);
  }

  uint16_t GetPrintRegLaneMask(PrintRegisterFormat format) {
    int print_as = format & kPrintRegAsVectorMask;
    if (print_as == kPrintRegAsScalar) return 1;

    // Vector formats, including SVE formats printed in Q-sized chunks.
    static const uint16_t masks[] = {0xffff, 0x5555, 0x1111, 0x0101, 0x0001};
    unsigned size_in_bytes_log2 = GetPrintRegLaneSizeInBytesLog2(format);
    VIXL_ASSERT(size_in_bytes_log2 < ArrayLength(masks));
    uint16_t mask = masks[size_in_bytes_log2];

    // Exclude lanes that aren't visible in D vectors.
    if (print_as == kPrintRegAsDVector) mask &= 0x00ff;
    return mask;
  }

  PrintRegisterFormat GetPrintRegisterFormatForSize(unsigned reg_size,
                                                    unsigned lane_size);

  PrintRegisterFormat GetPrintRegisterFormatForSize(unsigned size) {
    return GetPrintRegisterFormatForSize(size, size);
  }

  PrintRegisterFormat GetPrintRegisterFormatForSizeFP(unsigned size) {
    switch (size) {
      default:
        VIXL_UNREACHABLE();
        return kPrintDReg;
      case kDRegSizeInBytes:
        return kPrintDReg;
      case kSRegSizeInBytes:
        return kPrintSReg;
      case kHRegSizeInBytes:
        return kPrintHReg;
    }
  }

  PrintRegisterFormat GetPrintRegisterFormatTryFP(PrintRegisterFormat format) {
    if ((GetPrintRegLaneSizeInBytes(format) == kHRegSizeInBytes) ||
        (GetPrintRegLaneSizeInBytes(format) == kSRegSizeInBytes) ||
        (GetPrintRegLaneSizeInBytes(format) == kDRegSizeInBytes)) {
      return static_cast<PrintRegisterFormat>(format | kPrintRegAsFP);
    }
    return format;
  }

  PrintRegisterFormat GetPrintRegisterFormatForSizeTryFP(unsigned size) {
    return GetPrintRegisterFormatTryFP(GetPrintRegisterFormatForSize(size));
  }

  template <typename T>
  PrintRegisterFormat GetPrintRegisterFormat(T value) {
    return GetPrintRegisterFormatForSize(sizeof(value));
  }

  PrintRegisterFormat GetPrintRegisterFormat(double value) {
    VIXL_STATIC_ASSERT(sizeof(value) == kDRegSizeInBytes);
    return GetPrintRegisterFormatForSizeFP(sizeof(value));
  }

  PrintRegisterFormat GetPrintRegisterFormat(float value) {
    VIXL_STATIC_ASSERT(sizeof(value) == kSRegSizeInBytes);
    return GetPrintRegisterFormatForSizeFP(sizeof(value));
  }

  PrintRegisterFormat GetPrintRegisterFormat(Float16 value) {
    VIXL_STATIC_ASSERT(sizeof(Float16ToRawbits(value)) == kHRegSizeInBytes);
    return GetPrintRegisterFormatForSizeFP(sizeof(Float16ToRawbits(value)));
  }

  PrintRegisterFormat GetPrintRegisterFormat(VectorFormat vform);
  PrintRegisterFormat GetPrintRegisterFormatFP(VectorFormat vform);

  // Print all registers of the specified types.
  void PrintRegisters();
  void PrintVRegisters();
  void PrintZRegisters();
  void PrintSystemRegisters();

  // As above, but only print the registers that have been updated.
  void PrintWrittenRegisters();
  void PrintWrittenVRegisters();
  void PrintWrittenPRegisters();

  // As above, but respect LOG_REG and LOG_VREG.
  void LogWrittenRegisters() {
    if (ShouldTraceRegs()) PrintWrittenRegisters();
  }
  void LogWrittenVRegisters() {
    if (ShouldTraceVRegs()) PrintWrittenVRegisters();
  }
  void LogWrittenPRegisters() {
    if (ShouldTraceVRegs()) PrintWrittenPRegisters();
  }
  void LogAllWrittenRegisters() {
    LogWrittenRegisters();
    LogWrittenVRegisters();
    LogWrittenPRegisters();
  }

  // The amount of space to leave for a register name. This is used to keep the
  // values vertically aligned. The longest register name has the form
  // "z31<2047:1920>". The total overall value indentation must also take into
  // account the fixed formatting: "# {name}: 0x{value}".
  static const int kPrintRegisterNameFieldWidth = 14;

  // Print whole, individual register values.
  // - The format can be used to restrict how much of the register is printed,
  //   but such formats indicate that the unprinted high-order bits are zero and
  //   these helpers will assert that.
  // - If the format includes the kPrintRegAsFP flag then human-friendly FP
  //   value annotations will be printed.
  // - The suffix can be used to add annotations (such as memory access
  //   details), or to suppress the newline.
  void PrintRegister(int code,
                     PrintRegisterFormat format = kPrintXReg,
                     const char* suffix = "\n");
  void PrintVRegister(int code,
                      PrintRegisterFormat format = kPrintReg1Q,
                      const char* suffix = "\n");
  // PrintZRegister and PrintPRegister print over several lines, so they cannot
  // allow the suffix to be overridden.
  void PrintZRegister(int code, PrintRegisterFormat format = kPrintRegVnQ);
  void PrintPRegister(int code, PrintRegisterFormat format = kPrintRegVnQ);
  void PrintFFR(PrintRegisterFormat format = kPrintRegVnQ);
  // Print a single Q-sized part of a Z register, or the corresponding two-byte
  // part of a P register. These print single lines, and therefore allow the
  // suffix to be overridden. The format must include the kPrintRegPartial flag.
  void PrintPartialZRegister(int code,
                             int q_index,
                             PrintRegisterFormat format = kPrintRegVnQ,
                             const char* suffix = "\n");
  void PrintPartialPRegister(int code,
                             int q_index,
                             PrintRegisterFormat format = kPrintRegVnQ,
                             const char* suffix = "\n");
  void PrintPartialPRegister(const char* name,
                             const SimPRegister& reg,
                             int q_index,
                             PrintRegisterFormat format = kPrintRegVnQ,
                             const char* suffix = "\n");

  // Like Print*Register (above), but respect trace parameters.
  void LogRegister(unsigned code, PrintRegisterFormat format) {
    if (ShouldTraceRegs()) PrintRegister(code, format);
  }
  void LogVRegister(unsigned code, PrintRegisterFormat format) {
    if (ShouldTraceVRegs()) PrintVRegister(code, format);
  }
  void LogZRegister(unsigned code, PrintRegisterFormat format) {
    if (ShouldTraceVRegs()) PrintZRegister(code, format);
  }
  void LogPRegister(unsigned code, PrintRegisterFormat format) {
    if (ShouldTraceVRegs()) PrintPRegister(code, format);
  }
  void LogFFR(PrintRegisterFormat format) {
    if (ShouldTraceVRegs()) PrintFFR(format);
  }

  // Other state updates, including system registers.
  void PrintSystemRegister(SystemRegister id);
  void PrintTakenBranch(const Instruction* target);
  void PrintGCS(bool is_push, uint64_t addr, size_t entry);
  void LogSystemRegister(SystemRegister id) {
    if (ShouldTraceSysRegs()) PrintSystemRegister(id);
  }
  void LogTakenBranch(const Instruction* target) {
    if (ShouldTraceBranches()) PrintTakenBranch(target);
  }
  void LogGCS(bool is_push, uint64_t addr, size_t entry) {
    if (ShouldTraceSysRegs()) PrintGCS(is_push, addr, entry);
  }

  // Trace memory accesses.

  // Common, contiguous register accesses (such as for scalars).
  // The *Write variants automatically set kPrintRegPartial on the format.
  void PrintRead(int rt_code, PrintRegisterFormat format, uintptr_t address);
  void PrintExtendingRead(int rt_code,
                          PrintRegisterFormat format,
                          int access_size_in_bytes,
                          uintptr_t address);
  void PrintWrite(int rt_code, PrintRegisterFormat format, uintptr_t address);
  void PrintVRead(int rt_code, PrintRegisterFormat format, uintptr_t address);
  void PrintVWrite(int rt_code, PrintRegisterFormat format, uintptr_t address);
  // Simple, unpredicated SVE accesses always access the whole vector, and never
  // know the lane type, so there's no need to accept a `format`.
  void PrintZRead(int rt_code, uintptr_t address) {
    vregisters_[rt_code].NotifyRegisterLogged();
    PrintZAccess(rt_code, "<-", address);
  }
  void PrintZWrite(int rt_code, uintptr_t address) {
    PrintZAccess(rt_code, "->", address);
  }
  void PrintPRead(int rt_code, uintptr_t address) {
    pregisters_[rt_code].NotifyRegisterLogged();
    PrintPAccess(rt_code, "<-", address);
  }
  void PrintPWrite(int rt_code, uintptr_t address) {
    PrintPAccess(rt_code, "->", address);
  }
  void PrintWriteU64(uint64_t x, uintptr_t address) {
    fprintf(stream_,
            "#      0x%016lx -> %s0x%016" PRIxPTR "%s\n",
            x,
            clr_memory_address,
            address,
            clr_normal);
  }

  // Like Print* (above), but respect GetTraceParameters().
  void LogRead(int rt_code, PrintRegisterFormat format, uintptr_t address) {
    if (ShouldTraceRegs()) PrintRead(rt_code, format, address);
  }
  void LogExtendingRead(int rt_code,
                        PrintRegisterFormat format,
                        int access_size_in_bytes,
                        uintptr_t address) {
    if (ShouldTraceRegs()) {
      PrintExtendingRead(rt_code, format, access_size_in_bytes, address);
    }
  }
  void LogWrite(int rt_code, PrintRegisterFormat format, uintptr_t address) {
    if (ShouldTraceWrites()) PrintWrite(rt_code, format, address);
  }
  void LogVRead(int rt_code, PrintRegisterFormat format, uintptr_t address) {
    if (ShouldTraceVRegs()) PrintVRead(rt_code, format, address);
  }
  void LogVWrite(int rt_code, PrintRegisterFormat format, uintptr_t address) {
    if (ShouldTraceWrites()) PrintVWrite(rt_code, format, address);
  }
  void LogZRead(int rt_code, uintptr_t address) {
    if (ShouldTraceVRegs()) PrintZRead(rt_code, address);
  }
  void LogZWrite(int rt_code, uintptr_t address) {
    if (ShouldTraceWrites()) PrintZWrite(rt_code, address);
  }
  void LogPRead(int rt_code, uintptr_t address) {
    if (ShouldTraceVRegs()) PrintPRead(rt_code, address);
  }
  void LogPWrite(int rt_code, uintptr_t address) {
    if (ShouldTraceWrites()) PrintPWrite(rt_code, address);
  }
  void LogWriteU64(uint64_t x, uintptr_t address) {
    if (ShouldTraceWrites()) PrintWriteU64(x, address);
  }
  void LogMemTransfer(uintptr_t dst, uintptr_t src, uint8_t value) {
    if (ShouldTraceWrites()) PrintMemTransfer(dst, src, value);
  }
  // Helpers for the above, where the access operation is parameterised.
  // - For loads, set op = "<-".
  // - For stores, set op = "->".
  void PrintAccess(int rt_code,
                   PrintRegisterFormat format,
                   const char* op,
                   uintptr_t address);
  void PrintVAccess(int rt_code,
                    PrintRegisterFormat format,
                    const char* op,
                    uintptr_t address);
  void PrintMemTransfer(uintptr_t dst, uintptr_t src, uint8_t value);
  // Simple, unpredicated SVE accesses always access the whole vector, and never
  // know the lane type, so these don't accept a `format`.
  void PrintZAccess(int rt_code, const char* op, uintptr_t address);
  void PrintPAccess(int rt_code, const char* op, uintptr_t address);

  // Multiple-structure accesses.
  void PrintVStructAccess(int rt_code,
                          int reg_count,
                          PrintRegisterFormat format,
                          const char* op,
                          uintptr_t address);
  // Single-structure (single-lane) accesses.
  void PrintVSingleStructAccess(int rt_code,
                                int reg_count,
                                int lane,
                                PrintRegisterFormat format,
                                const char* op,
                                uintptr_t address);
  // Replicating accesses.
  void PrintVReplicatingStructAccess(int rt_code,
                                     int reg_count,
                                     PrintRegisterFormat format,
                                     const char* op,
                                     uintptr_t address);

  // Multiple-structure accesses.
  void PrintZStructAccess(int rt_code,
                          int reg_count,
                          const LogicPRegister& pg,
                          PrintRegisterFormat format,
                          int msize_in_bytes,
                          const char* op,
                          const LogicSVEAddressVector& addr);

  // Register-printing helper for all structured accessors.
  //
  // All lanes (according to `format`) are printed, but lanes indicated by
  // `focus_mask` are of particular interest. Each bit corresponds to a byte in
  // the printed register, in a manner similar to SVE's predicates. Currently,
  // this is used to determine when to print human-readable FP annotations.
  void PrintVRegistersForStructuredAccess(int rt_code,
                                          int reg_count,
                                          uint16_t focus_mask,
                                          PrintRegisterFormat format);

  // As for the VRegister variant, but print partial Z register names.
  void PrintZRegistersForStructuredAccess(int rt_code,
                                          int q_index,
                                          int reg_count,
                                          uint16_t focus_mask,
                                          PrintRegisterFormat format);

  // Print part of a memory access. This should be used for annotating
  // non-trivial accesses, such as structured or sign-extending loads. Call
  // Print*Register (or Print*RegistersForStructuredAccess), then
  // PrintPartialAccess for each contiguous access that makes up the
  // instruction.
  //
  //  access_mask:
  //      The lanes to be printed. Each bit corresponds to a byte in the printed
  //      register, in a manner similar to SVE's predicates, except that the
  //      lane size is not respected when interpreting lane_mask: unaligned bits
  //      must be zeroed.
  //
  //      This function asserts that this mask is non-zero.
  //
  //  future_access_mask:
  //      The lanes to be printed by a future invocation. This must be specified
  //      because vertical lines are drawn for partial accesses that haven't yet
  //      been printed. The format is the same as for accessed_mask.
  //
  //      If a lane is active in both `access_mask` and `future_access_mask`,
  //      `access_mask` takes precedence.
  //
  //  struct_element_count:
  //      The number of elements in each structure. For non-structured accesses,
  //      set this to one. Along with lane_size_in_bytes, this is used determine
  //      the size of each access, and to format the accessed value.
  //
  //  op:
  //      For stores, use "->". For loads, use "<-".
  //
  //  address:
  //      The address of this partial access. (Not the base address of the whole
  //      instruction.) The traced value is read from this address (according to
  //      part_count and lane_size_in_bytes) so it must be accessible, and when
  //      tracing stores, the store must have been executed before this function
  //      is called.
  //
  //  reg_size_in_bytes:
  //      The size of the register being accessed. This helper is usually used
  //      for V registers or Q-sized chunks of Z registers, so that is the
  //      default, but it is possible to use this to annotate X register
  //      accesses by specifying kXRegSizeInBytes.
  //
  // The return value is a future_access_mask suitable for the next iteration,
  // so that it is possible to execute this in a loop, until the mask is zero.
  // Note that accessed_mask must still be updated by the caller for each call.
  uint16_t PrintPartialAccess(uint16_t access_mask,
                              uint16_t future_access_mask,
                              int struct_element_count,
                              int lane_size_in_bytes,
                              const char* op,
                              uintptr_t address,
                              int reg_size_in_bytes = kQRegSizeInBytes);

  // Print an abstract register value. This works for all register types, and
  // can print parts of registers. This exists to ensure consistent formatting
  // of values.
  void PrintRegisterValue(const uint8_t* value,
                          int value_size,
                          PrintRegisterFormat format);
  template <typename T>
  void PrintRegisterValue(const T& sim_register, PrintRegisterFormat format) {
    PrintRegisterValue(sim_register.GetBytes(),
                       std::min(sim_register.GetSizeInBytes(),
                                kQRegSizeInBytes),
                       format);
  }

  // As above, but format as an SVE predicate value, using binary notation with
  // spaces between each bit so that they align with the Z register bytes that
  // they predicate.
  void PrintPRegisterValue(uint16_t value);

  void PrintRegisterValueFPAnnotations(const uint8_t* value,
                                       uint16_t lane_mask,
                                       PrintRegisterFormat format);
  template <typename T>
  void PrintRegisterValueFPAnnotations(const T& sim_register,
                                       uint16_t lane_mask,
                                       PrintRegisterFormat format) {
    PrintRegisterValueFPAnnotations(sim_register.GetBytes(), lane_mask, format);
  }
  template <typename T>
  void PrintRegisterValueFPAnnotations(const T& sim_register,
                                       PrintRegisterFormat format) {
    PrintRegisterValueFPAnnotations(sim_register.GetBytes(),
                                    GetPrintRegLaneMask(format),
                                    format);
  }

  VIXL_NO_RETURN void DoUnreachable(const Instruction* instr);
  void DoTrace(const Instruction* instr);
  void DoLog(const Instruction* instr);

  static const char* WRegNameForCode(unsigned code,
                                     Reg31Mode mode = Reg31IsZeroRegister);
  static const char* XRegNameForCode(unsigned code,
                                     Reg31Mode mode = Reg31IsZeroRegister);
  static const char* BRegNameForCode(unsigned code);
  static const char* HRegNameForCode(unsigned code);
  static const char* SRegNameForCode(unsigned code);
  static const char* DRegNameForCode(unsigned code);
  static const char* VRegNameForCode(unsigned code);
  static const char* ZRegNameForCode(unsigned code);
  static const char* PRegNameForCode(unsigned code);

  bool IsColouredTrace() const { return coloured_trace_; }
  VIXL_DEPRECATED("IsColouredTrace", bool coloured_trace() const) {
    return IsColouredTrace();
  }

  void SetColouredTrace(bool value);
  VIXL_DEPRECATED("SetColouredTrace", void set_coloured_trace(bool value)) {
    SetColouredTrace(value);
  }

  // Values for traces parameters defined in simulator-constants-aarch64.h in
  // enum TraceParameters.
  int GetTraceParameters() const { return trace_parameters_; }
  VIXL_DEPRECATED("GetTraceParameters", int trace_parameters() const) {
    return GetTraceParameters();
  }

  bool ShouldTraceWrites() const {
    return (GetTraceParameters() & LOG_WRITE) != 0;
  }
  bool ShouldTraceRegs() const {
    return (GetTraceParameters() & LOG_REGS) != 0;
  }
  bool ShouldTraceVRegs() const {
    return (GetTraceParameters() & LOG_VREGS) != 0;
  }
  bool ShouldTraceSysRegs() const {
    return (GetTraceParameters() & LOG_SYSREGS) != 0;
  }
  bool ShouldTraceBranches() const {
    return (GetTraceParameters() & LOG_BRANCH) != 0;
  }

  void SetTraceParameters(int parameters);
  VIXL_DEPRECATED("SetTraceParameters",
                  void set_trace_parameters(int parameters)) {
    SetTraceParameters(parameters);
  }

  // Clear the simulated local monitor to force the next store-exclusive
  // instruction to fail.
  void ClearLocalMonitor() { local_monitor_.Clear(); }

  void SilenceExclusiveAccessWarning() {
    print_exclusive_access_warning_ = false;
  }

  void CheckIsValidUnalignedAtomicAccess(int rn,
                                         uint64_t address,
                                         unsigned access_size) {
    // Verify that the address is available to the host.
    VIXL_ASSERT(address == static_cast<uintptr_t>(address));

    if (GetCPUFeatures()->Has(CPUFeatures::kUSCAT)) {
      // Check that the access falls entirely within one atomic access granule.
      if (AlignDown(address, kAtomicAccessGranule) !=
          AlignDown(address + access_size - 1, kAtomicAccessGranule)) {
        VIXL_ALIGNMENT_EXCEPTION();
      }
    } else {
      // Check that the access is aligned.
      if (AlignDown(address, access_size) != address) {
        VIXL_ALIGNMENT_EXCEPTION();
      }
    }

    // The sp must be aligned to 16 bytes when it is accessed.
    if ((rn == kSpRegCode) && (AlignDown(address, 16) != address)) {
      VIXL_ALIGNMENT_EXCEPTION();
    }
  }

  enum PointerType { kDataPointer, kInstructionPointer };

  struct PACKey {
    uint64_t high;
    uint64_t low;
    int number;
  };

  // Current implementation is that all pointers are tagged.
  bool HasTBI(uint64_t ptr, PointerType type) {
    USE(ptr, type);
    return true;
  }

  // Current implementation uses 48-bit virtual addresses.
  int GetBottomPACBit(uint64_t ptr, int ttbr) {
    USE(ptr, ttbr);
    VIXL_ASSERT((ttbr == 0) || (ttbr == 1));
    return 48;
  }

  // The top PAC bit is 55 for the purposes of relative bit fields with TBI,
  // however bit 55 is the TTBR bit regardless of TBI so isn't part of the PAC
  // codes in pointers.
  int GetTopPACBit(uint64_t ptr, PointerType type) {
    return HasTBI(ptr, type) ? 55 : 63;
  }

  // Armv8.3 Pointer authentication helpers.
  uint64_t CalculatePACMask(uint64_t ptr, PointerType type, int ext_bit);
  uint64_t ComputePAC(uint64_t data, uint64_t context, PACKey key);
  uint64_t AuthPAC(uint64_t ptr,
                   uint64_t context,
                   PACKey key,
                   PointerType type);
  uint64_t AddPAC(uint64_t ptr, uint64_t context, PACKey key, PointerType type);
  uint64_t StripPAC(uint64_t ptr, PointerType type);
  void PACHelper(int dst,
                 int src,
                 PACKey key,
                 decltype(&Simulator::AddPAC) pac_fn);

  // Armv8.5 MTE helpers.
  uint64_t ChooseNonExcludedTag(uint64_t tag,
                                uint64_t offset,
                                uint64_t exclude = 0) {
    VIXL_ASSERT(IsUint4(tag) && IsUint4(offset) && IsUint16(exclude));

    if (exclude == 0xffff) {
      return 0;
    }

    if (offset == 0) {
      while ((exclude & (uint64_t{1} << tag)) != 0) {
        tag = (tag + 1) % 16;
      }
    }

    while (offset > 0) {
      offset--;
      tag = (tag + 1) % 16;
      while ((exclude & (uint64_t{1} << tag)) != 0) {
        tag = (tag + 1) % 16;
      }
    }
    return tag;
  }

  uint64_t GetAddressWithAllocationTag(uint64_t addr, uint64_t tag) {
    VIXL_ASSERT(IsUint4(tag));
    return (addr & ~(UINT64_C(0xf) << 56)) | (tag << 56);
  }

#if __linux__
#define VIXL_HAS_SIMULATED_MMAP
  // Create or remove a mapping with memory protection. Memory attributes such
  // as MTE and BTI are represented by metadata in Simulator.
  void* Mmap(
      void* address, size_t length, int prot, int flags, int fd, off_t offset);

  int Munmap(void* address, size_t length, int prot);
#endif

  // The common CPUFeatures interface with the set of available features.

  CPUFeatures* GetCPUFeatures() {
    return cpu_features_auditor_.GetCPUFeatures();
  }

  void SetCPUFeatures(const CPUFeatures& cpu_features) {
    cpu_features_auditor_.SetCPUFeatures(cpu_features);
  }

  // The set of features that the simulator has encountered.
  const CPUFeatures& GetSeenFeatures() {
    return cpu_features_auditor_.GetSeenFeatures();
  }
  void ResetSeenFeatures() { cpu_features_auditor_.ResetSeenFeatures(); }

// Runtime call emulation support.
// It requires VIXL's ABI features, and C++11 or greater.
// Also, the initialisation of the tuples in RuntimeCall(Non)Void is incorrect
// in GCC before 4.9.1: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51253
#if defined(VIXL_HAS_ABI_SUPPORT) && __cplusplus >= 201103L && \
    (defined(_MSC_VER) || defined(__clang__) || GCC_VERSION_OR_NEWER(4, 9, 1))

#define VIXL_HAS_SIMULATED_RUNTIME_CALL_SUPPORT

// The implementation of the runtime call helpers require the functionality
// provided by `std::index_sequence`. It is only available from C++14, but
// we want runtime call simulation to work from C++11, so we emulate if
// necessary.
#if __cplusplus >= 201402L
  template <std::size_t... I>
  using local_index_sequence = std::index_sequence<I...>;
  template <typename... P>
  using __local_index_sequence_for = std::index_sequence_for<P...>;
#else
  // Emulate the behaviour of `std::index_sequence` and
  // `std::index_sequence_for`.
  // Naming follow the `std` names, prefixed with `emulated_`.
  template <size_t... I>
  struct emulated_index_sequence {};

  // A recursive template to create a sequence of indexes.
  // The base case (for `N == 0`) is declared outside of the class scope, as
  // required by C++.
  template <std::size_t N, size_t... I>
  struct emulated_make_index_sequence_helper
      : emulated_make_index_sequence_helper<N - 1, N - 1, I...> {};

  template <std::size_t N>
  struct emulated_make_index_sequence : emulated_make_index_sequence_helper<N> {
  };

  template <typename... P>
  struct emulated_index_sequence_for
      : emulated_make_index_sequence<sizeof...(P)> {};

  template <std::size_t... I>
  using local_index_sequence = emulated_index_sequence<I...>;
  template <typename... P>
  using __local_index_sequence_for = emulated_index_sequence_for<P...>;
#endif

  // Expand the argument tuple and perform the call.
  template <typename R, typename... P, std::size_t... I>
  R DoRuntimeCall(R (*function)(P...),
                  std::tuple<P...> arguments,
                  local_index_sequence<I...>) {
    USE(arguments);
    return function(std::get<I>(arguments)...);
  }

  template <typename R, typename... P>
  void RuntimeCallNonVoid(R (*function)(P...)) {
    ABI abi;
    std::tuple<P...> argument_operands{
        ReadGenericOperand<P>(abi.GetNextParameterGenericOperand<P>())...};
    R return_value = DoRuntimeCall(function,
                                   argument_operands,
                                   __local_index_sequence_for<P...>{});
    bool succeeded =
        WriteGenericOperand(abi.GetReturnGenericOperand<R>(), return_value);
    USE(succeeded);
    VIXL_ASSERT(succeeded);
  }

  template <typename R, typename... P>
  void RuntimeCallVoid(R (*function)(P...)) {
    ABI abi;
    std::tuple<P...> argument_operands{
        ReadGenericOperand<P>(abi.GetNextParameterGenericOperand<P>())...};
    DoRuntimeCall(function,
                  argument_operands,
                  __local_index_sequence_for<P...>{});
  }

  // We use `struct` for `void` return type specialisation.
  template <typename R, typename... P>
  struct RuntimeCallStructHelper {
    static void Wrapper(Simulator* simulator, uintptr_t function_pointer) {
      R (*function)(P...) = reinterpret_cast<R (*)(P...)>(function_pointer);
      simulator->RuntimeCallNonVoid(function);
    }
  };

  // Partial specialization when the return type is `void`.
  template <typename... P>
  struct RuntimeCallStructHelper<void, P...> {
    static void Wrapper(Simulator* simulator, uintptr_t function_pointer) {
      void (*function)(P...) =
          reinterpret_cast<void (*)(P...)>(function_pointer);
      simulator->RuntimeCallVoid(function);
    }
  };
#endif

  // Configure the simulated value of 'VL', which is the size of a Z register.
  // Because this cannot occur during a program's lifetime, this function also
  // resets the SVE registers.
  void SetVectorLengthInBits(unsigned vector_length);

  unsigned GetVectorLengthInBits() const { return vector_length_; }
  unsigned GetVectorLengthInBytes() const {
    VIXL_ASSERT((vector_length_ % kBitsPerByte) == 0);
    return vector_length_ / kBitsPerByte;
  }
  unsigned GetPredicateLengthInBits() const {
    VIXL_ASSERT((GetVectorLengthInBits() % kZRegBitsPerPRegBit) == 0);
    return GetVectorLengthInBits() / kZRegBitsPerPRegBit;
  }
  unsigned GetPredicateLengthInBytes() const {
    VIXL_ASSERT((GetVectorLengthInBytes() % kZRegBitsPerPRegBit) == 0);
    return GetVectorLengthInBytes() / kZRegBitsPerPRegBit;
  }

  unsigned RegisterSizeInBitsFromFormat(VectorFormat vform) const {
    if (IsSVEFormat(vform)) {
      return GetVectorLengthInBits();
    } else {
      return vixl::aarch64::RegisterSizeInBitsFromFormat(vform);
    }
  }

  unsigned RegisterSizeInBytesFromFormat(VectorFormat vform) const {
    unsigned size_in_bits = RegisterSizeInBitsFromFormat(vform);
    VIXL_ASSERT((size_in_bits % kBitsPerByte) == 0);
    return size_in_bits / kBitsPerByte;
  }

  int LaneCountFromFormat(VectorFormat vform) const {
    if (IsSVEFormat(vform)) {
      return GetVectorLengthInBits() / LaneSizeInBitsFromFormat(vform);
    } else {
      return vixl::aarch64::LaneCountFromFormat(vform);
    }
  }

  bool IsFirstActive(VectorFormat vform,
                     const LogicPRegister& mask,
                     const LogicPRegister& bits) {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      if (mask.IsActive(vform, i)) {
        return bits.IsActive(vform, i);
      }
    }
    return false;
  }

  bool AreNoneActive(VectorFormat vform,
                     const LogicPRegister& mask,
                     const LogicPRegister& bits) {
    for (int i = 0; i < LaneCountFromFormat(vform); i++) {
      if (mask.IsActive(vform, i) && bits.IsActive(vform, i)) {
        return false;
      }
    }
    return true;
  }

  bool IsLastActive(VectorFormat vform,
                    const LogicPRegister& mask,
                    const LogicPRegister& bits) {
    for (int i = LaneCountFromFormat(vform) - 1; i >= 0; i--) {
      if (mask.IsActive(vform, i)) {
        return bits.IsActive(vform, i);
      }
    }
    return false;
  }

  void PredTest(VectorFormat vform,
                const LogicPRegister& mask,
                const LogicPRegister& bits) {
    ReadNzcv().SetN(IsFirstActive(vform, mask, bits));
    ReadNzcv().SetZ(AreNoneActive(vform, mask, bits));
    ReadNzcv().SetC(!IsLastActive(vform, mask, bits));
    ReadNzcv().SetV(0);
    LogSystemRegister(NZCV);
  }

  SimPRegister& GetPTrue() { return pregister_all_true_; }

  template <typename T>
  size_t CleanGranuleTag(T address, size_t length = kMTETagGranuleInBytes) {
    size_t count = 0;
    for (size_t offset = 0; offset < length; offset += kMTETagGranuleInBytes) {
      count +=
          meta_data_.CleanMTETag(reinterpret_cast<uintptr_t>(address) + offset);
    }
    size_t expected =
        length / kMTETagGranuleInBytes + (length % kMTETagGranuleInBytes != 0);

    // Give a warning when the memory region that is being unmapped isn't all
    // either MTE protected or not.
    if (count != expected) {
      std::stringstream sstream;
      sstream << std::hex
              << "MTE WARNING : the memory region being unmapped "
                 "starting at address 0x"
              << reinterpret_cast<uint64_t>(address)
              << "is not fully MTE protected.\n";
      VIXL_WARNING(sstream.str().c_str());
    }
    return count;
  }

  template <typename T>
  void SetGranuleTag(T address,
                     int tag,
                     size_t length = kMTETagGranuleInBytes) {
    for (size_t offset = 0; offset < length; offset += kMTETagGranuleInBytes) {
      meta_data_.SetMTETag((uintptr_t)(address) + offset, tag);
    }
  }

  template <typename T>
  int GetGranuleTag(T address) {
    return meta_data_.GetMTETag(address);
  }

  // Generate a random address tag, and any tags specified in the input are
  // excluded from the selection.
  uint64_t GenerateRandomTag(uint16_t exclude = 0);

  // Register a new BranchInterception object. If 'function' is branched to
  // (e.g: "bl function") in the future; instead, if provided, 'callback' will
  // be called otherwise a runtime call will be performed on 'function'.
  //
  // For example: this can be used to always perform runtime calls on
  // non-AArch64 functions without using the macroassembler.
  template <typename R, typename... P>
  void RegisterBranchInterception(R (*function)(P...),
                                  InterceptionCallback callback = nullptr) {
    meta_data_.RegisterBranchInterception(*function, callback);
  }

  // Return the current output stream in use by the simulator.
  FILE* GetOutputStream() const { return stream_; }

  bool IsDebuggerEnabled() const { return debugger_enabled_; }

  void SetDebuggerEnabled(bool enabled) { debugger_enabled_ = enabled; }

  Debugger* GetDebugger() const { return debugger_.get(); }

#ifdef VIXL_ENABLE_IMPLICIT_CHECKS
  // Returns true if the faulting instruction address (usually the program
  // counter or instruction pointer) comes from an internal VIXL memory access.
  // This can be used by signal handlers to check if a signal was raised from
  // the simulator (via TryMemoryAccess) before the actual
  // access occurs.
  bool IsSimulatedMemoryAccess(uintptr_t fault_pc) const {
    return (fault_pc ==
            reinterpret_cast<uintptr_t>(&_vixl_internal_ReadMemory));
  }

  // Get the instruction address of the internal VIXL memory access continuation
  // label. Signal handlers can resume execution at this address to return to
  // TryMemoryAccess which will continue simulation.
  uintptr_t GetSignalReturnAddress() const {
    return reinterpret_cast<uintptr_t>(&_vixl_internal_AccessMemory_continue);
  }

  // Replace the fault address reported by the kernel with the actual faulting
  // address.
  //
  // This is required because TryMemoryAccess reads a section of
  // memory 1 byte at a time meaning the fault address reported may not be the
  // base address of memory being accessed.
  void ReplaceFaultAddress(siginfo_t* siginfo, void* context) {
#ifdef __x86_64__
    // The base address being accessed is passed in as the first argument to
    // _vixl_internal_ReadMemory.
    ucontext_t* uc = reinterpret_cast<ucontext_t*>(context);
    siginfo->si_addr = reinterpret_cast<void*>(uc->uc_mcontext.gregs[REG_RDI]);
#else
    USE(siginfo);
    USE(context);
#endif  // __x86_64__
  }
#endif  // VIXL_ENABLE_IMPLICIT_CHECKS

 protected:
  const char* clr_normal;
  const char* clr_flag_name;
  const char* clr_flag_value;
  const char* clr_reg_name;
  const char* clr_reg_value;
  const char* clr_vreg_name;
  const char* clr_vreg_value;
  const char* clr_preg_name;
  const char* clr_preg_value;
  const char* clr_memory_address;
  const char* clr_warning;
  const char* clr_warning_message;
  const char* clr_printf;
  const char* clr_branch_marker;

  // Simulation helpers ------------------------------------

  void ResetSystemRegisters();
  void ResetRegisters();
  void ResetVRegisters();
  void ResetPRegisters();
  void ResetFFR();

  bool ConditionPassed(Condition cond) {
    switch (cond) {
      case eq:
        return ReadZ();
      case ne:
        return !ReadZ();
      case hs:
        return ReadC();
      case lo:
        return !ReadC();
      case mi:
        return ReadN();
      case pl:
        return !ReadN();
      case vs:
        return ReadV();
      case vc:
        return !ReadV();
      case hi:
        return ReadC() && !ReadZ();
      case ls:
        return !(ReadC() && !ReadZ());
      case ge:
        return ReadN() == ReadV();
      case lt:
        return ReadN() != ReadV();
      case gt:
        return !ReadZ() && (ReadN() == ReadV());
      case le:
        return !(!ReadZ() && (ReadN() == ReadV()));
      case nv:
        VIXL_FALLTHROUGH();
      case al:
        return true;
      default:
        VIXL_UNREACHABLE();
        return false;
    }
  }

  bool ConditionPassed(Instr cond) {
    return ConditionPassed(static_cast<Condition>(cond));
  }

  bool ConditionFailed(Condition cond) { return !ConditionPassed(cond); }

  void AddSubHelper(const Instruction* instr, int64_t op2);
  uint64_t AddWithCarry(unsigned reg_size,
                        bool set_flags,
                        uint64_t left,
                        uint64_t right,
                        int carry_in = 0);
  std::pair<uint64_t, uint8_t> AddWithCarry(unsigned reg_size,
                                            uint64_t left,
                                            uint64_t right,
                                            int carry_in);
  vixl_uint128_t Add128(vixl_uint128_t x, vixl_uint128_t y);
  vixl_uint128_t Lsl128(vixl_uint128_t x, unsigned shift) const;
  vixl_uint128_t Eor128(vixl_uint128_t x, vixl_uint128_t y) const;
  vixl_uint128_t Mul64(uint64_t x, uint64_t y);
  vixl_uint128_t Neg128(vixl_uint128_t x);
  void LogicalHelper(const Instruction* instr, int64_t op2);
  void ConditionalCompareHelper(const Instruction* instr, int64_t op2);
  void LoadStoreHelper(const Instruction* instr,
                       int64_t offset,
                       AddrMode addrmode);
  void LoadStorePairHelper(const Instruction* instr, AddrMode addrmode);
  template <typename T>
  void CompareAndSwapHelper(const Instruction* instr);
  template <typename T>
  void CompareAndSwapPairHelper(const Instruction* instr);
  template <typename T>
  void AtomicMemorySimpleHelper(const Instruction* instr);
  template <typename T>
  void AtomicMemorySwapHelper(const Instruction* instr);
  template <typename T>
  void LoadAcquireRCpcHelper(const Instruction* instr);
  template <typename T1, typename T2>
  void LoadAcquireRCpcUnscaledOffsetHelper(const Instruction* instr);
  template <typename T>
  void StoreReleaseUnscaledOffsetHelper(const Instruction* instr);
  uintptr_t AddressModeHelper(unsigned addr_reg,
                              int64_t offset,
                              AddrMode addrmode);
  void NEONLoadStoreMultiStructHelper(const Instruction* instr,
                                      AddrMode addr_mode);
  void NEONLoadStoreSingleStructHelper(const Instruction* instr,
                                       AddrMode addr_mode);
  template <uint32_t mops_type>
  void MOPSPHelper(const Instruction* instr) {
    VIXL_ASSERT(instr->IsConsistentMOPSTriplet<mops_type>());

    int d = instr->GetRd();
    int n = instr->GetRn();
    int s = instr->GetRs();

    // Aliased registers and xzr are disallowed for Xd and Xn.
    if ((d == n) || (d == s) || (n == s) || (d == 31) || (n == 31)) {
      VisitUnallocated(instr);
    }

    // Additionally, Xs may not be xzr for cpy.
    if ((mops_type == "cpy"_h) && (s == 31)) {
      VisitUnallocated(instr);
    }

    // Bits 31 and 30 must be zero.
    if (instr->ExtractBits(31, 30) != 0) {
      VisitUnallocated(instr);
    }

    // Saturate copy count.
    uint64_t xn = ReadXRegister(n);
    int saturation_bits = (mops_type == "cpy"_h) ? 55 : 63;
    if ((xn >> saturation_bits) != 0) {
      xn = (UINT64_C(1) << saturation_bits) - 1;
      if (mops_type == "setg"_h) {
        // Align saturated value to granule.
        xn &= ~UINT64_C(kMTETagGranuleInBytes - 1);
      }
      WriteXRegister(n, xn);
    }

    ReadNzcv().SetN(0);
    ReadNzcv().SetZ(0);
    ReadNzcv().SetC(1);  // Indicates "option B" implementation.
    ReadNzcv().SetV(0);
  }

  int64_t ShiftOperand(unsigned reg_size,
                       uint64_t value,
                       Shift shift_type,
                       unsigned amount) const;
  int64_t ExtendValue(unsigned reg_width,
                      int64_t value,
                      Extend extend_type,
                      unsigned left_shift = 0) const;
  uint64_t PolynomialMult(uint64_t op1,
                          uint64_t op2,
                          int lane_size_in_bits) const;
  vixl_uint128_t PolynomialMult128(uint64_t op1,
                                   uint64_t op2,
                                   int lane_size_in_bits) const;

  bool ld1(VectorFormat vform, LogicVRegister dst, uint64_t addr);
  bool ld1(VectorFormat vform, LogicVRegister dst, int index, uint64_t addr);
  bool ld1r(VectorFormat vform, LogicVRegister dst, uint64_t addr);
  bool ld1r(VectorFormat vform,
            VectorFormat unpack_vform,
            LogicVRegister dst,
            uint64_t addr,
            bool is_signed = false);
  bool ld2(VectorFormat vform,
           LogicVRegister dst1,
           LogicVRegister dst2,
           uint64_t addr);
  bool ld2(VectorFormat vform,
           LogicVRegister dst1,
           LogicVRegister dst2,
           int index,
           uint64_t addr);
  bool ld2r(VectorFormat vform,
            LogicVRegister dst1,
            LogicVRegister dst2,
            uint64_t addr);
  bool ld3(VectorFormat vform,
           LogicVRegister dst1,
           LogicVRegister dst2,
           LogicVRegister dst3,
           uint64_t addr);
  bool ld3(VectorFormat vform,
           LogicVRegister dst1,
           LogicVRegister dst2,
           LogicVRegister dst3,
           int index,
           uint64_t addr);
  bool ld3r(VectorFormat vform,
            LogicVRegister dst1,
            LogicVRegister dst2,
            LogicVRegister dst3,
            uint64_t addr);
  bool ld4(VectorFormat vform,
           LogicVRegister dst1,
           LogicVRegister dst2,
           LogicVRegister dst3,
           LogicVRegister dst4,
           uint64_t addr);
  bool ld4(VectorFormat vform,
           LogicVRegister dst1,
           LogicVRegister dst2,
           LogicVRegister dst3,
           LogicVRegister dst4,
           int index,
           uint64_t addr);
  bool ld4r(VectorFormat vform,
            LogicVRegister dst1,
            LogicVRegister dst2,
            LogicVRegister dst3,
            LogicVRegister dst4,
            uint64_t addr);
  bool st1(VectorFormat vform, LogicVRegister src, uint64_t addr);
  bool st1(VectorFormat vform, LogicVRegister src, int index, uint64_t addr);
  bool st2(VectorFormat vform,
           LogicVRegister src,
           LogicVRegister src2,
           uint64_t addr);
  bool st2(VectorFormat vform,
           LogicVRegister src,
           LogicVRegister src2,
           int index,
           uint64_t addr);
  bool st3(VectorFormat vform,
           LogicVRegister src,
           LogicVRegister src2,
           LogicVRegister src3,
           uint64_t addr);
  bool st3(VectorFormat vform,
           LogicVRegister src,
           LogicVRegister src2,
           LogicVRegister src3,
           int index,
           uint64_t addr);
  bool st4(VectorFormat vform,
           LogicVRegister src,
           LogicVRegister src2,
           LogicVRegister src3,
           LogicVRegister src4,
           uint64_t addr);
  bool st4(VectorFormat vform,
           LogicVRegister src,
           LogicVRegister src2,
           LogicVRegister src3,
           LogicVRegister src4,
           int index,
           uint64_t addr);
  LogicVRegister cmp(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2,
                     Condition cond);
  LogicVRegister cmp(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     int imm,
                     Condition cond);
  LogicVRegister cmptst(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister add(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  // Add `value` to each lane of `src1`, treating `value` as unsigned for the
  // purposes of setting the saturation flags.
  LogicVRegister add_uint(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          uint64_t value);
  LogicVRegister addp(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicPRegister brka(LogicPRegister pd,
                      const LogicPRegister& pg,
                      const LogicPRegister& pn);
  LogicPRegister brkb(LogicPRegister pd,
                      const LogicPRegister& pg,
                      const LogicPRegister& pn);
  LogicPRegister brkn(LogicPRegister pdm,
                      const LogicPRegister& pg,
                      const LogicPRegister& pn);
  LogicPRegister brkpa(LogicPRegister pd,
                       const LogicPRegister& pg,
                       const LogicPRegister& pn,
                       const LogicPRegister& pm);
  LogicPRegister brkpb(LogicPRegister pd,
                       const LogicPRegister& pg,
                       const LogicPRegister& pn,
                       const LogicPRegister& pm);
  // dst = srca + src1 * src2
  LogicVRegister mla(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& srca,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  // dst = srca - src1 * src2
  LogicVRegister mls(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& srca,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister mul(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister mul(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2,
                     int index);
  LogicVRegister mla(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2,
                     int index);
  LogicVRegister mls(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2,
                     int index);
  LogicVRegister pmul(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister sdiv(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister udiv(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);

  typedef LogicVRegister (Simulator::*ByElementOp)(VectorFormat vform,
                                                   LogicVRegister dst,
                                                   const LogicVRegister& src1,
                                                   const LogicVRegister& src2,
                                                   int index);
  LogicVRegister fmul(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      int index);
  LogicVRegister fmla(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      int index);
  LogicVRegister fmlal(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       int index);
  LogicVRegister fmlal2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2,
                        int index);
  LogicVRegister fmls(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      int index);
  LogicVRegister fmlsl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       int index);
  LogicVRegister fmlsl2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2,
                        int index);
  LogicVRegister fmulx(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       int index);
  LogicVRegister smulh(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister umulh(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister sqdmull(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         int index);
  LogicVRegister sqdmlal(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         int index);
  LogicVRegister sqdmlsl(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         int index);
  LogicVRegister sqdmulh(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         int index);
  LogicVRegister sqrdmulh(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          int index);
  LogicVRegister sqrdmlah(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          int index);
  LogicVRegister sqrdmlsh(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          int index);
  LogicVRegister sub(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  // Subtract `value` from each lane of `src1`, treating `value` as unsigned for
  // the purposes of setting the saturation flags.
  LogicVRegister sub_uint(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          uint64_t value);
  LogicVRegister and_(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister orr(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister orn(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister eor(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister bic(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister bic(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     uint64_t imm);
  LogicVRegister bif(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister bit(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister bsl(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src_mask,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicVRegister cls(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicVRegister clz(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicVRegister cnot(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  LogicVRegister cnt(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicVRegister not_(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  LogicVRegister rbit(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  LogicVRegister rev(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicVRegister rev_byte(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src,
                          int rev_size);
  LogicVRegister rev16(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister rev32(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister rev64(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister addlp(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       bool is_signed,
                       bool do_accumulate);
  LogicVRegister saddlp(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister uaddlp(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister sadalp(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister uadalp(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister ror(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     int rotation);
  LogicVRegister rol(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     int rotation);
  LogicVRegister ext(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2,
                     int index);
  LogicVRegister rotate_elements_right(VectorFormat vform,
                                       LogicVRegister dst,
                                       const LogicVRegister& src,
                                       int index);
  template <typename T>
  LogicVRegister fcadd(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       int rot);
  LogicVRegister fcadd(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       int rot);
  template <typename T>
  LogicVRegister fcmla(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       const LogicVRegister& acc,
                       int index,
                       int rot);
  LogicVRegister fcmla(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       int index,
                       int rot);
  LogicVRegister fcmla(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       const LogicVRegister& acc,
                       int rot);
  template <typename T>
  LogicVRegister fadda(VectorFormat vform,
                       LogicVRegister acc,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);
  LogicVRegister fadda(VectorFormat vform,
                       LogicVRegister acc,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);
  LogicVRegister cadd(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      int rot,
                      bool saturate = false);
  LogicVRegister cmla(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& srca,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      int rot);
  LogicVRegister cmla(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& srca,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      int index,
                      int rot);
  LogicVRegister bgrp(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      bool do_bext = false);
  LogicVRegister bdep(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister histogram(VectorFormat vform,
                           LogicVRegister dst,
                           const LogicPRegister& pg,
                           const LogicVRegister& src1,
                           const LogicVRegister& src2,
                           bool do_segmented = false);
  LogicVRegister index(VectorFormat vform,
                       LogicVRegister dst,
                       uint64_t start,
                       uint64_t step);
  LogicVRegister ins_element(VectorFormat vform,
                             LogicVRegister dst,
                             int dst_index,
                             const LogicVRegister& src,
                             int src_index);
  LogicVRegister ins_immediate(VectorFormat vform,
                               LogicVRegister dst,
                               int dst_index,
                               uint64_t imm);
  LogicVRegister insr(VectorFormat vform, LogicVRegister dst, uint64_t imm);
  LogicVRegister dup_element(VectorFormat vform,
                             LogicVRegister dst,
                             const LogicVRegister& src,
                             int src_index);
  LogicVRegister dup_elements_to_segments(VectorFormat vform,
                                          LogicVRegister dst,
                                          const LogicVRegister& src,
                                          int src_index);
  LogicVRegister dup_elements_to_segments(
      VectorFormat vform,
      LogicVRegister dst,
      const std::pair<int, int>& src_and_index);
  LogicVRegister dup_immediate(VectorFormat vform,
                               LogicVRegister dst,
                               uint64_t imm);
  LogicVRegister mov(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicPRegister mov(LogicPRegister dst, const LogicPRegister& src);
  LogicVRegister mov_merging(VectorFormat vform,
                             LogicVRegister dst,
                             const SimPRegister& pg,
                             const LogicVRegister& src);
  LogicVRegister mov_zeroing(VectorFormat vform,
                             LogicVRegister dst,
                             const SimPRegister& pg,
                             const LogicVRegister& src);
  LogicVRegister mov_alternating(VectorFormat vform,
                                 LogicVRegister dst,
                                 const LogicVRegister& src,
                                 int start_at);
  LogicPRegister mov_merging(LogicPRegister dst,
                             const LogicPRegister& pg,
                             const LogicPRegister& src);
  LogicPRegister mov_zeroing(LogicPRegister dst,
                             const LogicPRegister& pg,
                             const LogicPRegister& src);
  LogicVRegister movi(VectorFormat vform, LogicVRegister dst, uint64_t imm);
  LogicVRegister mvni(VectorFormat vform, LogicVRegister dst, uint64_t imm);
  LogicVRegister orr(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     uint64_t imm);
  LogicVRegister sshl(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      bool shift_is_8bit = true);
  LogicVRegister ushl(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      bool shift_is_8bit = true);
  LogicVRegister sshr(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister ushr(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  // Perform a "conditional last" operation. The first part of the pair is true
  // if any predicate lane is active, false otherwise. The second part takes the
  // value of the last active (plus offset) lane, or last (plus offset) lane if
  // none active.
  std::pair<bool, uint64_t> clast(VectorFormat vform,
                                  const LogicPRegister& pg,
                                  const LogicVRegister& src2,
                                  int offset_from_last_active);
  LogicPRegister match(VectorFormat vform,
                       LogicPRegister dst,
                       const LogicVRegister& haystack,
                       const LogicVRegister& needles,
                       bool negate_match);
  LogicVRegister compact(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicPRegister& pg,
                         const LogicVRegister& src);
  LogicVRegister splice(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicPRegister& pg,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister sel(VectorFormat vform,
                     LogicVRegister dst,
                     const SimPRegister& pg,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2);
  LogicPRegister sel(LogicPRegister dst,
                     const LogicPRegister& pg,
                     const LogicPRegister& src1,
                     const LogicPRegister& src2);
  LogicVRegister sminmax(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         bool max);
  LogicVRegister smax(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister smin(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister sminmaxp(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          bool max);
  LogicVRegister smaxp(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister sminp(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister addp(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  LogicVRegister addv(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  LogicVRegister uaddlv(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister saddlv(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister sminmaxv(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicPRegister& pg,
                          const LogicVRegister& src,
                          bool max);
  LogicVRegister smaxv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister sminv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister uxtl(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src,
                      bool is_2 = false);
  LogicVRegister uxtl2(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister sxtl(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src,
                      bool is_2 = false);
  LogicVRegister sxtl2(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister uxt(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     unsigned from_size_in_bits);
  LogicVRegister sxt(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     unsigned from_size_in_bits);
  LogicVRegister tbl(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& ind);
  LogicVRegister tbl(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& tab2,
                     const LogicVRegister& ind);
  LogicVRegister tbl(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& tab2,
                     const LogicVRegister& tab3,
                     const LogicVRegister& ind);
  LogicVRegister tbl(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& tab2,
                     const LogicVRegister& tab3,
                     const LogicVRegister& tab4,
                     const LogicVRegister& ind);
  LogicVRegister Table(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& ind,
                       bool zero_out_of_bounds,
                       const LogicVRegister* tab1,
                       const LogicVRegister* tab2 = NULL,
                       const LogicVRegister* tab3 = NULL,
                       const LogicVRegister* tab4 = NULL);
  LogicVRegister tbx(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& ind);
  LogicVRegister tbx(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& tab2,
                     const LogicVRegister& ind);
  LogicVRegister tbx(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& tab2,
                     const LogicVRegister& tab3,
                     const LogicVRegister& ind);
  LogicVRegister tbx(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& tab,
                     const LogicVRegister& tab2,
                     const LogicVRegister& tab3,
                     const LogicVRegister& tab4,
                     const LogicVRegister& ind);
  LogicVRegister uaddl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister uaddl2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister uaddw(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister uaddw2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister saddl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister saddl2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister saddw(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister saddw2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister usubl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister usubl2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister usubw(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister usubw2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister ssubl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister ssubl2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister ssubw(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister ssubw2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister uminmax(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         bool max);
  LogicVRegister umax(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister umin(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister uminmaxp(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          bool max);
  LogicVRegister umaxp(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister uminp(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister uminmaxv(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicPRegister& pg,
                          const LogicVRegister& src,
                          bool max);
  LogicVRegister umaxv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister uminv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister trn1(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister trn2(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister zip1(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister zip2(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister uzp1(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister uzp2(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister shl(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     int shift);
  LogicVRegister scvtf(VectorFormat vform,
                       unsigned dst_data_size_in_bits,
                       unsigned src_data_size_in_bits,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src,
                       FPRounding round,
                       int fbits = 0);
  LogicVRegister scvtf(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int fbits,
                       FPRounding rounding_mode);
  LogicVRegister ucvtf(VectorFormat vform,
                       unsigned dst_data_size,
                       unsigned src_data_size,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src,
                       FPRounding round,
                       int fbits = 0);
  LogicVRegister ucvtf(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int fbits,
                       FPRounding rounding_mode);
  LogicVRegister sshll(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister sshll2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src,
                        int shift);
  LogicVRegister shll(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  LogicVRegister shll2(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister ushll(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister ushll2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src,
                        int shift);
  LogicVRegister sli(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     int shift);
  LogicVRegister sri(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src,
                     int shift);
  LogicVRegister sshr(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src,
                      int shift);
  LogicVRegister ushr(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src,
                      int shift);
  LogicVRegister ssra(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src,
                      int shift);
  LogicVRegister usra(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src,
                      int shift);
  LogicVRegister srsra(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister ursra(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister suqadd(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister usqadd(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister sqshl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister uqshl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister sqshlu(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src,
                        int shift);
  LogicVRegister abs(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicVRegister neg(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicVRegister extractnarrow(VectorFormat vform,
                               LogicVRegister dst,
                               bool dst_is_signed,
                               const LogicVRegister& src,
                               bool src_is_signed);
  LogicVRegister xtn(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src);
  LogicVRegister sqxtn(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister uqxtn(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister sqxtun(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister absdiff(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         bool is_signed);
  LogicVRegister saba(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister uaba(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister shrn(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src,
                      int shift);
  LogicVRegister shrn2(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister rshrn(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       int shift);
  LogicVRegister rshrn2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src,
                        int shift);
  LogicVRegister uqshrn(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src,
                        int shift);
  LogicVRegister uqshrn2(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src,
                         int shift);
  LogicVRegister uqrshrn(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src,
                         int shift);
  LogicVRegister uqrshrn2(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src,
                          int shift);
  LogicVRegister sqshrn(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src,
                        int shift);
  LogicVRegister sqshrn2(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src,
                         int shift);
  LogicVRegister sqrshrn(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src,
                         int shift);
  LogicVRegister sqrshrn2(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src,
                          int shift);
  LogicVRegister sqshrun(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src,
                         int shift);
  LogicVRegister sqshrun2(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src,
                          int shift);
  LogicVRegister sqrshrun(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src,
                          int shift);
  LogicVRegister sqrshrun2(VectorFormat vform,
                           LogicVRegister dst,
                           const LogicVRegister& src,
                           int shift);
  LogicVRegister sqrdmulh(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          bool round = true);
  LogicVRegister dot(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2,
                     bool is_src1_signed,
                     bool is_src2_signed);
  LogicVRegister sdot(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister udot(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister usdot(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister cdot(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& acc,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      int rot);
  LogicVRegister sqrdcmlah(VectorFormat vform,
                           LogicVRegister dst,
                           const LogicVRegister& srca,
                           const LogicVRegister& src1,
                           const LogicVRegister& src2,
                           int rot);
  LogicVRegister sqrdcmlah(VectorFormat vform,
                           LogicVRegister dst,
                           const LogicVRegister& srca,
                           const LogicVRegister& src1,
                           const LogicVRegister& src2,
                           int index,
                           int rot);
  LogicVRegister sqrdmlash(VectorFormat vform,
                           LogicVRegister dst,
                           const LogicVRegister& src1,
                           const LogicVRegister& src2,
                           bool round = true,
                           bool sub_op = false);
  LogicVRegister sqrdmlash_d(VectorFormat vform,
                             LogicVRegister dst,
                             const LogicVRegister& src1,
                             const LogicVRegister& src2,
                             bool round = true,
                             bool sub_op = false);
  LogicVRegister sqrdmlah(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          bool round = true);
  LogicVRegister sqrdmlsh(VectorFormat vform,
                          LogicVRegister dst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2,
                          bool round = true);
  LogicVRegister sqdmulh(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2);
  LogicVRegister matmul(VectorFormat vform_dst,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2,
                        bool src1_signed,
                        bool src2_signed);
  template <typename T>
  LogicVRegister fmatmul(VectorFormat vform,
                         LogicVRegister srcdst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2);
  LogicVRegister fmatmul(VectorFormat vform,
                         LogicVRegister srcdst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2);

  template <unsigned N>
  static void SHARotateEltsLeftOne(uint64_t (&x)[N]) {
    VIXL_STATIC_ASSERT(N == 4);
    uint64_t temp = x[3];
    x[3] = x[2];
    x[2] = x[1];
    x[1] = x[0];
    x[0] = temp;
  }

  template <uint32_t mode>
  LogicVRegister sha1(LogicVRegister srcdst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2) {
    uint64_t y = src1.Uint(kFormat4S, 0);
    uint64_t sd[4] = {};
    srcdst.UintArray(kFormat4S, sd);

    for (unsigned i = 0; i < ArrayLength(sd); i++) {
      uint64_t t = CryptoOp<mode>(sd[1], sd[2], sd[3]);

      y += RotateLeft(sd[0], 5, kSRegSize) + t;
      y += src2.Uint(kFormat4S, i);

      sd[1] = RotateLeft(sd[1], 30, kSRegSize);

      // y:sd = ROL(y:sd, 32)
      SHARotateEltsLeftOne(sd);
      std::swap(sd[0], y);
    }

    srcdst.SetUintArray(kFormat4S, sd);
    return srcdst;
  }

  LogicVRegister sha2h(LogicVRegister srcdst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       bool part1);
  LogicVRegister sha2su0(LogicVRegister srcdst, const LogicVRegister& src1);
  LogicVRegister sha2su1(LogicVRegister srcdst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2);
  LogicVRegister sha512h(LogicVRegister srcdst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2);
  LogicVRegister sha512h2(LogicVRegister srcdst,
                          const LogicVRegister& src1,
                          const LogicVRegister& src2);
  LogicVRegister sha512su0(LogicVRegister srcdst, const LogicVRegister& src1);
  LogicVRegister sha512su1(LogicVRegister srcdst,
                           const LogicVRegister& src1,
                           const LogicVRegister& src2);


  LogicVRegister aes(LogicVRegister srcdst,
                     const LogicVRegister& src1,
                     bool decrypt);
  LogicVRegister aesmix(LogicVRegister srcdst,
                        const LogicVRegister& src1,
                        bool inverse);

  LogicVRegister sm3partw1(LogicVRegister dst,
                           const LogicVRegister& src1,
                           const LogicVRegister& src2);
  LogicVRegister sm3partw2(LogicVRegister dst,
                           const LogicVRegister& src1,
                           const LogicVRegister& src2);
  LogicVRegister sm3ss1(LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2,
                        const LogicVRegister& src3);
  LogicVRegister sm3tt1(LogicVRegister srcdst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2,
                        int index,
                        bool is_a);
  LogicVRegister sm3tt2(LogicVRegister srcdst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2,
                        int index,
                        bool is_a);

  LogicVRegister sm4(LogicVRegister dst,
                     const LogicVRegister& src1,
                     const LogicVRegister& src2,
                     bool is_key);

#define NEON_3VREG_LOGIC_LIST(V) \
  V(addhn)                       \
  V(addhn2)                      \
  V(raddhn)                      \
  V(raddhn2)                     \
  V(subhn)                       \
  V(subhn2)                      \
  V(rsubhn)                      \
  V(rsubhn2)                     \
  V(pmull)                       \
  V(pmull2)                      \
  V(sabal)                       \
  V(sabal2)                      \
  V(uabal)                       \
  V(uabal2)                      \
  V(sabdl)                       \
  V(sabdl2)                      \
  V(uabdl)                       \
  V(uabdl2)                      \
  V(smull2)                      \
  V(umull2)                      \
  V(smlal2)                      \
  V(umlal2)                      \
  V(smlsl2)                      \
  V(umlsl2)                      \
  V(sqdmlal2)                    \
  V(sqdmlsl2)                    \
  V(sqdmull2)

#define DEFINE_LOGIC_FUNC(FXN)                   \
  LogicVRegister FXN(VectorFormat vform,         \
                     LogicVRegister dst,         \
                     const LogicVRegister& src1, \
                     const LogicVRegister& src2);
  NEON_3VREG_LOGIC_LIST(DEFINE_LOGIC_FUNC)
#undef DEFINE_LOGIC_FUNC

#define NEON_MULL_LIST(V) \
  V(smull)                \
  V(umull)                \
  V(smlal)                \
  V(umlal)                \
  V(smlsl)                \
  V(umlsl)                \
  V(sqdmlal)              \
  V(sqdmlsl)              \
  V(sqdmull)

#define DECLARE_NEON_MULL_OP(FN)                \
  LogicVRegister FN(VectorFormat vform,         \
                    LogicVRegister dst,         \
                    const LogicVRegister& src1, \
                    const LogicVRegister& src2, \
                    bool is_2 = false);
  NEON_MULL_LIST(DECLARE_NEON_MULL_OP)
#undef DECLARE_NEON_MULL_OP

#define NEON_FP3SAME_LIST(V) \
  V(fadd, FPAdd, false)      \
  V(fsub, FPSub, true)       \
  V(fmul, FPMul, true)       \
  V(fmulx, FPMulx, true)     \
  V(fdiv, FPDiv, true)       \
  V(fmax, FPMax, false)      \
  V(fmin, FPMin, false)      \
  V(fmaxnm, FPMaxNM, false)  \
  V(fminnm, FPMinNM, false)

#define DECLARE_NEON_FP_VECTOR_OP(FN, OP, PROCNAN) \
  template <typename T>                            \
  LogicVRegister FN(VectorFormat vform,            \
                    LogicVRegister dst,            \
                    const LogicVRegister& src1,    \
                    const LogicVRegister& src2);   \
  LogicVRegister FN(VectorFormat vform,            \
                    LogicVRegister dst,            \
                    const LogicVRegister& src1,    \
                    const LogicVRegister& src2);
  NEON_FP3SAME_LIST(DECLARE_NEON_FP_VECTOR_OP)
#undef DECLARE_NEON_FP_VECTOR_OP

#define NEON_FPPAIRWISE_LIST(V) \
  V(faddp, fadd, FPAdd)         \
  V(fmaxp, fmax, FPMax)         \
  V(fmaxnmp, fmaxnm, FPMaxNM)   \
  V(fminp, fmin, FPMin)         \
  V(fminnmp, fminnm, FPMinNM)

#define DECLARE_NEON_FP_PAIR_OP(FNP, FN, OP)      \
  LogicVRegister FNP(VectorFormat vform,          \
                     LogicVRegister dst,          \
                     const LogicVRegister& src1,  \
                     const LogicVRegister& src2); \
  LogicVRegister FNP(VectorFormat vform,          \
                     LogicVRegister dst,          \
                     const LogicVRegister& src);
  NEON_FPPAIRWISE_LIST(DECLARE_NEON_FP_PAIR_OP)
#undef DECLARE_NEON_FP_PAIR_OP

  enum FrintMode {
    kFrintToInteger = 0,
    kFrintToInt32 = 32,
    kFrintToInt64 = 64
  };

  template <typename T>
  LogicVRegister frecps(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister frecps(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  template <typename T>
  LogicVRegister frsqrts(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2);
  LogicVRegister frsqrts(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2);
  template <typename T>
  LogicVRegister fmla(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& srca,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister fmla(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& srca,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  template <typename T>
  LogicVRegister fmls(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& srca,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister fmls(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& srca,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister fnmul(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);

  LogicVRegister fmlal(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister fmlal2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister fmlsl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2);
  LogicVRegister fmlsl2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);

  template <typename T>
  LogicVRegister fcmp(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      Condition cond);
  LogicVRegister fcmp(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      Condition cond);
  LogicVRegister fabscmp(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src1,
                         const LogicVRegister& src2,
                         Condition cond);
  LogicVRegister fcmp_zero(VectorFormat vform,
                           LogicVRegister dst,
                           const LogicVRegister& src,
                           Condition cond);

  template <typename T>
  LogicVRegister fneg(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  LogicVRegister fneg(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src);
  template <typename T>
  LogicVRegister frecpx(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister frecpx(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister ftsmul(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister ftssel(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister ftmad(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src1,
                       const LogicVRegister& src2,
                       unsigned index);
  LogicVRegister fexpa(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister flogb(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  template <typename T>
  LogicVRegister fscale(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  LogicVRegister fscale(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src1,
                        const LogicVRegister& src2);
  template <typename T>
  LogicVRegister fabs_(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister fabs_(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister fabd(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2);
  LogicVRegister frint(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       FPRounding rounding_mode,
                       bool inexact_exception = false,
                       FrintMode frint_mode = kFrintToInteger);
  LogicVRegister fcvt(VectorFormat dst_vform,
                      VectorFormat src_vform,
                      LogicVRegister dst,
                      const LogicPRegister& pg,
                      const LogicVRegister& src);
  LogicVRegister fcvts(VectorFormat vform,
                       unsigned dst_data_size_in_bits,
                       unsigned src_data_size_in_bits,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src,
                       FPRounding round,
                       int fbits = 0);
  LogicVRegister fcvts(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       FPRounding rounding_mode,
                       int fbits = 0);
  LogicVRegister fcvtu(VectorFormat vform,
                       unsigned dst_data_size_in_bits,
                       unsigned src_data_size_in_bits,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src,
                       FPRounding round,
                       int fbits = 0);
  LogicVRegister fcvtu(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src,
                       FPRounding rounding_mode,
                       int fbits = 0);
  LogicVRegister fcvtl(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister fcvtl2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister fcvtn(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister fcvtn2(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister fcvtxn(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);
  LogicVRegister fcvtxn2(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src);
  LogicVRegister fsqrt(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister frsqrte(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src);
  LogicVRegister frecpe(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src,
                        FPRounding rounding);
  LogicVRegister ursqrte(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src);
  LogicVRegister urecpe(VectorFormat vform,
                        LogicVRegister dst,
                        const LogicVRegister& src);

  LogicPRegister pfalse(LogicPRegister dst);
  LogicPRegister pfirst(LogicPRegister dst,
                        const LogicPRegister& pg,
                        const LogicPRegister& src);
  LogicPRegister ptrue(VectorFormat vform, LogicPRegister dst, int pattern);
  LogicPRegister pnext(VectorFormat vform,
                       LogicPRegister dst,
                       const LogicPRegister& pg,
                       const LogicPRegister& src);

  LogicVRegister asrd(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      int shift);

  LogicVRegister andv(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicPRegister& pg,
                      const LogicVRegister& src);
  LogicVRegister eorv(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicPRegister& pg,
                      const LogicVRegister& src);
  LogicVRegister orv(VectorFormat vform,
                     LogicVRegister dst,
                     const LogicPRegister& pg,
                     const LogicVRegister& src);
  LogicVRegister saddv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);
  LogicVRegister sminv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);
  LogicVRegister smaxv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);
  LogicVRegister uaddv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);
  LogicVRegister uminv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);
  LogicVRegister umaxv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicPRegister& pg,
                       const LogicVRegister& src);

  LogicVRegister interleave_top_bottom(VectorFormat vform,
                                       LogicVRegister dst,
                                       const LogicVRegister& src);

  template <typename T>
  struct TFPPairOp {
    typedef T (Simulator::*type)(T a, T b);
  };

  template <typename T>
  LogicVRegister FPPairedAcrossHelper(VectorFormat vform,
                                      LogicVRegister dst,
                                      const LogicVRegister& src,
                                      typename TFPPairOp<T>::type fn,
                                      uint64_t inactive_value);

  LogicVRegister FPPairedAcrossHelper(
      VectorFormat vform,
      LogicVRegister dst,
      const LogicVRegister& src,
      typename TFPPairOp<vixl::internal::SimFloat16>::type fn16,
      typename TFPPairOp<float>::type fn32,
      typename TFPPairOp<double>::type fn64,
      uint64_t inactive_value);

  LogicVRegister fminv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister fmaxv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);
  LogicVRegister fminnmv(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src);
  LogicVRegister fmaxnmv(VectorFormat vform,
                         LogicVRegister dst,
                         const LogicVRegister& src);
  LogicVRegister faddv(VectorFormat vform,
                       LogicVRegister dst,
                       const LogicVRegister& src);

  static const uint32_t CRC32_POLY = 0x04C11DB7;
  static const uint32_t CRC32C_POLY = 0x1EDC6F41;
  uint32_t Poly32Mod2(unsigned n, uint64_t data, uint32_t poly);
  template <typename T>
  uint32_t Crc32Checksum(uint32_t acc, T val, uint32_t poly);
  uint32_t Crc32Checksum(uint32_t acc, uint64_t val, uint32_t poly);

  bool SysOp_W(int op, int64_t val);

  template <typename T>
  T FPRecipSqrtEstimate(T op);
  template <typename T>
  T FPRecipEstimate(T op, FPRounding rounding);
  template <typename T, typename R>
  R FPToFixed(T op, int fbits, bool is_signed, FPRounding rounding);

  void FPCompare(double val0, double val1, FPTrapFlags trap);
  double FPRoundInt(double value, FPRounding round_mode);
  double FPRoundInt(double value, FPRounding round_mode, FrintMode frint_mode);
  double FPRoundIntCommon(double value, FPRounding round_mode);
  double recip_sqrt_estimate(double a);
  double recip_estimate(double a);
  double FPRecipSqrtEstimate(double a);
  double FPRecipEstimate(double a);
  double FixedToDouble(int64_t src, int fbits, FPRounding round_mode);
  double UFixedToDouble(uint64_t src, int fbits, FPRounding round_mode);
  float FixedToFloat(int64_t src, int fbits, FPRounding round_mode);
  float UFixedToFloat(uint64_t src, int fbits, FPRounding round_mode);
  ::vixl::internal::SimFloat16 FixedToFloat16(int64_t src,
                                              int fbits,
                                              FPRounding round_mode);
  ::vixl::internal::SimFloat16 UFixedToFloat16(uint64_t src,
                                               int fbits,
                                               FPRounding round_mode);
  int16_t FPToInt16(double value, FPRounding rmode);
  int32_t FPToInt32(double value, FPRounding rmode);
  int64_t FPToInt64(double value, FPRounding rmode);
  uint16_t FPToUInt16(double value, FPRounding rmode);
  uint32_t FPToUInt32(double value, FPRounding rmode);
  uint64_t FPToUInt64(double value, FPRounding rmode);
  int32_t FPToFixedJS(double value);

  template <typename T>
  T FPAdd(T op1, T op2);

  template <typename T>
  T FPNeg(T op);

  template <typename T>
  T FPDiv(T op1, T op2);

  template <typename T>
  T FPMax(T a, T b);

  template <typename T>
  T FPMaxNM(T a, T b);

  template <typename T>
  T FPMin(T a, T b);

  template <typename T>
  T FPMinNM(T a, T b);

  template <typename T>
  T FPMulNaNs(T op1, T op2);

  template <typename T>
  T FPMul(T op1, T op2);

  template <typename T>
  T FPMulx(T op1, T op2);

  template <typename T>
  T FPMulAdd(T a, T op1, T op2);

  template <typename T>
  T FPSqrt(T op);

  template <typename T>
  T FPSub(T op1, T op2);

  template <typename T>
  T FPRecipStepFused(T op1, T op2);

  template <typename T>
  T FPRSqrtStepFused(T op1, T op2);

  // This doesn't do anything at the moment. We'll need it if we want support
  // for cumulative exception bits or floating-point exceptions.
  void FPProcessException() {}

  bool FPProcessNaNs(const Instruction* instr);

  // Pseudo Printf instruction
  void DoPrintf(const Instruction* instr);

  // Pseudo-instructions to configure CPU features dynamically.
  void DoConfigureCPUFeatures(const Instruction* instr);

  void DoSaveCPUFeatures(const Instruction* instr);
  void DoRestoreCPUFeatures(const Instruction* instr);

  // General arithmetic helpers ----------------------------

  // Add `delta` to the accumulator (`acc`), optionally saturate, then zero- or
  // sign-extend. Initial `acc` bits outside `n` are ignored, but the delta must
  // be a valid int<n>_t.
  uint64_t IncDecN(uint64_t acc,
                   int64_t delta,
                   unsigned n,
                   bool is_saturating = false,
                   bool is_signed = false);

  // SVE helpers -------------------------------------------
  LogicVRegister SVEBitwiseLogicalUnpredicatedHelper(LogicalOp op,
                                                     VectorFormat vform,
                                                     LogicVRegister zd,
                                                     const LogicVRegister& zn,
                                                     const LogicVRegister& zm);

  LogicPRegister SVEPredicateLogicalHelper(SVEPredicateLogicalOp op,
                                           LogicPRegister Pd,
                                           const LogicPRegister& pn,
                                           const LogicPRegister& pm);

  LogicVRegister SVEBitwiseImmHelper(SVEBitwiseLogicalWithImm_UnpredicatedOp op,
                                     VectorFormat vform,
                                     LogicVRegister zd,
                                     uint64_t imm);
  enum UnpackType { kHiHalf, kLoHalf };
  enum ExtendType { kSignedExtend, kUnsignedExtend };
  LogicVRegister unpk(VectorFormat vform,
                      LogicVRegister zd,
                      const LogicVRegister& zn,
                      UnpackType unpack_type,
                      ExtendType extend_type);

  LogicPRegister SVEIntCompareVectorsHelper(Condition cc,
                                            VectorFormat vform,
                                            LogicPRegister dst,
                                            const LogicPRegister& mask,
                                            const LogicVRegister& src1,
                                            const LogicVRegister& src2,
                                            bool is_wide_elements = false,
                                            FlagsUpdate flags = SetFlags);

  void SVEGatherLoadScalarPlusVectorHelper(const Instruction* instr,
                                           VectorFormat vform,
                                           SVEOffsetModifier mod);

  // Store each active zt<i>[lane] to `addr.GetElementAddress(lane, ...)`.
  //
  // `zt_code` specifies the code of the first register (zt). Each additional
  // register (up to `reg_count`) is `(zt_code + i) % 32`.
  //
  // This helper calls LogZWrite in the proper way, according to `addr`.
  void SVEStructuredStoreHelper(VectorFormat vform,
                                const LogicPRegister& pg,
                                unsigned zt_code,
                                const LogicSVEAddressVector& addr);
  // Load each active zt<i>[lane] from `addr.GetElementAddress(lane, ...)`.
  // Returns false if a load failed.
  bool SVEStructuredLoadHelper(VectorFormat vform,
                               const LogicPRegister& pg,
                               unsigned zt_code,
                               const LogicSVEAddressVector& addr,
                               bool is_signed = false);

  enum SVEFaultTolerantLoadType {
    // - Elements active in both FFR and pg are accessed as usual. If the access
    //   fails, the corresponding lane and all subsequent lanes are filled with
    //   an unpredictable value, and made inactive in FFR.
    //
    // - Elements active in FFR but not pg are set to zero.
    //
    // - Elements that are not active in FFR are filled with an unpredictable
    //   value, regardless of pg.
    kSVENonFaultLoad,

    // If type == kSVEFirstFaultLoad, the behaviour is the same, except that the
    // first active element is always accessed, regardless of FFR, and will
    // generate a real fault if it is inaccessible. If the lane is not active in
    // FFR, the actual value loaded into the result is still unpredictable.
    kSVEFirstFaultLoad
  };

  // Load with first-faulting or non-faulting load semantics, respecting and
  // updating FFR.
  void SVEFaultTolerantLoadHelper(VectorFormat vform,
                                  const LogicPRegister& pg,
                                  unsigned zt_code,
                                  const LogicSVEAddressVector& addr,
                                  SVEFaultTolerantLoadType type,
                                  bool is_signed);

  LogicVRegister SVEBitwiseShiftHelper(Shift shift_op,
                                       VectorFormat vform,
                                       LogicVRegister dst,
                                       const LogicVRegister& src1,
                                       const LogicVRegister& src2,
                                       bool is_wide_elements);

  // Pack all even- or odd-numbered elements of source vector side by side and
  // place in elements of lower half the destination vector, and leave the upper
  // half all zero.
  //    [...| H | G | F | E | D | C | B | A ]
  // => [...................| G | E | C | A ]
  LogicVRegister pack_even_elements(VectorFormat vform,
                                    LogicVRegister dst,
                                    const LogicVRegister& src);

  //    [...| H | G | F | E | D | C | B | A ]
  // => [...................| H | F | D | B ]
  LogicVRegister pack_odd_elements(VectorFormat vform,
                                   LogicVRegister dst,
                                   const LogicVRegister& src);

  LogicVRegister adcl(VectorFormat vform,
                      LogicVRegister dst,
                      const LogicVRegister& src1,
                      const LogicVRegister& src2,
                      bool top);

  template <typename T>
  LogicVRegister FTMaddHelper(VectorFormat vform,
                              LogicVRegister dst,
                              const LogicVRegister& src1,
                              const LogicVRegister& src2,
                              uint64_t coeff_pos,
                              uint64_t coeff_neg);

  // Return the first or last active lane, or -1 if none are active.
  int GetFirstActive(VectorFormat vform, const LogicPRegister& pg) const;
  int GetLastActive(VectorFormat vform, const LogicPRegister& pg) const;

  int CountActiveLanes(VectorFormat vform, const LogicPRegister& pg) const;

  // Count active and true lanes in `pn`.
  int CountActiveAndTrueLanes(VectorFormat vform,
                              const LogicPRegister& pg,
                              const LogicPRegister& pn) const;

  // Count the number of lanes referred to by `pattern`, given the vector
  // length. If `pattern` is not a recognised SVEPredicateConstraint, this
  // returns zero.
  int GetPredicateConstraintLaneCount(VectorFormat vform, int pattern) const;

  // Simulate a runtime call.
  void DoRuntimeCall(const Instruction* instr);

  // Processor state ---------------------------------------

  // Simulated monitors for exclusive access instructions.
  SimExclusiveLocalMonitor local_monitor_;
  SimExclusiveGlobalMonitor global_monitor_;

  // Output stream.
  FILE* stream_;
  PrintDisassembler* print_disasm_;

  // General purpose registers. Register 31 is the stack pointer.
  SimRegister registers_[kNumberOfRegisters];

  // Vector registers
  SimVRegister vregisters_[kNumberOfVRegisters];

  // SVE predicate registers.
  SimPRegister pregisters_[kNumberOfPRegisters];

  // SVE first-fault register.
  SimFFRRegister ffr_register_;

  // A pseudo SVE predicate register with all bits set to true.
  SimPRegister pregister_all_true_;

  // Program Status Register.
  // bits[31, 27]: Condition flags N, Z, C, and V.
  //               (Negative, Zero, Carry, Overflow)
  SimSystemRegister nzcv_;

  // Floating-Point Control Register
  SimSystemRegister fpcr_;

  // Only a subset of FPCR features are supported by the simulator. This helper
  // checks that the FPCR settings are supported.
  //
  // This is checked when floating-point instructions are executed, not when
  // FPCR is set. This allows generated code to modify FPCR for external
  // functions, or to save and restore it when entering and leaving generated
  // code.
  void AssertSupportedFPCR() {
    // No flush-to-zero support.
    VIXL_ASSERT(ReadFpcr().GetFZ() == 0);
    // Ties-to-even rounding only.
    VIXL_ASSERT(ReadFpcr().GetRMode() == FPTieEven);
    // No alternative half-precision support.
    VIXL_ASSERT(ReadFpcr().GetAHP() == 0);
  }

  static int CalcNFlag(uint64_t result, unsigned reg_size) {
    return (result >> (reg_size - 1)) & 1;
  }

  static int CalcZFlag(uint64_t result) { return (result == 0) ? 1 : 0; }

  static const uint32_t kConditionFlagsMask = 0xf0000000;

  Memory memory_;

  static const size_t kDefaultStackGuardStartSize = 0;
  static const size_t kDefaultStackGuardEndSize = 4 * 1024;
  static const size_t kDefaultStackUsableSize = 8 * 1024;

  Decoder* decoder_;
  // Indicates if the pc has been modified by the instruction and should not be
  // automatically incremented.
  bool pc_modified_;
  const Instruction* pc_;

  // Pointer to the last simulated instruction, used for checking the validity
  // of the current instruction with the previous instruction, such as movprfx.
  Instruction const* last_instr_;

  // Branch type register, used for branch target identification.
  BType btype_;

  // Next value of branch type register after the current instruction has been
  // decoded.
  BType next_btype_;

  // Global flag for enabling guarded pages.
  // TODO: implement guarding at page granularity, rather than globally.
  bool guard_pages_;

  static const char* xreg_names[];
  static const char* wreg_names[];
  static const char* breg_names[];
  static const char* hreg_names[];
  static const char* sreg_names[];
  static const char* dreg_names[];
  static const char* vreg_names[];
  static const char* zreg_names[];
  static const char* preg_names[];

 private:
  using FormToVisitorFnMap =
      std::unordered_map<uint32_t,
                         std::function<void(Simulator*, const Instruction*)>>;
  static const FormToVisitorFnMap* GetFormToVisitorFnMap();

  uint32_t form_hash_;

  static const PACKey kPACKeyIA;
  static const PACKey kPACKeyIB;
  static const PACKey kPACKeyDA;
  static const PACKey kPACKeyDB;
  static const PACKey kPACKeyGA;

  bool CanReadMemory(uintptr_t address, size_t size);

#ifndef _WIN32
  // CanReadMemory needs placeholder file descriptors, so we use a pipe. We can
  // save some system call overhead by opening them on construction, rather than
  // on every call to CanReadMemory.
  int placeholder_pipe_fd_[2];
#endif

  template <typename T>
  static T FPDefaultNaN();

  // Standard NaN processing.
  template <typename T>
  T FPProcessNaN(T op) {
    VIXL_ASSERT(IsNaN(op));
    if (IsSignallingNaN(op)) {
      FPProcessException();
    }
    return (ReadDN() == kUseDefaultNaN) ? FPDefaultNaN<T>() : ToQuietNaN(op);
  }

  template <typename T>
  T FPProcessNaNs(T op1, T op2) {
    if (IsSignallingNaN(op1)) {
      return FPProcessNaN(op1);
    } else if (IsSignallingNaN(op2)) {
      return FPProcessNaN(op2);
    } else if (IsNaN(op1)) {
      VIXL_ASSERT(IsQuietNaN(op1));
      return FPProcessNaN(op1);
    } else if (IsNaN(op2)) {
      VIXL_ASSERT(IsQuietNaN(op2));
      return FPProcessNaN(op2);
    } else {
      return 0.0;
    }
  }

  template <typename T>
  T FPProcessNaNs3(T op1, T op2, T op3) {
    if (IsSignallingNaN(op1)) {
      return FPProcessNaN(op1);
    } else if (IsSignallingNaN(op2)) {
      return FPProcessNaN(op2);
    } else if (IsSignallingNaN(op3)) {
      return FPProcessNaN(op3);
    } else if (IsNaN(op1)) {
      VIXL_ASSERT(IsQuietNaN(op1));
      return FPProcessNaN(op1);
    } else if (IsNaN(op2)) {
      VIXL_ASSERT(IsQuietNaN(op2));
      return FPProcessNaN(op2);
    } else if (IsNaN(op3)) {
      VIXL_ASSERT(IsQuietNaN(op3));
      return FPProcessNaN(op3);
    } else {
      return 0.0;
    }
  }

  // Construct a SimVRegister from a SimPRegister, where each byte-sized lane of
  // the destination is set to all true (0xff) when the corresponding
  // predicate flag is set, and false (0x00) otherwise.
  SimVRegister ExpandToSimVRegister(const SimPRegister& preg);

  // Set each predicate flag in pd where the corresponding assigned-sized lane
  // in vreg is non-zero. Clear the flag, otherwise. This is almost the opposite
  // operation to ExpandToSimVRegister(), except that any non-zero lane is
  // interpreted as true.
  void ExtractFromSimVRegister(VectorFormat vform,
                               SimPRegister& pd,  // NOLINT(runtime/references)
                               SimVRegister vreg);

  bool coloured_trace_;

  // A set of TraceParameters flags.
  int trace_parameters_;

  // Indicates whether the exclusive-access warning has been printed.
  bool print_exclusive_access_warning_;
  void PrintExclusiveAccessWarning();

  CPUFeaturesAuditor cpu_features_auditor_;
  std::vector<CPUFeatures> saved_cpu_features_;

  // linear_congruential_engine, used to simulate randomness with repeatable
  // behaviour (so that tests are deterministic). This is used to simulate RNDR
  // and RNDRRS, as well as to simulate a source of entropy for architecturally
  // undefined behaviour.
  std::linear_congruential_engine<uint64_t,
                                  0x5DEECE66D,
                                  0xB,
                                  static_cast<uint64_t>(1) << 48>
      rand_gen_;

  // A configurable size of SVE vector registers.
  unsigned vector_length_;

  // DC ZVA enable (= 0) status and block size.
  unsigned dczid_ = (0 << 4) | 4;  // 2^4 words => 64-byte block size.

  // Representation of memory attributes such as MTE tagging and BTI page
  // protection in addition to branch interceptions.
  MetaDataDepot meta_data_;

  // True if the debugger is enabled and might get entered.
  bool debugger_enabled_;

  // Debugger for the simulator.
  std::unique_ptr<Debugger> debugger_;

  // The Guarded Control Stack is represented using a vector, where the more
  // recently stored addresses are at higher-numbered indices.
  using GuardedControlStack = std::vector<uint64_t>;

  // The GCSManager handles the synchronisation of GCS across multiple
  // Simulator instances. Each Simulator has its own stack, but all share
  // a GCSManager instance. This allows exchanging stacks between Simulators
  // in a threaded application.
  class GCSManager {
   public:
    // Allocate a new Guarded Control Stack and add it to the vector of stacks.
    uint64_t AllocateStack() {
      const std::lock_guard<std::mutex> lock(stacks_mtx_);

      GuardedControlStack* new_stack = new GuardedControlStack;
      uint64_t result;

      // Put the new stack into the first available slot.
      for (result = 0; result < stacks_.size(); result++) {
        if (stacks_[result] == nullptr) {
          stacks_[result] = new_stack;
          break;
        }
      }

      // If there were no slots, create a new one.
      if (result == stacks_.size()) {
        stacks_.push_back(new_stack);
      }

      // Shift the index to look like a stack pointer aligned to a page.
      result <<= kPageSizeLog2;

      // Push the tagged index onto the new stack as a seal.
      new_stack->push_back(result + 1);
      return result;
    }

    // Free a Guarded Control Stack and set the stacks_ slot to null.
    void FreeStack(uint64_t gcs) {
      const std::lock_guard<std::mutex> lock(stacks_mtx_);
      uint64_t gcs_index = GetGCSIndex(gcs);
      GuardedControlStack* gcsptr = stacks_[gcs_index];
      if (gcsptr == nullptr) {
        VIXL_ABORT_WITH_MSG("Tried to free unallocated GCS ");
      } else {
        delete gcsptr;
        stacks_[gcs_index] = nullptr;
      }
    }

    // Get a pointer to the GCS vector using a GCS id.
    GuardedControlStack* GetGCSPtr(uint64_t gcs) const {
      return stacks_[GetGCSIndex(gcs)];
    }

   private:
    uint64_t GetGCSIndex(uint64_t gcs) const { return gcs >> 12; }

    std::vector<GuardedControlStack*> stacks_;
    std::mutex stacks_mtx_;
  };

  // A GCS id indicating no GCS has been allocated.
  static const uint64_t kGCSNoStack = kPageSize - 1;
  uint64_t gcs_;
  bool gcs_enabled_;

 public:
  GCSManager& GetGCSManager() {
    static GCSManager manager;
    return manager;
  }

  void EnableGCSCheck() { gcs_enabled_ = true; }
  void DisableGCSCheck() { gcs_enabled_ = false; }
  bool IsGCSCheckEnabled() const { return gcs_enabled_; }

 private:
  bool IsAllocatedGCS(uint64_t gcs) const { return gcs != kGCSNoStack; }
  void ResetGCSState() {
    GCSManager& m = GetGCSManager();
    if (IsAllocatedGCS(gcs_)) {
      m.FreeStack(gcs_);
    }
    ActivateGCS(m.AllocateStack());
    GCSPop();  // Remove seal.
  }

  GuardedControlStack* GetGCSPtr(uint64_t gcs) {
    GCSManager& m = GetGCSManager();
    GuardedControlStack* result = m.GetGCSPtr(gcs);
    return result;
  }
  GuardedControlStack* GetActiveGCSPtr() { return GetGCSPtr(gcs_); }

  uint64_t ActivateGCS(uint64_t gcs) {
    uint64_t outgoing_gcs = gcs_;
    gcs_ = gcs;
    return outgoing_gcs;
  }

  void GCSPush(uint64_t addr) {
    GetActiveGCSPtr()->push_back(addr);
    size_t entry = GetActiveGCSPtr()->size() - 1;
    LogGCS(/* is_push = */ true, addr, entry);
  }

  uint64_t GCSPop() {
    GuardedControlStack* gcs = GetActiveGCSPtr();
    if (gcs->empty()) {
      return 0;
    }
    uint64_t return_addr = gcs->back();
    size_t entry = gcs->size() - 1;
    gcs->pop_back();
    LogGCS(/* is_push = */ false, return_addr, entry);
    return return_addr;
  }

  uint64_t GCSPeek() {
    GuardedControlStack* gcs = GetActiveGCSPtr();
    if (gcs->empty()) {
      return 0;
    }
    uint64_t return_addr = gcs->back();
    return return_addr;
  }

  void ReportGCSFailure(const char* msg) {
    if (IsGCSCheckEnabled()) {
      GuardedControlStack* gcs = GetActiveGCSPtr();
      printf("%s", msg);
      if (gcs == nullptr) {
        printf("GCS pointer is null\n");
      } else {
        printf("GCS records, most recent first:\n");
        int most_recent_index = static_cast<int>(gcs->size()) - 1;
        for (int i = 0; i < 8; i++) {
          if (!gcs->empty()) {
            uint64_t entry = gcs->back();
            gcs->pop_back();
            int index = most_recent_index - i;
            printf(" gcs%" PRIu64 "[%d]: 0x%016" PRIx64 "\n",
                   gcs_,
                   index,
                   entry);
          }
        }
        printf("End of GCS records.\n");
      }
      VIXL_ABORT_WITH_MSG("GCS failed ");
    }
  }
};

#if defined(VIXL_HAS_SIMULATED_RUNTIME_CALL_SUPPORT) && __cplusplus < 201402L
// Base case of the recursive template used to emulate C++14
// `std::index_sequence`.
template <size_t... I>
struct Simulator::emulated_make_index_sequence_helper<0, I...>
    : Simulator::emulated_index_sequence<I...> {};
#endif

template <typename R, typename... P>
void MetaDataDepot::BranchInterception<R, P...>::operator()(
    Simulator* simulator) const {
  if (callback_ == nullptr) {
    Simulator::RuntimeCallStructHelper<R, P...>::
        Wrapper(simulator, reinterpret_cast<uint64_t>(function_));
  } else {
    callback_(reinterpret_cast<uint64_t>(function_));
  }
}

}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_INCLUDE_SIMULATOR_AARCH64

#endif  // VIXL_AARCH64_SIMULATOR_AARCH64_H_
