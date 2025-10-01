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
	PanicCount *TimingSeries // event where clients thrift handler panic'd

	// Instantaneous counts of current number of requests being worked on
	WorkingCount *AtomicCounter
}

// NewServerStats creates a new ServerStats object.
func NewServerStats(cfg *TimingConfig, statsPeriod time.Duration) *ServerStats {
	return &ServerStats{
		statsPeriod: statsPeriod,

		// instantaneous counters
		WorkingCount: &AtomicCounter{Counter: 0},

		// events/duration stats
		PanicCount: NewTimingSeries(cfg),
	}
}

// GetInts returns a map of server stats, ready for export.
func (stats *ServerStats) GetInts() map[string]int64 {
	ints := map[string]int64{}

	// instantaneous workers
	ints["running_workers"] = stats.WorkingCount.Get()

	// server event counters
	periodStr := fmt.Sprintf("%d", stats.statsPeriod/time.Second)

	s := stats.PanicCount.MustSummarize(stats.statsPeriod)
	ints["requests.processor_panics."+periodStr] = int64(s.Count)

	return ints
}
