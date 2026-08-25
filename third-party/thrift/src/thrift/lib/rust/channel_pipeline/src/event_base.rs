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

//! Future-to-EventBase bridge for channel-pipeline coroutines.
//!
//! # Pin contract
//!
//! A future may be self-referential once polled: an `async` block's state
//! machine can hold interior references into its own frame across an `await`.
//! `Pin` therefore requires the address used for the first `poll` to be the
//! address used for every later `poll` and for the eventual drop.
//!
//! [`TaskCell`] is that address. The future is moved into the cell before it is
//! polled at all, so the first poll — which still runs inline in the pipeline
//! callback — already observes the future's final location, and the future is
//! never relocated afterwards whether it completes inline or suspends.
//!
//! Honoring that contract costs one allocation per spawn: readiness is only
//! known after a poll, and the poll may not happen anywhere but the future's
//! final home. A future that completes on its first poll therefore still
//! allocates its cell. That cell is a single allocation holding the task state,
//! the scheduler, the future, the completion, and the payload inline; wakers,
//! completions, and scheduling add no further allocation.

use std::cell::UnsafeCell;
use std::future::Future;
use std::mem::MaybeUninit;
use std::panic::AssertUnwindSafe;
use std::pin::Pin;
use std::sync::Arc;
use std::sync::atomic::AtomicU8;
use std::sync::atomic::Ordering;
use std::task::Context;
use std::task::Poll;
use std::task::RawWaker;
use std::task::RawWakerVTable;
use std::task::Waker;
use std::thread::Result as UnwindResult;

use crate::ffi::ffi;

/// A non-`Send` future confined to its originating EventBase.
struct EventBaseLocalFuture<F>(F);

// SAFETY: this wrapper is constructed only by `EventBaseTask::start_local`.
// The task's first poll runs on the originating EventBase, and every later
// poll or cancellation is delivered by `EventBaseScheduler` on that same
// EventBase. Cross-thread wakers touch only the task cell's atomic state and
// scheduler; they never access or destroy the wrapped future.
unsafe impl<F> Send for EventBaseLocalFuture<F> {}

impl<F: Future> Future for EventBaseLocalFuture<F> {
    type Output = F::Output;

    fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
        // SAFETY: pinning the wrapper also pins its `future` field, which is
        // never moved before its destructor runs.
        unsafe { self.map_unchecked_mut(|this| &mut this.0) }.poll(context)
    }
}

/// A wake arrived and no poll has serviced it yet.
const NOTIFIED: u8 = 1 << 0;
/// The payload is installed: the task is schedulable and owns one reference.
const ARMED: u8 = 1 << 1;
/// A [`TaskToken`] for this task is queued and owns one reference.
const SCHEDULED: u8 = 1 << 2;
/// A scheduled poll is in progress on the EventBase.
const RUNNING: u8 = 1 << 3;
/// Terminal: the future, completion, and payload slots have been consumed.
const CLOSED: u8 = 1 << 4;

pub(crate) struct TaskToken {
    task: Option<usize>,
    run: fn(usize),
    cancel: fn(usize),
}

impl TaskToken {
    fn new(task: usize, run: fn(usize), cancel: fn(usize)) -> Self {
        Self {
            task: Some(task),
            run,
            cancel,
        }
    }

    #[cfg(test)]
    fn run(mut self) {
        (self.run)(self.task.take().expect("task token must own a task"));
    }

    fn into_parts(mut self) -> (usize, fn(usize), fn(usize)) {
        (
            self.task.take().expect("task token must own a task"),
            self.run,
            self.cancel,
        )
    }
}

impl Drop for TaskToken {
    fn drop(&mut self) {
        if let Some(task) = self.task.take() {
            (self.cancel)(task);
        }
    }
}

pub(crate) trait Scheduler: Send + Sync + 'static {
    fn enqueue(&self, task: TaskToken);
}

impl<F> Scheduler for F
where
    F: Fn(TaskToken) + Send + Sync + 'static,
{
    fn enqueue(&self, task: TaskToken) {
        self(task);
    }
}

pub(crate) struct EventBaseScheduler {
    event_base: usize,
}

impl Scheduler for EventBaseScheduler {
    fn enqueue(&self, task: TaskToken) {
        let (task, run, cancel) = task.into_parts();
        // SAFETY: native code consumes the task's Arc reference exactly once
        // through either run or cancel.
        unsafe {
            ffi::enqueue_in_event_base(self.event_base as *mut ffi::EventBase, task, run, cancel);
        }
    }
}

/// The task's single allocation and the future's permanent address.
///
/// `future` and `complete` are initialized when the cell is created, before any
/// poll. `payload` is initialized by [`TaskCell::arm`]. Whichever call wins the
/// [`CLOSED`] transition consumes all initialized slots exactly once.
struct TaskCell<F, C, P, S> {
    state: AtomicU8,
    scheduler: S,
    future: UnsafeCell<MaybeUninit<F>>,
    complete: UnsafeCell<MaybeUninit<C>>,
    payload: UnsafeCell<MaybeUninit<P>>,
}

// SAFETY: worker threads access only state, scheduler, and Arc counts. Future,
// completion, and payload slots are installed and consumed on the EventBase.
unsafe impl<F: Send, C: Send, P: Send, S: Send> Send for TaskCell<F, C, P, S> {}
// SAFETY: slot access is EventBase-confined and atomic state serializes
// publication, polling, completion, and cancellation.
unsafe impl<F: Send, C: Send, P: Send, S: Sync> Sync for TaskCell<F, C, P, S> {}

impl<F, C, P, S> TaskCell<F, C, P, S>
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    /// Allocate the cell and move the future into its permanent address.
    fn create(scheduler: S, future: F, complete: C) -> Arc<Self> {
        Arc::new(Self {
            state: AtomicU8::new(0),
            scheduler,
            future: UnsafeCell::new(MaybeUninit::new(future)),
            complete: UnsafeCell::new(MaybeUninit::new(complete)),
            payload: UnsafeCell::new(MaybeUninit::uninit()),
        })
    }

    /// Poll the future where it lives, containing any panic.
    ///
    /// The caller must hold exclusive access to the future slot: either the
    /// pre-arm phase on the spawning callback, or [`RUNNING`] on the EventBase.
    fn poll_future(task: &Arc<Self>) -> UnwindResult<Poll<F::Output>> {
        let raw_waker = RawWaker::new(Arc::as_ptr(task).cast(), borrowed_vtable::<F, C, P, S>());
        // SAFETY: the caller's Arc keeps the cell live for the whole poll, and
        // cloning promotes the borrowed waker to an owned Arc reference.
        let waker = unsafe { Waker::from_raw(raw_waker) };
        let mut context = Context::from_waker(&waker);
        std::panic::catch_unwind(AssertUnwindSafe(|| {
            // SAFETY: the future was moved into this stable allocation before
            // any poll and is never relocated, so pinning it here upholds the
            // Pin contract. Exclusive access is the caller's precondition.
            let future = unsafe { (*task.future.get()).assume_init_mut() };
            unsafe { Pin::new_unchecked(future) }.poll(&mut context)
        }))
    }

    /// Install the payload and make the suspended task schedulable.
    ///
    /// Consumes `task`: that reference becomes the armed task lifetime, which
    /// [`TaskCell::finish_ready`] or [`TaskCell::close`] releases.
    fn arm(task: Arc<Self>, payload: P) {
        // SAFETY: arm runs once on the EventBase before ARMED is published, so
        // nothing can observe the payload slot before this write.
        unsafe { (*task.payload.get()).write(payload) };
        let previous = task.state.fetch_or(ARMED, Ordering::Release);
        debug_assert_eq!(previous & (ARMED | SCHEDULED | RUNNING | CLOSED), 0);
        task.schedule_if_notified();
        std::mem::forget(task);
    }

    fn schedule_if_notified(&self) {
        let mut state = self.state.load(Ordering::Acquire);
        loop {
            if state & NOTIFIED == 0
                || state & ARMED == 0
                || state & (SCHEDULED | RUNNING | CLOSED) != 0
            {
                return;
            }
            let next = state | SCHEDULED;
            match self
                .state
                .compare_exchange_weak(state, next, Ordering::AcqRel, Ordering::Acquire)
            {
                Ok(_) => {
                    self.enqueue_ref();
                    return;
                }
                Err(current) => state = current,
            }
        }
    }

    fn notify(&self) {
        let mut state = self.state.load(Ordering::Acquire);
        loop {
            if state & CLOSED != 0 {
                return;
            }
            let should_schedule = state & ARMED != 0 && state & (SCHEDULED | RUNNING) == 0;
            let mut next = state | NOTIFIED;
            if should_schedule {
                next |= SCHEDULED;
            }
            match self
                .state
                .compare_exchange_weak(state, next, Ordering::AcqRel, Ordering::Acquire)
            {
                Ok(_) => {
                    if should_schedule {
                        self.enqueue_ref();
                    }
                    return;
                }
                Err(current) => state = current,
            }
        }
    }

    fn enqueue_ref(&self) {
        let pointer = self as *const Self;
        // SAFETY: self is owned by at least the caller's Arc or the armed task
        // lifetime reference. The increment is transferred to TaskToken.
        unsafe { Arc::increment_strong_count(pointer) };
        self.scheduler.enqueue(TaskToken::new(
            pointer as usize,
            run_task::<F, C, P, S>,
            cancel_task::<F, C, P, S>,
        ));
    }

    fn enter_running(&self) -> bool {
        let mut state = self.state.load(Ordering::Acquire);
        loop {
            if state & CLOSED != 0 {
                return false;
            }
            debug_assert_ne!(state & SCHEDULED, 0);
            let next = (state & !(SCHEDULED | NOTIFIED)) | RUNNING;
            match self
                .state
                .compare_exchange_weak(state, next, Ordering::AcqRel, Ordering::Acquire)
            {
                Ok(_) => return true,
                Err(current) => state = current,
            }
        }
    }

    fn poll(task: &Arc<Self>) {
        if !task.enter_running() {
            return;
        }
        match Self::poll_future(task) {
            Ok(Poll::Ready(output)) => task.finish_ready(output),
            Ok(Poll::Pending) => task.finish_pending(),
            Err(_) => task.close(),
        }
    }

    fn finish_pending(&self) {
        let mut state = self.state.load(Ordering::Acquire);
        loop {
            debug_assert_ne!(state & RUNNING, 0);
            let reschedule = state & NOTIFIED != 0;
            let mut next = state & !(RUNNING | NOTIFIED);
            if reschedule {
                next |= SCHEDULED;
            }
            match self
                .state
                .compare_exchange_weak(state, next, Ordering::AcqRel, Ordering::Acquire)
            {
                Ok(_) => {
                    if reschedule {
                        self.enqueue_ref();
                    }
                    return;
                }
                Err(current) => state = current,
            }
        }
    }

    /// Win the exclusive right to consume the slots, returning the prior state.
    fn claim_closed(&self) -> Option<u8> {
        let mut state = self.state.load(Ordering::Acquire);
        loop {
            if state & CLOSED != 0 {
                return None;
            }
            let next = (state | CLOSED) & !(NOTIFIED | SCHEDULED | RUNNING);
            match self
                .state
                .compare_exchange_weak(state, next, Ordering::AcqRel, Ordering::Acquire)
            {
                Ok(_) => return Some(state),
                Err(current) => state = current,
            }
        }
    }

    fn finish_ready(&self, output: F::Output) {
        let Some(previous) = self.claim_closed() else {
            return;
        };
        debug_assert_ne!(previous & ARMED, 0);
        // SAFETY: this call won CLOSED and therefore consumes each initialized
        // slot exactly once. ARMED guarantees the payload slot is initialized.
        let (complete, payload) = unsafe {
            (*self.future.get()).assume_init_drop();
            (
                (*self.complete.get()).assume_init_read(),
                (*self.payload.get()).assume_init_read(),
            )
        };
        // SAFETY: arm transferred one reference to the armed task lifetime,
        // which ends here. The polling token's reference keeps the cell alive
        // until run_task returns.
        unsafe { Arc::decrement_strong_count(self as *const Self) };
        let _ = std::panic::catch_unwind(AssertUnwindSafe(|| complete(payload, output)));
    }

    /// End the task without running its completion.
    ///
    /// Covers inline completion (the caller took the output), a panicking poll,
    /// and a queued token destroyed without running.
    fn close(&self) {
        let Some(previous) = self.claim_closed() else {
            return;
        };
        // SAFETY: this call won CLOSED and therefore consumes each initialized
        // slot exactly once. The future and completion slots are initialized at
        // creation; the payload slot only once ARMED is published, and only an
        // armed task holds the reference released here.
        unsafe {
            (*self.future.get()).assume_init_drop();
            (*self.complete.get()).assume_init_drop();
            if previous & ARMED != 0 {
                (*self.payload.get()).assume_init_drop();
                Arc::decrement_strong_count(self as *const Self);
            }
        }
    }
}

unsafe fn clone_owned<F, C, P, S>(data: *const ()) -> RawWaker
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    unsafe { Arc::increment_strong_count(data.cast::<TaskCell<F, C, P, S>>()) };
    RawWaker::new(data, owned_vtable::<F, C, P, S>())
}

unsafe fn wake_owned<F, C, P, S>(data: *const ())
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    let task = unsafe { Arc::from_raw(data.cast::<TaskCell<F, C, P, S>>()) };
    task.notify();
}

unsafe fn wake_by_ref<F, C, P, S>(data: *const ())
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    unsafe { &*data.cast::<TaskCell<F, C, P, S>>() }.notify();
}

unsafe fn drop_owned<F, C, P, S>(data: *const ())
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    unsafe { Arc::decrement_strong_count(data.cast::<TaskCell<F, C, P, S>>()) };
}

unsafe fn drop_borrowed(_: *const ()) {}

fn owned_vtable<F, C, P, S>() -> &'static RawWakerVTable
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    &RawWakerVTable::new(
        clone_owned::<F, C, P, S>,
        wake_owned::<F, C, P, S>,
        wake_by_ref::<F, C, P, S>,
        drop_owned::<F, C, P, S>,
    )
}

fn borrowed_vtable<F, C, P, S>() -> &'static RawWakerVTable
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    &RawWakerVTable::new(
        clone_owned::<F, C, P, S>,
        wake_by_ref::<F, C, P, S>,
        wake_by_ref::<F, C, P, S>,
        drop_borrowed,
    )
}

fn run_task<F, C, P, S>(task: usize)
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    let _ = std::panic::catch_unwind(AssertUnwindSafe(|| {
        // SAFETY: TaskToken owns the Arc reference transferred by enqueue_ref.
        let task = unsafe { Arc::from_raw(task as *const TaskCell<F, C, P, S>) };
        TaskCell::poll(&task);
    }));
}

fn cancel_task<F, C, P, S>(task: usize)
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    let _ = std::panic::catch_unwind(AssertUnwindSafe(|| {
        // SAFETY: TaskToken owns the Arc reference transferred by enqueue_ref.
        let task = unsafe { Arc::from_raw(task as *const TaskCell<F, C, P, S>) };
        task.close();
    }));
}

pub(crate) enum FirstPoll<T, F, C, P, S>
where
    F: Future<Output = T> + Send + 'static,
    T: Send + 'static,
    C: FnOnce(P, T) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    Ready(T),
    Pending(PendingTask<F, C, P, S>),
    Panicked,
}

/// A future that suspended on its first poll, waiting for its payload.
///
/// The future already lives at its final address inside the task cell. Dropping
/// this handle without calling [`PendingTask::install`] ends the task.
pub(crate) struct PendingTask<F, C, P, S>
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    task: Option<Arc<TaskCell<F, C, P, S>>>,
}

impl<F, C, P, S> PendingTask<F, C, P, S>
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    pub(crate) fn install(mut self, payload: P) {
        let task = self
            .task
            .take()
            .expect("pending task must own a task reference");
        TaskCell::arm(task, payload);
    }
}

impl<F, C, P, S> Drop for PendingTask<F, C, P, S>
where
    F: Future + Send + 'static,
    F::Output: Send + 'static,
    C: FnOnce(P, F::Output) + Send + 'static,
    P: Send + 'static,
    S: Scheduler,
{
    fn drop(&mut self) {
        if let Some(task) = self.task.take() {
            task.close();
        }
    }
}

pub(crate) struct EventBaseTask;

impl EventBaseTask {
    fn check_event_base(event_base: *mut ffi::EventBase) {
        assert!(!event_base.is_null(), "pipeline EventBase must not be null");
        debug_assert!(
            // SAFETY: event_base is live and was checked non-null above.
            unsafe { ffi::is_in_event_base_thread(event_base) },
            "coroutine bootstrap poll must run on the pipeline EventBase",
        );
    }

    pub(crate) fn poll<F, C, P>(
        event_base: *mut ffi::EventBase,
        future: F,
        complete: C,
    ) -> FirstPoll<F::Output, F, C, P, EventBaseScheduler>
    where
        F: Future + Send + 'static,
        F::Output: Send + 'static,
        C: FnOnce(P, F::Output) + Send + 'static,
        P: Send + 'static,
    {
        Self::check_event_base(event_base);
        Self::poll_inner(
            future,
            complete,
            EventBaseScheduler {
                event_base: event_base as usize,
            },
        )
    }

    fn poll_inner<F, C, P, S>(
        future: F,
        complete: C,
        scheduler: S,
    ) -> FirstPoll<F::Output, F, C, P, S>
    where
        F: Future + Send + 'static,
        F::Output: Send + 'static,
        C: FnOnce(P, F::Output) + Send + 'static,
        P: Send + 'static,
        S: Scheduler,
    {
        let task = TaskCell::create(scheduler, future, complete);
        match TaskCell::poll_future(&task) {
            Ok(Poll::Ready(output)) => {
                task.close();
                FirstPoll::Ready(output)
            }
            Ok(Poll::Pending) => FirstPoll::Pending(PendingTask { task: Some(task) }),
            Err(_) => {
                task.close();
                FirstPoll::Panicked
            }
        }
    }

    pub(crate) fn start(
        event_base: *mut ffi::EventBase,
        future: impl Future<Output = ()> + Send + 'static,
    ) {
        match Self::poll(event_base, future, |(), ()| {}) {
            FirstPoll::Ready(()) | FirstPoll::Panicked => {}
            FirstPoll::Pending(task) => task.install(()),
        }
    }

    pub(crate) fn start_local(
        event_base: *mut ffi::EventBase,
        future: impl Future<Output = ()> + 'static,
    ) {
        Self::start(event_base, EventBaseLocalFuture(future));
    }

    #[cfg(test)]
    fn start_with(
        future: impl Future<Output = ()> + Send + 'static,
        schedule: impl Fn(TaskToken) + Send + Sync + 'static,
    ) {
        match Self::poll_inner(future, |(), ()| {}, schedule) {
            FirstPoll::Ready(()) | FirstPoll::Panicked => {}
            FirstPoll::Pending(task) => task.install(()),
        }
    }
}

#[cfg(test)]
mod tests {
    use std::collections::VecDeque;
    use std::sync::Mutex;
    use std::sync::atomic::AtomicUsize;

    use super::*;

    type TaskQueue = Arc<Mutex<VecDeque<TaskToken>>>;

    fn start_test(future: impl Future<Output = ()> + Send + 'static) -> TaskQueue {
        let queue = Arc::new(Mutex::new(VecDeque::new()));
        let scheduled = Arc::clone(&queue);
        EventBaseTask::start_with(future, move |task| {
            scheduled
                .lock()
                .unwrap_or_else(|error| error.into_inner())
                .push_back(task);
        });
        queue
    }

    fn pop_task(queue: &TaskQueue) -> TaskToken {
        queue
            .lock()
            .unwrap_or_else(|error| error.into_inner())
            .pop_front()
            .expect("task should be scheduled")
    }

    fn queue_len(queue: &TaskQueue) -> usize {
        queue
            .lock()
            .unwrap_or_else(|error| error.into_inner())
            .len()
    }

    struct ReadyFuture {
        polls: Arc<AtomicUsize>,
        drops: Arc<AtomicUsize>,
    }

    impl Future for ReadyFuture {
        type Output = ();

        fn poll(self: Pin<&mut Self>, _context: &mut Context<'_>) -> Poll<Self::Output> {
            self.polls.fetch_add(1, Ordering::Relaxed);
            Poll::Ready(())
        }
    }

    impl Drop for ReadyFuture {
        fn drop(&mut self) {
            self.drops.fetch_add(1, Ordering::Relaxed);
        }
    }

    #[test]
    fn ready_future_completes_inline_without_scheduling() {
        let polls = Arc::new(AtomicUsize::new(0));
        let drops = Arc::new(AtomicUsize::new(0));
        let queue = start_test(ReadyFuture {
            polls: Arc::clone(&polls),
            drops: Arc::clone(&drops),
        });

        assert_eq!(polls.load(Ordering::Relaxed), 1);
        assert_eq!(drops.load(Ordering::Relaxed), 1);
        assert_eq!(queue_len(&queue), 0);
    }

    struct AddressFuture {
        polls: Arc<AtomicUsize>,
        addresses: Arc<Mutex<Vec<usize>>>,
    }

    impl Future for AddressFuture {
        type Output = ();

        fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
            self.addresses
                .lock()
                .unwrap_or_else(|error| error.into_inner())
                .push(&*self as *const Self as usize);
            if self.polls.fetch_add(1, Ordering::Relaxed) == 0 {
                context.waker().wake_by_ref();
                Poll::Pending
            } else {
                Poll::Ready(())
            }
        }
    }

    /// The Pin contract: a future polled once must never be relocated, so the
    /// inline first poll must already observe the future's final address.
    #[test]
    fn future_is_polled_at_a_stable_address() {
        let polls = Arc::new(AtomicUsize::new(0));
        let addresses = Arc::new(Mutex::new(Vec::new()));
        let queue = start_test(AddressFuture {
            polls: Arc::clone(&polls),
            addresses: Arc::clone(&addresses),
        });

        pop_task(&queue).run();

        let addresses = addresses.lock().unwrap_or_else(|error| error.into_inner());
        assert_eq!(
            addresses.len(),
            2,
            "the future should be polled inline and once more after the wake",
        );
        assert_eq!(
            addresses[0], addresses[1],
            "the future must not move between its first and later polls",
        );
    }

    struct SelfWakeFuture {
        polls: Arc<AtomicUsize>,
    }

    impl Future for SelfWakeFuture {
        type Output = ();

        fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
            if self.polls.fetch_add(1, Ordering::Relaxed) == 0 {
                context.waker().wake_by_ref();
                Poll::Pending
            } else {
                Poll::Ready(())
            }
        }
    }

    #[test]
    fn self_wake_during_first_poll_schedules_one_task() {
        let polls = Arc::new(AtomicUsize::new(0));
        let queue = start_test(SelfWakeFuture {
            polls: Arc::clone(&polls),
        });

        assert_eq!(queue_len(&queue), 1);
        pop_task(&queue).run();
        assert_eq!(polls.load(Ordering::Relaxed), 2);
        assert_eq!(queue_len(&queue), 0);
    }

    struct ConcurrentWakeFuture {
        polls: Arc<AtomicUsize>,
        wake_count: usize,
    }

    impl Future for ConcurrentWakeFuture {
        type Output = ();

        fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
            if self.polls.fetch_add(1, Ordering::Relaxed) != 0 {
                return Poll::Ready(());
            }
            let threads = (0..self.wake_count)
                .map(|_| {
                    let waker = context.waker().clone();
                    std::thread::spawn(move || waker.wake())
                })
                .collect::<Vec<_>>();
            for thread in threads {
                thread.join().expect("wake thread should not panic");
            }
            Poll::Pending
        }
    }

    #[test]
    fn concurrent_first_poll_wakes_coalesce_into_one_task() {
        let polls = Arc::new(AtomicUsize::new(0));
        let queue = start_test(ConcurrentWakeFuture {
            polls: Arc::clone(&polls),
            wake_count: 8,
        });

        assert_eq!(queue_len(&queue), 1);
        pop_task(&queue).run();
        assert_eq!(polls.load(Ordering::Relaxed), 2);
        assert_eq!(queue_len(&queue), 0);
    }

    struct SharedWakerFuture {
        polls: Arc<AtomicUsize>,
        wake_count: usize,
    }

    impl Future for SharedWakerFuture {
        type Output = ();

        fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
            if self.polls.fetch_add(1, Ordering::Relaxed) != 0 {
                return Poll::Ready(());
            }
            let wake_count = self.wake_count;
            // `Waker: Sync` makes `&Waker` `Send`, and a scoped thread borrows
            // without the `'static` bound that would force a clone. Every wake
            // below therefore reaches the task through the *same* waker the
            // first poll was handed, with no clone in between.
            let waker = context.waker();
            std::thread::scope(|scope| {
                for _ in 0..wake_count {
                    scope.spawn(move || waker.wake_by_ref());
                }
            });
            Poll::Pending
        }
    }

    /// Companion to [`concurrent_first_poll_wakes_coalesce_into_one_task`],
    /// which can only exercise cloned wakers: `thread::spawn` requires
    /// `'static`, so it forces a clone. Sharing the un-cloned waker is the case
    /// that would catch non-atomic state behind the first-poll waker.
    #[test]
    fn concurrent_wakes_through_one_shared_waker_coalesce_into_one_task() {
        let polls = Arc::new(AtomicUsize::new(0));
        let queue = start_test(SharedWakerFuture {
            polls: Arc::clone(&polls),
            wake_count: 8,
        });

        assert_eq!(
            queue_len(&queue),
            1,
            "concurrent wakes on one shared waker must coalesce into a single scheduled poll",
        );
        pop_task(&queue).run();
        assert_eq!(polls.load(Ordering::Relaxed), 2);
        assert_eq!(queue_len(&queue), 0);
    }

    struct RetainedWakerFuture {
        polls: Arc<AtomicUsize>,
        wakers: std::sync::mpsc::Sender<Waker>,
    }

    impl Future for RetainedWakerFuture {
        type Output = ();

        fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
            let poll = self.polls.fetch_add(1, Ordering::Relaxed);
            if poll == 0 {
                self.wakers
                    .send(context.waker().clone())
                    .expect("waker receiver should remain alive");
            }
            if poll == 2 {
                Poll::Ready(())
            } else {
                Poll::Pending
            }
        }
    }

    #[test]
    fn waker_retained_from_first_poll_forwards_all_later_wakes() {
        let polls = Arc::new(AtomicUsize::new(0));
        let (wakers, receiver) = std::sync::mpsc::channel();
        let queue = start_test(RetainedWakerFuture {
            polls: Arc::clone(&polls),
            wakers,
        });
        let first_poll_waker = receiver
            .recv()
            .expect("first poll should publish its waker");

        first_poll_waker.wake_by_ref();
        pop_task(&queue).run();
        assert_eq!(polls.load(Ordering::Relaxed), 2);

        first_poll_waker.wake();
        pop_task(&queue).run();
        assert_eq!(polls.load(Ordering::Relaxed), 3);
        assert_eq!(queue_len(&queue), 0);
    }

    struct PendingFuture {
        drops: Arc<AtomicUsize>,
        wakers: std::sync::mpsc::Sender<Waker>,
    }

    impl Future for PendingFuture {
        type Output = ();

        fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
            self.wakers
                .send(context.waker().clone())
                .expect("waker receiver should remain alive");
            Poll::Pending
        }
    }

    impl Drop for PendingFuture {
        fn drop(&mut self) {
            self.drops.fetch_add(1, Ordering::Relaxed);
        }
    }

    #[test]
    fn dropping_queued_task_cancels_and_drops_future_once() {
        let drops = Arc::new(AtomicUsize::new(0));
        let (wakers, receiver) = std::sync::mpsc::channel();
        let queue = start_test(PendingFuture {
            drops: Arc::clone(&drops),
            wakers,
        });
        receiver
            .recv()
            .expect("first poll should publish its waker")
            .wake();

        drop(pop_task(&queue));

        assert_eq!(drops.load(Ordering::Relaxed), 1);
        assert_eq!(queue_len(&queue), 0);
    }

    struct PanicFuture {
        drops: Arc<AtomicUsize>,
        pending_first: bool,
    }

    impl Future for PanicFuture {
        type Output = ();

        fn poll(mut self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<Self::Output> {
            if self.pending_first {
                self.pending_first = false;
                context.waker().wake_by_ref();
                Poll::Pending
            } else {
                panic!("intentional task poll panic");
            }
        }
    }

    impl Drop for PanicFuture {
        fn drop(&mut self) {
            self.drops.fetch_add(1, Ordering::Relaxed);
        }
    }

    #[test]
    fn initial_poll_panic_is_contained_and_drops_future_once() {
        let drops = Arc::new(AtomicUsize::new(0));
        let queue = start_test(PanicFuture {
            drops: Arc::clone(&drops),
            pending_first: false,
        });

        assert_eq!(drops.load(Ordering::Relaxed), 1);
        assert_eq!(queue_len(&queue), 0);
    }

    #[test]
    fn scheduled_poll_panic_is_contained_and_drops_future_once() {
        let drops = Arc::new(AtomicUsize::new(0));
        let queue = start_test(PanicFuture {
            drops: Arc::clone(&drops),
            pending_first: true,
        });

        pop_task(&queue).run();

        assert_eq!(drops.load(Ordering::Relaxed), 1);
        assert_eq!(queue_len(&queue), 0);
    }

    #[test]
    fn dropping_pending_task_before_install_drops_future_once() {
        let drops = Arc::new(AtomicUsize::new(0));
        let (wakers, receiver) = std::sync::mpsc::channel();
        let first_poll = EventBaseTask::poll_inner(
            PendingFuture {
                drops: Arc::clone(&drops),
                wakers,
            },
            |(), ()| {},
            |_task: TaskToken| unreachable!("an uninstalled task must not schedule"),
        );
        receiver
            .recv()
            .expect("first poll should publish its waker");

        match first_poll {
            FirstPoll::Pending(task) => drop(task),
            _ => panic!("a never-ready future should suspend"),
        }

        assert_eq!(drops.load(Ordering::Relaxed), 1);
    }

    struct WakeDuringLaterPoll {
        polls: Arc<AtomicUsize>,
    }

    impl Future for WakeDuringLaterPoll {
        type Output = ();

        fn poll(self: Pin<&mut Self>, context: &mut Context<'_>) -> Poll<()> {
            match self.polls.fetch_add(1, Ordering::Relaxed) {
                0 | 1 => {
                    context.waker().wake_by_ref();
                    Poll::Pending
                }
                _ => Poll::Ready(()),
            }
        }
    }

    #[test]
    fn wake_during_later_poll_reschedules_exactly_once() {
        let polls = Arc::new(AtomicUsize::new(0));
        let queue = start_test(WakeDuringLaterPoll {
            polls: Arc::clone(&polls),
        });

        assert_eq!(queue_len(&queue), 1);
        pop_task(&queue).run();
        assert_eq!(queue_len(&queue), 1);
        pop_task(&queue).run();

        assert_eq!(polls.load(Ordering::Relaxed), 3);
        assert_eq!(queue_len(&queue), 0);
    }

    #[test]
    fn retained_waker_after_completion_is_a_noop() {
        let polls = Arc::new(AtomicUsize::new(0));
        let (wakers, receiver) = std::sync::mpsc::channel();
        let queue = start_test(RetainedWakerFuture {
            polls: Arc::clone(&polls),
            wakers,
        });
        let waker = receiver.recv().expect("first poll should publish waker");

        waker.wake_by_ref();
        pop_task(&queue).run();
        waker.wake_by_ref();
        pop_task(&queue).run();
        assert_eq!(polls.load(Ordering::Relaxed), 3);

        waker.wake();
        assert_eq!(polls.load(Ordering::Relaxed), 3);
        assert_eq!(queue_len(&queue), 0);
    }
}
