// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::HashMap;
use std::borrow::Cow;
use std::fmt;
use std::fmt::Display;
use std::path::Path;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::Mutex;
use std::thread::JoinHandle;
use std::time::Duration;
use std::time::Instant;

pub(crate) struct Timing {
    total: Duration,
    histogram: hdrhistogram::Histogram<u64>,
    worst: Option<(Duration, PathBuf)>,
}

impl Timing {
    pub(crate) fn from_duration<'a>(time: Duration, path: impl Into<Cow<'a, Path>>) -> Self {
        let mut histogram = hdrhistogram::Histogram::new(3).unwrap();
        histogram.record(time.as_micros() as u64).unwrap();
        Timing {
            total: time,
            histogram,
            worst: Some((time, path.into().into_owned())),
        }
    }

    pub(crate) fn from_secs_f64<'a>(t: f64, path: impl Into<Cow<'a, Path>>) -> Self {
        Self::from_duration(Duration::from_secs_f64(t), path)
    }

    fn is_empty(&self) -> bool {
        self.histogram.is_empty()
    }

    fn mean(&self) -> Duration {
        Duration::from_micros(self.histogram.mean() as u64)
    }

    fn max(&self) -> Duration {
        Duration::from_micros(self.histogram.max())
    }

    fn value_at_percentile(&self, q: f64) -> Duration {
        Duration::from_micros(self.histogram.value_at_percentile(q))
    }

    fn worst(&self) -> Option<&Path> {
        self.worst.as_ref().map(|(_, path)| path.as_path())
    }
}

impl std::default::Default for Timing {
    fn default() -> Self {
        Self {
            total: Duration::from_secs(0),
            histogram: hdrhistogram::Histogram::new(3).unwrap(),
            worst: None,
        }
    }
}

impl std::ops::AddAssign for Timing {
    fn add_assign(&mut self, rhs: Self) {
        self.total += rhs.total;
        self.histogram.add(rhs.histogram).unwrap();
        self.worst = match (self.worst.take(), rhs.worst) {
            (None, None) => None,
            (lhs @ Some(_), None) => lhs,
            (None, rhs @ Some(_)) => rhs,
            (Some(lhs), Some(rhs)) => {
                if lhs.0 > rhs.0 {
                    Some(lhs)
                } else {
                    Some(rhs)
                }
            }
        }
    }
}

pub(crate) trait DurationEx {
    fn display(&self) -> DurationDisplay;
}

impl DurationEx for Duration {
    fn display(&self) -> DurationDisplay {
        DurationDisplay(*self)
    }
}

pub(crate) struct DurationDisplay(Duration);

impl Display for DurationDisplay {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.0 > Duration::from_secs(2) {
            write!(f, "{:.3}s", self.0.as_secs_f64())
        } else if self.0 > Duration::from_millis(2) {
            write!(f, "{:.3}ms", (self.0.as_micros() as f64) / 1_000.0)
        } else {
            write!(f, "{}us", self.0.as_micros())
        }
    }
}

pub(crate) fn to_hms(time: usize) -> String {
    if time >= 5400 {
        // > 90m
        format!(
            "{:02}h:{:02}m:{:02}s",
            time / 3600,
            (time % 3600) / 60,
            time % 60
        )
    } else if time > 90 {
        // > 90s
        format!("{:02}m:{:02}s", time / 60, time % 60)
    } else {
        format!("{}s", time)
    }
}

pub(crate) fn report_stat(indent: &str, what: &str, timing: &Timing) {
    if timing.is_empty() {
        return;
    }

    eprint!(
        "{}{}: total: {}, avg: {}",
        indent,
        what,
        timing.total.display(),
        timing.mean().display()
    );
    eprint!(", P50: {}", timing.value_at_percentile(50.0).display());
    eprint!(", P90: {}", timing.value_at_percentile(90.0).display());
    eprint!(", P99: {}", timing.value_at_percentile(99.0).display());
    eprintln!(", max: {}", timing.max().display());

    if let Some(worst) = timing.worst() {
        eprintln!("{}  (max in {})", indent, worst.display());
    }
}

struct StatusSharedData {
    count: usize,
    finished: bool,
    last_long_run_check: Instant,
    // start time for each active file
    processing: HashMap<PathBuf, Instant>,
    start: Instant,
    total: usize,
}

impl StatusSharedData {
    fn report_status(&mut self) {
        let wall = self.start.elapsed();
        let count = self.count;

        if self.total < 10 {
            return;
        }

        let wall_per_sec = if !wall.is_zero() {
            ((count as f64) / wall.as_secs_f64()) as usize
        } else {
            0
        };

        let remaining = if wall_per_sec > 0 {
            let left = (self.total - count) / wall_per_sec;
            format!(", {} remaining", to_hms(left))
        } else {
            "".into()
        };
        eprint!(
            "\rProcessed {count} / {total} in {wall:.3} ({wall_per_sec}/s{remaining})            ",
            total = self.total,
            wall = wall.display(),
        );

        if self.last_long_run_check.elapsed().as_secs() > 10 {
            let mut lead = "\nWARNING: Long-running files:\n";

            for (name, start) in self.processing.iter_mut() {
                let elapsed = start.elapsed();
                if elapsed.as_secs() > 10 {
                    eprintln!("{lead}    {} ({})", name.display(), elapsed.display());
                    lead = "";
                }
            }

            self.last_long_run_check = Instant::now();
        }
    }
}

pub(crate) struct StatusTicker {
    shared: Arc<Mutex<StatusSharedData>>,
    handle: JoinHandle<()>,
}

impl StatusTicker {
    pub(crate) fn new(total: usize) -> Self {
        let shared = Arc::new(Mutex::new(StatusSharedData {
            count: 0,
            finished: false,
            last_long_run_check: Instant::now(),
            processing: Default::default(),
            start: Instant::now(),
            total,
        }));

        let handle = Self::run_thread(&shared);

        Self { handle, shared }
    }

    fn run_thread(shared: &Arc<Mutex<StatusSharedData>>) -> JoinHandle<()> {
        let shared = Arc::clone(shared);

        std::thread::spawn(move || {
            loop {
                let mut shared = shared.lock().unwrap();
                if shared.finished {
                    break;
                }

                shared.report_status();
                drop(shared);

                std::thread::sleep(Duration::from_millis(1000));
            }
        })
    }

    pub(crate) fn finish(self) -> (usize, Duration) {
        let mut shared = self.shared.lock().unwrap();
        shared.finished = true;
        let count = shared.count;
        let duration = shared.start.elapsed();
        drop(shared);

        self.handle.join().unwrap();
        (count, duration)
    }

    pub(crate) fn start_file(&self, path: &Path) {
        let mut shared = self.shared.lock().unwrap();
        shared.processing.insert(path.to_path_buf(), Instant::now());
    }

    pub(crate) fn finish_file(&self, path: &Path) {
        let mut shared = self.shared.lock().unwrap();
        shared.count += 1;
        shared.processing.remove(path);
    }
}
