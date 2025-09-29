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
	"testing"
	"time"

	"github.com/stretchr/testify/require"
)

func TestStats(t *testing.T) {
	timingConfig := NewTimingConfig(60 * time.Second)
	stats := NewServerStats(timingConfig, 60*time.Second)

	ints := stats.GetInts()
	expectedInts := map[string]int64{
		"connections.client_ended.60":               0,
		"connections.connection_preempted_work.60":  0,
		"connections.not_listening.avg.60":          0,
		"connections.not_listening.p95.60":          0,
		"connections.not_listening.p99.60":          0,
		"connections.protocol_error.60":             0,
		"requests.duration_read.avg.60":             0,
		"requests.duration_read.p95.60":             0,
		"requests.duration_read.p99.60":             0,
		"requests.duration_schedule_work.avg.60":    0,
		"requests.duration_schedule_work.p95.60":    0,
		"requests.duration_schedule_work.p99.60":    0,
		"requests.duration_schedule_write.avg.60":   0,
		"requests.duration_schedule_write.p95.60":   0,
		"requests.duration_schedule_write.p99.60":   0,
		"requests.duration_working.avg.60":          0,
		"requests.duration_working.p95.60":          0,
		"requests.duration_working.p99.60":          0,
		"requests.duration_write.avg.60":            0,
		"requests.duration_write.p95.60":            0,
		"requests.duration_write.p99.60":            0,
		"requests.no_such_thrift_function.60":       0,
		"requests.pipelining_unsupported_client.60": 0,
		"requests.processor_panics.60":              0,
		"requests.scheduling_work":                  0,
		"requests.scheduling_write":                 0,
		"requests.task_expired.60":                  0,
		"requests.total_response_time.avg.60":       0,
		"requests.total_response_time.p95.60":       0,
		"requests.total_response_time.p99.60":       0,
		"requests.workers_busy.60":                  0,
		"requests.working":                          0,
		"requests.writing":                          0,
		"running_workers":                           0,
	}
	require.Equal(t, expectedInts, ints)
}
