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
	ClientClosed *TimingSeries // # of client closed connections

	PanicCount *TimingSeries // event where clients thrift handler panic'd

	// Instantaneous counts of current number of requests being worked on
	ConnCount            *AtomicCounter
	SchedulingWorkCount  *AtomicCounter
	WorkingCount         *AtomicCounter
	SchedulingWriteCount *AtomicCounter
	WritingCount         *AtomicCounter
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
		ClientClosed: NewTimingSeries(cfg),

		PanicCount: NewTimingSeries(cfg),
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
	s = stats.PanicCount.MustSummarize(stats.statsPeriod)
	ints["requests.processor_panics."+periodStr] = int64(s.Count)

	return ints
}
