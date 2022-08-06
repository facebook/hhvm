// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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

use anyhow::bail;
use anyhow::Result;
use clap::Parser;
use log::info;
use multifile_rust as multifile;
use rayon::prelude::*;

use crate::compile::SingleFileOpts;
use crate::profile;
use crate::profile::DurationEx;
use crate::profile::MaxValue;
use crate::profile::StatusTicker;
use crate::profile::Timing;

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
    profile: &mut compile::Profile,
) -> Result<()> {
    let content = fs::read(f)?;
    let files = multifile::to_files(f, content)?;
    for (f, content) in files {
        let f = f.as_ref();
        let result = std::panic::catch_unwind(|| {
            let mut profile1 = compile::Profile::default();
            let result =
                crate::compile::process_single_file(compile_opts, f.into(), content, &mut profile1);
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
    codegen_bytes: i64,
    max_parse: MaxValue<i64>,
    max_lower: MaxValue<i64>,
    max_error: MaxValue<i64>,
    max_rewrite: MaxValue<i64>,
    max_emitter: MaxValue<i64>,
    total_t: Timing,
    parsing_t: Timing,
    codegen_t: Timing,
    printing_t: Timing,
}

impl ProfileAcc {
    fn fold(mut self, other: Self) -> Self {
        self.fold_with(other);
        self
    }

    fn fold_with(&mut self, other: Self) {
        self.codegen_bytes += other.codegen_bytes;
        self.max_parse.fold_with(other.max_parse);
        self.max_lower.fold_with(other.max_lower);
        self.max_error.fold_with(other.max_error);
        self.max_rewrite.fold_with(other.max_rewrite);
        self.max_emitter.fold_with(other.max_emitter);
        self.total_t.fold_with(other.total_t);
        self.parsing_t.fold_with(other.parsing_t);
        self.codegen_t.fold_with(other.codegen_t);
        self.printing_t.fold_with(other.printing_t);
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
            codegen_bytes: profile.codegen_bytes,
            max_parse: MaxValue::new(profile.parse_peak, path.to_path_buf()),
            max_lower: MaxValue::new(profile.lower_peak, path.to_path_buf()),
            max_error: MaxValue::new(profile.error_peak, path.to_path_buf()),
            max_rewrite: MaxValue::new(profile.rewrite_peak, path.to_path_buf()),
            max_emitter: MaxValue::new(profile.emitter_peak, path.into_owned()),
        }
    }

    fn report_final(&self, wall: Duration, count: usize, total: usize) -> std::io::Result<()> {
        if total >= 10 {
            let wall_per_sec = if !wall.is_zero() {
                ((count as f64) / wall.as_secs_f64()) as usize
            } else {
                0
            };

            // Done, print final stats.
            eprintln!(
                "\rProcessed {count} in {wall} ({wall_per_sec}/s) arenas={arenas:.3}MiB  ",
                wall = wall.display(),
                arenas = self.codegen_bytes as f64 / (1024 * 1024) as f64,
            );
        }

        let mut w = std::io::stderr();
        profile::report_stat(&mut w, "", "total time", &self.total_t)?;
        profile::report_stat(&mut w, "  ", "parsing time", &self.parsing_t)?;
        profile::report_stat(&mut w, "  ", "codegen time", &self.codegen_t)?;
        profile::report_stat(&mut w, "  ", "printing time", &self.printing_t)?;
        self.max_parse.report(&mut w, "parser stack peak")?;
        self.max_lower.report(&mut w, "lowerer stack peak")?;
        self.max_error.report(&mut w, "check_error stack peak")?;
        self.max_rewrite.report(&mut w, "rewrite stack peak")?;
        self.max_emitter.report(&mut w, "emitter stack peak")?;
        Ok(())
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
        let mut profile = compile::Profile::default();
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

    profile.report_final(duration, count, total)?;
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
