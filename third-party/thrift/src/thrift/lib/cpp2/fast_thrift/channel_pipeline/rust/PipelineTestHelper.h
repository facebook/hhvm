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

#include <cstdint>

namespace channel_pipeline_rust::test {

struct BehaviorTestResult;
struct PipelineTestResult;
struct PositionTestResult;
struct LifecycleOrderResult;
struct ReadinessCycleResult;
struct RearmResult;
struct BidirectionalResult;
struct ProbeResult;
struct TeardownResult;
struct PanicResult;
struct IdentityResult;
struct AllocationResult;
struct CopyResult;
struct CloseResult;
struct StateMachineResult;
struct PanicRetentionResult;
struct ExceptionPreserveResult;
struct ReentrancyResult;
struct AdapterExtResult;
struct EventNoopResult;
struct ForwardUnknownResult;
struct ContextHandleTestResult;
struct ContextHandleFireResult;
struct ContextHandleSandwichResult;
struct ContextHandleExceptionResult;

PipelineTestResult run_pipeline_test() noexcept;
BehaviorTestResult run_behavior_test() noexcept;
PositionTestResult run_position_test_first() noexcept;
PositionTestResult run_position_test_middle() noexcept;
PositionTestResult run_position_test_last() noexcept;

LifecycleOrderResult run_lifecycle_order_test() noexcept;
ReadinessCycleResult run_read_recovery_test() noexcept;
ReadinessCycleResult run_write_recovery_test() noexcept;
RearmResult run_read_rearm_test() noexcept;
RearmResult run_multi_handler_rearm_test() noexcept;
BidirectionalResult run_bidirectional_test() noexcept;
ProbeResult run_readiness_probe_test() noexcept;
TeardownResult run_close_while_armed_test() noexcept;
TeardownResult run_inactive_while_armed_test() noexcept;
TeardownResult run_destroy_while_armed_test() noexcept;
PanicResult run_panic_containment_test() noexcept;

IdentityResult run_identity_test() noexcept;
AllocationResult run_allocation_probe_test() noexcept;
CopyResult run_copy_probe_test() noexcept;
CloseResult run_close_probe_test() noexcept;

StateMachineResult run_state_machine_test() noexcept;
PanicRetentionResult run_panic_retention_test() noexcept;
ExceptionPreserveResult run_exception_preserve_test() noexcept;
ReentrancyResult run_reentrancy_test() noexcept;

AdapterExtResult run_adapter_ext_test() noexcept;
EventNoopResult run_event_noop_test() noexcept;
ForwardUnknownResult run_forward_unknown_test() noexcept;
ContextHandleTestResult run_context_handle_test(uint32_t scenario) noexcept;
ContextHandleFireResult run_context_handle_fire_test(
    uint32_t scenario) noexcept;
ContextHandleSandwichResult run_context_handle_sandwich_test(
    uint32_t scenario) noexcept;
ContextHandleExceptionResult run_context_handle_exception_test(
    uint32_t scenario) noexcept;

} // namespace channel_pipeline_rust::test
