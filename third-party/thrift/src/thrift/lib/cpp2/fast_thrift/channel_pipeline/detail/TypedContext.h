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

#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#include <folly/CPortability.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

namespace apache::thrift::fast_thrift::channel_pipeline::detail {

/**
 * True iff type T appears in the std::tuple type `Tuple`.
 */
template <typename Tuple, typename T>
struct TupleHasType;

template <typename T, typename... Ts>
struct TupleHasType<std::tuple<Ts...>, T>
    : std::disjunction<std::is_same<T, Ts>...> {};

/**
 * TypedContext is the compile-time-typed view of a handler context for a
 * pipeline that registered pipeline-level state via
 * PipelineBuilder::addState<T>().
 *
 * It is a zero-overhead wrapper around a ContextImpl reference: it forwards the
 * entire context API unchanged and adds a single, type-safe accessor,
 * `state<T>()`. `StateTuple` is the std::tuple of every registered state type;
 * `state<T>()` is well-formed only for a `T` in that tuple, so accessing an
 * unregistered state type is a compile error rather than a silent
 * reinterpretation.
 *
 * Handlers never name this type: they are written as `template <typename
 * Context>` and receive a TypedContext only when their pipeline registered
 * state. Because it converts implicitly to ContextImpl&, handlers and helpers
 * that accept a plain ContextImpl& (e.g. ContextHandle) keep working
 * unchanged. Pipelines with no registered state pass the bare ContextImpl and
 * never instantiate this wrapper.
 *
 * Lifetime: constructed on the stack by the dispatch thunk for the duration of
 * a single handler call; it holds a reference to the pipeline-owned
 * ContextImpl and owns nothing.
 */
template <typename StateTuple>
class TypedContext {
 public:
  explicit TypedContext(ContextImpl& ctx) noexcept : ctx_(ctx) {}

  /**
   * Access pipeline-level state of type T registered via
   * PipelineBuilder::addState<T>(). Constrained so that a T which was not
   * registered fails to compile at the call site.
   */
  template <typename T>
    requires TupleHasType<StateTuple, T>::value
  FOLLY_ALWAYS_INLINE T& state() const noexcept {
    return std::get<T>(*static_cast<StateTuple*>(ctx_.pipelineState_));
  }

  // Implicit conversion so handlers/helpers that take a plain ContextImpl&
  // (e.g. ContextHandle{ctx}) accept a TypedContext transparently.
  operator ContextImpl&() const noexcept { return ctx_; }

  // === Forwarded ContextImpl API ===
  //
  // Every ContextImpl method a handler can call is forwarded verbatim. The
  // forwarders share one perfect-forwarding shape, so keeping this in sync with
  // ContextImpl is a single line per method (and it covers templated members
  // such as fireEvent). If ContextImpl gains a handler-facing method and no
  // line is added here, the omission surfaces as a compile error at the
  // handler's call site (in state-using pipelines only), never a silent gap.
  //
  // The forwarders are const: TypedContext is a non-owning view holding a
  // ContextImpl& (like state<T>() and operator ContextImpl&), so wrapper
  // constness does not propagate to the referent. Making them const lets a
  // `const TypedContext&` reach the const ContextImpl accessors (handlerId,
  // handlerIndex) just as a `ContextImpl&` would.
#define FT_TYPED_CONTEXT_FORWARD(NAME)                                        \
  template <typename... Args>                                                 \
  FOLLY_ALWAYS_INLINE decltype(auto) NAME(Args&&... args) const noexcept(     \
      noexcept(std::declval<ContextImpl&>().NAME(std::declval<Args>()...))) { \
    return ctx_.NAME(std::forward<Args>(args)...);                            \
  }

  FT_TYPED_CONTEXT_FORWARD(handlerId)
  FT_TYPED_CONTEXT_FORWARD(activate)
  FT_TYPED_CONTEXT_FORWARD(fireRead)
  FT_TYPED_CONTEXT_FORWARD(fireWrite)
  FT_TYPED_CONTEXT_FORWARD(fireException)
  FT_TYPED_CONTEXT_FORWARD(fireEvent)
  FT_TYPED_CONTEXT_FORWARD(deactivate)
  FT_TYPED_CONTEXT_FORWARD(pipeline)
  FT_TYPED_CONTEXT_FORWARD(allocate)
  FT_TYPED_CONTEXT_FORWARD(copyBuffer)
  FT_TYPED_CONTEXT_FORWARD(eventBase)
  FT_TYPED_CONTEXT_FORWARD(close)
  FT_TYPED_CONTEXT_FORWARD(awaitWriteReady)
  FT_TYPED_CONTEXT_FORWARD(cancelAwaitWriteReady)
  FT_TYPED_CONTEXT_FORWARD(isAwaitingWriteReady)
  FT_TYPED_CONTEXT_FORWARD(awaitReadReady)
  FT_TYPED_CONTEXT_FORWARD(cancelAwaitReadReady)
  FT_TYPED_CONTEXT_FORWARD(isAwaitingReadReady)
  FT_TYPED_CONTEXT_FORWARD(handlerIndex)

#undef FT_TYPED_CONTEXT_FORWARD

 private:
  ContextImpl& ctx_;
};

/**
 * The context type a handler receives for a pipeline whose registered state is
 * `StateTuple`: the bare ContextImpl when nothing was registered (zero
 * overhead, unchanged behavior), otherwise a TypedContext<StateTuple>.
 */
template <typename StateTuple>
using ContextFor = std::conditional_t<
    std::tuple_size_v<StateTuple> == 0,
    ContextImpl,
    TypedContext<StateTuple>>;

/**
 * Construct the persistent TypedContext view for `ctx` into its storage. Called
 * once per context by the builder, for state-using pipelines, after contexts
 * are finalized (the same stability point nextCtx_/prevCtx_ rely on). The view
 * is a reference wrapper (trivially destructible), so it needs no teardown.
 */
template <typename StateTuple>
FOLLY_ALWAYS_INLINE void initTypedView(ContextImpl& ctx) noexcept {
  static_assert(
      sizeof(TypedContext<StateTuple>) <= sizeof(void*) &&
          alignof(TypedContext<StateTuple>) <= alignof(void*),
      "TypedContext must fit the pointer-sized context view slot");
  static_assert(
      std::is_trivially_destructible_v<TypedContext<StateTuple>>,
      "TypedContext must be trivially destructible: it is constructed into raw "
      "context storage and its destructor is never run");
  ::new (ctx.typedViewStorage()) TypedContext<StateTuple>{ctx};
}

/**
 * The context reference handed to a handler for every callback: the bare
 * ContextImpl& for a stateless pipeline (zero overhead), otherwise the
 * pipeline-stable TypedContext<StateTuple>& constructed by initTypedView. It is
 * a stable reference in both cases, so a handler may cache it (e.g. in
 * handlerAdded) for deferred use. Bind with `decltype(auto)` to avoid copying.
 */
template <typename StateTuple>
FOLLY_ALWAYS_INLINE decltype(auto) contextFor(ContextImpl& ctx) noexcept {
  if constexpr (std::tuple_size_v<StateTuple> == 0) {
    return (ctx);
  } else {
    return *std::launder(
        reinterpret_cast<TypedContext<StateTuple>*>(ctx.typedViewStorage()));
  }
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::detail
