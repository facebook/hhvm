// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Reverse;
use std::sync::atomic::AtomicU64;
use std::sync::atomic::Ordering;
use std::time::SystemTime;

/// These are some nanosecond counters we use to roughly bucket where time is going.
///
/// They just collect wall clock time, so can't distinguish between 10 threads working
/// on something versus just one. Use a real profiler if you want that kind of detail.
#[derive(Default)]
pub(crate) struct Timers {
    pub(crate) doc_move_gains_nanos: AtomicU64,
    pub(crate) leaf_nanos: AtomicU64,
    pub(crate) remove_useless_nanos: AtomicU64,
    pub(crate) row_move_gains_nanos: AtomicU64,
    pub(crate) setup_nanos: AtomicU64,
    pub(crate) sort_nanos: AtomicU64,
    pub(crate) swap_nanos: AtomicU64,
}

impl Timers {
    fn all_times(&self) -> Vec<(&'static str, u64)> {
        let result: Vec<_> = [
            ("doc_move_gains", &self.doc_move_gains_nanos),
            ("leaf", &self.leaf_nanos),
            ("remove_useless", &self.remove_useless_nanos),
            ("row_move_gains", &self.row_move_gains_nanos),
            ("setup", &self.setup_nanos),
            ("sort", &self.sort_nanos),
            ("swap", &self.swap_nanos),
        ]
        .into_iter()
        .map(|(name, counter)| (name, counter.load(Ordering::Relaxed)))
        .collect();

        // Make sure we didn't forget any newly added fields above.
        let num_timer_fields = std::mem::size_of::<Timers>() / std::mem::size_of::<AtomicU64>();
        assert_eq!(result.len(), num_timer_fields);

        result
    }

    pub(crate) fn is_empty(&self) -> bool {
        self.all_times().into_iter().all(|(_, n)| n == 0)
    }

    pub(crate) fn log(&self, title: &str) {
        let mut times = self.all_times();
        times.sort_by_key(|x| (Reverse(x.1), x.0));

        let total_nanos: u64 = times.iter().map(|t| t.1).sum();
        let mut running_nanos = 0u64;

        log::info!(
            "Time breakdown, {}: {:.2}s total, {}",
            title,
            total_nanos as f64 * 1e-9,
            times
                .into_iter()
                .map(|(name, nanos)| {
                    running_nanos += nanos;

                    // Avoid division by zero.
                    let denom = std::cmp::max(total_nanos, 1) as f64;

                    format!(
                        "{}: {:.2}s ({:.2}%, {:.2}% so far)",
                        name,
                        nanos as f64 * 1e-9,
                        nanos as f64 * 100.0 / denom,
                        running_nanos as f64 * 100.0 / denom
                    )
                })
                .collect::<Vec<_>>()
                .join(", ")
        );
    }
}

pub(crate) struct TimerGuard<'a> {
    start: SystemTime,
    counter: Option<&'a AtomicU64>,
}

impl<'a> TimerGuard<'a> {
    pub(crate) fn new(counter: &'a AtomicU64) -> Self {
        Self {
            start: SystemTime::now(),
            counter: Some(counter),
        }
    }

    fn finish(&mut self, now: &SystemTime) {
        if let Some(counter) = self.counter.take() {
            if let Ok(duration) = now.duration_since(self.start) {
                counter.fetch_add(duration.as_nanos() as u64, Ordering::Relaxed);
            }
        }
    }

    /// End one timer at the same moment we start this one.
    pub(crate) fn handoff(mut ending: TimerGuard<'_>, counter: &'a AtomicU64) -> Self {
        let start = SystemTime::now();
        ending.finish(&start);
        Self {
            start,
            counter: Some(counter),
        }
    }
}

impl Drop for TimerGuard<'_> {
    fn drop(&mut self) {
        self.finish(&SystemTime::now());
    }
}
