// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use std::path::PathBuf;
use std::time::Duration;

use anyhow::ensure;
use clap::Parser;
use itertools::Itertools;
use multifile_rust as multifile;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use regex::Regex;
use thiserror::Error;

use crate::compile::SingleFileOpts;
use crate::profile;
use crate::profile::DurationEx;
use crate::profile::StatusTicker;
use crate::profile::Timing;
use crate::regex;

// Several of these would be better as the underlying error (anyhow::Error or
// std::io::Error) but then we couldn't derive Hash or Eq.
#[derive(Error, Debug, Hash, PartialEq, Eq)]
enum VerifyError {
    #[error("assemble error {0}")]
    AssembleError(String),
    #[error("compile error {0}")]
    CompileError(String),
    #[error("i/o error: {0}")]
    IoError(String),
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
    #[error("semantic units mismatch: {0}")]
    SemanticUnitMismatchError(String),
}

impl From<std::io::Error> for VerifyError {
    fn from(err: std::io::Error) -> Self {
        VerifyError::IoError(err.to_string())
    }
}

type Result<T, E = VerifyError> = std::result::Result<T, E>;

thread_local! {
    pub static PANIC_MSG: RefCell<Option<String>> = RefCell::new(None);
}

#[derive(Parser, Debug)]
struct CommonOpts {
    #[clap(flatten)]
    files: crate::FileOpts,

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

#[derive(Parser, Debug)]
struct AssembleOpts {
    #[clap(flatten)]
    common: CommonOpts,
}

impl AssembleOpts {
    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut ProfileAcc) -> Result<()> {
        let pre_alloc = bumpalo::Bump::default();
        let mut compile_profile = compile::Profile::default();
        let (env, pre_unit) = compile_php_file(
            &pre_alloc,
            path,
            content,
            &self.common.single_file_opts,
            &mut compile_profile,
        )?;

        let mut output: Vec<u8> = Vec::new();
        let mut print_profile = compile::Profile::default();
        compile::unit_to_string(&env, &mut output, &pre_unit, &mut print_profile)
            .map_err(|err| VerifyError::PrintError(err.to_string()))?;

        let post_alloc = bumpalo::Bump::default();
        let (asm_result, assemble_t) = Timing::time(path, || {
            crate::assemble::assemble_from_bytes(
                &post_alloc,
                &output,
                /* print tokens */ false,
            )
        });
        let (post_unit, _) = asm_result
            .map_err(|err| VerifyError::AssembleError(truncate_pos_err(err.to_string())))?;

        let (result, verify_t) = Timing::time(path, || {
            crate::cmp_unit::cmp_hack_c_unit(&pre_unit, &post_unit)
        });

        let total_t = compile_profile.codegen_t
            + compile_profile.parser_profile.total_t
            + print_profile.printing_t
            + assemble_t.total()
            + verify_t.total();

        profile.fold_with(ProfileAcc {
            total_t: Timing::from_duration(total_t, path),
            codegen_t: Timing::from_duration(compile_profile.codegen_t, path),
            parsing_t: Timing::from_duration(compile_profile.parser_profile.parsing_t, path),
            lowering_t: Timing::from_duration(compile_profile.parser_profile.lowering_t, path),
            printing_t: Timing::from_duration(print_profile.printing_t, path),
            assemble_t,
            verify_t,
            ..Default::default()
        });

        result.map_err(VerifyError::UnitMismatchError)
    }
}

#[derive(Parser, Debug)]
struct IrOpts {
    #[clap(flatten)]
    common: CommonOpts,
}

impl IrOpts {
    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut ProfileAcc) -> Result<()> {
        let pre_alloc = bumpalo::Bump::default();
        let mut compile_profile = compile::Profile::default();
        let (_env, pre_unit) = compile_php_file(
            &pre_alloc,
            path,
            content,
            &self.common.single_file_opts,
            &mut compile_profile,
        )?;

        let post_alloc = bumpalo::Bump::default();
        let (ir, bc_to_ir_t) = Timing::time(path, || bc_to_ir::bc_to_ir(&pre_unit));
        let (post_unit, ir_to_bc_t) = Timing::time(path, || ir_to_bc::ir_to_bc(&post_alloc, ir));

        let (result, verify_t) =
            Timing::time(path, || sem_diff::sem_diff_unit(&pre_unit, &post_unit));

        let total_t = compile_profile.codegen_t
            + compile_profile.parser_profile.total_t
            + bc_to_ir_t.total()
            + ir_to_bc_t.total()
            + verify_t.total();

        profile.fold_with(ProfileAcc {
            total_t: Timing::from_duration(total_t, path),
            codegen_t: Timing::from_duration(compile_profile.codegen_t, path),
            parsing_t: Timing::from_duration(compile_profile.parser_profile.parsing_t, path),
            lowering_t: Timing::from_duration(compile_profile.parser_profile.lowering_t, path),
            bc_to_ir_t,
            ir_to_bc_t,
            verify_t,
            ..Default::default()
        });

        result.map_err(|err| VerifyError::SemanticUnitMismatchError(err.to_string()))
    }
}

#[derive(Parser, Debug)]
enum Mode {
    /// Compile files and save the resulting HHAS and interior hhbc::Unit. Assemble the HHAS files and save the resulting Unit. Compare Unit.
    Assemble(AssembleOpts),
    Ir(IrOpts),
}

impl Mode {
    fn common(&self) -> &CommonOpts {
        match self {
            Mode::Assemble(AssembleOpts { common, .. }) | Mode::Ir(IrOpts { common, .. }) => common,
        }
    }

    fn common_mut(&mut self) -> &mut CommonOpts {
        match self {
            Mode::Assemble(AssembleOpts { common, .. }) | Mode::Ir(IrOpts { common, .. }) => common,
        }
    }

    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut ProfileAcc) -> Result<()> {
        match self {
            Mode::Assemble(opts) => opts.verify_file(path, content, profile),
            Mode::Ir(opts) => opts.verify_file(path, content, profile),
        }
    }
}

#[derive(Parser, Debug)]
pub struct Opts {
    #[clap(subcommand)]
    mode: Mode,
}

fn compile_php_file<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    path: &'a Path,
    content: Vec<u8>,
    single_file_opts: &'a SingleFileOpts,
    profile: &mut compile::Profile,
) -> Result<(compile::NativeEnv, hhbc::Unit<'arena>)> {
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

fn with_catch_panics<F>(profile: &mut ProfileAcc, long_msg: bool, action: F) -> Result<()>
where
    F: FnOnce(&mut ProfileAcc) -> Result<()> + std::panic::UnwindSafe,
{
    let result = std::panic::catch_unwind(|| {
        let mut inner_profile = ProfileAcc::default();
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

struct ProfileAcc {
    passed: bool,
    error_histogram: HashMap<VerifyError, (usize, PathBuf)>,
    assemble_t: Timing,
    bc_to_ir_t: Timing,
    codegen_t: Timing,
    ir_to_bc_t: Timing,
    parsing_t: Timing,
    lowering_t: Timing,
    printing_t: Timing,
    total_t: Timing,
    verify_t: Timing,
}

impl std::default::Default for ProfileAcc {
    fn default() -> Self {
        Self {
            passed: true,
            error_histogram: Default::default(),
            assemble_t: Default::default(),
            bc_to_ir_t: Default::default(),
            codegen_t: Default::default(),
            ir_to_bc_t: Default::default(),
            parsing_t: Default::default(),
            lowering_t: Default::default(),
            printing_t: Default::default(),
            total_t: Default::default(),
            verify_t: Default::default(),
        }
    }
}

impl ProfileAcc {
    fn fold_with(&mut self, other: Self) {
        self.passed = self.passed && other.passed;
        for (err, (n, example)) in other.error_histogram {
            self.error_histogram
                .entry(err)
                .or_insert_with(|| (0, example))
                .0 += n;
        }
        self.assemble_t.fold_with(other.assemble_t);
        self.bc_to_ir_t.fold_with(other.bc_to_ir_t);
        self.codegen_t.fold_with(other.codegen_t);
        self.ir_to_bc_t.fold_with(other.ir_to_bc_t);
        self.parsing_t.fold_with(other.parsing_t);
        self.lowering_t.fold_with(other.lowering_t);
        self.printing_t.fold_with(other.printing_t);
        self.total_t.fold_with(other.total_t);
        self.verify_t.fold_with(other.verify_t);
    }

    fn fold(mut self, other: Self) -> Self {
        self.fold_with(other);
        self
    }

    fn report_final(
        &self,
        wall: Duration,
        count: usize,
        total: usize,
        show_all: bool,
    ) -> Result<()> {
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

        if self.passed {
            println!("All files passed.");
        } else {
            println!("Failed to complete");
        }

        let num_show = if show_all {
            self.error_histogram.len()
        } else {
            20
        };

        // The # of files that failed are sum of error_histogram's values' usize field
        if !self.error_histogram.is_empty() {
            println!("{}/{} files passed", total - self.num_failed(), total);
            println!("Failure histogram:");
            for (k, v) in self
                .error_histogram
                .iter()
                .sorted_by(|a, b| a.1.0.cmp(&b.1.0).reverse())
                .take(num_show)
            {
                println!("  {:3} ({}): {}", v.0, v.1.display(), k);
            }
            if self.error_histogram.len() > 20 {
                println!(
                    "  (and {} unreported)",
                    self.error_histogram.len() - num_show
                );
            }
            println!();
        }

        let mut w = std::io::stdout();
        profile::report_stat(&mut w, "", "total time", &self.total_t)?;
        profile::report_stat(&mut w, "  ", "parsing time", &self.parsing_t)?;
        profile::report_stat(&mut w, "  ", "lowering time", &self.lowering_t)?;
        profile::report_stat(&mut w, "  ", "codegen time", &self.codegen_t)?;
        profile::report_stat(&mut w, "  ", "printing time", &self.printing_t)?;
        profile::report_stat(&mut w, "  ", "assemble time", &self.assemble_t)?;
        profile::report_stat(&mut w, "  ", "bc_to_ir time", &self.bc_to_ir_t)?;
        profile::report_stat(&mut w, "  ", "ir_to_bc time", &self.ir_to_bc_t)?;
        profile::report_stat(&mut w, "  ", "verify time", &self.verify_t)?;

        Ok(())
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
    let mut acc = ProfileAcc::default();

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
        let mut file_profile = ProfileAcc::default();
        let result = if mode.common().panic_fuse {
            mode.verify_file(&path, content, &mut file_profile)
        } else {
            with_catch_panics(&mut file_profile, mode.common().long_msg, |file_profile| {
                mode.verify_file(&path, content, file_profile)
            })
        };
        acc = acc.fold(file_profile);
        if let Err(err) = result {
            return acc.record_error(err, &path);
        }
    }

    acc
}

fn verify_files(files: &[PathBuf], mode: &Mode) -> anyhow::Result<()> {
    let total = files.len();

    let status_ticker = StatusTicker::new(total);

    let verify_one_file_ = |acc: ProfileAcc, f: &PathBuf| -> ProfileAcc {
        status_ticker.start_file(f);
        let profile = verify_one_file(f, mode);
        status_ticker.finish_file(f);
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

    let (count, duration) = status_ticker.finish();

    profile.report_final(duration, count, total, mode.common().show_all)?;

    ensure!(
        profile.passed,
        "{} files failed to verify",
        profile.num_failed()
    );
    Ok(())
}

pub fn run(mut opts: Opts) -> anyhow::Result<()> {
    let num_threads = opts.mode.common().num_threads;
    let files = opts
        .mode
        .common_mut()
        .files
        .collect_input_files(num_threads)?;
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
