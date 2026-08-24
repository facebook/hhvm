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

//! Rust handler trait and result type for `channel_pipeline`.

use std::future::Future;
use std::pin::Pin;
use std::sync::Arc;
use std::sync::Mutex;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::AtomicU32;
use std::sync::atomic::AtomicU64;
use std::sync::atomic::Ordering;
use std::task::Context;
use std::task::Poll;
use std::task::Waker;

use crate::adapter::BytesPtr;
use crate::context::CallbackContext;
use crate::context::ContextHandle;
use crate::context::PipelineError;
use crate::coro_handler::CoroExceptionHandle;
use crate::coro_handler::CoroReadHandle;
use crate::coro_handler::CoroWriteHandle;
use crate::erased::RustTypeErasedBox;

/// FFI-stable result type mirroring C++ `channel_pipeline::Result`.
///
/// Discriminant values (`repr(i32)`) match the C++ enum exactly:
/// `Success=0`, `Backpressure=1`, `Error=2`. The mapping is verified at
/// compile time in the unit test below.
///
/// Data-path panics (in `on_read`/`on_write`) are caught at the FFI boundary
/// and mapped to `Error`. Any out-of-range discriminant received from the FFI
/// is also coerced to `Error`.
///
/// `Backpressure` signals that the current message was accepted but the
/// upstream should slow down. The C++ shim arms the corresponding readiness
/// hook when Rust returns `Backpressure`. `Error` signals that the operation
/// failed; the pipeline may close the connection.
#[repr(i32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HandlerResult {
    Success = 0,
    Backpressure = 1,
    Error = 2,
}

impl HandlerResult {
    pub(crate) fn from_ffi(value: i32) -> Self {
        match value {
            0 => Self::Success,
            1 => Self::Backpressure,
            _ => Self::Error,
        }
    }
}

/// A Rust handler in a `channel_pipeline` pipeline.
///
/// # Contract
///
/// All callbacks are invoked on the pipeline's EventBase thread, with an
/// exclusive borrowed [`CallbackContext`] that cannot escape the call (it is
/// `!Send`/`!Sync` and lifetime-bounded to the callback frame). The handler
/// owns its own state (mutable via `&mut self`) and produces a
/// [`HandlerResult`] from every data callback.
///
/// The default implementation for every method is a no-op pass-through:
/// data callbacks forward the buffer downstream, and lifecycle callbacks do
/// nothing. Override only the methods you need.
///
/// # Thread safety
///
/// `RustHandler` implementations must be `Send + 'static` so they can be
/// boxed and moved into the pipeline at construction time. Implementations are
/// NOT required to be `Sync`; each callback is invoked exclusively on the
/// EventBase thread with no concurrent access.
///
/// # Panic containment
///
/// Panics in any callback are caught at the FFI boundary via `catch_unwind`.
/// Data-path panics (`on_read`, `on_write`) map to [`HandlerResult::Error`].
/// Void callbacks (lifecycle, readiness, `on_exception`) swallow panics and
/// return normally to C++. After `on_exception`, the C++ shim always fires
/// the exception downstream regardless of what the Rust callback did.
///
/// # Async work
///
/// A callback may use [`CallbackContext::spawn`] to start a `Send + 'static`
/// Rust future on the pipeline's EventBase. The first poll runs inline; later
/// wakes return to that EventBase. Completion receives the existing move-only
/// [`ContextHandle`] so it can resume the pipeline from the captured position.
///
/// # Backpressure
///
/// Return [`HandlerResult::Backpressure`] to signal that the current message
/// was accepted but the upstream should slow down. The C++ shim automatically
/// arms the corresponding readiness hook when Rust returns `Backpressure`.
/// When the transport clears, the shim cancels the hook and invokes
/// `on_read_ready`/`on_write_ready` before the Rust call.
///
/// # Typed events — not exposed
///
/// Native C++ pipelines use typed events (write-completion chain, close
/// signals) via a separate `fireEvent` channel. These are not exposed to Rust
/// handlers today because no concrete Rust handler consumer uses
/// them. A future extension would add append-only event enum support while
/// preserving the `CallbackContext` `!Send` invariant.
pub trait RustHandler: Send + 'static {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        ctx.fire_read(m)
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        ctx.fire_write(m)
    }
    fn on_exception(&mut self, _ctx: &mut CallbackContext<'_>) {}
    fn on_read_ready(&mut self, _ctx: &mut CallbackContext<'_>) {}
    fn on_write_ready(&mut self, _ctx: &mut CallbackContext<'_>) {}
    fn on_pipeline_active(&mut self, _ctx: &mut CallbackContext<'_>) {}
    fn on_pipeline_inactive(&mut self, _ctx: &mut CallbackContext<'_>) {}
    fn handler_added(&mut self, _ctx: &mut CallbackContext<'_>) {}
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {}
}

/// A pass-through handler that forwards all data and ignores all lifecycle events.
///
/// All `RustHandler` default implementations are no-ops. `NoopHandler` simply
/// uses those defaults without overriding anything. Useful as a placeholder
/// during development, as a baseline for benchmarks, or as a base when only a
/// small subset of callbacks needs custom logic.
pub struct NoopHandler;
impl RustHandler for NoopHandler {}

// Phase 1/2 shared counters (intentionally not reset across threads without synchronization;
// tests using them must run single-threaded per Buck test harness)
static READS: AtomicU32 = AtomicU32::new(0);
static WRITES: AtomicU32 = AtomicU32::new(0);
static EXCEPTIONS: AtomicU32 = AtomicU32::new(0);
static READ_READY: AtomicU32 = AtomicU32::new(0);
static WRITE_READY: AtomicU32 = AtomicU32::new(0);
static ADDED: AtomicU32 = AtomicU32::new(0);
static ACTIVE: AtomicU32 = AtomicU32::new(0);
static INACTIVE: AtomicU32 = AtomicU32::new(0);
static REMOVED: AtomicU32 = AtomicU32::new(0);

pub(crate) struct CountingTestHandler;
pub(crate) struct BackpressureTestHandler;
pub(crate) struct ErrorTestHandler;
pub(crate) struct PanickingTestHandler;
pub(crate) struct LifecycleTestHandler;
pub(crate) struct ForwardingTestHandler;
pub(crate) struct ContextHandleTestHandler {
    scenario: u32,
}

static CONTEXT_HANDLE_TEST_SLOT: Mutex<Vec<ContextHandle>> = Mutex::new(Vec::new());

fn move_context_handle(handle: ContextHandle) -> ContextHandle {
    handle
}

/// A future that suspends until a worker thread wakes it.
///
/// The worker blocks on a waker handoff instead of polling a shared slot, so no
/// lock is needed: readiness cannot be published before the waker arrives, which
/// also removes the re-check a lock-based slot would require after storing.
struct WorkerWakeFuture {
    ready: Arc<AtomicBool>,
    wakers: std::sync::mpsc::Sender<Waker>,
}

impl WorkerWakeFuture {
    fn new() -> (Self, std::sync::mpsc::Receiver<()>) {
        let ready = Arc::new(AtomicBool::new(false));
        let worker_ready = Arc::clone(&ready);
        let (wakers, waker_receiver) = std::sync::mpsc::channel::<Waker>();
        let (woke, woke_receiver) = std::sync::mpsc::sync_channel(1);
        std::thread::spawn(move || {
            // A cancelled task drops the future and disconnects the channel;
            // there is nothing to wake in that case.
            let Ok(waker) = waker_receiver.recv() else {
                return;
            };
            worker_ready.store(true, Ordering::Release);
            waker.wake();
            woke.send(()).expect("wake observer should remain alive");
        });
        (Self { ready, wakers }, woke_receiver)
    }
}

impl Future for WorkerWakeFuture {
    type Output = ();

    fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
        if self.ready.load(Ordering::Acquire) {
            return Poll::Ready(());
        }
        self.wakers
            .send(context.waker().clone())
            .expect("worker should still be awaiting the waker");
        Poll::Pending
    }
}

impl RustHandler for ContextHandleTestHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        match self.scenario {
            0 => drop(ctx.context_handle()),
            1 => {
                let local = move_context_handle(ctx.context_handle());
                let (sender, receiver) = std::sync::mpsc::channel();
                sender
                    .send(local)
                    .expect("move-chain receiver should remain alive");
                drop(
                    receiver
                        .recv()
                        .expect("move-chain sender should provide the handle"),
                );
            }
            2 => {
                let handle = ctx.context_handle();
                std::thread::spawn(move || drop(handle))
                    .join()
                    .expect("off-thread ContextHandle drop should not panic");
            }
            3 | 5 => context_handle_test_slot().push(ctx.context_handle()),
            4 => {
                let mut slot = context_handle_test_slot();
                slot.push(ctx.context_handle());
                slot.push(ctx.context_handle());
            }
            6 => {
                let _handle = ctx.context_handle();
                panic!("intentional panic with a live ContextHandle");
            }
            7 => {
                let handle = move_context_handle(ctx.context_handle());
                drop(handle);
            }
            8 => ctx.context_handle().fire_read(msg.take::<BytesPtr>()),
            10 => {
                let handle = ctx.context_handle();
                let message = msg.take::<BytesPtr>();
                std::thread::spawn(move || handle.fire_read(message))
                    .join()
                    .expect("worker fire_read should not panic");
            }
            12 => {
                let handle = ctx.context_handle();
                let message = msg.take::<BytesPtr>();
                ctx.close();
                handle.fire_read(message);
            }
            14 => ctx
                .context_handle()
                .fire_exception(PipelineError::new("deferred exception \u{03bb}")),
            15 => {
                let handle = ctx.context_handle();
                std::thread::spawn(move || {
                    handle.fire_exception(PipelineError::new("deferred exception \u{03bb}"));
                })
                .join()
                .expect("worker fire_exception should not panic");
            }
            16 => {
                let handle = ctx.context_handle();
                ctx.close();
                handle.fire_exception(PipelineError::new("suppressed exception"));
            }
            17 => ctx.context_handle().fire_exception(PipelineError::new("")),
            18 => ctx
                .context_handle()
                .fire_exception(PipelineError::new("x".repeat(4096))),
            19 => {
                let handle = ctx.context_handle();
                std::thread::spawn(move || {
                    handle.fire_exception(PipelineError::new("queued before close"));
                })
                .join()
                .expect("worker fire_exception should enqueue before close");
                ctx.close();
            }
            20 => {
                let mut handler =
                    CoroReadHandle::<BytesPtr, _>::new(|message| async move { message });
                return handler.fire_read(ctx, msg);
            }
            21 => {
                let (wake, woke) = WorkerWakeFuture::new();
                let mut wake = Some(wake);
                let mut handler = CoroReadHandle::<BytesPtr, _>::new(move |message| {
                    let wake = wake.take().expect("coroutine method should run once");
                    async move {
                        wake.await;
                        message
                    }
                });
                let result = handler.fire_read(ctx, msg);
                woke.recv().expect("worker should issue the EventBase wake");
                return result;
            }
            26 => {
                let mut first_poll = true;
                let mut handler = CoroReadHandle::<BytesPtr, _>::new(move |message| {
                    let pending = first_poll;
                    first_poll = false;
                    async move {
                        if pending {
                            struct YieldOnce(bool);
                            impl Future for YieldOnce {
                                type Output = ();

                                fn poll(
                                    mut self: Pin<&mut Self>,
                                    context: &mut Context<'_>,
                                ) -> Poll<()> {
                                    if self.0 {
                                        Poll::Ready(())
                                    } else {
                                        self.0 = true;
                                        context.waker().wake_by_ref();
                                        Poll::Pending
                                    }
                                }
                            }
                            YieldOnce(false).await;
                        }
                        message
                    }
                });
                return handler.fire_read(ctx, msg);
            }
            27 | 28 => {
                let (wake, woke) = WorkerWakeFuture::new();
                let result = ctx.spawn_deferred_read(msg, wake, |deferred, ()| {
                    deferred.resume();
                });
                woke.recv().expect("worker should issue the EventBase wake");
                if self.scenario == 28 {
                    ctx.close();
                }
                return result;
            }
            29 => {
                let deferred = ctx
                    .defer_read(msg)
                    .expect("the callback should own one live inbound message");
                std::thread::spawn(move || drop(deferred))
                    .join()
                    .expect("off-thread DeferredRead cancellation should not panic");
            }
            24 => {
                drop(msg.take::<BytesPtr>());
                let mut handler = CoroExceptionHandle::new(|error| async move { error });
                return handler
                    .fire_exception(ctx, PipelineError::new("deferred exception \u{03bb}"));
            }
            25 => {
                drop(msg.take::<BytesPtr>());
                let (wake, woke) = WorkerWakeFuture::new();
                let mut wake = Some(wake);
                let mut handler = CoroExceptionHandle::new(move |error| {
                    let wake = wake.take().expect("coroutine method should run once");
                    async move {
                        wake.await;
                        error
                    }
                });
                let result =
                    handler.fire_exception(ctx, PipelineError::new("deferred exception \u{03bb}"));
                woke.recv().expect("worker should issue the EventBase wake");
                return result;
            }
            scenario => panic!("unknown ContextHandle test scenario {scenario}"),
        }
        HandlerResult::Success
    }

    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        match self.scenario {
            9 => ctx.context_handle().fire_write(msg.take::<BytesPtr>()),
            11 => {
                let handle = ctx.context_handle();
                let message = msg.take::<BytesPtr>();
                std::thread::spawn(move || handle.fire_write(message))
                    .join()
                    .expect("worker fire_write should not panic");
            }
            13 => {
                let handle = ctx.context_handle();
                let message = msg.take::<BytesPtr>();
                ctx.close();
                handle.fire_write(message);
            }
            20 | 21 => return ctx.fire_write(msg.take::<BytesPtr>()),
            22 => {
                let mut handler =
                    CoroWriteHandle::<BytesPtr, _>::new(|message| async move { message });
                return handler.fire_write(ctx, msg);
            }
            23 => {
                let (wake, woke) = WorkerWakeFuture::new();
                let mut wake = Some(wake);
                let mut handler = CoroWriteHandle::<BytesPtr, _>::new(move |message| {
                    let wake = wake.take().expect("coroutine method should run once");
                    async move {
                        wake.await;
                        message
                    }
                });
                let result = handler.fire_write(ctx, msg);
                woke.recv().expect("worker should issue the EventBase wake");
                return result;
            }
            scenario => panic!("unknown ContextHandle write test scenario {scenario}"),
        }
        HandlerResult::Success
    }

    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        REMOVED.fetch_add(1, Ordering::Relaxed);
    }
}

fn context_handle_test_slot() -> std::sync::MutexGuard<'static, Vec<ContextHandle>> {
    CONTEXT_HANDLE_TEST_SLOT
        .lock()
        .unwrap_or_else(|poisoned| poisoned.into_inner())
}

pub(crate) fn new_context_handle_test_handler(scenario: u32) -> ContextHandleTestHandler {
    ContextHandleTestHandler { scenario }
}

pub(crate) fn reset_context_handle_test_slot() {
    context_handle_test_slot().clear();
}

pub(crate) fn context_handle_test_slot_len() -> u32 {
    context_handle_test_slot()
        .len()
        .try_into()
        .expect("ContextHandle test slot length should fit in u32")
}

pub(crate) fn drop_one_context_handle_for_test() {
    drop(context_handle_test_slot().pop());
}

pub(crate) fn drop_all_context_handles_for_test() {
    context_handle_test_slot().clear();
}

impl RustHandler for CountingTestHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        READS.fetch_add(1, Ordering::Relaxed);
        ctx.fire_read(m)
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        WRITES.fetch_add(1, Ordering::Relaxed);
        ctx.fire_write(m)
    }
}

impl RustHandler for ForwardingTestHandler {
    // Forwards the message downstream WITHOUT taking/inspecting it — the
    // "forward what you don't understand" pass-through, via RustTypeErasedBox.
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        ctx.forward_read(msg)
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        ctx.forward_write(msg)
    }
}

impl RustHandler for BackpressureTestHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
}

impl RustHandler for ErrorTestHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Error
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Error
    }
}

impl RustHandler for PanickingTestHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        panic!("intentional Rust handler read panic")
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        panic!("intentional Rust handler write panic")
    }
}

impl RustHandler for LifecycleTestHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_exception(&mut self, _ctx: &mut CallbackContext<'_>) {
        EXCEPTIONS.fetch_add(1, Ordering::Relaxed);
    }
    fn on_read_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        READ_READY.fetch_add(1, Ordering::Relaxed);
    }
    fn on_write_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        WRITE_READY.fetch_add(1, Ordering::Relaxed);
    }
    fn handler_added(&mut self, _ctx: &mut CallbackContext<'_>) {
        ADDED.fetch_add(1, Ordering::Relaxed);
    }
    fn on_pipeline_active(&mut self, _ctx: &mut CallbackContext<'_>) {
        ACTIVE.fetch_add(1, Ordering::Relaxed);
    }
    fn on_pipeline_inactive(&mut self, _ctx: &mut CallbackContext<'_>) {
        INACTIVE.fetch_add(1, Ordering::Relaxed);
    }
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        REMOVED.fetch_add(1, Ordering::Relaxed);
    }
}

// Shared ordering sequence for Phase 2 lifecycle order tests.
// Phase 5 avoids this sequence to prevent parallel-test pollution.
static SEQUENCE: Mutex<String> = Mutex::new(String::new());
static PROBE_CHECKS: AtomicU32 = AtomicU32::new(0);

fn record(event: &str, id: u32) {
    let mut guard = SEQUENCE.lock().unwrap_or_else(|p| p.into_inner());
    if !guard.is_empty() {
        guard.push(',');
    }
    guard.push_str(event);
    guard.push('#');
    guard.push_str(&id.to_string());
}

fn record_event(event: &str) {
    let mut guard = SEQUENCE.lock().unwrap_or_else(|p| p.into_inner());
    if !guard.is_empty() {
        guard.push(',');
    }
    guard.push_str(event);
}

pub(crate) struct LifecycleOrderHandler {
    id: u32,
}

impl RustHandler for LifecycleOrderHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        record("read", self.id);
        ctx.fire_read(m)
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        record("write", self.id);
        ctx.fire_write(m)
    }
    fn on_exception(&mut self, _ctx: &mut CallbackContext<'_>) {
        record("exception", self.id);
    }
    fn on_pipeline_active(&mut self, _ctx: &mut CallbackContext<'_>) {
        record("active", self.id);
    }
    fn on_pipeline_inactive(&mut self, _ctx: &mut CallbackContext<'_>) {
        record("inactive", self.id);
    }
    fn handler_added(&mut self, _ctx: &mut CallbackContext<'_>) {
        record("added", self.id);
    }
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        record("removed", self.id);
    }
}

pub(crate) struct RecoveringReadHandler {
    blocked: bool,
}

impl RustHandler for RecoveringReadHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        if self.blocked {
            record_event("read_bp");
            return HandlerResult::Backpressure;
        }
        record_event("read_ok");
        ctx.fire_read(m)
    }
    fn on_read_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        record_event("read_ready");
        self.blocked = false;
    }
    fn on_write_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        record_event("unexpected_write_ready");
    }
}

pub(crate) struct RecoveringWriteHandler {
    blocked: bool,
}

impl RustHandler for RecoveringWriteHandler {
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        if self.blocked {
            record_event("write_bp");
            return HandlerResult::Backpressure;
        }
        record_event("write_ok");
        ctx.fire_write(m)
    }
    fn on_write_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        record_event("write_ready");
        self.blocked = false;
    }
    fn on_read_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        record_event("unexpected_read_ready");
    }
}

pub(crate) struct BidirectionalHandler {
    read_blocked: bool,
    write_blocked: bool,
}

impl RustHandler for BidirectionalHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        if self.read_blocked {
            return HandlerResult::Backpressure;
        }
        ctx.fire_read(m)
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        if self.write_blocked {
            return HandlerResult::Backpressure;
        }
        ctx.fire_write(m)
    }
    fn on_read_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        record_event("read_ready");
        self.read_blocked = false;
    }
    fn on_write_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        record_event("write_ready");
        self.write_blocked = false;
    }
}

pub(crate) struct ReadRearmHandler {
    rearms_left: u32,
}

impl RustHandler for ReadRearmHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_read_ready(&mut self, ctx: &mut CallbackContext<'_>) {
        record_event("read_ready");
        if self.rearms_left > 0 {
            self.rearms_left -= 1;
            ctx.await_read_ready();
        }
    }
}

pub(crate) struct ReadinessProbeHandler;

impl RustHandler for ReadinessProbeHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let mut checks = 0u32;
        if !ctx.is_awaiting_read_ready() {
            checks += 1;
        }
        ctx.await_read_ready();
        if ctx.is_awaiting_read_ready() {
            checks += 1;
        }
        ctx.await_read_ready();
        if ctx.is_awaiting_read_ready() {
            checks += 1;
        }
        if !ctx.is_awaiting_write_ready() {
            checks += 1;
        }
        ctx.await_write_ready();
        if ctx.is_awaiting_write_ready() {
            checks += 1;
        }
        ctx.cancel_read_ready();
        if !ctx.is_awaiting_read_ready() {
            checks += 1;
        }
        if ctx.is_awaiting_write_ready() {
            checks += 1;
        }
        ctx.cancel_read_ready();
        if !ctx.is_awaiting_read_ready() {
            checks += 1;
        }
        ctx.cancel_write_ready();
        if !ctx.is_awaiting_write_ready() {
            checks += 1;
        }
        ctx.cancel_write_ready();
        if !ctx.is_awaiting_write_ready() {
            checks += 1;
        }
        PROBE_CHECKS.store(checks, Ordering::Relaxed);
        HandlerResult::Success
    }
}

pub(crate) const READINESS_PROBE_CHECK_COUNT: u32 = 10;

pub(crate) struct ReadyRearmBenchHandler;

impl RustHandler for ReadyRearmBenchHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_read_ready(&mut self, ctx: &mut CallbackContext<'_>) {
        ctx.await_read_ready();
    }
    fn on_write_ready(&mut self, ctx: &mut CallbackContext<'_>) {
        ctx.await_write_ready();
    }
}

pub(crate) struct PanickingLifecycleHandler;

pub(crate) struct RearmOnRemovedHandler;

impl RustHandler for RearmOnRemovedHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn handler_removed(&mut self, ctx: &mut CallbackContext<'_>) {
        ctx.await_read_ready();
        ctx.await_write_ready();
        REMOVED.fetch_add(1, Ordering::Relaxed);
    }
}

impl RustHandler for PanickingLifecycleHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        HandlerResult::Backpressure
    }
    fn on_read_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional Rust read-ready panic")
    }
    fn on_write_ready(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional Rust write-ready panic")
    }
    fn on_exception(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional Rust exception panic")
    }
    fn on_pipeline_active(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional Rust active panic")
    }
    fn on_pipeline_inactive(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional Rust inactive panic")
    }
    fn handler_added(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional Rust added panic")
    }
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional Rust removed panic")
    }
}

static PHASE3_HANDLER_ID: AtomicU64 = AtomicU64::new(0);
static PHASE3_ALLOC_CHECKS: AtomicU32 = AtomicU32::new(0);
static PHASE3_COPY_CHECKS: AtomicU32 = AtomicU32::new(0);
static PHASE3_CLOSE_CHECKS: AtomicU32 = AtomicU32::new(0);

pub(crate) struct IdentityHandler;

impl RustHandler for IdentityHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        PHASE3_HANDLER_ID.store(ctx.handler_id(), Ordering::Relaxed);
        ctx.fire_read(m)
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        PHASE3_HANDLER_ID.store(ctx.handler_id(), Ordering::Relaxed);
        HandlerResult::Success
    }
    fn handler_added(&mut self, ctx: &mut CallbackContext<'_>) {
        PHASE3_HANDLER_ID.store(ctx.handler_id(), Ordering::Relaxed);
    }
}

pub(crate) struct AllocationProbeHandler;

impl RustHandler for AllocationProbeHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let mut ok = 0u32;
        if let Some(buf) = ctx.allocate(0) {
            let _ = buf.chain_data_len();
            ok += 1;
        }
        if let Some(buf) = ctx.allocate(1) {
            ok += 1;
            let _ = buf.chain_data_len();
        }
        if let Some(buf) = ctx.allocate(64) {
            ok += 1;
            let _ = buf.chain_data_len();
        }
        if let Some(buf) = ctx.allocate(65536) {
            ok += 1;
            let _ = buf.chain_data_len();
        }
        if let Some(buf) = ctx.copy_from_slice(&[]) {
            ok += 1;
            let _ = buf.chain_data_len();
        }
        if let Some(buf) = ctx.copy_from_slice(&[0xde, 0xad, 0xbe, 0xef]) {
            ok += 1;
            let _ = buf.first_chunk();
        }
        PHASE3_ALLOC_CHECKS.store(ok, Ordering::Relaxed);
        HandlerResult::Success
    }
}
pub(crate) const ALLOCATION_PROBE_CHECK_COUNT: u32 = 6;

pub(crate) struct CopyProbeHandler;

impl RustHandler for CopyProbeHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        let mut ok = 0u32;
        let inbound_len = m.chain_data_len();
        if inbound_len > 0 {
            ok += 1;
        }
        let original = [1u8, 2, 3, 4, 5];
        let deep = ctx.copy_from_slice(&original);
        if let Some(ref buf) = deep {
            if buf.chain_data_len() == original.len() {
                ok += 1;
            }
            if !buf.first_chunk().is_empty() && buf.first_chunk()[0] == 1 {
                ok += 1;
            }
        }
        if let Some(ref buf) = deep {
            if let Some(cloned) = ctx.clone_chain(buf) {
                if cloned.chain_data_len() == buf.chain_data_len() {
                    ok += 1;
                }
            }
            if let Some(coalesced) = ctx.coalesced_copy(buf) {
                if coalesced.chain_data_len() == buf.chain_data_len() {
                    ok += 1;
                }
                if coalesced.chain_element_count() == 1 {
                    ok += 1;
                }
            }
            if let Some(one) = ctx.clone_one(buf) {
                if one.chain_data_len() == buf.chain_data_len() {
                    ok += 1;
                }
                if one.chain_element_count() == 1 {
                    ok += 1;
                }
            }
        }
        if let Some(zero) = ctx.copy_from_slice(&[]) {
            let _ = zero.chain_data_len();
            ok += 1;
        }
        PHASE3_COPY_CHECKS.store(ok, Ordering::Relaxed);
        ctx.fire_read(m)
    }
}
pub(crate) const COPY_PROBE_CHECK_COUNT: u32 = 9;

pub(crate) struct CloseProbeHandler;

impl RustHandler for CloseProbeHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        let mut ok = 0u32;
        if !ctx.is_closed() {
            ok += 1;
        }
        if ctx.allocate(16).is_some() {
            ok += 1;
        }
        if ctx.copy_from_slice(b"pre-close").is_some() {
            ok += 1;
        }
        ctx.close();
        if ctx.is_closed() {
            ok += 1;
        }
        if ctx.allocate(8).is_some() {
            ok += 1;
        }
        if ctx.copy_from_slice(b"post-close").is_some() {
            ok += 1;
        }
        ctx.close();
        if ctx.is_closed() {
            ok += 1;
        }
        let fw = ctx.fire_read(m);
        if fw == HandlerResult::Success {
            ok += 1;
        }
        PHASE3_CLOSE_CHECKS.store(ok, Ordering::Relaxed);
        HandlerResult::Success
    }
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        REMOVED.fetch_add(1, Ordering::Relaxed);
    }
}
pub(crate) const CLOSE_PROBE_CHECK_COUNT: u32 = 8;

// ── Phase 5: dedicated atomics, no global SEQUENCE interference ─────────────

static PHASE5_STATE: Mutex<u32> = Mutex::new(0);
static PHASE5_EXCEPTION_PRESERVED: AtomicU32 = AtomicU32::new(0);
static PHASE5_REENTRANCY: AtomicU32 = AtomicU32::new(0);
static PHASE5_P5_ADDED: AtomicU32 = AtomicU32::new(0);
static PHASE5_P5_ACTIVE: AtomicU32 = AtomicU32::new(0);
static PHASE5_P5_READ: AtomicU32 = AtomicU32::new(0);
static PHASE5_P5_WRITE: AtomicU32 = AtomicU32::new(0);
static PHASE5_P5_INACTIVE: AtomicU32 = AtomicU32::new(0);
static PHASE5_P5_REMOVED: AtomicU32 = AtomicU32::new(0);
static PHASE5_P5_SEQUENCE: Mutex<String> = Mutex::new(String::new());

fn p5_record(event: &str, id: u32) {
    let mut guard = PHASE5_P5_SEQUENCE.lock().unwrap_or_else(|p| p.into_inner());
    if !guard.is_empty() {
        guard.push(',');
    }
    guard.push_str(event);
    guard.push('#');
    guard.push_str(&id.to_string());
}

pub(crate) struct StateMachineHandler {
    id: u32,
}

impl RustHandler for StateMachineHandler {
    fn handler_added(&mut self, _ctx: &mut CallbackContext<'_>) {
        if let Ok(mut bits) = PHASE5_STATE.lock() {
            *bits |= 1;
        }
        PHASE5_P5_ADDED.fetch_add(1, Ordering::Relaxed);
        p5_record("added", self.id);
    }
    fn on_pipeline_active(&mut self, _ctx: &mut CallbackContext<'_>) {
        if let Ok(mut bits) = PHASE5_STATE.lock() {
            *bits |= 2;
        }
        PHASE5_P5_ACTIVE.fetch_add(1, Ordering::Relaxed);
        p5_record("active", self.id);
    }
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        PHASE5_P5_READ.fetch_add(1, Ordering::Relaxed);
        p5_record("read", self.id);
        let r = ctx.fire_read(m);
        if r == HandlerResult::Success {
            if let Ok(mut bits) = PHASE5_STATE.lock() {
                *bits |= 4;
            }
        }
        HandlerResult::Success
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        PHASE5_P5_WRITE.fetch_add(1, Ordering::Relaxed);
        p5_record("write", self.id);
        let r = ctx.fire_write(m);
        if r == HandlerResult::Success {
            if let Ok(mut bits) = PHASE5_STATE.lock() {
                *bits |= 8;
            }
        }
        HandlerResult::Success
    }
    fn on_pipeline_inactive(&mut self, _ctx: &mut CallbackContext<'_>) {
        if let Ok(mut bits) = PHASE5_STATE.lock() {
            *bits |= 16;
        }
        PHASE5_P5_INACTIVE.fetch_add(1, Ordering::Relaxed);
        p5_record("inactive", self.id);
    }
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        if let Ok(mut bits) = PHASE5_STATE.lock() {
            *bits |= 32;
        }
        PHASE5_P5_REMOVED.fetch_add(1, Ordering::Relaxed);
        p5_record("removed", self.id);
    }
}
pub(crate) const STATE_MACHINE_STAGE_COUNT: u32 = 6;

pub(crate) struct PanicRetentionHandler;

impl RustHandler for PanicRetentionHandler {
    fn on_read(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        panic!("intentional read panic with borrowed ctx")
    }
    fn on_write(
        &mut self,
        _ctx: &mut CallbackContext<'_>,
        _msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        panic!("intentional write panic with borrowed ctx")
    }
    // Lifecycle panics contained via contain_with_context — prove non-escape.
    fn on_pipeline_active(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional active panic with borrowed ctx")
    }
    fn on_pipeline_inactive(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional inactive panic with borrowed ctx")
    }
    fn handler_added(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional added panic with borrowed ctx")
    }
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional removed panic with borrowed ctx")
    }
    fn on_exception(&mut self, _ctx: &mut CallbackContext<'_>) {
        panic!("intentional exception panic with borrowed ctx")
    }
}

pub(crate) struct ExceptionPreserveHandler;

impl RustHandler for ExceptionPreserveHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        ctx.fire_read(m)
    }
    fn on_exception(&mut self, _ctx: &mut CallbackContext<'_>) {
        PHASE5_EXCEPTION_PRESERVED.fetch_add(1, Ordering::Relaxed);
    }
}

pub(crate) struct ReentrancyHandler;

impl RustHandler for ReentrancyHandler {
    fn handler_added(&mut self, _ctx: &mut CallbackContext<'_>) {
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        PHASE5_P5_ADDED.fetch_add(1, Ordering::Relaxed);
    }
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        PHASE5_P5_READ.fetch_add(1, Ordering::Relaxed);
        ctx.fire_read(m)
    }
    fn on_write(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        let m = msg.take::<BytesPtr>();
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        PHASE5_P5_WRITE.fetch_add(1, Ordering::Relaxed);
        ctx.fire_write(m)
    }
    fn on_read_ready(&mut self, ctx: &mut CallbackContext<'_>) {
        // Reentrant re-arm inside ready callback — must stay alloc-free, one-shot per gen.
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        ctx.await_read_ready();
    }
    fn on_write_ready(&mut self, ctx: &mut CallbackContext<'_>) {
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        ctx.await_write_ready();
    }
    fn on_pipeline_active(&mut self, _ctx: &mut CallbackContext<'_>) {
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        PHASE5_P5_ACTIVE.fetch_add(1, Ordering::Relaxed);
    }
    fn on_pipeline_inactive(&mut self, _ctx: &mut CallbackContext<'_>) {
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        PHASE5_P5_INACTIVE.fetch_add(1, Ordering::Relaxed);
    }
    fn handler_removed(&mut self, _ctx: &mut CallbackContext<'_>) {
        PHASE5_REENTRANCY.fetch_add(1, Ordering::Relaxed);
        PHASE5_P5_REMOVED.fetch_add(1, Ordering::Relaxed);
    }
}

pub(crate) fn new_state_machine_handler(id: u32) -> StateMachineHandler {
    StateMachineHandler { id }
}
pub(crate) fn new_panic_retention_handler() -> PanicRetentionHandler {
    PanicRetentionHandler
}
pub(crate) fn new_exception_preserve_handler() -> ExceptionPreserveHandler {
    ExceptionPreserveHandler
}
pub(crate) fn new_reentrancy_handler() -> ReentrancyHandler {
    ReentrancyHandler
}

pub(crate) fn new_identity_handler() -> IdentityHandler {
    IdentityHandler
}
pub(crate) fn new_allocation_probe_handler() -> AllocationProbeHandler {
    AllocationProbeHandler
}
pub(crate) fn new_copy_probe_handler() -> CopyProbeHandler {
    CopyProbeHandler
}
pub(crate) fn new_close_probe_handler() -> CloseProbeHandler {
    CloseProbeHandler
}

pub(crate) fn phase3_handler_id() -> u64 {
    PHASE3_HANDLER_ID.load(Ordering::Relaxed)
}
pub(crate) fn phase3_alloc_checks() -> u32 {
    PHASE3_ALLOC_CHECKS.load(Ordering::Relaxed)
}
pub(crate) fn phase3_copy_checks() -> u32 {
    PHASE3_COPY_CHECKS.load(Ordering::Relaxed)
}
pub(crate) fn phase3_close_checks() -> u32 {
    PHASE3_CLOSE_CHECKS.load(Ordering::Relaxed)
}

pub(crate) fn phase5_state() -> u32 {
    PHASE5_STATE.lock().map(|g| *g).unwrap_or(0)
}
pub(crate) fn phase5_p5_sequence() -> String {
    PHASE5_P5_SEQUENCE
        .lock()
        .map(|g| g.clone())
        .unwrap_or_else(|p| p.into_inner().clone())
}
pub(crate) fn phase5_exception_preserved() -> u32 {
    PHASE5_EXCEPTION_PRESERVED.load(Ordering::Relaxed)
}
pub(crate) fn phase5_reentrancy() -> u32 {
    PHASE5_REENTRANCY.load(Ordering::Relaxed)
}
pub(crate) fn phase5_p5_counts() -> [u32; 6] {
    [
        PHASE5_P5_ADDED.load(Ordering::Relaxed),
        PHASE5_P5_ACTIVE.load(Ordering::Relaxed),
        PHASE5_P5_READ.load(Ordering::Relaxed),
        PHASE5_P5_WRITE.load(Ordering::Relaxed),
        PHASE5_P5_INACTIVE.load(Ordering::Relaxed),
        PHASE5_P5_REMOVED.load(Ordering::Relaxed),
    ]
}

pub(crate) fn new_lifecycle_order_handler(id: u32) -> LifecycleOrderHandler {
    LifecycleOrderHandler { id }
}
pub(crate) fn new_recovering_read_handler() -> RecoveringReadHandler {
    RecoveringReadHandler { blocked: true }
}
pub(crate) fn new_recovering_write_handler() -> RecoveringWriteHandler {
    RecoveringWriteHandler { blocked: true }
}
pub(crate) fn new_bidirectional_handler() -> BidirectionalHandler {
    BidirectionalHandler {
        read_blocked: true,
        write_blocked: true,
    }
}
pub(crate) fn new_read_rearm_handler(rearms: u32) -> ReadRearmHandler {
    ReadRearmHandler {
        rearms_left: rearms,
    }
}

pub(crate) fn test_sequence() -> String {
    SEQUENCE
        .lock()
        .map(|g| g.clone())
        .unwrap_or_else(|p| p.into_inner().clone())
}
pub(crate) fn test_probe_checks() -> u32 {
    PROBE_CHECKS.load(Ordering::Relaxed)
}

pub(crate) fn reset_test_callback_counts() {
    for counter in [
        &READS,
        &WRITES,
        &EXCEPTIONS,
        &READ_READY,
        &WRITE_READY,
        &ADDED,
        &ACTIVE,
        &INACTIVE,
        &REMOVED,
    ] {
        counter.store(0, Ordering::Relaxed);
    }
    PROBE_CHECKS.store(0, Ordering::Relaxed);
    PHASE3_HANDLER_ID.store(0, Ordering::Relaxed);
    PHASE3_ALLOC_CHECKS.store(0, Ordering::Relaxed);
    PHASE3_COPY_CHECKS.store(0, Ordering::Relaxed);
    PHASE3_CLOSE_CHECKS.store(0, Ordering::Relaxed);
    match PHASE5_STATE.lock() {
        Ok(mut b) => *b = 0,
        Err(p) => *p.into_inner() = 0,
    }
    match PHASE5_P5_SEQUENCE.lock() {
        Ok(mut s) => s.clear(),
        Err(p) => p.into_inner().clear(),
    }
    PHASE5_EXCEPTION_PRESERVED.store(0, Ordering::Relaxed);
    PHASE5_REENTRANCY.store(0, Ordering::Relaxed);
    PHASE5_P5_ADDED.store(0, Ordering::Relaxed);
    PHASE5_P5_ACTIVE.store(0, Ordering::Relaxed);
    PHASE5_P5_READ.store(0, Ordering::Relaxed);
    PHASE5_P5_WRITE.store(0, Ordering::Relaxed);
    PHASE5_P5_INACTIVE.store(0, Ordering::Relaxed);
    PHASE5_P5_REMOVED.store(0, Ordering::Relaxed);
    match SEQUENCE.lock() {
        Ok(mut s) => s.clear(),
        Err(p) => p.into_inner().clear(),
    }
}

pub(crate) fn test_read_callbacks() -> u32 {
    READS.load(Ordering::Relaxed)
}
pub(crate) fn test_write_callbacks() -> u32 {
    WRITES.load(Ordering::Relaxed)
}
pub(crate) fn test_lifecycle_counts() -> [u32; 7] {
    [
        EXCEPTIONS.load(Ordering::Relaxed),
        READ_READY.load(Ordering::Relaxed),
        WRITE_READY.load(Ordering::Relaxed),
        ADDED.load(Ordering::Relaxed),
        ACTIVE.load(Ordering::Relaxed),
        INACTIVE.load(Ordering::Relaxed),
        REMOVED.load(Ordering::Relaxed),
    ]
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn handler_result_discriminants_match_cpp() {
        assert_eq!(HandlerResult::Success as i32, 0);
        assert_eq!(HandlerResult::Backpressure as i32, 1);
        assert_eq!(HandlerResult::Error as i32, 2);
        assert_eq!(HandlerResult::from_ffi(99), HandlerResult::Error);
    }
}
