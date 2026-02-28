/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <mcrouter/lib/FiberLocalInternal.h>

namespace facebook::mcrouter::detail {
namespace {

static bool gFinalized = false;
static size_t gNextOffset = 0;
static size_t gMaxAlign = alignof(size_t);
static_assert(alignof(size_t) == sizeof(size_t), "");
// Mutex used to serialize FlsRegistry::registerFls() and
// FlsRegistry::finalize()
static std::mutex gFlsRegistryFinalizeMutex;

struct FlsEntry {
  FlsEntry(
      size_t off,
      detail::FlsRegistry::Constructor c,
      detail::FlsRegistry::Destructor d)
      : offset(off), constructor(c), destructor(d) {}

  size_t offset;
  detail::FlsRegistry::Constructor constructor;
  detail::FlsRegistry::Destructor destructor;
};

using FlsEntryListPtr = std::shared_ptr<std::vector<FlsEntry>>;

static const FlsEntryListPtr& flsEntries() {
  // NOTE: a function-static shared_ptr has the following benefits:
  // 1. Eliminate initialization ordering problems. 'entires' will be
  //    initialized on the first call to flsEntries().
  // 2. The shared_ptr allows us to keep the underlying vector alive even while
  //    the main thread is shutting down by storing a copy of the shared_ptr in
  //    each set of thread locals we allocate.
  static auto entries =
      std::make_shared<typename FlsEntryListPtr::element_type>();
  return entries;
}

} // namespace

void FlsRegistry::allocate() {
  if (UNLIKELY(!gFinalized)) {
    std::lock_guard<std::mutex> lock(gFlsRegistryFinalizeMutex);
    gFinalized = true;
  }

  auto& storage = localStorage();
  if (storage != nullptr) {
    return;
  }

  const auto entries = flsEntries();
  CHECK(entries) << "attempted to allocate local storage after shutdown";

  void* memory;
  // Memory layout:   | locals | FlsEntryListPtr |
  const size_t total_memory = gNextOffset + sizeof(FlsEntryListPtr);
  auto rv = posix_memalign(&memory, gMaxAlign, total_memory);
  CHECK(rv == 0) << "failed to allocate local storage, return value " << rv;

  // Keep a copy of the 'entries' shared_ptr in the local storage
  new (static_cast<char*>(memory) + gNextOffset) FlsEntryListPtr(entries);
  auto const n_fls_entries = entries->size();
  for (size_t i = 0; i < n_fls_entries; ++i) {
    const auto& entry = (*entries)[i];
    entry.constructor(static_cast<char*>(memory) + entry.offset);
  }
  // Important to set this here since malloc() shims could access
  // local storage
  storage = memory;
}

void FlsRegistry::freeLocalStorage(void* opaque_locals) noexcept {
  if (opaque_locals != nullptr) {
    // Pull the 'entries' shared_ptr from the local storage - this
    // prevents races if the main thread shuts down before other
    // threads complete (since the main thread will clear the
    // shared_ptr in flsEntries()).
    FlsEntryListPtr& entries = *reinterpret_cast<FlsEntryListPtr*>(
        static_cast<char*>(opaque_locals) + gNextOffset);
    auto const n_fls_entries = entries->size();
    for (size_t i = 0; i < n_fls_entries; ++i) {
      const auto& entry = (*entries)[i];
      entry.destructor(static_cast<char*>(opaque_locals) + entry.offset);
    }
    entries.reset();
    ::free(opaque_locals);
  }
}

FlsRegistry::FlsHandle FlsRegistry::registerFls(
    size_t object_size,
    size_t object_align,
    Constructor constructor,
    Destructor destructor) {
  assert(object_size >= object_align);
  assert(constructor != nullptr);
  assert(destructor != nullptr);
  std::lock_guard<std::mutex> lock(gFlsRegistryFinalizeMutex);
  CHECK(!gFinalized)
      << "attempt to register a new fiber local after fiber local "
         "storage has been allocated";
  auto offset = gNextOffset;
  if (offset % object_align != 0) {
    offset = offset + object_size - offset % object_align;
  }
  assert(offset % object_align == 0);
  gNextOffset = offset + object_size;
  gMaxAlign = std::max(gMaxAlign, object_align);
  const auto entries = flsEntries();
  CHECK(entries) << "attempted to register a fiber local after shutdown";
  entries->emplace_back(offset, constructor, destructor);
  return offset;
}

bool FlsRegistry::isSafe(FlsHandle offset) {
  return offset < gNextOffset;
}

} // namespace facebook::mcrouter::detail
