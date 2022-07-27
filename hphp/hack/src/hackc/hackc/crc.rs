// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::compile::SingleFileOpts;
use crate::profile;
use crate::profile::DurationEx;
use crate::profile::StatusTicker;
use crate::profile::Timing;
use anyhow::bail;
use anyhow::Result;
use clap::Parser;
use compile::Profile;
use log::info;
use multifile_rust as multifile;
use rayon::prelude::*;
use std::borrow::Cow;
use std::fs;
use std::hash::Hash;
use std::hash::Hasher;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::Ordering;
use std::sync::Arc;
use std::sync::Mutex;
use std::time::Duration;
use std::time::Instant;

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

#[derive(Parser, Clone, Debug)]
pub struct Opts {
    #[allow(dead_code)]
    #[clap(flatten)]
    single_file_opts: SingleFileOpts,

    /// Number of parallel worker threads. By default, or if set to 0, use num-cpu threads.
    #[clap(long, default_value = "0")]
    num_threads: usize,

    /// The input Hack files or directories to process.
    #[clap(name = "PATH")]
    paths: Vec<PathBuf>,
}

fn process_one_file(
    writer: &SyncWrite,
    f: &Path,
    compile_opts: &SingleFileOpts,
    profile: &mut Profile,
) -> Result<()> {
    let content = fs::read(f)?;
    let files = multifile::to_files(f, content)?;
    for (f, content) in files {
        let f = f.as_ref();
        let result = std::panic::catch_unwind(|| {
            let mut profile1 = Profile::default();
            let result = crate::compile::process_single_file(
                &compile_opts,
                f.into(),
                content,
                &mut profile1,
            );
            (result, profile1)
        });
        match result {
            Ok((Err(e), profile1)) => {
                // No panic - but called function failed.
                *profile += profile1;
                writeln!(writer.lock().unwrap(), "{}: error ({})", f.display(), e)?;
                bail!("failed");
            }
            Ok((Ok(output), profile1)) => {
                // No panic and called function succeeded.
                *profile += profile1;
                let mut hasher = std::collections::hash_map::DefaultHasher::new();
                output.hash(&mut hasher);
                let crc = hasher.finish();
                writeln!(writer.lock().unwrap(), "{}: {:016x}", f.display(), crc)?;
            }
            Err(_) => {
                // Called function panic'd.
                writeln!(writer.lock().unwrap(), "{}: panic", f.display())?;
                bail!("panic");
            }
        }
    }

    Ok(())
}

#[derive(Default)]
struct ProfileAcc {
    sum: Profile,
    max_parse: Profile,
    max_parse_file: PathBuf,
    max_lower: Profile,
    max_lower_file: PathBuf,
    max_error: Profile,
    max_error_file: PathBuf,
    max_rewrite: Profile,
    max_rewrite_file: PathBuf,
    max_emitter: Profile,
    max_emitter_file: PathBuf,
    total_t: Timing,
    parsing_t: Timing,
    codegen_t: Timing,
    printing_t: Timing,
}

impl ProfileAcc {
    fn fold(mut self, other: Self) -> Self {
        self.sum += other.sum;
        if other.max_parse.parse_peak > self.max_parse.parse_peak {
            self.max_parse = other.max_parse;
            self.max_parse_file = other.max_parse_file;
        }
        if other.max_lower.lower_peak > self.max_lower.lower_peak {
            self.max_lower = other.max_lower;
            self.max_lower_file = other.max_lower_file;
        }
        if other.max_error.error_peak > self.max_error.error_peak {
            self.max_error = other.max_error;
            self.max_error_file = other.max_error_file;
        }
        if other.max_rewrite.rewrite_peak > self.max_rewrite.rewrite_peak {
            self.max_rewrite = other.max_rewrite;
            self.max_rewrite_file = other.max_rewrite_file;
        }
        if other.max_emitter.emitter_peak > self.max_emitter.emitter_peak {
            self.max_emitter = other.max_emitter;
            self.max_emitter_file = other.max_emitter_file;
        }
        self.total_t += other.total_t;
        self.parsing_t += other.parsing_t;
        self.codegen_t += other.codegen_t;
        self.printing_t += other.printing_t;
        self
    }

    fn from_compile<'a>(profile: compile::Profile, path: impl Into<Cow<'a, Path>>) -> Self {
        let path = path.into();
        let total_t = profile.codegen_t + profile.parsing_t + profile.printing_t;
        let total_t = Timing::from_secs_f64(total_t, path.clone());
        let codegen_t = Timing::from_secs_f64(profile.codegen_t, path.clone());
        let parsing_t = Timing::from_secs_f64(profile.parsing_t, path.clone());
        let printing_t = Timing::from_secs_f64(profile.printing_t, path.clone());
        ProfileAcc {
            total_t,
            codegen_t,
            parsing_t,
            printing_t,
            sum: profile.clone(),
            max_parse: profile.clone(),
            max_parse_file: path.to_path_buf(),
            max_lower: profile.clone(),
            max_lower_file: path.to_path_buf(),
            max_error: profile.clone(),
            max_error_file: path.to_path_buf(),
            max_rewrite: profile.clone(),
            max_rewrite_file: path.to_path_buf(),
            max_emitter: profile,
            max_emitter_file: path.into_owned(),
        }
    }

    fn report_final(&self, wall: Duration, count: usize, total: usize) {
        if total >= 10 {
            let wall_per_sec = if !wall.is_zero() {
                ((count as f64) / wall.as_secs_f64()) as usize
            } else {
                0
            };

            // Done, print final stats.
            eprintln!(
                "\rProcessed {count} in {wall} ({wall_per_sec}/s) cpu={cpu:.3}s arenas={arenas:.3}MiB  ",
                wall = wall.display(),
                cpu = self.sum.total_sec(),
                arenas = self.sum.codegen_bytes as f64 / (1024 * 1024) as f64,
            );
        }

        profile::report_stat("", "total time", &self.total_t);
        profile::report_stat("  ", "parsing time", &self.parsing_t);
        profile::report_stat("  ", "codegen time", &self.codegen_t);
        profile::report_stat("  ", "printing time", &self.printing_t);
        eprintln!(
            "parser stack peak {} in {}",
            self.max_parse.parse_peak,
            self.max_parse_file.display()
        );
        eprintln!(
            "lowerer stack peak {} in {}",
            self.max_lower.lower_peak,
            self.max_lower_file.display()
        );
        eprintln!(
            "check_error stack peak {} in {}",
            self.max_error.error_peak,
            self.max_error_file.display()
        );
        eprintln!(
            "rewrite stack peak {} in {}",
            self.max_rewrite.rewrite_peak,
            self.max_rewrite_file.display()
        );
        eprint!(
            "emitter stack peak {} in {}",
            self.max_emitter.emitter_peak,
            self.max_emitter_file.display()
        );
    }
}

fn crc_files(
    writer: &SyncWrite,
    files: &[PathBuf],
    num_threads: usize,
    compile_opts: &SingleFileOpts,
) -> Result<()> {
    let total = files.len();
    let passed = Arc::new(AtomicBool::new(true));

    let status_ticker = StatusTicker::new(total);

    let count_one_file = |acc: ProfileAcc, f: &PathBuf| -> ProfileAcc {
        let mut profile = Profile::default();
        status_ticker.start_file(f);
        let file_passed = process_one_file(writer, f.as_path(), compile_opts, &mut profile).is_ok();
        if !file_passed {
            passed.store(false, Ordering::Release);
        }
        status_ticker.finish_file(f);
        acc.fold(ProfileAcc::from_compile(profile, f))
    };

    let profile = if num_threads == 1 {
        files.iter().fold(ProfileAcc::default(), count_one_file)
    } else {
        files
            .par_iter()
            .with_max_len(1)
            .fold(ProfileAcc::default, count_one_file)
            .reduce(ProfileAcc::default, |x, y| x.fold(y))
    };

    let (count, duration) = status_ticker.finish();

    profile.report_final(duration, count, total);
    eprintln!();

    if !passed.load(Ordering::Acquire) {
        bail!("Failed to complete");
    }

    Ok(())
}

pub fn run(opts: Opts) -> Result<()> {
    let writer: SyncWrite = Mutex::new(Box::new(std::io::stdout()));

    info!("Collecting files");
    let files = {
        let start = Instant::now();
        let files = crate::util::collect_files(&opts.paths, None, opts.num_threads)?;
        let duration = start.elapsed();
        info!("{} files found in {}", files.len(), duration.display());
        files
    };

    crc_files(&writer, &files, opts.num_threads, &opts.single_file_opts)?;

    Ok(())
}
