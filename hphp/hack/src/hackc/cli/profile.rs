// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::fmt;
use std::fmt::Display;
use std::path::Path;
use std::path::PathBuf;
use std::sync::Arc;
use std::thread::JoinHandle;
use std::time::Duration;
use std::time::Instant;

use hash::HashMap;
use parking_lot::Mutex;

#[derive(Debug)]
pub(crate) struct Timing {
    total: Duration,
    histogram: hdrhistogram::Histogram<u64>,
    worst: Option<(Duration, PathBuf)>,
}

impl Timing {
    pub(crate) fn time<'a, T>(
        path: impl Into<Cow<'a, Path>>,
        f: impl FnOnce() -> T,
    ) -> (T, Timing) {
        let (r, t) = profile_rust::time(f);
        (r, Timing::from_duration(t, path))
    }

    pub(crate) fn from_duration<'a>(time: Duration, path: impl Into<Cow<'a, Path>>) -> Self {
        let mut histogram = hdrhistogram::Histogram::new(3).unwrap();
        histogram.record(time.as_micros() as u64).unwrap();
        Timing {
            total: time,
            histogram,
            worst: Some((time, path.into().into_owned())),
        }
    }

    pub(crate) fn total(&self) -> Duration {
        self.total
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

    pub(crate) fn fold_with(&mut self, rhs: Self) {
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
        };
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

pub(crate) fn report_stat(
    w: &mut impl std::io::Write,
    indent: &str,
    what: &str,
    timing: &Timing,
) -> std::io::Result<()> {
    if timing.is_empty() {
        return Ok(());
    }

    write!(
        w,
        "{}{}: total: {}, avg: {}",
        indent,
        what,
        timing.total.display(),
        timing.mean().display()
    )?;
    write!(w, ", P50: {}", timing.value_at_percentile(50.0).display())?;
    write!(w, ", P90: {}", timing.value_at_percentile(90.0).display())?;
    write!(w, ", P99: {}", timing.value_at_percentile(99.0).display())?;
    writeln!(w, ", max: {}", timing.max().display())?;

    if let Some(worst) = timing.worst() {
        eprintln!("{}  (max in {})", indent, worst.display());
    }

    Ok(())
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
                let mut shared = shared.lock();
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
        let mut shared = self.shared.lock();
        shared.finished = true;
        let count = shared.count;
        let duration = shared.start.elapsed();
        drop(shared);

        self.handle.join().unwrap();
        (count, duration)
    }

    pub(crate) fn start_file(&self, path: &Path) {
        let mut shared = self.shared.lock();
        shared.processing.insert(path.to_path_buf(), Instant::now());
    }

    pub(crate) fn finish_file(&self, path: &Path) {
        let mut shared = self.shared.lock();
        shared.count += 1;
        shared.processing.remove(path);
    }
}

#[derive(Debug)]
pub(crate) enum MaxValue<T: Ord> {
    Unset,
    Set(T, PathBuf),
}

impl<T: Ord> std::default::Default for MaxValue<T> {
    fn default() -> Self {
        Self::Unset
    }
}

impl<T: Ord> MaxValue<T> {
    pub(crate) fn new<'s>(value: T, file: impl Into<Cow<'s, Path>>) -> Self {
        Self::Set(value, file.into().to_path_buf())
    }

    pub(crate) fn fold_with(&mut self, other: Self) {
        match (self, other) {
            (_, Self::Unset) => {}
            (lhs @ Self::Unset, value) => {
                *lhs = value;
            }
            (Self::Set(a, file_a), Self::Set(b, file_b)) => {
                if *a < b {
                    *a = b;
                    *file_a = file_b;
                }
            }
        }
    }

    pub(crate) fn value(&self) -> &T {
        match self {
            Self::Unset => panic!("unset value"),
            Self::Set(value, _) => value,
        }
    }

    pub(crate) fn file(&self) -> &Path {
        match self {
            Self::Unset => panic!("unset value"),
            Self::Set(_, file) => file,
        }
    }
}

impl<T: Ord + Display> MaxValue<T> {
    pub(crate) fn report(&self, w: &mut impl std::io::Write, why: &str) -> std::io::Result<()> {
        writeln!(w, "{} {} (in {})", why, self.value(), self.file().display())
    }
}

#[derive(Debug, Default)]
pub(crate) struct ParserProfile {
    pub parse_peak: MaxValue<u64>,
    pub lower_peak: MaxValue<u64>,
    pub error_peak: MaxValue<u64>,

    pub arena_bytes: u64,

    pub parsing_t: Timing,
    pub lowering_t: Timing,
    pub elaboration_t: Timing,
    pub error_t: Timing,
    pub total_t: Timing,
}

impl ParserProfile {
    pub fn from_parser<'s>(
        parser: aast_parser::ParserProfile,
        file: impl Into<Cow<'s, Path>>,
    ) -> Self {
        let file = file.into();
        let aast_parser::ParserProfile {
            parse_peak,
            lower_peak,
            error_peak,
            arena_bytes,
            parsing_t,
            lowering_t,
            elaboration_t,
            error_t,
            total_t,
        } = parser;
        Self {
            parse_peak: MaxValue::new(parse_peak, file.to_path_buf()),
            lower_peak: MaxValue::new(lower_peak, file.to_path_buf()),
            error_peak: MaxValue::new(error_peak, file.to_path_buf()),
            arena_bytes,
            parsing_t: Timing::from_duration(parsing_t, file.to_path_buf()),
            lowering_t: Timing::from_duration(lowering_t, file.to_path_buf()),
            elaboration_t: Timing::from_duration(elaboration_t, file.to_path_buf()),
            error_t: Timing::from_duration(error_t, file.to_path_buf()),
            total_t: Timing::from_duration(total_t, file.to_path_buf()),
        }
    }

    pub fn fold_with(&mut self, other: Self) {
        self.parse_peak.fold_with(other.parse_peak);
        self.lower_peak.fold_with(other.lower_peak);
        self.error_peak.fold_with(other.error_peak);
        self.arena_bytes += other.arena_bytes;
        self.parsing_t.fold_with(other.parsing_t);
        self.lowering_t.fold_with(other.lowering_t);
        self.elaboration_t.fold_with(other.elaboration_t);
        self.error_t.fold_with(other.error_t);
        self.total_t.fold_with(other.total_t);
    }
}
