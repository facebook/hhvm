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

package thrift

import "time"

// noopServerObserver is a no-op implementation of ServerObserver
// This serves as the default observer when no other implementation is provided
type noopServerObserver struct {
}

// Compile time interface check - ensure noopServerObserver implements ServerObserver
var _ ServerObserver = (*noopServerObserver)(nil)

// newNoopServerObserver creates a new noopServerObserver
func newNoopServerObserver() ServerObserver {
	return &noopServerObserver{}
}

// All methods are no-op implementations

// Connection lifecycle events
func (*noopServerObserver) ConnDropped()           {}
func (*noopServerObserver) ConnAccepted()          {}
func (*noopServerObserver) ConnTLSAccepted()       {}
func (*noopServerObserver) ReceivedHeaderRequest() {}

// Request processing events
func (*noopServerObserver) TaskKilled()          {}
func (*noopServerObserver) TaskTimeout()         {}
func (*noopServerObserver) DeclaredException()   {}
func (*noopServerObserver) UndeclaredException() {}
func (*noopServerObserver) ServerOverloaded()    {}
func (*noopServerObserver) ReceivedRequest()     {}
func (*noopServerObserver) SentReply()           {}
func (*noopServerObserver) ActiveRequests(_ int) {}

// Timing stats
func (*noopServerObserver) ProcessDelay(_ time.Duration) {}
func (*noopServerObserver) ProcessTime(_ time.Duration)  {}

// Function-level stats (no-op implementations)
func (*noopServerObserver) ReceivedRequestForFunction(_ string) {}
func (*noopServerObserver) AnyExceptionForFunction(_ string)    {}
