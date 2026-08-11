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

use std::sync::atomic::AtomicUsize;
use std::sync::atomic::Ordering;

use channel_pipeline as _;
use channel_pipeline::ContextHandle;
use channel_pipeline::HandlerResult;

static NATIVE_DESTRUCTION_CALLS: AtomicUsize = AtomicUsize::new(0);
static NATIVE_DESTRUCTION_DROPS: AtomicUsize = AtomicUsize::new(0);

fn native_destruction_call(_task: usize) {
    NATIVE_DESTRUCTION_CALLS.fetch_add(1, Ordering::Relaxed);
}

fn native_destruction_drop(_task: usize) {
    NATIVE_DESTRUCTION_DROPS.fetch_add(1, Ordering::Relaxed);
}

fn run_with_timeout(test: impl FnOnce() + Send + 'static) {
    let (sender, receiver) = std::sync::mpsc::sync_channel(1);
    std::thread::spawn(move || {
        let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(test));
        let _ = sender.send(result);
    });
    match receiver.recv_timeout(std::time::Duration::from_secs(5)) {
        Ok(Ok(())) => {}
        Ok(Err(panic)) => std::panic::resume_unwind(panic),
        Err(error) => panic!("test exceeded five-second deadline: {error}"),
    }
}

#[cxx::bridge(namespace = "channel_pipeline_rust::test")]
mod ffi {
    struct PipelineTestResult {
        cpp_before_reads: u32,
        cpp_after_reads: u32,
        cpp_before_writes: u32,
        cpp_after_writes: u32,
        tail_reads: u32,
        head_writes: u32,
        written_length: u64,
        written_first_byte: u8,
        rust_reads: u32,
        rust_writes: u32,
    }

    struct BehaviorTestResult {
        success_read: i32,
        backpressure_read: i32,
        backpressure_write: i32,
        error_read: i32,
        error_write: i32,
        panic_read: i32,
        panic_write: i32,
        mismatch_read: i32,
        mismatch_write: i32,
        empty_read: i32,
        downstream_backpressure_read: i32,
        downstream_error_write: i32,
        read_pointer_identity_preserved: bool,
        write_pointer_identity_preserved: bool,
        exceptions: u32,
        read_ready: u32,
        write_ready: u32,
        added: u32,
        active: u32,
        inactive: u32,
        removed: u32,
    }

    struct PositionTestResult {
        tail_reads: u32,
        head_writes: u32,
        written_length: u64,
        written_first_byte: u8,
        rust_reads: u32,
        rust_writes: u32,
    }

    struct LifecycleOrderResult {
        sequence: String,
        tail_reads: u32,
        head_writes: u32,
    }

    struct ReadinessCycleResult {
        sequence: String,
        armed_after_backpressure: bool,
        disarmed_after_ready: bool,
        terminal_after_recovery: u32,
        first_result: i32,
        recovered_result: i32,
    }

    struct RearmResult {
        sequence: String,
        armed_after_last_ready: bool,
    }

    struct BidirectionalResult {
        sequence: String,
        read_armed_after_read_bp: bool,
        write_armed_after_read_bp: bool,
        write_armed_after_write_bp: bool,
        read_armed_after_write_bp: bool,
        write_armed_after_read_ready: bool,
        read_disarmed_after_read_ready: bool,
        write_disarmed_after_write_ready: bool,
    }

    struct ProbeResult {
        checks_passed: u32,
        expected_checks: u32,
        read_armed_after: bool,
        write_armed_after: bool,
    }

    struct TeardownResult {
        read_armed_before: bool,
        write_armed_before: bool,
        read_armed_after: bool,
        write_armed_after: bool,
        read_ready_after: u32,
        write_ready_after: u32,
        removed: u32,
        inactive: u32,
    }

    struct PanicResult {
        completed: bool,
        tail_exceptions: u32,
    }

    struct IdentityResult {
        observed_id: u64,
        expected_id: u64,
        is_nonzero: bool,
        matches_tag: bool,
        tail_reads: u32,
    }

    struct AllocationResult {
        checks_passed: u32,
        expected_checks: u32,
        allocator_invocations: u32,
    }

    struct CopyResult {
        checks_passed: u32,
        expected_checks: u32,
        tail_reads: u32,
        pointer_preserved: bool,
    }

    struct CloseResult {
        checks_passed: u32,
        expected_checks: u32,
        tail_reads: u32,
        removed: u32,
        inactive: u32,
        top_level_is_error: bool,
    }

    struct StateMachineResult {
        sequence: String,
        state_bits: u32,
        tail_reads: u32,
        head_writes: u32,
        removed: u32,
        inactive: u32,
        closed: bool,
        top_level_read_is_error: bool,
        top_level_write_is_error: bool,
    }

    struct PanicRetentionResult {
        completed: bool,
        read_is_error: bool,
        write_is_error: bool,
        tail_exceptions: u32,
    }

    struct ExceptionPreserveResult {
        rust_observed: u32,
        tail_exceptions: u32,
        tail_saw_original: bool,
    }

    struct ReentrancyResult {
        reentrancy_count: u32,
        tail_reads: u32,
        completed: bool,
    }

    // ── Phase 6: adapter extensibility + event noop ──────────────────────

    struct AdapterExtResult {
        pointer_identity: bool,
        null_rejected: bool,
        wrong_type_rejected: bool,
        empty_rejected: bool,
        chain_independence: bool,
        checks_passed: u32,
        expected_checks: u32,
    }

    struct EventNoopResult {
        no_event_is_noop: bool,
        out_of_range_noop: bool,
        empty_payload_delivered: bool,
        subscriber_count_for_A: u32,
    }

    struct ForwardUnknownResult {
        tail_reads: u32,
        head_writes: u32,
        read_is_error: bool,
        write_is_error: bool,
    }

    struct ContextHandleTestResult {
        removed_before_owner_release: u32,
        removed_after_owner_release: u32,
        removed_after_first_drop: u32,
        removed_after_final_drop: u32,
        stored_handles: u32,
        read_result: i32,
        allocation_delta: u64,
        jemalloc_available: bool,
    }

    struct ContextHandleFireResult {
        endpoint_calls_before_fence: u32,
        endpoint_calls_after_fence: u32,
        removed_after_owner_release: u32,
        removed_after_fence: u32,
        pointer_identity_preserved: bool,
        allocation_delta: u64,
        jemalloc_available: bool,
    }

    struct ContextHandleExceptionResult {
        before_exceptions_before_fence: u32,
        before_exceptions_after_fence: u32,
        after_exceptions_before_fence: u32,
        after_exceptions_after_fence: u32,
        tail_exceptions_before_fence: u32,
        tail_exceptions_after_fence: u32,
        removed_after_owner_release: u32,
        removed_after_fence: u32,
        message_preserved: bool,
    }

    struct ContextHandleSandwichResult {
        before_reads_before_fence: u32,
        before_reads_after_fence: u32,
        after_reads_before_fence: u32,
        after_reads_after_fence: u32,
        before_writes_before_fence: u32,
        before_writes_after_fence: u32,
        after_writes_before_fence: u32,
        after_writes_after_fence: u32,
        endpoint_calls_before_fence: u32,
        endpoint_calls_after_fence: u32,
        pointer_identity_preserved: bool,
    }

    extern "Rust" {
        fn native_destruction_call(task: usize);
        fn native_destruction_drop(task: usize);
    }

    unsafe extern "C++" {
        include!("thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/PipelineTestHelper.h");

        fn run_pipeline_test() -> PipelineTestResult;
        fn run_behavior_test() -> BehaviorTestResult;
        fn run_position_test_first() -> PositionTestResult;
        fn run_position_test_middle() -> PositionTestResult;
        fn run_position_test_last() -> PositionTestResult;
        fn run_lifecycle_order_test() -> LifecycleOrderResult;
        fn run_read_recovery_test() -> ReadinessCycleResult;
        fn run_write_recovery_test() -> ReadinessCycleResult;
        fn run_read_rearm_test() -> RearmResult;
        fn run_multi_handler_rearm_test() -> RearmResult;
        fn run_bidirectional_test() -> BidirectionalResult;
        fn run_readiness_probe_test() -> ProbeResult;
        fn run_close_while_armed_test() -> TeardownResult;
        fn run_inactive_while_armed_test() -> TeardownResult;
        fn run_destroy_while_armed_test() -> TeardownResult;
        fn run_panic_containment_test() -> PanicResult;
        fn run_identity_test() -> IdentityResult;
        fn run_allocation_probe_test() -> AllocationResult;
        fn run_copy_probe_test() -> CopyResult;
        fn run_close_probe_test() -> CloseResult;
        fn run_state_machine_test() -> StateMachineResult;
        fn run_panic_retention_test() -> PanicRetentionResult;
        fn run_exception_preserve_test() -> ExceptionPreserveResult;
        fn run_reentrancy_test() -> ReentrancyResult;
        fn run_adapter_ext_test() -> AdapterExtResult;
        fn run_event_noop_test() -> EventNoopResult;
        fn run_forward_unknown_test() -> ForwardUnknownResult;
        fn run_context_handle_test(scenario: u32) -> ContextHandleTestResult;
        fn run_context_handle_fire_test(scenario: u32) -> ContextHandleFireResult;
        fn run_context_handle_sandwich_test(scenario: u32) -> ContextHandleSandwichResult;
        fn run_context_handle_exception_test(scenario: u32) -> ContextHandleExceptionResult;
        unsafe fn run_event_base_destruction_test(task: usize, call: fn(usize), drop: fn(usize));
    }
}

#[test]
fn native_event_base_destruction_drains_queued_task_once() {
    NATIVE_DESTRUCTION_CALLS.store(0, Ordering::Relaxed);
    NATIVE_DESTRUCTION_DROPS.store(0, Ordering::Relaxed);

    // SAFETY: the task token is inert test data and both function pointers stay
    // valid for the duration of the synchronous native helper.
    unsafe {
        ffi::run_event_base_destruction_test(1, native_destruction_call, native_destruction_drop);
    }

    assert_eq!(NATIVE_DESTRUCTION_CALLS.load(Ordering::Relaxed), 1);
    assert_eq!(NATIVE_DESTRUCTION_DROPS.load(Ordering::Relaxed), 0);
}

#[test]
fn rust_callbacks_drive_real_cpp_pipeline() {
    let result = ffi::run_pipeline_test();
    assert_eq!(result.cpp_before_reads, 1);
    assert_eq!(result.cpp_after_reads, 1);
    assert_eq!(result.tail_reads, 1);
    assert_eq!(result.cpp_after_writes, 1);
    assert_eq!(result.cpp_before_writes, 1);
    assert_eq!(result.head_writes, 1);
    assert_eq!(result.written_length, 5);
    assert_eq!(result.written_first_byte, 0xcd);
    assert_eq!(result.rust_reads, 1);
    assert_eq!(result.rust_writes, 1);
}

#[test]
fn synchronous_handler_behavior_is_contained_and_observable() {
    let result = ffi::run_behavior_test();
    assert_eq!(result.success_read, 0);
    assert_eq!(result.backpressure_read, 1);
    assert_eq!(result.backpressure_write, 1);
    assert_eq!(result.error_read, 2);
    assert_eq!(result.error_write, 2);
    assert_eq!(result.panic_read, 2);
    assert_eq!(result.panic_write, 2);
    assert_eq!(result.mismatch_read, 2);
    assert_eq!(result.mismatch_write, 2);
    assert_eq!(result.empty_read, 2);
    assert_eq!(result.downstream_backpressure_read, 1);
    assert_eq!(result.downstream_error_write, 2);
    assert!(result.read_pointer_identity_preserved);
    assert!(result.write_pointer_identity_preserved);
    assert_eq!(result.exceptions, 1);
    assert_eq!(result.read_ready, 1);
    assert_eq!(result.write_ready, 1);
    assert_eq!(result.added, 1);
    assert_eq!(result.active, 1);
    assert_eq!(result.inactive, 1);
    assert_eq!(result.removed, 1);
}

#[test]
fn rust_handler_works_in_first_position() {
    let result = ffi::run_position_test_first();
    assert_eq!(result.tail_reads, 1);
    assert_eq!(result.head_writes, 1);
    assert_eq!(result.rust_reads, 1);
    assert_eq!(result.rust_writes, 1);
}

#[test]
fn rust_handler_works_in_middle_position() {
    let result = ffi::run_position_test_middle();
    assert_eq!(result.tail_reads, 1);
    assert_eq!(result.head_writes, 1);
    assert_eq!(result.rust_reads, 1);
    assert_eq!(result.rust_writes, 1);
}

#[test]
fn rust_handler_works_in_last_position() {
    let result = ffi::run_position_test_last();
    assert_eq!(result.tail_reads, 1);
    assert_eq!(result.head_writes, 1);
    assert_eq!(result.rust_reads, 1);
    assert_eq!(result.rust_writes, 1);
}

#[test]
fn lifecycle_callbacks_fire_in_native_order() {
    let result = ffi::run_lifecycle_order_test();
    let expected = "added#1,added#2,\
                    active#1,active#2,\
                    read#1,read#2,\
                    write#2,write#1,\
                    exception#1,exception#2,\
                    inactive#2,inactive#1,\
                    removed#2,removed#1";
    assert_eq!(result.sequence, expected);
    assert_eq!(result.tail_reads, 1);
    assert_eq!(result.head_writes, 1);
}

#[test]
fn read_backpressure_recovers_with_one_shot_wakeup() {
    let result = ffi::run_read_recovery_test();
    assert_eq!(result.first_result, HandlerResult::Backpressure as i32);
    assert!(result.armed_after_backpressure);
    assert_eq!(result.sequence.matches("read_ready").count(), 1);
    assert!(result.disarmed_after_ready);
    assert_eq!(result.recovered_result, HandlerResult::Success as i32);
    assert_eq!(result.terminal_after_recovery, 1);
    assert_eq!(result.sequence, "read_bp,read_ready,read_ok");
}

#[test]
fn write_backpressure_recovers_with_one_shot_wakeup() {
    let result = ffi::run_write_recovery_test();
    assert_eq!(result.first_result, HandlerResult::Backpressure as i32);
    assert!(result.armed_after_backpressure);
    assert_eq!(result.sequence.matches("write_ready").count(), 1);
    assert!(result.disarmed_after_ready);
    assert_eq!(result.recovered_result, HandlerResult::Success as i32);
    assert_eq!(result.terminal_after_recovery, 1);
    assert_eq!(result.sequence, "write_bp,write_ready,write_ok");
}

#[test]
fn explicit_rearm_delivers_one_wakeup_per_arm() {
    let result = ffi::run_read_rearm_test();
    assert_eq!(result.sequence.matches("read_ready").count(), 3);
    assert!(!result.armed_after_last_ready);
}

#[test]
fn multi_handler_rearms_are_deferred_to_the_next_notification() {
    let result = ffi::run_multi_handler_rearm_test();
    assert!(result.armed_after_last_ready);
}

#[test]
fn read_and_write_backpressure_are_independent() {
    let result = ffi::run_bidirectional_test();
    assert!(result.read_armed_after_read_bp);
    assert!(!result.write_armed_after_read_bp);
    assert!(result.write_armed_after_write_bp);
    assert!(result.read_armed_after_write_bp);
    assert!(result.write_armed_after_read_ready);
    assert!(result.read_disarmed_after_read_ready);
    assert!(result.write_disarmed_after_write_ready);
}

#[test]
fn safe_readiness_api_is_idempotent_and_independent() {
    let result = ffi::run_readiness_probe_test();
    assert_eq!(result.checks_passed, result.expected_checks);
    assert!(!result.read_armed_after);
    assert!(!result.write_armed_after);
}

#[test]
fn close_while_armed_clears_hooks_and_blocks_callbacks() {
    let result = ffi::run_close_while_armed_test();
    assert!(result.read_armed_before && result.write_armed_before);
    assert!(!result.read_armed_after && !result.write_armed_after);
    assert_eq!(result.read_ready_after, 0);
    assert_eq!(result.write_ready_after, 0);
    assert_eq!(result.removed, 1);
}

#[test]
fn inactive_while_armed_cancels_hooks() {
    let result = ffi::run_inactive_while_armed_test();
    assert!(result.read_armed_before && result.write_armed_before);
    assert!(!result.read_armed_after && !result.write_armed_after);
    assert_eq!(result.read_ready_after, 0);
    assert_eq!(result.write_ready_after, 0);
    assert_eq!(result.inactive, 1);
    assert_eq!(result.removed, 1);
}

#[test]
fn destroy_while_armed_unlinks_hooks_and_removes() {
    let result = ffi::run_destroy_while_armed_test();
    assert!(result.read_armed_before && result.write_armed_before);
    assert_eq!(result.removed, 1);
}

#[test]
fn panics_in_lifecycle_and_ready_callbacks_are_contained() {
    let result = ffi::run_panic_containment_test();
    assert!(result.completed);
    assert_eq!(result.tail_exceptions, 1);
}

#[test]
fn handler_identity_is_stable_and_matches_tag() {
    let result = ffi::run_identity_test();
    assert!(result.is_nonzero);
    assert_eq!(result.observed_id, result.expected_id);
    assert!(result.matches_tag);
    assert_eq!(result.tail_reads, 1);
}

#[test]
fn allocation_apis_cover_zero_and_boundaries() {
    let result = ffi::run_allocation_probe_test();
    assert_eq!(result.checks_passed, result.expected_checks);
    assert!(result.allocator_invocations >= 2);
}

#[test]
fn explicit_copy_vs_move_documented_and_independent() {
    let result = ffi::run_copy_probe_test();
    assert_eq!(result.checks_passed, result.expected_checks);
    assert_eq!(result.tail_reads, 1);
    assert!(result.pointer_preserved);
}

#[test]
fn close_behavior_idempotent_and_edges_defined() {
    let result = ffi::run_close_probe_test();
    assert_eq!(result.checks_passed, result.expected_checks);
    assert_eq!(result.tail_reads, 1);
    assert_eq!(result.removed, 1);
    assert!(result.top_level_is_error);
}

#[test]
fn state_machine_lifecycle_order_and_closed_edges() {
    let result = ffi::run_state_machine_test();
    assert!(
        result.sequence.contains("added#1")
            && result.sequence.contains("added#2")
            && result.sequence.contains("active#1")
            && result.sequence.contains("removed#1"),
        "state machine must observe added→active→data→close→removed reverse, got: {}",
        result.sequence
    );
    assert!(result.closed, "close() must clear hooks / terminal");
    assert_eq!(result.removed, 2, "handlerRemoved twice across close");
    assert!(
        result.top_level_read_is_error && result.top_level_write_is_error,
        "top-level fire after close returns Error"
    );
    assert_eq!(result.tail_reads, 1);
    assert_eq!(result.head_writes, 1);
}

#[test]
fn panic_retention_and_non_escape_contained() {
    let result = ffi::run_panic_retention_test();
    assert!(result.completed, "pipeline must survive panics");
    assert!(
        result.read_is_error && result.write_is_error,
        "panicking data callbacks map to Error"
    );
    assert_eq!(
        result.tail_exceptions, 1,
        "exception still forwards after panic"
    );
}

#[test]
fn exception_wrapper_preserved_no_double_forward() {
    let result = ffi::run_exception_preserve_test();
    assert_eq!(result.rust_observed, 1, "on_exception exactly once");
    assert_eq!(result.tail_exceptions, 1);
    assert!(result.tail_saw_original, "original exception preserved");
}

#[test]
fn reentrancy_and_rearm_deterministic() {
    let result = ffi::run_reentrancy_test();
    assert!(result.completed);
    assert!(
        result.reentrancy_count >= 6,
        "reentrancy lifecycle+data+re-arm count >=6, got {}",
        result.reentrancy_count
    );
    assert_eq!(result.tail_reads, 1);
}

// ── Phase 6: adapter extensibility + event noop ───────────────────────────

#[test]
fn adapter_extensibility_stable_and_fallible() {
    let result = ffi::run_adapter_ext_test();
    assert!(
        result.pointer_identity,
        "BytesPtr round-trip must preserve pointer identity via makeBytes+erase_and_box+get"
    );
    assert!(result.null_rejected, "tryBox(nullptr) must -> nullopt");
    assert!(
        result.wrong_type_rejected,
        "box uint32_t{{42}} -> tryTake<BytesPtr> must -> nullopt"
    );
    assert!(
        result.empty_rejected,
        "empty TypeErasedBox -> tryTake must -> nullopt"
    );
    assert!(
        result.chain_independence,
        "IOBuf chain must be preserved through adapter round-trip; zero allocation proof; chain independence"
    );
    assert_eq!(
        result.checks_passed, result.expected_checks,
        "all {} checks must pass, got {}",
        result.expected_checks, result.checks_passed
    );
}

#[test]
fn event_subsystem_noop_and_const_ref() {
    let result = ffi::run_event_noop_test();
    assert!(
        result.no_event_is_noop,
        "NoEvent default (eventListCount_=0) fireEvent out-of-range no-op per PipelineImpl.cpp:321-337"
    );
    assert!(
        result.out_of_range_noop,
        "out-of-range event id (>=Count) must be no-op, no callback, covers NoEvent disabled"
    );
    assert!(
        result.empty_payload_delivered,
        "empty TypeErasedBox pure signal delivered via const ref non-consuming; dispatch only subscribed O(s); per-event list"
    );
    assert!(
        result.subscriber_count_for_A >= 2,
        "Alpha subscriber count proof of dispatch: empty + typed payload, got {}",
        result.subscriber_count_for_A
    );
}

#[test]
fn context_handle_layout_is_two_words_and_pointer_aligned() {
    run_with_timeout(|| {
        assert_eq!(
            std::mem::size_of::<ContextHandle>(),
            2 * std::mem::size_of::<usize>()
        );
        assert_eq!(
            std::mem::align_of::<ContextHandle>(),
            std::mem::align_of::<usize>()
        );
    });
}

#[test]
fn context_handle_is_send_not_sync_clone_or_copy() {
    run_with_timeout(|| {
        static_assertions::assert_impl_all!(ContextHandle: Send);
        static_assertions::assert_not_impl_any!(ContextHandle: Sync, Clone, Copy);
    });
}

#[test]
fn native_context_handle_token_traits_match_rust_abi() {
    run_with_timeout(|| {
        assert_eq!(
            std::mem::size_of::<ContextHandle>(),
            2 * std::mem::size_of::<*mut ()>()
        );
        assert_eq!(
            std::mem::align_of::<ContextHandle>(),
            std::mem::align_of::<*mut ()>()
        );
    });
}

fn assert_final_teardown_once(result: &ffi::ContextHandleTestResult) {
    assert_eq!(result.removed_before_owner_release, 0);
    assert_eq!(result.removed_after_final_drop, 1);
}

#[test]
fn context_handle_acquires_and_drops_on_event_base() {
    let result = ffi::run_context_handle_test(0);
    assert_final_teardown_once(&result);
    assert_eq!(result.removed_after_owner_release, 1);
}

#[test]
fn context_handle_rust_move_chain_drops_once() {
    let result = ffi::run_context_handle_test(1);
    assert_final_teardown_once(&result);
    assert_eq!(result.removed_after_owner_release, 1);
}

#[test]
fn context_handle_cross_thread_round_trip_drops_on_event_base() {
    let result = ffi::run_context_handle_test(2);
    assert_eq!(result.removed_after_owner_release, 0);
    assert_final_teardown_once(&result);
}

#[test]
fn context_handle_retains_pipeline_until_drop() {
    let result = ffi::run_context_handle_test(3);
    assert_eq!(result.stored_handles, 1);
    assert_eq!(result.removed_after_owner_release, 0);
    assert_final_teardown_once(&result);
}

#[test]
fn context_handle_multiple_handles_release_on_last_drop() {
    let result = ffi::run_context_handle_test(4);
    assert_eq!(result.stored_handles, 2);
    assert_eq!(result.removed_after_owner_release, 0);
    assert_eq!(result.removed_after_first_drop, 0);
    assert_final_teardown_once(&result);
}

#[test]
fn context_handle_survives_pipeline_close_until_drop() {
    let result = ffi::run_context_handle_test(5);
    assert_eq!(result.stored_handles, 1);
    assert_eq!(result.removed_after_owner_release, 1);
    assert_eq!(result.removed_after_final_drop, 1);
}

#[test]
fn context_handle_panic_unwind_releases_guard() {
    let result = ffi::run_context_handle_test(6);
    assert_eq!(result.read_result, HandlerResult::Error as i32);
    assert_final_teardown_once(&result);
    assert_eq!(result.removed_after_owner_release, 1);
}

#[test]
fn context_handle_acquire_move_drop_allocates_nothing() {
    let result = ffi::run_context_handle_test(7);
    assert_final_teardown_once(&result);
    if result.jemalloc_available {
        assert_eq!(result.allocation_delta, 0);
    }
}

fn assert_context_handle_inline_fire(result: &ffi::ContextHandleFireResult) {
    assert_eq!(result.endpoint_calls_before_fence, 1);
    assert_eq!(result.endpoint_calls_after_fence, 1);
    assert!(result.pointer_identity_preserved);
    assert_eq!(result.removed_after_owner_release, 1);
    assert_eq!(result.removed_after_fence, 1);
}

fn assert_context_handle_worker_fire(result: &ffi::ContextHandleFireResult) {
    assert_eq!(result.endpoint_calls_before_fence, 0);
    assert_eq!(result.endpoint_calls_after_fence, 1);
    assert!(result.pointer_identity_preserved);
    assert_eq!(result.removed_after_owner_release, 0);
    assert_eq!(result.removed_after_fence, 1);
}

#[test]
fn context_handle_fire_read_runs_immediately_on_event_base() {
    assert_context_handle_inline_fire(&ffi::run_context_handle_fire_test(8));
}

#[test]
fn context_handle_fire_write_runs_immediately_on_event_base() {
    assert_context_handle_inline_fire(&ffi::run_context_handle_fire_test(9));
}

#[test]
fn context_handle_fire_read_from_worker_enqueues_on_event_base() {
    assert_context_handle_worker_fire(&ffi::run_context_handle_fire_test(10));
}

#[test]
fn context_handle_fire_write_from_worker_enqueues_on_event_base() {
    assert_context_handle_worker_fire(&ffi::run_context_handle_fire_test(11));
}

fn assert_context_handle_read_sandwich(result: &ffi::ContextHandleSandwichResult, inline: bool) {
    assert_eq!(result.before_reads_before_fence, 1);
    assert_eq!(result.before_reads_after_fence, 1);
    assert_eq!(result.after_reads_before_fence, u32::from(inline));
    assert_eq!(result.after_reads_after_fence, 1);
    assert_eq!(result.before_writes_before_fence, 0);
    assert_eq!(result.before_writes_after_fence, 0);
    assert_eq!(result.after_writes_before_fence, 0);
    assert_eq!(result.after_writes_after_fence, 0);
    assert_eq!(result.endpoint_calls_before_fence, u32::from(inline));
    assert_eq!(result.endpoint_calls_after_fence, 1);
    assert!(result.pointer_identity_preserved);
}

fn assert_context_handle_write_sandwich(result: &ffi::ContextHandleSandwichResult, inline: bool) {
    assert_eq!(result.before_reads_before_fence, 0);
    assert_eq!(result.before_reads_after_fence, 0);
    assert_eq!(result.after_reads_before_fence, 0);
    assert_eq!(result.after_reads_after_fence, 0);
    assert_eq!(result.after_writes_before_fence, 1);
    assert_eq!(result.after_writes_after_fence, 1);
    assert_eq!(result.before_writes_before_fence, u32::from(inline));
    assert_eq!(result.before_writes_after_fence, 1);
    assert_eq!(result.endpoint_calls_before_fence, u32::from(inline));
    assert_eq!(result.endpoint_calls_after_fence, 1);
    assert!(result.pointer_identity_preserved);
}

#[test]
fn context_handle_inline_read_resumes_inside_cpp_sandwich() {
    assert_context_handle_read_sandwich(&ffi::run_context_handle_sandwich_test(8), true);
}

#[test]
fn context_handle_inline_write_resumes_inside_cpp_sandwich() {
    assert_context_handle_write_sandwich(&ffi::run_context_handle_sandwich_test(9), true);
}

#[test]
fn context_handle_worker_read_resumes_inside_cpp_sandwich() {
    assert_context_handle_read_sandwich(&ffi::run_context_handle_sandwich_test(10), false);
}

#[test]
fn context_handle_worker_write_resumes_inside_cpp_sandwich() {
    assert_context_handle_write_sandwich(&ffi::run_context_handle_sandwich_test(11), false);
}

#[test]
fn coro_read_ready_future_polls_inline_and_resumes_read() {
    assert_context_handle_read_sandwich(&ffi::run_context_handle_sandwich_test(20), true);
}

#[test]
fn coro_read_worker_wake_repolls_and_resumes_read() {
    assert_context_handle_read_sandwich(&ffi::run_context_handle_sandwich_test(21), false);
}

#[test]
fn coro_write_ready_future_polls_inline_and_resumes_write() {
    assert_context_handle_write_sandwich(&ffi::run_context_handle_sandwich_test(22), true);
}

#[test]
fn coro_write_worker_wake_repolls_and_resumes_write() {
    assert_context_handle_write_sandwich(&ffi::run_context_handle_sandwich_test(23), false);
}

fn assert_context_handle_exception(
    result: &ffi::ContextHandleExceptionResult,
    delivered_before_fence: bool,
) {
    assert_eq!(result.before_exceptions_before_fence, 0);
    assert_eq!(result.before_exceptions_after_fence, 0);
    assert_eq!(
        result.after_exceptions_before_fence,
        u32::from(delivered_before_fence)
    );
    assert_eq!(result.after_exceptions_after_fence, 1);
    assert_eq!(
        result.tail_exceptions_before_fence,
        u32::from(delivered_before_fence)
    );
    assert_eq!(result.tail_exceptions_after_fence, 1);
    assert!(result.message_preserved);
    assert_eq!(
        result.removed_after_owner_release,
        u32::from(delivered_before_fence)
    );
    assert_eq!(result.removed_after_fence, 1);
}

#[test]
fn context_handle_fire_exception_runs_immediately_inside_cpp_sandwich() {
    assert_context_handle_exception(&ffi::run_context_handle_exception_test(14), true);
}

#[test]
fn context_handle_fire_exception_from_worker_resumes_inside_cpp_sandwich() {
    assert_context_handle_exception(&ffi::run_context_handle_exception_test(15), false);
}

#[test]
fn coro_exception_ready_future_polls_inline_and_resumes_exception() {
    assert_context_handle_exception(&ffi::run_context_handle_exception_test(24), true);
}

#[test]
fn coro_exception_worker_wake_repolls_and_resumes_exception() {
    assert_context_handle_exception(&ffi::run_context_handle_exception_test(25), false);
}

#[test]
fn context_handle_fire_exception_preserves_empty_message() {
    assert_context_handle_exception(&ffi::run_context_handle_exception_test(17), true);
}

#[test]
fn context_handle_fire_exception_preserves_long_message() {
    assert_context_handle_exception(&ffi::run_context_handle_exception_test(18), true);
}

#[test]
fn context_handle_fire_exception_enqueued_before_close_is_suppressed() {
    let result = ffi::run_context_handle_exception_test(19);
    assert_eq!(result.before_exceptions_before_fence, 0);
    assert_eq!(result.before_exceptions_after_fence, 0);
    assert_eq!(result.after_exceptions_before_fence, 0);
    assert_eq!(result.after_exceptions_after_fence, 0);
    assert_eq!(result.tail_exceptions_before_fence, 0);
    assert_eq!(result.tail_exceptions_after_fence, 0);
    assert!(!result.message_preserved);
    assert_eq!(result.removed_after_owner_release, 1);
    assert_eq!(result.removed_after_fence, 1);
}

#[test]
fn context_handle_fire_exception_after_close_is_suppressed() {
    let result = ffi::run_context_handle_exception_test(16);
    assert_eq!(result.before_exceptions_before_fence, 0);
    assert_eq!(result.before_exceptions_after_fence, 0);
    assert_eq!(result.after_exceptions_before_fence, 0);
    assert_eq!(result.after_exceptions_after_fence, 0);
    assert_eq!(result.tail_exceptions_before_fence, 0);
    assert_eq!(result.tail_exceptions_after_fence, 0);
    assert!(!result.message_preserved);
    assert_eq!(result.removed_after_owner_release, 1);
    assert_eq!(result.removed_after_fence, 1);
}

#[test]
fn context_handle_fire_read_after_close_is_suppressed() {
    let result = ffi::run_context_handle_fire_test(12);
    assert_eq!(result.endpoint_calls_before_fence, 0);
    assert_eq!(result.endpoint_calls_after_fence, 0);
    assert!(!result.pointer_identity_preserved);
    assert_eq!(result.removed_after_owner_release, 1);
    assert_eq!(result.removed_after_fence, 1);
}

#[test]
fn context_handle_fire_write_after_close_is_suppressed() {
    let result = ffi::run_context_handle_fire_test(13);
    assert_eq!(result.endpoint_calls_before_fence, 0);
    assert_eq!(result.endpoint_calls_after_fence, 0);
    assert!(!result.pointer_identity_preserved);
    assert_eq!(result.removed_after_owner_release, 1);
    assert_eq!(result.removed_after_fence, 1);
}

#[test]
fn handler_forwards_message_via_erased_box_without_taking() {
    // Forward-unknown: the Rust handler forwards the RustTypeErasedBox
    // downstream WITHOUT calling take (it never inspects the concrete type).
    // The inbound message reaches the tail and the outbound reaches the head,
    // unchanged and without error.
    let result = ffi::run_forward_unknown_test();
    assert_eq!(
        result.tail_reads, 1,
        "forwarded inbound message reaches the tail"
    );
    assert_eq!(
        result.head_writes, 1,
        "forwarded outbound message reaches the head"
    );
    assert!(
        !result.read_is_error,
        "pass-through forward is not an error"
    );
    assert!(
        !result.write_is_error,
        "pass-through forward is not an error"
    );
}
