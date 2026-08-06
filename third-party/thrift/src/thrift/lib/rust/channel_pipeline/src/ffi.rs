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

//! CXX boundary declarations for the synchronous channel_pipeline Rust bridge.
//!
//! Every `unsafe extern "C++"` declaration carries explicit invariant
//! contracts at its declaration site. The public Rust API in [`context`] and
//! [`handler`] has no unsafe callable surface.
//!
//! Factory functions (`rust_handler_new_*`) and test query functions are
//! public because CXX requires Rust-exported functions to be reachable from
//! the generated C++ header. They are not part of the public user-facing API.

#[cxx::bridge(namespace = "channel_pipeline_rust")]
pub(crate) mod ffi {
    extern "Rust" {
        type RustHandlerOpaque;

        fn rust_handler_new_noop() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_counting_test() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_forwarding_test() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_backpressure_test() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_error_test() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_panicking_test() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_lifecycle_test() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_lifecycle_order(id: u32) -> Box<RustHandlerOpaque>;
        fn rust_handler_new_recovering_read() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_recovering_write() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_bidirectional() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_read_rearm(rearms: u32) -> Box<RustHandlerOpaque>;
        fn rust_handler_new_readiness_probe() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_ready_rearm_bench() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_panicking_lifecycle() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_rearm_on_removed() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_identity() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_allocation_probe() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_copy_probe() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_close_probe() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_state_machine(id: u32) -> Box<RustHandlerOpaque>;
        fn rust_handler_new_panic_retention() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_exception_preserve() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_reentrancy() -> Box<RustHandlerOpaque>;
        fn rust_handler_new_context_handle_test(scenario: u32) -> Box<RustHandlerOpaque>;
        fn rust_handler_reset_test_counts();
        fn rust_handler_reset_context_handle_test_slot();
        fn rust_handler_context_handle_test_slot_len() -> u32;
        fn rust_handler_drop_one_context_handle_for_test();
        fn rust_handler_drop_all_context_handles_for_test();
        fn rust_handler_test_read_callbacks() -> u32;
        fn rust_handler_test_write_callbacks() -> u32;
        fn rust_handler_test_exception_callbacks() -> u32;
        fn rust_handler_test_read_ready_callbacks() -> u32;
        fn rust_handler_test_write_ready_callbacks() -> u32;
        fn rust_handler_test_added_callbacks() -> u32;
        fn rust_handler_test_active_callbacks() -> u32;
        fn rust_handler_test_inactive_callbacks() -> u32;
        fn rust_handler_test_removed_callbacks() -> u32;
        fn rust_handler_test_sequence() -> String;
        fn rust_handler_test_probe_checks() -> u32;
        fn rust_handler_readiness_probe_check_count() -> u32;
        fn rust_handler_phase3_handler_id() -> u64;
        fn rust_handler_phase3_alloc_checks() -> u32;
        fn rust_handler_phase3_copy_checks() -> u32;
        fn rust_handler_phase3_close_checks() -> u32;
        fn rust_handler_allocation_probe_check_count() -> u32;
        fn rust_handler_copy_probe_check_count() -> u32;
        fn rust_handler_close_probe_check_count() -> u32;
        fn rust_handler_phase5_state() -> u32;
        fn rust_handler_phase5_p5_sequence() -> String;
        fn rust_handler_phase5_exception_preserved() -> u32;
        fn rust_handler_phase5_reentrancy() -> u32;
        fn rust_handler_phase5_p5_counts() -> Vec<u32>;
        fn rust_handler_state_machine_stage_count() -> u32;

        fn rust_handler_on_read(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
            msg: Pin<&mut TypeErasedBox>,
        ) -> i32;
        fn rust_handler_on_write(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
            msg: Pin<&mut TypeErasedBox>,
        ) -> i32;
        fn rust_handler_on_exception(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
        );
        fn rust_handler_on_read_ready(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
        );
        fn rust_handler_on_write_ready(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
        );
        fn rust_handler_on_pipeline_active(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
        );
        fn rust_handler_on_pipeline_inactive(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
        );
        fn rust_handler_handler_added(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
        );
        fn rust_handler_handler_removed(
            handler: &mut RustHandlerOpaque,
            context: Pin<&mut FfiCallbackContext>,
        );
    }

    unsafe extern "C++" {
        include!("thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/CallbackContext.h");

        #[cxx_name = "CallbackContext"]
        type FfiCallbackContext;

        /// SAFETY: `storage` must point to two pointer-sized, pointer-aligned,
        /// uninitialized words that remain valid until `destroy_context_handle`.
        #[cxx_name = "initContextHandle"]
        unsafe fn init_context_handle(self: Pin<&mut FfiCallbackContext>, storage: *mut u8);

        #[cxx_name = "fireRead"]
        fn fire_read(self: Pin<&mut FfiCallbackContext>, message: UniquePtr<IOBuf>) -> i32;

        #[cxx_name = "fireWrite"]
        fn fire_write(self: Pin<&mut FfiCallbackContext>, message: UniquePtr<IOBuf>) -> i32;

        #[cxx_name = "forwardRead"]
        fn forward_read(self: Pin<&mut FfiCallbackContext>) -> i32;
        #[cxx_name = "forwardWrite"]
        fn forward_write(self: Pin<&mut FfiCallbackContext>) -> i32;

        #[cxx_name = "awaitReadReady"]
        fn await_read_ready(self: Pin<&mut FfiCallbackContext>);
        #[cxx_name = "cancelReadReady"]
        fn cancel_read_ready(self: Pin<&mut FfiCallbackContext>);
        #[cxx_name = "isAwaitingReadReady"]
        fn is_awaiting_read_ready(self: &FfiCallbackContext) -> bool;
        #[cxx_name = "awaitWriteReady"]
        fn await_write_ready(self: Pin<&mut FfiCallbackContext>);
        #[cxx_name = "cancelWriteReady"]
        fn cancel_write_ready(self: Pin<&mut FfiCallbackContext>);
        #[cxx_name = "isAwaitingWriteReady"]
        fn is_awaiting_write_ready(self: &FfiCallbackContext) -> bool;

        #[cxx_name = "handlerId"]
        fn handler_id(self: &FfiCallbackContext) -> u64;
        #[cxx_name = "allocate"]
        fn allocate(self: Pin<&mut FfiCallbackContext>, size: usize) -> UniquePtr<IOBuf>;
        #[cxx_name = "copyBuffer"]
        unsafe fn copy_buffer_from_slice(
            self: Pin<&mut FfiCallbackContext>,
            data: *const u8,
            size: usize,
        ) -> UniquePtr<IOBuf>;
        #[cxx_name = "cloneBufferChain"]
        fn clone_buffer_chain(
            self: Pin<&mut FfiCallbackContext>,
            buffer: &IOBuf,
        ) -> UniquePtr<IOBuf>;
        #[cxx_name = "cloneOne"]
        fn clone_one(self: Pin<&mut FfiCallbackContext>, buffer: &IOBuf) -> UniquePtr<IOBuf>;
        #[cxx_name = "coalescedCopy"]
        fn coalesced_copy(self: Pin<&mut FfiCallbackContext>, buffer: &IOBuf) -> UniquePtr<IOBuf>;
        #[cxx_name = "close"]
        fn close(self: Pin<&mut FfiCallbackContext>);
        #[cxx_name = "isClosed"]
        fn is_closed(self: &FfiCallbackContext) -> bool;

        /// SAFETY: `storage` must contain one live token constructed by
        /// `init_context_handle`. This consumes the token exactly once and
        /// continues immediately on its EventBase or enqueues there otherwise.
        #[cxx_name = "fireContextHandleRead"]
        unsafe fn fire_context_handle_read(storage: *mut u8, message: UniquePtr<IOBuf>);
        /// SAFETY: same contract as `fire_context_handle_read`.
        #[cxx_name = "fireContextHandleWrite"]
        unsafe fn fire_context_handle_write(storage: *mut u8, message: UniquePtr<IOBuf>);
        /// SAFETY: same token contract as `fire_context_handle_read`.
        /// `message_data` must reference `message_size` initialized bytes, and
        /// native code must copy them before this call returns.
        #[cxx_name = "fireContextHandleException"]
        unsafe fn fire_context_handle_exception(
            storage: *mut u8,
            message_data: *const u8,
            message_size: usize,
        );

        /// SAFETY: `storage` must contain one live token constructed by
        /// `init_context_handle`, and this call must consume it exactly once.
        /// The token may be consumed from any thread; native code schedules its
        /// live guard back to the originating EventBase when necessary.
        #[cxx_name = "destroyContextHandle"]
        unsafe fn destroy_context_handle(storage: *mut u8);
    }

    #[namespace = "folly"]
    unsafe extern "C++" {
        include!("folly/io/IOBuf.h");
        type IOBuf = iobuf::IOBuf;
    }

    unsafe extern "C++" {
        include!("thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustTypeErasedBox.h");

        /// Opaque C++ `TypeErasedBox`, borrowed by Rust as `Pin<&mut ..>` (it
        /// lives on the stack; Rust never owns or heap-allocates it). The typed
        /// take is a zero-copy `unsafe` relocate done entirely on the Rust side
        /// (see `RustTypeErasedBox::take`); `reset` flips it to empty afterward.
        #[namespace = "apache::thrift::fast_thrift::channel_pipeline"]
        type TypeErasedBox;

        fn rust_teb_reset(teb: Pin<&mut TypeErasedBox>);
        fn rust_teb_is_empty(teb: &TypeErasedBox) -> bool;
        fn rust_teb_holds_bytes(teb: &TypeErasedBox) -> bool;
    }
}

use crate::context::CallbackContext;
use crate::erased::RustTypeErasedBox;
use crate::ffi::ffi::TypeErasedBox;
use crate::handler::BackpressureTestHandler;
use crate::handler::CountingTestHandler;
use crate::handler::ErrorTestHandler;
use crate::handler::ForwardingTestHandler;
use crate::handler::HandlerResult;
use crate::handler::LifecycleTestHandler;
use crate::handler::NoopHandler;
use crate::handler::PanickingLifecycleHandler;
use crate::handler::PanickingTestHandler;
use crate::handler::ReadinessProbeHandler;
use crate::handler::RustHandler;

fn boxed(handler: impl RustHandler) -> Box<RustHandlerOpaque> {
    Box::new(RustHandlerOpaque {
        inner: Box::new(handler),
    })
}

/// Type-erased Rust handler owned by the C++ `RustHandler<Context>` shim.
///
/// C++ holds a `rust::Box<RustHandlerOpaque>` for the pipeline lifetime.
/// Instances are created via factory functions exposed through the CXX bridge
/// (e.g., `rust_handler_new_noop`, or user-defined factories). The box is
/// destroyed when the C++ shim is destructed during LIFO pipeline teardown,
/// which drops the inner `Box<dyn RustHandler>`.
pub struct RustHandlerOpaque {
    inner: Box<dyn RustHandler>,
}

pub fn rust_handler_new_noop() -> Box<RustHandlerOpaque> {
    boxed(NoopHandler)
}
pub fn rust_handler_new_counting_test() -> Box<RustHandlerOpaque> {
    boxed(CountingTestHandler)
}
pub fn rust_handler_new_forwarding_test() -> Box<RustHandlerOpaque> {
    boxed(ForwardingTestHandler)
}
pub fn rust_handler_new_backpressure_test() -> Box<RustHandlerOpaque> {
    boxed(BackpressureTestHandler)
}
pub fn rust_handler_new_error_test() -> Box<RustHandlerOpaque> {
    boxed(ErrorTestHandler)
}
pub fn rust_handler_new_panicking_test() -> Box<RustHandlerOpaque> {
    boxed(PanickingTestHandler)
}
pub fn rust_handler_new_lifecycle_test() -> Box<RustHandlerOpaque> {
    boxed(LifecycleTestHandler)
}
pub fn rust_handler_new_lifecycle_order(id: u32) -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_lifecycle_order_handler(id))
}
pub fn rust_handler_new_recovering_read() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_recovering_read_handler())
}
pub fn rust_handler_new_recovering_write() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_recovering_write_handler())
}
pub fn rust_handler_new_bidirectional() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_bidirectional_handler())
}
pub fn rust_handler_new_read_rearm(rearms: u32) -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_read_rearm_handler(rearms))
}
pub fn rust_handler_new_readiness_probe() -> Box<RustHandlerOpaque> {
    boxed(ReadinessProbeHandler)
}
pub fn rust_handler_new_ready_rearm_bench() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::ReadyRearmBenchHandler)
}
pub fn rust_handler_new_panicking_lifecycle() -> Box<RustHandlerOpaque> {
    boxed(PanickingLifecycleHandler)
}
pub fn rust_handler_new_rearm_on_removed() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::RearmOnRemovedHandler)
}
pub fn rust_handler_new_identity() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_identity_handler())
}
pub fn rust_handler_new_allocation_probe() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_allocation_probe_handler())
}
pub fn rust_handler_new_copy_probe() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_copy_probe_handler())
}
pub fn rust_handler_new_close_probe() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_close_probe_handler())
}
pub fn rust_handler_new_state_machine(id: u32) -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_state_machine_handler(id))
}
pub fn rust_handler_new_panic_retention() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_panic_retention_handler())
}
pub fn rust_handler_new_exception_preserve() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_exception_preserve_handler())
}
pub fn rust_handler_new_reentrancy() -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_reentrancy_handler())
}
pub fn rust_handler_new_context_handle_test(scenario: u32) -> Box<RustHandlerOpaque> {
    boxed(crate::handler::new_context_handle_test_handler(scenario))
}
pub fn rust_handler_reset_test_counts() {
    crate::handler::reset_test_callback_counts();
}
pub fn rust_handler_reset_context_handle_test_slot() {
    crate::handler::reset_context_handle_test_slot();
}
pub fn rust_handler_context_handle_test_slot_len() -> u32 {
    crate::handler::context_handle_test_slot_len()
}
pub fn rust_handler_drop_one_context_handle_for_test() {
    crate::handler::drop_one_context_handle_for_test();
}
pub fn rust_handler_drop_all_context_handles_for_test() {
    crate::handler::drop_all_context_handles_for_test();
}
pub fn rust_handler_test_read_callbacks() -> u32 {
    crate::handler::test_read_callbacks()
}
pub fn rust_handler_test_write_callbacks() -> u32 {
    crate::handler::test_write_callbacks()
}
pub fn rust_handler_test_exception_callbacks() -> u32 {
    crate::handler::test_lifecycle_counts()[0]
}
pub fn rust_handler_test_read_ready_callbacks() -> u32 {
    crate::handler::test_lifecycle_counts()[1]
}
pub fn rust_handler_test_write_ready_callbacks() -> u32 {
    crate::handler::test_lifecycle_counts()[2]
}
pub fn rust_handler_test_added_callbacks() -> u32 {
    crate::handler::test_lifecycle_counts()[3]
}
pub fn rust_handler_test_active_callbacks() -> u32 {
    crate::handler::test_lifecycle_counts()[4]
}
pub fn rust_handler_test_inactive_callbacks() -> u32 {
    crate::handler::test_lifecycle_counts()[5]
}
pub fn rust_handler_test_removed_callbacks() -> u32 {
    crate::handler::test_lifecycle_counts()[6]
}
pub fn rust_handler_test_sequence() -> String {
    crate::handler::test_sequence()
}
pub fn rust_handler_test_probe_checks() -> u32 {
    crate::handler::test_probe_checks()
}
pub fn rust_handler_readiness_probe_check_count() -> u32 {
    crate::handler::READINESS_PROBE_CHECK_COUNT
}
pub fn rust_handler_phase3_handler_id() -> u64 {
    crate::handler::phase3_handler_id()
}
pub fn rust_handler_phase3_alloc_checks() -> u32 {
    crate::handler::phase3_alloc_checks()
}
pub fn rust_handler_phase3_copy_checks() -> u32 {
    crate::handler::phase3_copy_checks()
}
pub fn rust_handler_phase3_close_checks() -> u32 {
    crate::handler::phase3_close_checks()
}
pub fn rust_handler_allocation_probe_check_count() -> u32 {
    crate::handler::ALLOCATION_PROBE_CHECK_COUNT
}
pub fn rust_handler_copy_probe_check_count() -> u32 {
    crate::handler::COPY_PROBE_CHECK_COUNT
}
pub fn rust_handler_close_probe_check_count() -> u32 {
    crate::handler::CLOSE_PROBE_CHECK_COUNT
}
pub fn rust_handler_phase5_state() -> u32 {
    crate::handler::phase5_state()
}
pub fn rust_handler_phase5_p5_sequence() -> String {
    crate::handler::phase5_p5_sequence()
}
pub fn rust_handler_phase5_exception_preserved() -> u32 {
    crate::handler::phase5_exception_preserved()
}
pub fn rust_handler_phase5_reentrancy() -> u32 {
    crate::handler::phase5_reentrancy()
}
pub fn rust_handler_phase5_p5_counts() -> Vec<u32> {
    crate::handler::phase5_p5_counts().to_vec()
}
pub fn rust_handler_state_machine_stage_count() -> u32 {
    crate::handler::STATE_MACHINE_STAGE_COUNT
}

fn dispatch_erased(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
    msg: std::pin::Pin<&mut TypeErasedBox>,
    callback: impl FnOnce(
        &mut dyn RustHandler,
        &mut CallbackContext<'_>,
        RustTypeErasedBox<'_>,
    ) -> HandlerResult,
) -> i32 {
    // Empty box → Error (mirrors prior null check + empty check in C++ shim).
    if ffi::rust_teb_is_empty(msg.as_ref().get_ref()) {
        return HandlerResult::Error as i32;
    }
    // SAFETY: `AssertUnwindSafe` is sound here because:
    // (1) The closure is not resumed after a panic — `catch_unwind` catches
    //     and discards the panic payload; there is no resume point.
    // (2) `handler` may have partially mutated state after a Rust panic, but
    //     the pipeline immediately returns `Error` to C++ and does not
    //     re-invoke the handler with that intermediate state, so no memory-
    //     safety invariant is violated.
    // (3) `context` and `msg` are stack-scoped `Pin<&mut>` borrows that are
    //     not accessed after the closure exits (whether by normal return or
    //     by panic).
    // (4) `msg` borrow is consumed by the callback (take empties it); if the
    //     closure unwinds the box retains its original state.
    std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        callback(
            handler.inner.as_mut(),
            &mut CallbackContext::new(context),
            RustTypeErasedBox::new(msg),
        ) as i32
    }))
    .unwrap_or(HandlerResult::Error as i32)
}

pub fn rust_handler_on_read(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
    msg: std::pin::Pin<&mut TypeErasedBox>,
) -> i32 {
    dispatch_erased(handler, context, msg, |handler, context, msg| {
        handler.on_read(context, msg)
    })
}
pub fn rust_handler_on_write(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
    msg: std::pin::Pin<&mut TypeErasedBox>,
) -> i32 {
    dispatch_erased(handler, context, msg, |handler, context, msg| {
        handler.on_write(context, msg)
    })
}

fn contain_with_context(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
    callback: impl FnOnce(&mut dyn RustHandler, &mut CallbackContext<'_>),
) {
    // SAFETY: `AssertUnwindSafe` is sound here because:
    // (1) The closure is not resumed after a panic; `catch_unwind` catches and
    //     discards the panic payload with no resume.
    // (2) `handler` may have partially mutated state after a Rust panic, but
    //     the pipeline silently returns to C++ (these are void lifecycle
    //     callbacks), so no memory-safety invariant is violated.
    // (3) `context` is a stack-scoped `Pin<&mut>` that is not accessed after
    //     the closure exits (whether by normal return or by panic).
    let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        callback(handler.inner.as_mut(), &mut CallbackContext::new(context));
    }));
}
pub fn rust_handler_on_exception(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
) {
    contain_with_context(handler, context, |handler, context| {
        handler.on_exception(context)
    });
}
pub fn rust_handler_on_read_ready(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
) {
    contain_with_context(handler, context, |handler, context| {
        handler.on_read_ready(context)
    });
}
pub fn rust_handler_on_write_ready(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
) {
    contain_with_context(handler, context, |handler, context| {
        handler.on_write_ready(context)
    });
}
pub fn rust_handler_on_pipeline_active(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
) {
    contain_with_context(handler, context, |handler, context| {
        handler.on_pipeline_active(context);
    });
}
pub fn rust_handler_on_pipeline_inactive(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
) {
    contain_with_context(handler, context, |handler, context| {
        handler.on_pipeline_inactive(context);
    });
}
pub fn rust_handler_handler_added(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
) {
    contain_with_context(handler, context, |handler, context| {
        handler.handler_added(context)
    });
}
pub fn rust_handler_handler_removed(
    handler: &mut RustHandlerOpaque,
    context: std::pin::Pin<&mut ffi::FfiCallbackContext>,
) {
    contain_with_context(handler, context, |handler, context| {
        handler.handler_removed(context)
    });
}
