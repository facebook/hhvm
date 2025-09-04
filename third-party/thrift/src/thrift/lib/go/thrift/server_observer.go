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

// ServerObserver interface defines methods for observing server events
// for monitoring and troubleshooting purposes.
type ServerObserver interface {
	// Connection lifecycle events
	ConnDropped()  // server-initiated connection close
	ConnAccepted() // new connection successfully accepted

	// Request processing events
	TaskKilled()                    // request rejected due to parsing errors or shutdown
	TaskTimeout()                   // request processing exceeded timeout
	DeclaredException()             // handler returned declared error type
	UndeclaredException()           // handler returned unexpected error or panic
	ServerOverloaded()              // request rejected due to load shedding
	QueueTimeout()                  // request timeout before processing began
	ReceivedRequest()               // complete request received and parsed
	SentReply()                     // response successfully written to connection
	ActiveRequests(numRequests int) // current count of processing requests

	// Timing metrics
	ProcessDelay(delay time.Duration)                // time from request received to handler start
	ProcessTime(duration time.Duration)              // time spent in handler execution
	SocketSuccessProcessTime(duration time.Duration) // time from connection accept to first request completion
}
