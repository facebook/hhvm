/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/dynamic/detail/SmallBuffer.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

#include <folly/io/IOBuf.h>

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>

using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::dynamic::detail;

// Test types of various sizes
struct Tiny {
  uint64_t value;
};
static_assert(sizeof(Tiny) == 8);

struct Small {
  uint64_t a, b;
};
static_assert(sizeof(Small) == 16);

struct Medium {
  uint64_t a, b, c, d;
};
static_assert(sizeof(Medium) == 32);

struct Large {
  uint64_t data[15]; // 120 bytes - max for new TypeErasedBox
};
static_assert(sizeof(Large) == 120);

struct TooLarge {
  uint64_t data[16]; // 128 bytes - won't fit in TypeErasedBox
};
static_assert(sizeof(TooLarge) == 128);

// Non-trivial type (like unique_ptr<IOBuf>)
using BenchBytesPtr = std::unique_ptr<folly::IOBuf>;

template <typename T>
void printLayout(const char* name) {
  std::cout << "  " << std::left << std::setw(45) << name << " = " << sizeof(T)
            << " bytes" << std::endl;
}

void printSeparator(const char* title) {
  std::cout << "\n=== " << title << " ===" << std::endl;
}

int main() {
  printSeparator("TYPE SIZES");
  printLayout<Tiny>("Tiny (1x uint64_t)");
  printLayout<Small>("Small (2x uint64_t)");
  printLayout<Medium>("Medium (4x uint64_t)");
  printLayout<Large>("Large (15x uint64_t) - max for TypeErasedBox");
  printLayout<TooLarge>("TooLarge (16x uint64_t) - won't fit");
  printLayout<BenchBytesPtr>("BenchBytesPtr (unique_ptr<IOBuf>)");

  printSeparator("TypeErasedBox LAYOUT (Zero-Cost Wrapper)");
  std::cout << "  Total size:           " << sizeof(TypeErasedBox) << " bytes"
            << std::endl;
  std::cout << "  Inline capacity:      " << TypeErasedBox::kInlineCapacity
            << " bytes" << std::endl;
  std::cout << "  Inline alignment:     " << TypeErasedBox::kInlineAlign
            << " bytes" << std::endl;
  std::cout << "  Overhead:             "
            << sizeof(TypeErasedBox) - TypeErasedBox::kInlineCapacity
            << " bytes" << std::endl;
  std::cout << "  Fits Tiny:            "
            << (TypeErasedBox::fits_inline<Tiny>() ? "YES" : "NO") << std::endl;
  std::cout << "  Fits Small:           "
            << (TypeErasedBox::fits_inline<Small>() ? "YES" : "NO")
            << std::endl;
  std::cout << "  Fits Medium:          "
            << (TypeErasedBox::fits_inline<Medium>() ? "YES" : "NO")
            << std::endl;
  std::cout << "  Fits Large (120B):    "
            << (TypeErasedBox::fits_inline<Large>() ? "YES" : "NO")
            << std::endl;
  std::cout << "  Fits BenchBytesPtr:        "
            << (TypeErasedBox::fits_inline<BenchBytesPtr>() ? "YES" : "NO")
            << std::endl;

  printSeparator("SmallBuffer (Base Class) LAYOUT");

  using BaseBuffer = SmallBuffer<120, alignof(void*), true>;

  std::cout << "  SmallBuffer<120, 8, true> size: " << sizeof(BaseBuffer)
            << " bytes" << std::endl;
  std::cout << "  TypeErasedBox size:            " << sizeof(TypeErasedBox)
            << " bytes" << std::endl;

  printSeparator("ZERO-COST VERIFICATION");

#ifdef NDEBUG
  std::cout << "  Build: RELEASE (NDEBUG defined)" << std::endl;
  std::cout << "  TypeErasedBox debug fields: DISABLED" << std::endl;
  std::cout << "  sizeof(SmallBuffer<120,8,true>): " << sizeof(BaseBuffer)
            << std::endl;
  std::cout << "  sizeof(TypeErasedBox):           " << sizeof(TypeErasedBox)
            << std::endl;

  if (sizeof(TypeErasedBox) == sizeof(BaseBuffer)) {
    std::cout << "\n  ✓ ZERO-COST WRAPPER CONFIRMED!" << std::endl;
    std::cout
        << "    TypeErasedBox == SmallBuffer in size (no runtime overhead)"
        << std::endl;
  } else {
    std::cout << "\n  ✗ WARNING: TypeErasedBox has overhead in release build!"
              << std::endl;
  }
#else
  std::cout << "  Build: DEBUG (NDEBUG not defined)" << std::endl;
  std::cout << "  TypeErasedBox debug fields: ENABLED" << std::endl;
  std::cout << "  sizeof(SmallBuffer<120,8,true>): " << sizeof(BaseBuffer)
            << std::endl;
  std::cout << "  sizeof(TypeErasedBox):           " << sizeof(TypeErasedBox)
            << std::endl;
  std::cout << "  Debug overhead:                 "
            << sizeof(TypeErasedBox) - sizeof(BaseBuffer)
            << " bytes (type_ pointer only)" << std::endl;
#endif

  printSeparator("EFFICIENCY COMPARISON");
  std::cout << std::fixed << std::setprecision(1);
  std::cout << "\n  TypeErasedBox:              "
            << TypeErasedBox::kInlineCapacity << " / " << sizeof(TypeErasedBox)
            << " = "
            << (static_cast<double>(TypeErasedBox::kInlineCapacity) /
                sizeof(TypeErasedBox) * 100)
            << "% usable" << std::endl;
  std::cout << "  SmallBuffer<120, 8, true>: 120 / " << sizeof(BaseBuffer)
            << " = " << (120.0 / sizeof(BaseBuffer) * 100) << "% usable"
            << std::endl;

  printSeparator("FEATURE SUMMARY");
  std::cout << R"(
  TypeErasedBox is now a zero-cost wrapper over SmallBuffer<120, 8, true>:

  Feature                         Status
  ────────────────────────────────────────────────────────────────
  Inline capacity                 120 bytes (was 56, +114.3% improvement)
  Total size (release)            128 bytes (two L1 cache lines)
  Total size (debug)              136 bytes (+8 for type_ pointer)
  Move semantics                  YES (delegated to SmallBuffer)
  Copy semantics                  NO (move-only)
  Compile-time size enforcement   YES (static_assert)
  Debug type checking             YES (only in debug builds)
  Heap allocation fallback        NO (inline-only, use unique_ptr)
  Zero runtime overhead           YES (in release builds)
)" << std::endl;

  printSeparator("MIGRATION NOTES");
  std::cout << R"(
  API changes from old TypeErasedBox:

  - kInlineCapacity:  56 bytes → 120 bytes (64 more bytes!)
  - isInline():       REMOVED (always inline now)
  - Heap allocation:  REMOVED (use unique_ptr for large types)
  - reset():          ADDED (explicit empty state)

  If you had types between 57-120 bytes that used heap allocation,
  they now fit inline automatically!

  For types > 120 bytes, wrap them in unique_ptr:
    erase_and_box(std::make_unique<LargeType>(...))
)" << std::endl;

  return 0;
}
