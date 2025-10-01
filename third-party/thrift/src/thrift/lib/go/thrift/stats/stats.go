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

package stats

import (
	"fmt"
	"time"
)

// ServerStats holds relevant Thwork server stats.
type ServerStats struct {
	// reporting period for stats
	statsPeriod time.Duration

	// counters
	ClientClosed            *TimingSeries // # of client closed connections
	ConnectionPreemptedWork *TimingSeries // event where we didn't perform work due to connection being closed

	ProtocolError   *TimingSeries // event where client spoke invalid protocol
	PanicCount      *TimingSeries // event where clients thrift handler panic'd
	NoSuchFunction  *TimingSeries // event where client called a non-existent function
	QueueingTimeout *TimingSeries // event where we didn't perform work due to client timeout exceeded

	// Instantaneous counts of current number of requests being worked on
	ConnCount            *AtomicCounter
	SchedulingWorkCount  *AtomicCounter
	WorkingCount         *AtomicCounter
	SchedulingWriteCount *AtomicCounter
	WritingCount         *AtomicCounter

	// workersBusy defines a server event where no workers are available to accept
	// available work. This can signal that we should increase # workers (in the case
	// that workers are all blocking) or that the server is overloaded
	// (in the case the work is all CPU bound)
	WorkersBusy *TimingSeries
	// pipeliningUnsupportedClient is an event where a request is coming from
	// a client that doesn't support out of order responses (see tHeader bit).
	// This means thwork cannot return out of order responses, and in this case,
	// thwork will execute the work + respond inline (no pipelining) rather than
	// send work to worker pool and fan-in the responses in-order.
	// In practice, this doesn't happen unless we are talking to thwork over raw
	// connections (not SR/SRProxy) from a pipeline-unsupported client (like Golang)
	PipeliningUnsupportedClient *TimingSeries
}

// NewServerStats creates a new ServerStats object.
func NewServerStats(cfg *TimingConfig, statsPeriod time.Duration) *ServerStats {
	return &ServerStats{
		statsPeriod: statsPeriod,

		// instantaneous counters
		ConnCount:            &AtomicCounter{Counter: 0},
		SchedulingWorkCount:  &AtomicCounter{Counter: 0},
		WorkingCount:         &AtomicCounter{Counter: 0},
		SchedulingWriteCount: &AtomicCounter{Counter: 0},
		WritingCount:         &AtomicCounter{Counter: 0},

		// events/duration stats
		ClientClosed:            NewTimingSeries(cfg),
		ConnectionPreemptedWork: NewTimingSeries(cfg),

		ProtocolError:               NewTimingSeries(cfg),
		PanicCount:                  NewTimingSeries(cfg),
		WorkersBusy:                 NewTimingSeries(cfg),
		PipeliningUnsupportedClient: NewTimingSeries(cfg),
		NoSuchFunction:              NewTimingSeries(cfg),
		QueueingTimeout:             NewTimingSeries(cfg),
	}
}

// GetInts returns a map of server stats, ready for export.
func (stats *ServerStats) GetInts() map[string]int64 {
	ints := map[string]int64{}

	// instantaneous workers
	ints["running_workers"] = stats.WorkingCount.Get()

	// instantaneous request pipeline counters
	ints["requests.scheduling_work"] = stats.SchedulingWorkCount.Get()
	ints["requests.working"] = stats.WorkingCount.Get()
	ints["requests.scheduling_write"] = stats.SchedulingWriteCount.Get()
	ints["requests.writing"] = stats.WritingCount.Get()

	// server event counters
	periodStr := fmt.Sprintf("%d", stats.statsPeriod/time.Second)

	s := stats.ClientClosed.MustSummarize(stats.statsPeriod)
	ints["connections.client_ended."+periodStr] = int64(s.Count)
	s = stats.ConnectionPreemptedWork.MustSummarize(stats.statsPeriod)
	ints["connections.connection_preempted_work."+periodStr] = int64(s.Count)
	s = stats.ProtocolError.MustSummarize(stats.statsPeriod)
	ints["connections.protocol_error."+periodStr] = int64(s.Count)
	s = stats.WorkersBusy.MustSummarize(stats.statsPeriod)
	ints["requests.workers_busy."+periodStr] = int64(s.Count)
	s = stats.PipeliningUnsupportedClient.MustSummarize(stats.statsPeriod)
	ints["requests.pipelining_unsupported_client."+periodStr] = int64(s.Count)
	s = stats.QueueingTimeout.MustSummarize(stats.statsPeriod)
	ints["requests.task_expired."+periodStr] = int64(s.Count)
	s = stats.PanicCount.MustSummarize(stats.statsPeriod)
	ints["requests.processor_panics."+periodStr] = int64(s.Count)
	s = stats.NoSuchFunction.MustSummarize(stats.statsPeriod)
	ints["requests.no_such_thrift_function."+periodStr] = int64(s.Count)

	return ints
}
