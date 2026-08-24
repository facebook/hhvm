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

//! Borrowed, callback-scoped access to the C++ pipeline context, plus the
//! move-only captured continuation and coroutine spawn entry points.

use std::cell::Cell;
use std::marker::PhantomData;
use std::mem::MaybeUninit;
use std::pin::Pin;
use std::rc::Rc;

use crate::adapter::BytesPtr;
use crate::adapter::RustMessageAdapter;
use crate::erased::BorrowedMessageAdapter;
use crate::event_base::EventBaseTask;
use crate::event_base::FirstPoll;
use crate::ffi::ffi::FfiCallbackContext;
use crate::handler::HandlerResult;

/// Owned error that can be sent through a deferred pipeline continuation.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PipelineError {
    message: String,
}

impl PipelineError {
    pub fn new(message: impl Into<String>) -> Self {
        Self {
            message: message.into(),
        }
    }

    pub fn message(&self) -> &str {
        &self.message
    }
}

impl std::fmt::Display for PipelineError {
    fn fmt(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        formatter.write_str(&self.message)
    }
}

impl std::error::Error for PipelineError {}

/// Move-only continuation handle retaining its pipeline context.
///
/// The token is stored inline and is safe to relocate when Rust moves this
/// value. It is intentionally neither `Clone` nor `Copy`. Dropping it from any
/// thread releases its native pipeline guard on the originating EventBase.
pub struct ContextHandle {
    storage: [MaybeUninit<usize>; 2],
    _not_sync: PhantomData<Cell<()>>,
}

// SAFETY: native construction gives this token unique ownership of a pipeline
// guard. Moving it between threads only relocates its two pointer-sized words;
// native destruction runs inline on the EventBase or schedules the live token
// back there before Rust's inline storage expires.
unsafe impl Send for ContextHandle {}

impl ContextHandle {
    /// Continue an inbound message from this captured pipeline position.
    ///
    /// This consumes the handle. The continuation runs immediately when called
    /// on the originating EventBase and is otherwise enqueued onto that
    /// EventBase. Moving `BytesPtr` into the native `TypeErasedBox` is zero-copy.
    pub fn fire_read(self, message: BytesPtr) {
        let mut handle = std::mem::ManuallyDrop::new(self);
        // SAFETY: `handle` owns one live token and ManuallyDrop prevents Rust
        // Drop from consuming it again after native code moves it away.
        unsafe {
            crate::ffi::ffi::fire_context_handle_read(
                handle.storage.as_mut_ptr().cast(),
                message.into_cpp(),
            );
        }
    }

    /// Continue an exception from this captured pipeline position.
    ///
    /// Native code copies the message into an owning `folly::exception_wrapper`
    /// before this call returns. The continuation then runs immediately on the
    /// EventBase or is enqueued there using the same one-shot semantics as data.
    pub fn fire_exception(self, error: PipelineError) {
        let mut handle = std::mem::ManuallyDrop::new(self);
        // SAFETY: native code consumes the token exactly once and copies the
        // borrowed message before `error` is dropped at the end of this call.
        unsafe {
            crate::ffi::ffi::fire_context_handle_exception(
                handle.storage.as_mut_ptr().cast(),
                error.message().as_ptr(),
                error.message().len(),
            );
        }
    }

    /// Continue an outbound message from this captured pipeline position.
    ///
    /// This has the same one-shot, immediate-or-enqueued semantics as
    /// [`ContextHandle::fire_read`].
    pub fn fire_write(self, message: BytesPtr) {
        let mut handle = std::mem::ManuallyDrop::new(self);
        // SAFETY: see `fire_read`; native code consumes the token exactly once.
        unsafe {
            crate::ffi::ffi::fire_context_handle_write(
                handle.storage.as_mut_ptr().cast(),
                message.into_cpp(),
            );
        }
    }
}

impl Drop for ContextHandle {
    fn drop(&mut self) {
        // SAFETY: `ContextHandle` uniquely owns one live token initialized by
        // `CallbackContext::context_handle`. Native destruction either consumes
        // it inline or moves it to the EventBase before this storage expires.
        unsafe {
            crate::ffi::ffi::destroy_context_handle(self.storage.as_mut_ptr().cast());
        }
    }
}

/// Move-only ownership of an inbound message suspended at its pipeline
/// position.
///
/// The original type-erased C++ message remains intact inside this token. It
/// may be inspected or mutated on the originating EventBase and then resumed
/// exactly once. Dropping the token cancels the read. Resume and cancellation
/// are safe from any thread; native code performs delivery and destruction on
/// the EventBase that owns the pipeline.
pub struct DeferredRead {
    storage: MaybeUninit<usize>,
    _not_sync: PhantomData<Cell<()>>,
}

// SAFETY: the native token has unique ownership of both the message and its
// pipeline guard. Cross-thread resume and destruction consume the token and
// enqueue it onto the originating EventBase before touching or destroying the
// EventBase-owned values.
unsafe impl Send for DeferredRead {}

impl DeferredRead {
    /// Borrow a typed view of the intact message on its originating EventBase.
    ///
    /// Returns `None` off the owning EventBase. The returned borrow prevents
    /// this token from being resumed or dropped while the message is in use.
    pub fn borrow<M: BorrowedMessageAdapter>(&mut self) -> Option<M::View<'_>> {
        // SAFETY: this token uniquely owns one live native deferred-read token.
        // Native code returns its message only when called on the owning
        // EventBase, and the resulting borrow is tied to `&mut self`.
        let message =
            unsafe { crate::ffi::ffi::deferred_read_message(self.storage.as_mut_ptr().cast()) };
        if message.is_null() {
            return None;
        }
        // SAFETY: native code returned the address of the live message owned by
        // this token. `&mut self` guarantees exclusive access for the lifetime
        // of the pinned borrow and the token cannot move or be consumed then.
        let message = unsafe { &mut *message };
        assert!(
            M::holds(message),
            "DeferredRead::borrow: box does not hold the requested type"
        );
        // SAFETY: the unconditional `M::holds` check establishes the adapter's
        // exact C++ type, and the view remains tied to this exclusive borrow.
        Some(unsafe { M::borrow(Pin::new_unchecked(message)) })
    }

    /// Resume the original inbound message from its captured pipeline
    /// position. Delivery is suppressed if the pipeline has closed.
    pub fn resume(self) {
        let mut deferred = std::mem::ManuallyDrop::new(self);
        // SAFETY: `deferred` owns one live token and ManuallyDrop prevents its
        // destructor from consuming that token a second time.
        unsafe {
            crate::ffi::ffi::resume_deferred_read(deferred.storage.as_mut_ptr().cast());
        }
    }
}

impl Drop for DeferredRead {
    fn drop(&mut self) {
        // SAFETY: `DeferredRead` uniquely owns one token initialized by
        // `CallbackContext::defer_read`; native code consumes it exactly once.
        unsafe {
            crate::ffi::ffi::destroy_deferred_read(self.storage.as_mut_ptr().cast());
        }
    }
}

/// Borrowed, callback-scoped view of the live C++ pipeline context.
///
/// Each data or lifecycle callback receives an exclusive mutable reference to a
/// `CallbackContext` bound to the current FFI stack frame. The reference cannot
/// escape the callback: the lifetime `'callback` prevents storing it, and the
/// structural `!Send`/`!Sync` markers (via `PhantomData<Rc<()>>`) prevent
/// moving it to another thread or sharing a reference across threads.
///
/// # Why `!Send` and `!Sync`
///
/// The pipeline is single-threaded: all `fire_read`, `fire_write`, and
/// lifecycle calls must originate on the pipeline's EventBase thread. If
/// `CallbackContext` were `Send` or `Sync`, Rust code could invoke pipeline
/// operations from another thread, silently violating that invariant and
/// corrupting pipeline state. `PhantomData<Rc<()>>` makes both auto-traits
/// unimplementable at compile time, producing a hard error if any code
/// attempts to send or share a `CallbackContext`.
///
/// # Lifecycle state machine
///
/// States driven by `PipelineImpl` (see
/// `channel_pipeline/PipelineImpl.h`):
///
/// 1. **Added** — `handler_added` called in build order (head→tail).
///    `handler_id()` is stable and non-zero; pipeline is live.
/// 2. **Active** — `on_pipeline_active` called in same order. Data path
///    (`fire_read` / `fire_write`) is valid.
/// 3. **Inactive** — `on_pipeline_inactive` called in reverse order
///    (tail→head). The C++ shim cancels both readiness hooks before invoking
///    Rust; no readiness callback can enter Rust after the inactive transition.
/// 4. **Closing** — any `close()` call is idempotent and terminal: clears
///    readiness lists, event lists, and queues `handler_removed` in LIFO
///    order. Within the same callback that called `close()`,
///    `fire_read`/`fire_write` still forward to the tail/head endpoints.
/// 5. **Removed** — `handler_removed` called in LIFO order, once. No
///    callback may enter Rust after removal; the C++ shim cancels hooks in
///    `on_pipeline_inactive` and `handler_removed`.
///
/// # One-shot forwarding
///
/// `fire_read` and `fire_write` are guarded by a `forwarded_` flag in the C++
/// `CallbackContext`: the message box can be forwarded at most once per
/// callback invocation. A second call — or a call with a null message or null
/// box — returns `HandlerResult::Error` without side effects.
///
/// # Panic containment
///
/// Any Rust panic is caught at the FFI boundary via `catch_unwind` before
/// reaching C++. Data-path panics map to `HandlerResult::Error`. Lifecycle and
/// readiness panics are swallowed silently; the C++ shim continues execution.
pub struct CallbackContext<'callback> {
    inner: Pin<&'callback mut FfiCallbackContext>,
    _not_send_or_sync: PhantomData<Rc<()>>,
}

impl<'callback> CallbackContext<'callback> {
    pub(crate) fn new(inner: Pin<&'callback mut FfiCallbackContext>) -> Self {
        Self {
            inner,
            _not_send_or_sync: PhantomData,
        }
    }

    /// Start a Rust future on this pipeline's EventBase.
    ///
    /// The task owns the existing move-only continuation handle, which retains
    /// the pipeline and its EventBase until completion or cancellation. Its
    /// first poll runs inline in the current callback; wakes schedule later
    /// polls back onto this same EventBase. `complete` consumes the continuation
    /// and the future's output when the task becomes ready.
    pub fn spawn<T, Fut, Complete>(&mut self, future: Fut, complete: Complete)
    where
        T: Send + 'static,
        Fut: Future<Output = T> + Send + 'static,
        Complete: FnOnce(ContextHandle, T) + Send + 'static,
    {
        let event_base = self.inner.as_ref().get_ref().event_base();
        let continuation = self.context_handle();
        EventBaseTask::start(event_base, async move {
            complete(continuation, future.await);
        });
    }

    /// Poll a future now and capture the pipeline continuation only if it suspends.
    ///
    /// The future is first placed at its final pinned address and polled inline on
    /// the current EventBase callback. If it is ready, `ready` runs immediately
    /// with this borrowed context and its [`HandlerResult`] becomes the callback's
    /// result; no [`ContextHandle`] is created. If it is pending, the task takes a
    /// new one-shot `ContextHandle`, later polls remain on the same EventBase, and
    /// `complete` receives that handle with the output. The current callback then
    /// returns [`HandlerResult::Success`] because ownership of its in-flight work
    /// has moved into the task.
    ///
    /// Panics are contained by [`EventBaseTask`]. A panic does not invoke either
    /// completion callback and is reported here as [`HandlerResult::Success`] so
    /// unwinding never crosses the C++ FFI boundary.
    pub(crate) fn spawn_deferred<T, Fut, Ready, Complete>(
        &mut self,
        future: Fut,
        ready: Ready,
        complete: Complete,
    ) -> HandlerResult
    where
        T: Send + 'static,
        Fut: Future<Output = T> + Send + 'static,
        Ready: FnOnce(&mut Self, T) -> HandlerResult,
        Complete: FnOnce(ContextHandle, T) + Send + 'static,
    {
        let event_base = self.inner.as_ref().get_ref().event_base();
        match EventBaseTask::poll(event_base, future, complete) {
            FirstPoll::Ready(output) => ready(self, output),
            FirstPoll::Pending(task) => {
                task.install(self.context_handle());
                HandlerResult::Success
            }
            FirstPoll::Panicked => HandlerResult::Success,
        }
    }

    /// Create a move-only continuation handle retaining this pipeline context.
    pub fn context_handle(&mut self) -> ContextHandle {
        let mut handle = ContextHandle {
            storage: [MaybeUninit::uninit(); 2],
            _not_sync: PhantomData,
        };
        // SAFETY: `storage` is exactly two pointer-aligned words and remains
        // exclusively owned by `handle` until its native destructor runs.
        unsafe {
            self.inner
                .as_mut()
                .init_context_handle(handle.storage.as_mut_ptr().cast());
        }
        handle
    }

    /// Suspend the current inbound message without unpacking or copying it.
    ///
    /// Returns `None` if this callback has no message, the message is empty, or
    /// it was already forwarded. Dropping the returned token cancels delivery;
    /// [`DeferredRead::resume`] continues it from this handler's exact position.
    pub fn defer_read(
        &mut self,
        message: crate::erased::RustTypeErasedBox<'_>,
    ) -> Option<DeferredRead> {
        let _ = &message;
        let mut deferred = DeferredRead {
            storage: MaybeUninit::uninit(),
            _not_sync: PhantomData,
        };
        // SAFETY: storage is one pointer-aligned word and remains
        // exclusively owned by `deferred`. On false, native code constructed
        // nothing, so mem::forget prevents running a destructor on garbage.
        let initialized = unsafe {
            self.inner
                .as_mut()
                .init_deferred_read(deferred.storage.as_mut_ptr().cast())
        };
        if initialized {
            Some(deferred)
        } else {
            std::mem::forget(deferred);
            None
        }
    }

    /// Suspend the current inbound message while a future runs on this
    /// pipeline's EventBase.
    ///
    /// Completion receives the intact message token and may borrow, mutate,
    /// resume, or cancel it. The task owns the token throughout suspension, so
    /// panic or task destruction safely cancels the read.
    ///
    /// If the current callback has no live message, the message is empty, or it
    /// was already forwarded, this returns HandlerResult::Error without
    /// polling future or invoking complete.
    pub fn spawn_deferred_read<T, Fut, Complete>(
        &mut self,
        message: crate::erased::RustTypeErasedBox<'_>,
        future: Fut,
        complete: Complete,
    ) -> HandlerResult
    where
        T: Send + 'static,
        Fut: Future<Output = T> + Send + 'static,
        Complete: FnOnce(DeferredRead, T) + Send + 'static,
    {
        let Some(deferred) = self.defer_read(message) else {
            return HandlerResult::Error;
        };
        let event_base = self.inner.as_ref().get_ref().event_base();
        EventBaseTask::start(event_base, async move {
            complete(deferred, future.await);
        });
        HandlerResult::Success
    }

    /// Forward the inbound buffer downstream and return the result.
    ///
    /// The buffer is moved into the C++ message box and forwarded via
    /// `context_.fireRead`. This is a one-shot operation: a second call within
    /// the same callback, or a call after the pipeline is closed, returns
    /// `HandlerResult::Error` without side effects (guarded by the C++
    /// `forwarded_` flag).
    pub fn fire_read(&mut self, message: BytesPtr) -> HandlerResult {
        HandlerResult::from_ffi(self.inner.as_mut().fire_read(message.into_cpp()))
    }

    /// Forward the outbound buffer upstream and return the result.
    ///
    /// Symmetric to `fire_read` but for the outbound direction. Subject to the
    /// same one-shot and null-rejection guards.
    pub fn fire_write(&mut self, message: BytesPtr) -> HandlerResult {
        HandlerResult::from_ffi(self.inner.as_mut().fire_write(message.into_cpp()))
    }

    /// Forward the inbound message downstream UNCHANGED, without recovering its
    /// type ("forward what you don't understand" — the Netty pass-through). The
    /// whole [`RustTypeErasedBox`] is moved on, so the handler need not (and
    /// does not) `take` it. Consuming `_msg` releases its borrow and prevents a
    /// later `take`; the box is forwarded via the context's own reference to it.
    /// One-shot like [`fire_read`].
    ///
    /// [`fire_read`]: CallbackContext::fire_read
    /// [`RustTypeErasedBox`]: crate::erased::RustTypeErasedBox
    pub fn forward_read(&mut self, _msg: crate::erased::RustTypeErasedBox<'_>) -> HandlerResult {
        HandlerResult::from_ffi(self.inner.as_mut().forward_read())
    }

    /// Symmetric outbound pass-through of the whole message box.
    pub fn forward_write(&mut self, _msg: crate::erased::RustTypeErasedBox<'_>) -> HandlerResult {
        HandlerResult::from_ffi(self.inner.as_mut().forward_write())
    }

    /// Arm the native one-shot inbound readiness hook.
    ///
    /// When the transport signals that it is readable again, the C++ shim
    /// cancels the hook and then calls [`RustHandler::on_read_ready`]. The hook
    /// is one-shot per arm: it fires once and must be re-armed inside
    /// `on_read_ready` if further notifications are needed. Idempotent: calling
    /// while already armed is a no-op.
    pub fn await_read_ready(&mut self) {
        self.inner.as_mut().await_read_ready();
    }

    /// Idempotently cancel the native inbound readiness hook.
    ///
    /// Safe to call regardless of whether the hook is currently armed.
    pub fn cancel_read_ready(&mut self) {
        self.inner.as_mut().cancel_read_ready();
    }

    /// Return whether the native inbound readiness hook is armed.
    pub fn is_awaiting_read_ready(&self) -> bool {
        self.inner.as_ref().get_ref().is_awaiting_read_ready()
    }

    /// Arm the native one-shot outbound readiness hook.
    ///
    /// When the transport signals write readiness, the C++ shim cancels the
    /// hook and calls [`RustHandler::on_write_ready`]. Semantics mirror
    /// `await_read_ready`.
    pub fn await_write_ready(&mut self) {
        self.inner.as_mut().await_write_ready();
    }

    /// Idempotently cancel the native outbound readiness hook.
    ///
    /// Safe to call regardless of whether the hook is currently armed.
    pub fn cancel_write_ready(&mut self) {
        self.inner.as_mut().cancel_write_ready();
    }

    /// Return whether the native outbound readiness hook is armed.
    pub fn is_awaiting_write_ready(&self) -> bool {
        self.inner.as_ref().get_ref().is_awaiting_write_ready()
    }

    /// Return the stable handler identity for this context.
    ///
    /// This is an FNV-64 hash of the handler tag registered at build time,
    /// computed once during pipeline initialization. It is non-zero for any
    /// valid installed handler; zero indicates an error path (e.g., exception
    /// handler invocation without a context). The value never changes for the
    /// lifetime of the handler.
    pub fn handler_id(&self) -> u64 {
        self.inner.as_ref().handler_id()
    }

    /// Allocate a buffer using the pipeline's allocator, with a `IOBuf::create`
    /// fallback.
    ///
    /// Returns `None` if allocation fails. A size of 0 is valid and returns an
    /// empty IOBuf chain. The call is `noexcept` on the C++ side.
    pub fn allocate(&mut self, size: usize) -> Option<BytesPtr> {
        let ptr = self.inner.as_mut().allocate(size);
        if ptr.is_null() {
            None
        } else {
            Some(BytesPtr::new(ptr))
        }
    }

    /// Deep-copy the given bytes into a new single-node IOBuf (allocation + memcpy).
    ///
    /// An empty slice produces an empty IOBuf. Returns `None` on allocation
    /// failure. This bypasses the pipeline allocator and always uses
    /// `folly::IOBuf::copyBuffer`. Use `allocate` when you want the pipeline
    /// allocator; use this when you need to materialize a byte slice into an
    /// owned buffer.
    pub fn copy_from_slice(&mut self, data: &[u8]) -> Option<BytesPtr> {
        let (ptr, len) = if data.is_empty() {
            (std::ptr::null(), 0usize)
        } else {
            (data.as_ptr(), data.len())
        };
        // SAFETY:
        // (1) When `data` is non-empty, `ptr = data.as_ptr()` is valid for
        //     `data.len()` bytes because `data` is a shared reference that
        //     lives for at least this synchronous call frame.
        // (2) When `data` is empty, `ptr` is null and `len` is 0; the C++
        //     `copyBuffer` implementation treats null+0 as "create empty
        //     IOBuf" (see CallbackContext.cpp:96-97) without dereferencing.
        // (3) The C++ function copies the pointed-to bytes immediately and
        //     does not retain the pointer past the call.
        let out = unsafe { self.inner.as_mut().copy_buffer_from_slice(ptr, len) };
        if out.is_null() {
            None
        } else {
            Some(BytesPtr::new(out))
        }
    }

    /// Shallow-clone the entire chain via `IOBuf::clone` (refcount increment, no data copy).
    ///
    /// Returns `None` if `buffer` is null or allocation fails.
    pub fn clone_chain(&mut self, buffer: &BytesPtr) -> Option<BytesPtr> {
        let inner_ref = buffer.as_iobuf_ref()?;
        let ptr = self.inner.as_mut().clone_buffer_chain(inner_ref);
        if ptr.is_null() {
            None
        } else {
            Some(BytesPtr::new(ptr))
        }
    }

    /// Shallow-clone the first node only via `IOBuf::cloneOne` (refcount increment, no data copy).
    ///
    /// Returns `None` if `buffer` is null or allocation fails.
    pub fn clone_one(&mut self, buffer: &BytesPtr) -> Option<BytesPtr> {
        let inner_ref = buffer.as_iobuf_ref()?;
        let ptr = self.inner.as_mut().clone_one(inner_ref);
        if ptr.is_null() {
            None
        } else {
            Some(BytesPtr::new(ptr))
        }
    }

    /// Deep-copy the entire chain into a single contiguous IOBuf (allocation + memcpy of chain length).
    ///
    /// Use when a contiguous byte view is required and refcount sharing is
    /// insufficient. Returns `None` if `buffer` is null or allocation fails.
    /// Empty chains produce an empty IOBuf.
    pub fn coalesced_copy(&mut self, buffer: &BytesPtr) -> Option<BytesPtr> {
        let inner_ref = buffer.as_iobuf_ref()?;
        let ptr = self.inner.as_mut().coalesced_copy(inner_ref);
        if ptr.is_null() {
            None
        } else {
            Some(BytesPtr::new(ptr))
        }
    }

    /// Idempotently close the pipeline (terminal operation).
    ///
    /// Clears readiness lists and event lists, then fires `handler_removed` in
    /// LIFO order. After `close()`, `is_closed()` returns `true`. Within the
    /// same callback that called `close()`, `fire_read`/`fire_write` still
    /// forward to the tail/head endpoints. Subsequent pipeline calls return
    /// `Result::Error`. This call is `noexcept` on the C++ side.
    pub fn close(&mut self) {
        self.inner.as_mut().close();
    }

    /// Return `true` if the pipeline is closed or the owning pipeline pointer is null.
    pub fn is_closed(&self) -> bool {
        self.inner.as_ref().is_closed()
    }
}
