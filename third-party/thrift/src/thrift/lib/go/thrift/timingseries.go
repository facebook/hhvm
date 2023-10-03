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

import (
	"fmt"
	"math"
	"sync/atomic"
	"time"
)

// AtomicCounter is a simple non-blocking incr/decr counter for instantaneous values exported by thrift
type AtomicCounter struct {
	Counter int64
}

// Incr non blocking increments by 1
func (ac *AtomicCounter) Incr() int64 {
	return atomic.AddInt64(&ac.Counter, 1)
}

// Decr non blocking increments by 1
func (ac *AtomicCounter) Decr() int64 {
	return atomic.AddInt64(&ac.Counter, -1)
}

// Get non blocking gets current value
func (ac *AtomicCounter) Get() int64 {
	return atomic.LoadInt64(&ac.Counter)
}

// TimingSeries keeps rolling statistics for duration based
// events over a period of time.  As each event occurs you
// call .Record() to record the event, and then can request
// variable length statistics reports via .Summarize()
//
// Usage:
//   ts := NewTimingSeries(nil)
//   start := time.Now()
//   doSomethingThatYouWantToMeasure()
//   ts.Record(time.Since(start))
//
//   summary := ts.Summarize(60 * time.Second)
//   fmt.Printf("%s\n", summary.String())
type TimingSeries struct {
	start     time.Time
	buckets   []bucket
	pending   bucket
	interval  time.Duration
	precision time.Duration
}

// TimingConfig is the configuration for a TimingSeries.
type TimingConfig struct {
	// How far back to record events.  Longer durations require slightly
	// more computation.  For example, If you are wishing to report 60s
	// summary data, then 120s should be fine.
	History time.Duration
	// Precision of collected data.  tl;dr; Use time.Microsecond.
	//
	// Events consist of a duration which we quantize into units of
	// precision.  Higher (smaller) precisions required we store larger
	// numbers.
	//
	// For this reason, don't use use precision much greater than what
	// you need.  A microsecond should be sufficient in most cases and
	// is almost always safe if you are measuring less than an hour's worth
	// of event data each second.
	Precision time.Duration
	// Most granular interval to record.  1s is recommended.  More granularity
	// gives you finer grained reporting at the cost of marginally increased
	// computation during event recording and summary generation.
	Interval time.Duration
}

// DefaultConfig records timing data at μs precision, remembers 120s
// of history, and has 1 second granularity.
// PRO TIP: use the defaults.
var DefaultConfig = TimingConfig{
	History:   120 * time.Second,
	Precision: time.Microsecond,
	Interval:  time.Second,
}

// noDuration is a special duration value that will cause the timing series to
// record counts and status, but not timing data
const noDuration = time.Duration(-1)

// NewTimingSeries allocate a new TimingSeries.  If cfg is nil,
// defaults will be used.
func NewTimingSeries(cfg *TimingConfig) *TimingSeries {
	if cfg == nil {
		cfg = &DefaultConfig
	}

	numBuckets := int(cfg.History / cfg.Interval)

	ts := TimingSeries{
		start:     time.Now(),
		interval:  cfg.Interval,
		buckets:   make([]bucket, numBuckets),
		precision: cfg.Precision,
	}

	// initialize the pending bucket
	ts.pending.min = math.MaxUint64

	return &ts
}

// Record is a thread-safe non-blocking call that records an
// observed duration
func (ts *TimingSeries) Record(duration time.Duration) {
	ts.record(duration, nil)
}

// RecordValue records an observed value rather than duration. All other semantics
// remain the same as Record
func (ts *TimingSeries) RecordValue(val int64) {
	ts.record(time.Duration(val)*ts.precision, nil)
}

// RecordWithStatus is a thread-safe non-blocking call that records an
// observed duration AND a success or fail bit.  This allows one to
// use the TimingSeries for both (i.e.) API status metrics and API
// latency/duration metrics.
func (ts *TimingSeries) RecordWithStatus(duration time.Duration, status bool) {
	ts.record(duration, &status)
}

// RecordEvent bumps the success counter of the timing series.  This is useful
// when you wish to use timingseries to record simple events with time bucketing
func (ts *TimingSeries) RecordEvent() {
	ts.record(noDuration, nil)
}

func (ts *TimingSeries) record(duration time.Duration, status *bool) {
	ts.updateBuckets()

	atomic.AddUint64(&ts.pending.count, 1)

	if status != nil {
		if *status {
			atomic.AddUint64(&ts.pending.success, 1)
		} else {
			atomic.AddUint64(&ts.pending.fail, 1)
		}
	}

	// if there is no duration, then return early
	if duration == noDuration {
		return
	}

	// now quantize the duration into an integer multiplier of
	// the desired precision
	quantized := uint64(duration / ts.precision)

	atomic.AddUint64(&ts.pending.sum, quantized)
	atomic.AddUint64(&ts.pending.sos, quantized*quantized)

	// when updating min and max, take into account the fact that
	// others may be updating simultaneously.
	// 1. load the current value
	// 2. (value may change)
	// 3. test the current value against the new candidate
	// 4. if the candidate is not viable (gt or lt), done
	// 5. if the candidate is viable, atomically store it and
	//    update candidate value with the new current value
	// 6. goto 1
	minKnown := quantized
	for {
		cur := atomic.LoadUint64(&ts.pending.min)
		if cur > minKnown {
			minKnown = atomic.SwapUint64(&ts.pending.min, minKnown)
		} else {
			break
		}
	}

	// max
	maxKnown := quantized
	for {
		cur := atomic.LoadUint64(&ts.pending.max)
		if cur < maxKnown {
			maxKnown = atomic.SwapUint64(&ts.pending.max, maxKnown)
		} else {
			break
		}
	}
}

// Summary describes the duration events that occurred over an
// interval provided to the .Summarize() method of TimingSeries
type Summary struct {
	// the number of events
	Count uint64
	// the Maximum duration
	Maximum time.Duration
	// the Minimum duration
	Minimum time.Duration
	// the average duration
	Average time.Duration
	// the p99 duration
	P99 time.Duration
	// the p95 duration
	P95 time.Duration
	// the p90 duration
	P90 time.Duration
	// the p99.9 duration
	P999 time.Duration
	// time period analyzed
	Period time.Duration
	// The number of successful events (requires .RecordWithStatus())
	Success uint64
	// The number of failed events (requires .RecordWithStatus())
	Fail uint64
}

func (s *Summary) String() string {
	str := fmt.Sprintf("Over a %s period:\n", s.Period)
	str += fmt.Sprintf("  %d requests processed. min %s.  max %s\n",
		s.Count, s.Minimum, s.Maximum)
	// if success + fail == count then clearly the client is recording status
	// and so should we
	if s.Count > 0 && s.Success+s.Fail == s.Count {
		str += fmt.Sprintf("  %d failures, %0.2f error rate\n",
			s.Fail, float64(s.Fail)/float64(s.Count))
	}
	str += fmt.Sprintf("  avg: %s\n", s.Average)
	str += fmt.Sprintf("  90th: %s / 95th %s / 99th %s / 99.9th %s\n", s.P90, s.P95, s.P99, s.P999)
	return str
}

func (ts *TimingSeries) decrementBid(bid *uint32) {
	if *bid == 0 {
		*bid = uint32(len(ts.buckets) - 1)
	} else {
		*bid--
	}
}

func (ts *TimingSeries) incrementBid(bid *uint32) {
	*bid++
	*bid %= uint32(len(ts.buckets))
}

// MustSummarize returns satistics for the defined period and panics
// if a period greater than the history of the timing series is specified
func (ts *TimingSeries) MustSummarize(period time.Duration) *Summary {
	s, err := ts.Summarize(period)
	if err != nil {
		panic(err.Error())
	}
	return s
}

// Summarize satistics for the defined period
func (ts *TimingSeries) Summarize(period time.Duration) (*Summary, error) {
	ts.updateBuckets()

	// now skip the current interval as it is incomplete.
	_, bid := ts.bucketID()
	ts.decrementBid(&bid)

	// do we have sufficient data to summarize the specified period?
	numBuckets := int(period / ts.interval)
	if numBuckets > len(ts.buckets) {
		return nil, fmt.Errorf("data maintained for %s, which is less that %s requested",
			ts.interval*time.Duration(len(ts.buckets)), period)
	}
	var summary Summary
	summary.Period = period
	var sum, sos uint64
	for numBuckets > 0 {
		if ts.buckets[bid].count > 0 {
			max := time.Duration(ts.buckets[bid].max) * ts.precision
			if max > summary.Maximum || summary.Count == 0 {
				summary.Maximum = max
			}
			min := time.Duration(ts.buckets[bid].min) * ts.precision
			if min < summary.Minimum || summary.Count == 0 {
				summary.Minimum = min
			}
			summary.Count += ts.buckets[bid].count
			summary.Success += ts.buckets[bid].success
			summary.Fail += ts.buckets[bid].fail
			sum += ts.buckets[bid].sum
			sos += ts.buckets[bid].sos
		}
		ts.decrementBid(&bid)
		numBuckets--
	}
	if summary.Count > 0 {
		summary.Average = (time.Duration(sum) * ts.precision) / time.Duration(summary.Count)

		// Using the running sum and sum of squares of data samples,
		// we can figure out the standard deviation (σ), which in turn
		// lets us calculate percentiles.
		// S₁ = ∑ᵢ aᵢ
		// S₂ = ∑ᵢ aᵢ²
		// μ = S₁/n
		// σ² = S₂/n - μ²

		S1 := float64(sum)
		S2 := float64(sos)
		n := float64(summary.Count)
		μ := S1 / n
		σ2 := (S2 / n) - math.Pow(μ, 2.)
		σ := math.Sqrt(σ2)

		// now that we found σ, get our percentiles
		// X = μ + Zσ
		//
		// For Z values and their correspondence to percentiles,
		// find a chart.  like this one:
		// https://en.wikipedia.org/wiki/Standard_normal_table
		// or generate it here:
		// https://surfstat.anu.edu.au/surfstat-home/tables/normal.php
		summary.P90 = time.Duration(μ+1.282*σ) * ts.precision
		summary.P95 = time.Duration(μ+1.645*σ) * ts.precision
		summary.P99 = time.Duration(μ+2.326*σ) * ts.precision
		summary.P999 = time.Duration(μ+3.09*σ) * ts.precision
	}

	return &summary, nil
}

// bucket holds a set of measurments for a single interval
type bucket struct {
	// epoch is a monotonically increasing integer that is
	// implemented as the number of intervals since the instantiation
	// of the timing series.  epochs are useful to determine which
	// buckets are out of date - and in related to our lazy cleanup
	// strategy.  If an interval elapses without any events, we can
	// know this because the epoch will no be in the expected
	// sequence
	epoch uint64
	// the number of events
	count uint64
	// NOTE: success and fail allow one to use only this time series
	// implementation for both latency AND call success/fail metrics.
	// This is conflating two arguably distinct jobs for the goals of
	// efficiency and convenience.
	//
	// the number of "successful" events
	success uint64
	// the number of "failed" events
	fail uint64
	// the sum of the value of the events in this interval
	sum uint64
	// the minimum value event in this interval
	min uint64
	// the maximum value event in the interval
	max uint64
	// the sum of squares of the values of events that occurred in this
	// interval, which will allow us to approximate σ later without
	// keeping around lots of samples.
	//
	// can we safely keep sum of squares in a uint64?
	// √(2⁶⁴) μs == 4294967296μs == 71.6 minutes
	// If our precision is set at 1 μs then we can record
	// about an hour of elapsed operations time per second.
	// Assuming that operation time corresponds roughly to
	// computation time, and we're pretty darn safe.
	sos uint64
}

// determine the current bucket into which we're collecting data as
// well as the current epoch
func (ts *TimingSeries) bucketID() (epoch uint64, bucketid uint32) {
	epoch = uint64(time.Since(ts.start).Round(ts.interval) / (ts.interval))
	bucketid = uint32(epoch % uint64(len(ts.buckets)))
	return
}

// updateBuckets uses atomic integer operations to ensure threadsafe operations
// without locking.
//
// The first thread to call updateBuckets when the pending bucket is out of date
// merges the data into the ringbuffer and scans backwards through history to
// update (zero) any buckets which have stale data.
func (ts *TimingSeries) updateBuckets() {
	epoch, bid := ts.bucketID()

	// does the pending bucket need updating?
	oldepoch := atomic.LoadUint64(&(ts.pending.epoch))

	if oldepoch >= epoch {
		// we are in the past - which is possible under heavily loaded servers -
		// a thread that synthesized the epoch from the clock after us actually
		// got the "lock" before us. to understand where this is possible,
		// remove this return and run the unit test several times.
		return
	}

	// try to update (effectively, grab the lock that doesn't exist)
	if !atomic.CompareAndSwapUint64(&(ts.pending.epoch), oldepoch, epoch) {
		// someone else updated pending, its their job to do the update
		return
	}

	// oops!  the pending bucket is out of date, and it is our job
	// to merge it into the ringbuffer.
	var tgt *bucket
	// was the last update before our reporting period?
	if epoch-oldepoch > uint64(len(ts.buckets)) {
		// throwaway the data
		tgt = &bucket{}
	} else {
		tgtid := uint32(oldepoch % uint64(len(ts.buckets)))
		tgt = &(ts.buckets[tgtid])
	}
	// now let's zero the pending bucket into the target.  it is
	// possible that some of the data will be spread across two buckets,
	// which is fine.
	tgt.epoch = oldepoch
	tgt.count = atomic.SwapUint64(&ts.pending.count, 0)
	tgt.success = atomic.SwapUint64(&ts.pending.success, 0)
	tgt.fail = atomic.SwapUint64(&ts.pending.fail, 0)
	tgt.sum = atomic.SwapUint64(&ts.pending.sum, 0)
	tgt.min = atomic.SwapUint64(&ts.pending.min, math.MaxUint64)
	tgt.max = atomic.SwapUint64(&ts.pending.max, 0)
	tgt.sos = atomic.SwapUint64(&ts.pending.sos, 0)

	// finally, let's walk back through the ringbuffer and zero out
	// buckets that are out of date skipping the one we just updated
	for i := 0; i < len(ts.buckets); i++ {
		// don't go back before the beginning of time
		if epoch == 0 {
			break
		}
		ts.decrementBid(&bid)
		epoch--
		// skip what we just updated
		if ts.buckets[bid].epoch == oldepoch {
			continue
		}
		// if we've gone back to a bucket that is up to date, we're done
		if ts.buckets[bid].epoch == epoch {
			break
		}
		// otherwise zero and continue working backwards
		ts.buckets[bid].epoch = epoch
		ts.buckets[bid].count = 0
		ts.buckets[bid].success = 0
		ts.buckets[bid].fail = 0
		ts.buckets[bid].sum = 0
		ts.buckets[bid].min = math.MaxUint64
		ts.buckets[bid].max = 0
		ts.buckets[bid].sos = 0
	}
}
