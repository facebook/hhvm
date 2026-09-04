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

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

#include <folly/CppAttributes.h>
#include <folly/lang/SafeAssert.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

namespace apache::thrift::fast_thrift::thrift {

// An extension's identity. Derived from its name at compile time, so a lookup
// compares against an immediate rather than hashing or consulting RTTI.
using ExtensionId = std::uint64_t;

/**
 * Declares an extension's identity. Place it in the extension's own type,
 * alongside the state it wants a context to carry:
 *
 *   struct SapExtension {
 *     EXTENSION_ID(sap);
 *     using ConnState = PerConnectionInternalFields;
 *     using RequestState = PerRequestInternalFields;
 *   };
 *
 * One identity serves both scopes; which one a lookup reaches is decided by
 * which context it is asked of. An extension that wants storage at only one
 * scope declares only that alias, and asking a context for the other does not
 * compile.
 */
#define EXTENSION_ID(name)                                                  \
  static constexpr ::apache::thrift::fast_thrift::thrift::ExtensionId kId = \
      ::apache::thrift::fast_thrift::channel_pipeline::fnv1a_hash(#name)

namespace detail {

/**
 * One installed extension's slot: the id it answers to, and which pointer in a
 * context's slot array belongs to it.
 */
struct ExtensionSlotEntry {
  ExtensionId id{0};
  std::uint16_t index{0};
};

} // namespace detail

/**
 * The slot plan for one scope: which extensions installed, and which slot each
 * one owns.
 *
 * Built while the server starts, once all extensions have registered, and
 * immutable after. Every connection and request on that server reads the same
 * instance without synchronization. Indices are assigned in registration order,
 * which no reader observes: a lookup is by id.
 *
 * A server with no installed extensions builds an empty layout, and contexts
 * under it carry no slots.
 */
class ExtensionLayout {
 public:
  ExtensionLayout() = default;

  const detail::ExtensionSlotEntry* entries() const noexcept {
    return entries_.data();
  }
  std::size_t entryCount() const noexcept { return entries_.size(); }
  std::size_t slotCount() const noexcept { return entries_.size(); }
  bool empty() const noexcept { return entries_.empty(); }

 private:
  friend class ExtensionLayoutBuilder;

  std::vector<detail::ExtensionSlotEntry> entries_;
};

/**
 * Accumulates installed extensions into a layout.
 *
 * Used once per scope while the server starts, before it accepts anything.
 */
class ExtensionLayoutBuilder {
 public:
  /**
   * Reserves a slot for `id`.
   *
   * A slot holds one pointer. What it points at, and who owns that, is the
   * extension's business — the framework never constructs or destroys it.
   *
   * A repeated id aborts. Two extensions reaching one slot is the failure this
   * mechanism exists to prevent, and startup is the only point at which it can
   * be caught rather than silently aliasing state under traffic.
   */
  void add(ExtensionId id) {
    for (const auto& entry : layout_.entries_) {
      FOLLY_SAFE_CHECK(
          entry.id != id, "two fast_thrift extensions share one id");
    }
    layout_.entries_.push_back(
        detail::ExtensionSlotEntry{
            .id = id,
            .index = static_cast<std::uint16_t>(layout_.entries_.size())});
  }

  ExtensionLayout build() && { return std::move(layout_); }

 private:
  ExtensionLayout layout_;
};

/**
 * A context's extension slots: one pointer per installed extension.
 *
 * The framework owns the array, never what the pointers reach. An extension
 * decides whether its slot holds state it allocated, or is itself the payload —
 * the event-handler bridge stores a `Cpp2RequestContext*` it already owns, and
 * allocates nothing.
 *
 * Slots are installed after construction rather than built in, so a context
 * created without a layout — every test that does not exercise an extension —
 * costs nothing and reads every lookup as "not installed".
 *
 * Realistic deployments install a handful of extensions, so the array is held
 * inline up to `kInlineSlots` and spills to the heap beyond. That is a
 * threshold, not a limit: a server past it still works, it just allocates.
 *
 * Non-copyable and non-movable: a context stays where it was built.
 */
class ExtensionSlots {
 public:
  static constexpr std::size_t kInlineSlots = 2;

  ExtensionSlots() = default;
  ~ExtensionSlots() = default;

  // Rejected rather than bound: a layout that dies at the end of the install
  // statement leaves every later lookup reading freed memory.
  void install(ExtensionLayout&&) = delete;

  /**
   * Points this context at `layout` and zeroes its slots. Called once, by
   * whoever creates the context, before any handler sees it. `layout` must
   * outlive the context.
   */
  void install(const ExtensionLayout& layout) {
    FOLLY_SAFE_CHECK(layout_ == nullptr, "extension slots installed twice");
    if (layout.empty()) {
      return;
    }
    layout_ = &layout;
    if (layout_->slotCount() > kInlineSlots) {
      heap_ = std::make_unique<void*[]>(layout_->slotCount());
      slots_ = heap_.get();
    } else {
      slots_ = inline_;
    }
    std::fill_n(slots_, layout_->slotCount(), nullptr);
  }

  /**
   * The pointer stored under `id`, or null when no extension installed one or
   * none has published yet.
   *
   * Unchecked in the same sense as any slot: `State` must be the type the
   * extension stored under this id.
   */
  template <class State>
  State* FOLLY_NULLABLE find(ExtensionId id) const noexcept {
    const auto* entry = findEntry(id);
    return entry == nullptr ? nullptr
                            : static_cast<State*>(slots_[entry->index]);
  }

  /**
   * Publishes `state` under `id`. Non-owning: the extension keeps the object
   * alive for as long as the context can reach it, and clears the slot when it
   * does not.
   *
   * Aborts when `id` is not installed, which is a wiring error rather than a
   * runtime condition.
   */
  void set(ExtensionId id, void* FOLLY_NULLABLE state) noexcept {
    const auto* entry = findEntry(id);
    FOLLY_SAFE_CHECK(entry != nullptr, "extension is not installed");
    slots_[entry->index] = state;
  }

  // Whether `id` has a slot on this context at all.
  bool installed(ExtensionId id) const noexcept {
    return findEntry(id) != nullptr;
  }

  ExtensionSlots(const ExtensionSlots&) = delete;
  ExtensionSlots& operator=(const ExtensionSlots&) = delete;
  ExtensionSlots(ExtensionSlots&&) = delete;
  ExtensionSlots& operator=(ExtensionSlots&&) = delete;

 private:
  const detail::ExtensionSlotEntry* FOLLY_NULLABLE
  findEntry(ExtensionId id) const noexcept {
    if (layout_ == nullptr) {
      return nullptr;
    }
    const auto* entries = layout_->entries();
    const std::size_t count = layout_->entryCount();
    for (std::size_t i = 0; i < count; ++i) {
      if (entries[i].id == id) {
        return &entries[i];
      }
    }
    return nullptr;
  }

  const ExtensionLayout* FOLLY_NULLABLE layout_{nullptr};
  void** FOLLY_NULLABLE slots_{nullptr};
  void* inline_[kInlineSlots]{};
  std::unique_ptr<void*[]> heap_;
};

} // namespace apache::thrift::fast_thrift::thrift
