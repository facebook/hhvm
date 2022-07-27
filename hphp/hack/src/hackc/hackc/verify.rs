// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::compile::SingleFileOpts;
use crate::regex;
use anyhow::ensure;
use clap::Parser;
use compile::Profile;
use itertools::Itertools;
use log::info;
use multifile_rust as multifile;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use regex::Regex;
use std::cell::RefCell;
use std::collections::HashMap;
use std::fmt;
use std::fmt::Display;
use std::fs;
use std::path::Path;
use std::path::PathBuf;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::AtomicUsize;
use std::sync::atomic::Ordering;
use std::sync::Arc;
use std::time::Duration;
use std::time::Instant;
use thiserror::Error;

// Several of these would be better as the underlying error (anyhow::Error or
// std::io::Error) but then we couldn't derive Hash or Eq.
#[derive(Error, Debug, Hash, PartialEq, Eq)]
enum VerifyError {
    #[error("assemble error {0}")]
    AssembleError(String),
    #[error("compile error {0}")]
    CompileError(String),
    #[error("multifile error: {0}")]
    MultifileError(String),
    #[error("panic {0}")]
    Panic(String),
    #[error("printing error: {0}")]
    PrintError(String),
    #[error("unable to read file: {0}")]
    ReadError(String),
    #[error("units mismatch: {0}")]
    UnitMismatchError(crate::cmp_unit::CmpError),
}

type Result<T, E = VerifyError> = std::result::Result<T, E>;

thread_local! {
    pub static PANIC_MSG: RefCell<Option<String>> = RefCell::new(None);
}

#[derive(Clone, Parser, Debug)]
struct CommonOpts {
    /// The input Hack files or directories to process.
    #[clap(name = "PATH")]
    paths: Vec<PathBuf>,

    /// Print full error messages
    #[clap(short = 'l')]
    long_msg: bool,

    #[allow(dead_code)]
    #[clap(flatten)]
    single_file_opts: SingleFileOpts,

    /// Number of parallel worker threads. By default, or if set to 0, use num-cpu threads.
    #[clap(long, default_value = "0")]
    num_threads: usize,

    /// If true then panics will abort the process instead of being caught.
    #[clap(long)]
    panic_fuse: bool,

    /// Print all errors
    #[clap(short = 'a')]
    show_all: bool,
}

#[derive(Clone, Parser, Debug)]
struct AssembleOpts {
    #[clap(flatten)]
    common: CommonOpts,
}

impl AssembleOpts {
    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut Profile) -> Result<()> {
        let pre_alloc = bumpalo::Bump::default();
        let (env, pre_unit) = compile_php_file(
            &pre_alloc,
            path,
            content,
            &self.common.single_file_opts,
            profile,
        )?;

        let mut output: Vec<u8> = Vec::new();
        let mut profile = compile::Profile::default();
        compile::unit_to_string(&env, &mut output, &pre_unit, &mut profile)
            .map_err(|err| VerifyError::PrintError(err.to_string()))?;

        let post_alloc = bumpalo::Bump::default();
        let (post_unit, _) = crate::assemble::assemble_from_bytes(
            &post_alloc,
            &output,
            /* print tokens */ false,
        )
        .map_err(|err| VerifyError::AssembleError(truncate_pos_err(err.to_string())))?;

        crate::cmp_unit::cmp_hack_c_unit(&pre_unit, &post_unit)
            .map_err(VerifyError::UnitMismatchError)
    }
}

#[derive(Clone, Parser, Debug)]
enum Mode {
    Assemble(AssembleOpts),
}

impl Mode {
    fn common(&self) -> &CommonOpts {
        match self {
            Mode::Assemble(AssembleOpts { common, .. }) => common,
        }
    }

    fn common_mut(&mut self) -> &mut CommonOpts {
        match self {
            Mode::Assemble(AssembleOpts { common, .. }) => common,
        }
    }

    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut Profile) -> Result<()> {
        match self {
            Mode::Assemble(opts) => opts.verify_file(path, content, profile),
        }
    }
}

#[derive(Parser, Clone, Debug)]
pub struct Opts {
    #[clap(subcommand)]
    mode: Mode,
}

fn compile_php_file<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    path: &'a Path,
    content: Vec<u8>,
    single_file_opts: &'a SingleFileOpts,
    profile: &mut Profile,
) -> Result<(compile::NativeEnv<'a>, hhbc::hackc_unit::HackCUnit<'arena>)> {
    let filepath = RelativePath::make(Prefix::Dummy, path.to_path_buf());
    let source_text = SourceText::make(RcOc::new(filepath.clone()), &content);
    let env = crate::compile::native_env(filepath, single_file_opts);
    let unit = compile::unit_from_text(alloc, source_text, &env, None, profile)
        .map_err(|err| VerifyError::CompileError(err.to_string()))?;
    Ok((env, unit))
}

/// Truncates "Pos { line: 5, col: 2}" to "Pos ...", because in verify tool this isn't important
fn truncate_pos_err(err_str: String) -> String {
    let pos_reg = regex!(r"Pos \{ line: \d+, col: \d+ \}");
    pos_reg.replace(err_str.as_str(), "Pos {...}").to_string()
}

fn with_catch_panics<F>(profile: &mut Profile, long_msg: bool, action: F) -> Result<()>
where
    F: FnOnce(&mut Profile) -> Result<()> + std::panic::UnwindSafe,
{
    let result = std::panic::catch_unwind(|| {
        let mut inner_profile = Profile::default();
        let ok = action(&mut inner_profile);
        (ok, inner_profile)
    });
    match result {
        Ok((e, inner_profile)) => {
            *profile = inner_profile;
            e
        }
        Err(err) => {
            let msg = if let Some(msg) = PANIC_MSG.with(|tls| tls.borrow_mut().take()) {
                msg
            } else {
                panic_message::panic_message(&err).to_string()
            };

            let msg = truncate_pos_err(msg);
            let mut msg = msg.replace('\n', "\\n");
            if !long_msg && msg.len() > 80 {
                msg.truncate(77);
                msg.push_str("...");
            }
            Err(VerifyError::Panic(msg))
        }
    }
}

struct Timing {
    total: Duration,
    histogram: hdrhistogram::Histogram<u64>,
    worst: Option<(Duration, PathBuf)>,
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

trait DurationEx {
    fn display(&self) -> DurationDisplay;
}

impl DurationEx for Duration {
    fn display(&self) -> DurationDisplay {
        DurationDisplay(*self)
    }
}

struct DurationDisplay(Duration);

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

fn to_hms(time: usize) -> String {
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

fn report_status(wall: Duration, count: usize, total: usize) {
    if total < 10 {
        return;
    }

    let wall_per_sec = if !wall.is_zero() {
        ((count as f64) / wall.as_secs_f64()) as usize
    } else {
        0
    };

    let remaining = if wall_per_sec > 0 {
        let left = (total - count) / wall_per_sec;
        format!(", {} remaining", to_hms(left))
    } else {
        "".into()
    };
    eprint!(
        "\rProcessed {count} / {total} in {:.3} ({wall_per_sec}/s{remaining})            ",
        wall.display(),
    );
}

fn report_final(wall: Duration, count: usize, total: usize, profile: &ProfileAcc, show_all: bool) {
    if total >= 10 {
        let wall_per_sec = if !wall.is_zero() {
            ((count as f64) / wall.as_secs_f64()) as usize
        } else {
            0
        };
        eprintln!(
            "\rProcessed {count} files in {wall} ({wall_per_sec}/s)",
            wall = wall.display(),
        );
    }

    if profile.passed {
        println!("All files passed.");
    } else {
        println!("Failed to complete");
    }

    let num_show = if show_all {
        profile.error_histogram.len()
    } else {
        20
    };

    // The # of files that failed are sum of error_histogram's values' usize field
    if !profile.error_histogram.is_empty() {
        println!("{}/{} files passed", total - profile.num_failed(), total);
        println!("Failure histogram:");
        for (k, v) in profile
            .error_histogram
            .iter()
            .sorted_by(|a, b| a.1.0.cmp(&b.1.0).reverse())
            .take(num_show)
        {
            println!("  {:3} ({}): {}", v.0, v.1.display(), k);
        }
        if profile.error_histogram.len() > 20 {
            println!(
                "  (and {} unreported)",
                profile.error_histogram.len() - num_show
            );
        }
        println!();
    }
}

struct ProfileAcc {
    passed: bool,
    error_histogram: HashMap<VerifyError, (usize, PathBuf)>,
}

impl std::default::Default for ProfileAcc {
    fn default() -> Self {
        Self {
            passed: true,
            error_histogram: Default::default(),
        }
    }
}

impl ProfileAcc {
    fn fold(mut self, other: Self) -> Self {
        self.passed = self.passed && other.passed;
        for (err, (n, example)) in other.error_histogram {
            self.error_histogram
                .entry(err)
                .or_insert_with(|| (0, example))
                .0 += n;
        }
        self
    }

    fn record_error(mut self, err: VerifyError, f: &Path) -> Self {
        self.passed = false;
        self.error_histogram
            .entry(err)
            .or_insert_with(|| (0, f.to_path_buf()))
            .0 += 1;
        self
    }

    fn num_failed(&self) -> usize {
        self.error_histogram
            .values()
            .fold(0, |acc, (val, _)| acc + val)
    }
}

fn verify_one_file(path: &Path, mode: &Mode) -> ProfileAcc {
    let acc = ProfileAcc::default();

    let content = match fs::read(path) {
        Ok(content) => content,
        Err(err) => {
            return acc.record_error(VerifyError::ReadError(err.to_string()), path);
        }
    };

    let files = match multifile::to_files(path, content) {
        Ok(files) => files,
        Err(err) => {
            return acc.record_error(VerifyError::MultifileError(err.to_string()), path);
        }
    };

    for (path, content) in files {
        let mut file_profile = Profile::default();
        let result = if mode.common().panic_fuse {
            mode.verify_file(&path, content, &mut file_profile)
        } else {
            with_catch_panics(&mut file_profile, mode.common().long_msg, |profile| {
                mode.verify_file(&path, content, profile)
            })
        };
        if let Err(err) = result {
            return acc.record_error(err, &path);
        }
    }

    acc
}

fn verify_files(files: &[PathBuf], mode: &Mode) -> anyhow::Result<()> {
    let total = files.len();
    let count = Arc::new(AtomicUsize::new(0));
    let finished = Arc::new(AtomicBool::new(false));
    let start = Instant::now();

    let status_handle = {
        let count = count.clone();
        let finished = finished.clone();
        std::thread::spawn(move || {
            while !finished.load(Ordering::Acquire) {
                report_status(start.elapsed(), count.load(Ordering::Acquire), total);
                std::thread::sleep(Duration::from_millis(1000));
            }
        })
    };

    let verify_one_file_ = |acc: ProfileAcc, f: &PathBuf| -> ProfileAcc {
        let profile = verify_one_file(f, mode);
        count.fetch_add(1, Ordering::Release);
        acc.fold(profile)
    };

    println!("Files: {}", files.len());

    let profile = if mode.common().num_threads == 1 {
        files.iter().fold(ProfileAcc::default(), verify_one_file_)
    } else {
        files
            .par_iter()
            .with_max_len(1)
            .fold(ProfileAcc::default, verify_one_file_)
            .reduce(ProfileAcc::default, ProfileAcc::fold)
    };

    finished.store(true, Ordering::Release);
    let duration = start.elapsed();

    status_handle.join().unwrap();

    report_final(
        duration,
        count.load(Ordering::Acquire),
        total,
        &profile,
        mode.common().show_all,
    );

    ensure!(
        profile.passed,
        "{} files failed to verify",
        profile.num_failed()
    );
    Ok(())
}

pub fn run(mut opts: Opts) -> anyhow::Result<()> {
    eprint!("Collecting files...");
    info!("Collecting files");
    let files = {
        let start = Instant::now();
        let files = crate::util::collect_files(
            &opts.mode.common().paths,
            None,
            opts.mode.common().num_threads,
        )?;
        let duration = start.elapsed();
        info!("{} files found in {}", files.len(), duration.display());
        files
    };
    eprint!("\r");

    if files.len() == 1 {
        opts.mode.common_mut().panic_fuse = true;
    }

    let mut old_hook = None;
    if !opts.mode.common().panic_fuse {
        old_hook = Some(std::panic::take_hook());
        std::panic::set_hook(Box::new(|info| {
            let msg = panic_message::panic_info_message(info);
            let msg = if let Some(loc) = info.location() {
                let mut file = loc.file().to_string();
                if file.len() > 20 {
                    file.replace_range(0..file.len() - 17, "...");
                }
                format!("{}@{}: {}", file, loc.line(), msg)
            } else {
                format!("<unknown>: {}", msg)
            };
            PANIC_MSG.with(|tls| {
                *tls.borrow_mut() = Some(msg);
            });
        }));
    }

    verify_files(&files, &opts.mode)?;

    if let Some(old_hook) = old_hook {
        std::panic::set_hook(old_hook);
    }

    Ok(())
}
