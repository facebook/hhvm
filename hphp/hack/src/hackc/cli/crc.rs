// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::fs;
use std::hash::Hash;
use std::hash::Hasher;
use std::path::Path;
use std::path::PathBuf;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::Ordering;
use std::sync::Arc;
use std::time::Duration;
use std::time::Instant;

use anyhow::bail;
use anyhow::Result;
use clap::Args;
use multifile_rust as multifile;
use parking_lot::Mutex;
use rayon::prelude::*;

use crate::compile::SingleFileOpts;
use crate::profile;
use crate::profile::DurationEx;
use crate::profile::MaxValue;
use crate::profile::StatusTicker;
use crate::profile::Timing;
use crate::util::SyncWrite;

#[derive(Args, Debug)]
pub struct Opts {
    #[allow(dead_code)]
    #[command(flatten)]
    single_file_opts: SingleFileOpts,

    /// Number of parallel worker threads. By default, or if set to 0, use num-cpu threads.
    #[clap(long, default_value = "0")]
    num_threads: usize,

    #[command(flatten)]
    files: crate::FileOpts,
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
                *profile = compile::Profile::fold(std::mem::take(profile), profile1);
                writeln!(writer.lock(), "{}: error ({})", f.display(), e)?;
                bail!("failed");
            }
            Ok((Ok(output), profile1)) => {
                // No panic and called function succeeded.
                *profile = compile::Profile::fold(std::mem::take(profile), profile1);
                let mut hasher = std::collections::hash_map::DefaultHasher::new();
                output.hash(&mut hasher);
                let crc = hasher.finish();
                writeln!(writer.lock(), "{}: {:016x}", f.display(), crc)?;
            }
            Err(_) => {
                // Called function panic'd.
                writeln!(writer.lock(), "{}: panic", f.display())?;
                bail!("panic");
            }
        }
    }

    Ok(())
}

#[derive(Default)]
struct ProfileAcc {
    parser_profile: crate::profile::ParserProfile,
    codegen_bytes: u64,
    max_rewrite: MaxValue<u64>,
    max_emitter: MaxValue<u64>,
    total_t: Timing,
    codegen_t: Timing,
    bc_to_ir_t: Timing,
    ir_to_bc_t: Timing,
    printing_t: Timing,
}

impl ProfileAcc {
    fn fold(mut self, other: Self) -> Self {
        self.fold_with(other);
        self
    }

    fn fold_with(&mut self, other: Self) {
        self.parser_profile.fold_with(other.parser_profile);
        self.codegen_bytes += other.codegen_bytes;
        self.max_rewrite.fold_with(other.max_rewrite);
        self.max_emitter.fold_with(other.max_emitter);
        self.total_t.fold_with(other.total_t);
        self.codegen_t.fold_with(other.codegen_t);
        self.bc_to_ir_t.fold_with(other.bc_to_ir_t);
        self.ir_to_bc_t.fold_with(other.ir_to_bc_t);
        self.printing_t.fold_with(other.printing_t);
    }

    fn from_compile<'a>(
        profile: compile::Profile,
        total_t: Duration,
        path: impl Into<Cow<'a, Path>>,
    ) -> Self {
        let path = path.into();
        let total_t = Timing::from_duration(total_t, path.clone());
        let codegen_t = Timing::from_duration(profile.codegen_t, path.clone());
        let bc_to_ir_t = Timing::from_duration(profile.bc_to_ir_t, path.clone());
        let ir_to_bc_t = Timing::from_duration(profile.ir_to_bc_t, path.clone());
        let printing_t = Timing::from_duration(profile.printing_t, path.clone());
        let parser_profile =
            crate::profile::ParserProfile::from_parser(profile.parser_profile, path.as_ref());
        ProfileAcc {
            total_t,
            codegen_t,
            bc_to_ir_t,
            ir_to_bc_t,
            printing_t,
            codegen_bytes: profile.codegen_bytes,
            max_rewrite: MaxValue::new(profile.rewrite_peak, path.to_path_buf()),
            max_emitter: MaxValue::new(profile.emitter_peak, path.to_path_buf()),
            parser_profile,
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
        let p = &self.parser_profile;
        profile::report_stat(&mut w, "", "total time", &self.total_t)?;
        profile::report_stat(&mut w, "  ", "ast-gen time", &p.total_t)?;
        profile::report_stat(&mut w, "    ", "parsing time", &p.parsing_t)?;
        profile::report_stat(&mut w, "    ", "lowering time", &p.lowering_t)?;
        profile::report_stat(&mut w, "    ", "elaboration time", &p.elaboration_t)?;
        profile::report_stat(&mut w, "    ", "error check time", &p.error_t)?;
        profile::report_stat(&mut w, "  ", "codegen time", &self.codegen_t)?;
        profile::report_stat(&mut w, "  ", "bc_to_ir time", &self.bc_to_ir_t)?;
        profile::report_stat(&mut w, "  ", "ir_to_bc time", &self.ir_to_bc_t)?;
        profile::report_stat(&mut w, "  ", "printing time", &self.printing_t)?;
        p.parse_peak.report(&mut w, "parser stack peak")?;
        p.lower_peak.report(&mut w, "lowerer stack peak")?;
        p.error_peak.report(&mut w, "check_error stack peak")?;
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
        let total_t = Instant::now();
        let file_passed = process_one_file(writer, f.as_path(), compile_opts, &mut profile).is_ok();
        if !file_passed {
            passed.store(false, Ordering::Release);
        }
        status_ticker.finish_file(f);
        acc.fold(ProfileAcc::from_compile(profile, total_t.elapsed(), f))
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
    let files = opts.files.collect_input_files(opts.num_threads)?;
    crc_files(&writer, &files, opts.num_threads, &opts.single_file_opts)?;

    Ok(())
}
