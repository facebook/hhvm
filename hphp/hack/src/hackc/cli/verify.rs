// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::fs;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;
use std::sync::Arc;
use std::time::Duration;

use anyhow::ensure;
use clap::Args;
use clap::Parser;
use decl_provider::SelfProvider;
use hash::HashMap;
use hash::HashSet;
use hhbc::SrcLoc;
use itertools::Itertools;
use multifile_rust as multifile;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use regex::Regex;
use relative_path::Prefix;
use relative_path::RelativePath;
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
    #[error("ir units mismatch: {0}")]
    IrUnitMismatchError(String),
    #[error("multifile error: {0}")]
    MultifileError(String),
    #[error("panic {0}")]
    Panic(String),
    #[error("parse error: {0}")]
    ParseError(String, SrcLoc),
    #[error("printing error: {0}")]
    PrintError(String),
    #[error("unable to read file: {0}")]
    ReadError(String),
    #[error("runtime error: {0}")]
    RuntimeError(String, SrcLoc),
    #[error("units mismatch: {0}")]
    UnitMismatchError(cmp::CmpError),
    #[error("semantic units mismatch: {0}")]
    SemanticUnitMismatchError(String),
}

impl VerifyError {
    fn loc(&self) -> Option<&SrcLoc> {
        match self {
            VerifyError::ParseError(_, loc) | VerifyError::RuntimeError(_, loc) => Some(loc),
            VerifyError::AssembleError(_)
            | VerifyError::CompileError(_)
            | VerifyError::IoError(_)
            | VerifyError::IrUnitMismatchError(_)
            | VerifyError::MultifileError(_)
            | VerifyError::Panic(_)
            | VerifyError::PrintError(_)
            | VerifyError::ReadError(_)
            | VerifyError::UnitMismatchError(_)
            | VerifyError::SemanticUnitMismatchError(_) => None,
        }
    }
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

#[derive(Args, Debug)]
struct CommonOpts {
    #[command(flatten)]
    files: crate::FileOpts,

    /// Print full error messages
    #[clap(short = 'l')]
    long_msg: bool,

    #[allow(dead_code)]
    #[command(flatten)]
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

    /// Print all files
    #[clap(long)]
    show_verbose: bool,
}

#[derive(Args, Debug)]
struct AssembleOpts {
    #[command(flatten)]
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
        let (asm_result, assemble_t) =
            Timing::time(path, || assemble::assemble_from_bytes(&post_alloc, &output));
        let (post_unit, _) = asm_result.map_err(|err| {
            VerifyError::AssembleError(truncate_pos_err(err.root_cause().to_string()))
        })?;

        let (result, verify_t) = Timing::time(path, || {
            cmp::cmp_unit::cmp_hack_c_unit(&pre_unit, &post_unit)
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

#[derive(Args, Debug)]
struct CompileOpts {
    #[command(flatten)]
    common: CommonOpts,
}

impl CompileOpts {
    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut ProfileAcc) -> Result<()> {
        let alloc = bumpalo::Bump::default();
        let mut compile_profile = compile::Profile::default();
        let (_env, unit) = compile_php_file(
            &alloc,
            path,
            content,
            &self.common.single_file_opts,
            &mut compile_profile,
        )?;

        profile.fold_with(ProfileAcc {
            total_t: Timing::from_duration(
                compile_profile.codegen_t + compile_profile.parser_profile.total_t,
                path,
            ),
            codegen_t: Timing::from_duration(compile_profile.codegen_t, path),
            parsing_t: Timing::from_duration(compile_profile.parser_profile.parsing_t, path),
            lowering_t: Timing::from_duration(compile_profile.parser_profile.lowering_t, path),
            ..Default::default()
        });

        if let ffi::Maybe::Just(hhbc::Fatal { op, loc, message }) = unit.fatal {
            use hhbc::FatalOp;
            let msg = message.as_bstr().to_string();
            let err = match op {
                FatalOp::Parse => VerifyError::ParseError(msg, loc),
                FatalOp::Runtime | FatalOp::RuntimeOmitFrame => VerifyError::RuntimeError(msg, loc),
                _ => VerifyError::CompileError(msg),
            };
            return Err(err);
        }

        Ok(())
    }
}

#[derive(Args, Debug)]
struct IrOpts {
    #[command(flatten)]
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
        let (ir, bc_to_ir_t) = Timing::time(path, || bc_to_ir::bc_to_ir(&pre_unit, path));

        ir.strings.debug_for_each(|_id, s| {
            if !profile.global_strings.contains(s) {
                profile.global_strings.insert(s.to_owned());
            }
        });

        self.verify_print_roundtrip(&ir)?;

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

        result.map_err(|err| VerifyError::SemanticUnitMismatchError(err.root_cause().to_string()))
    }

    fn verify_print_roundtrip(&self, ir: &ir::Unit<'_>) -> Result<()> {
        let mut as_string = String::new();
        ir::print::print_unit(&mut as_string, ir, false)
            .map_err(|err| VerifyError::PrintError(err.to_string()))?;
        let alloc2 = bumpalo::Bump::default();
        let strings2 = Arc::new(ir::StringInterner::default());
        let ir2 = ir::assemble::unit_from_string(&as_string, Arc::clone(&strings2), &alloc2)
            .map_err(|err| {
                let mut err = err.root_cause().to_string();
                let err = if let Some(idx) = err.find(':') {
                    err.split_off(idx)
                } else {
                    err
                };
                VerifyError::AssembleError(err)
            })?;
        cmp::cmp_ir::cmp_ir(ir, &ir2)
            .map_err(|err| VerifyError::IrUnitMismatchError(err.to_string()))
    }
}

#[derive(Args, Debug)]
struct InferOpts {
    #[clap(flatten)]
    common: CommonOpts,

    /// Attempt to keep going instead of panicking on unimplemented code.
    #[clap(long)]
    keep_going: bool,
}

impl InferOpts {
    fn setup(&self) {
        textual::KEEP_GOING.store(self.keep_going, std::sync::atomic::Ordering::Release);
    }

    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut ProfileAcc) -> Result<()> {
        // For Infer verify we just make sure that the file can compile without
        // errors.
        let alloc = bumpalo::Bump::default();
        let mut compile_profile = compile::Profile::default();
        let (_env, unit) = compile_php_file(
            &alloc,
            path,
            content,
            &self.common.single_file_opts,
            &mut compile_profile,
        )?;
        let (ir, bc_to_ir_t) = Timing::time(path, || bc_to_ir::bc_to_ir(&unit, path));

        let (result, textual_t) = Timing::time(path, || {
            let mut out = Vec::new();
            textual::textual_writer(&mut out, path, ir, false, false, false)
        });

        let total_t = compile_profile.codegen_t
            + compile_profile.parser_profile.total_t
            + bc_to_ir_t.total()
            + textual_t.total();

        profile.fold_with(ProfileAcc {
            total_t: Timing::from_duration(total_t, path),
            codegen_t: Timing::from_duration(compile_profile.codegen_t, path),
            parsing_t: Timing::from_duration(compile_profile.parser_profile.parsing_t, path),
            lowering_t: Timing::from_duration(compile_profile.parser_profile.lowering_t, path),
            bc_to_ir_t,
            ..Default::default()
        });

        result.map_err(|err| VerifyError::CompileError(format!("{err:?}")))
    }
}

#[derive(Parser, Debug)]
enum Mode {
    /// Assemble the input HHAS files to verify that they can be assembled
    /// successfully.  This is similar to the 'assemble' major mode but doesn't
    /// produce any output files.
    Assemble(AssembleOpts),
    /// Compile the input Hack files to verify that they can compile
    /// successfully.  This is similar to the 'compile' major mode but doesn't
    /// produce any output files.
    Compile(CompileOpts),
    /// Compile the input Hack files and convert to Infer to verify that they
    /// can compile successfully. This is similar to the 'compile-infer' major
    /// mode but doesn't produce any output files.
    Infer(InferOpts),
    /// Compile the input Hack files into HHAS, convert them to IR, convert the
    /// resulting IR back to HHAS and then verify that the resulting HHAS
    /// matches the original HHAS to ensure that the HHAS -> IR -> HHAS pipeline
    /// is working properly.
    Ir(IrOpts),
}

impl Mode {
    fn common(&self) -> &CommonOpts {
        match self {
            Mode::Assemble(AssembleOpts { common, .. })
            | Mode::Compile(CompileOpts { common, .. })
            | Mode::Infer(InferOpts { common, .. })
            | Mode::Ir(IrOpts { common, .. }) => common,
        }
    }

    fn common_mut(&mut self) -> &mut CommonOpts {
        match self {
            Mode::Assemble(AssembleOpts { common, .. })
            | Mode::Compile(CompileOpts { common, .. })
            | Mode::Infer(InferOpts { common, .. })
            | Mode::Ir(IrOpts { common, .. }) => common,
        }
    }

    fn setup(&self) {
        match self {
            Mode::Assemble(_) | Mode::Compile(_) | Mode::Ir(_) => {}
            Mode::Infer(opts) => opts.setup(),
        }
    }

    fn verify_file(&self, path: &Path, content: Vec<u8>, profile: &mut ProfileAcc) -> Result<()> {
        match self {
            Mode::Assemble(opts) => opts.verify_file(path, content, profile),
            Mode::Compile(opts) => opts.verify_file(path, content, profile),
            Mode::Infer(opts) => opts.verify_file(path, content, profile),
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
    let source_text = SourceText::make(Arc::new(filepath.clone()), &content);
    let env = crate::compile::native_env(filepath, single_file_opts)
        .map_err(|err| VerifyError::CompileError(err.to_string()))?;
    let decl_arena = bumpalo::Bump::new();
    let decl_provider = SelfProvider::wrap_existing_provider(
        None,
        env.to_decl_parser_options(),
        source_text.clone(),
        &decl_arena,
    );
    let unit = compile::unit_from_text(alloc, source_text, &env, decl_provider, profile)
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

#[derive(Debug)]
struct ErrorStat {
    count: usize,
    example: PathBuf,
}

struct ProfileAcc {
    passed: bool,
    error_histogram: HashMap<String, ErrorStat>,
    assemble_t: Timing,
    bc_to_ir_t: Timing,
    codegen_t: Timing,
    ir_to_bc_t: Timing,
    parsing_t: Timing,
    lowering_t: Timing,
    printing_t: Timing,
    total_t: Timing,
    verify_t: Timing,
    global_strings: HashSet<Vec<u8>>,
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
            global_strings: Default::default(),
        }
    }
}

impl ProfileAcc {
    fn fold_with(&mut self, other: Self) {
        self.passed = self.passed && other.passed;
        for (err, stat) in other.error_histogram {
            let e = self
                .error_histogram
                .entry(err)
                .or_insert_with(|| ErrorStat {
                    count: 0,
                    example: stat.example,
                });
            e.count += stat.count;
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

        match (
            self.global_strings.is_empty(),
            other.global_strings.is_empty(),
        ) {
            (false, false) => {
                self.global_strings.extend(other.global_strings);
            }
            (true, false) => {
                self.global_strings = other.global_strings;
            }
            (_, true) => {}
        }
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
            println!(
                "{}/{} files passed ({:.1}%)",
                total - self.num_failed(),
                total,
                (100.0 - (self.num_failed() * 100) as f64 / total as f64)
            );
            println!("Failure histogram:");
            for (k, v) in self
                .error_histogram
                .iter()
                .sorted_by(|a, b| a.1.count.cmp(&b.1.count).reverse())
                .take(num_show)
            {
                println!("  {:3} ({}): {}", v.count, v.example.display(), k);
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

        if !self.global_strings.is_empty() {
            let mut hist: hdrhistogram::Histogram<u64> = hdrhistogram::Histogram::new(3).unwrap();
            let mut total = 0;
            for s in self.global_strings.iter() {
                let len = s.len() as u64;
                total += len;
                hist.record(len).unwrap();
            }

            writeln!(w)?;
            writeln!(w, "String Interning:")?;
            writeln!(w, "  {} unique strings, {total} bytes total", hist.len())?;
            write!(w, "  min: {}B", hist.min())?;
            write!(w, ", P50: {}B", hist.value_at_percentile(50.0))?;
            write!(w, ", P90: {}B", hist.value_at_percentile(90.0))?;
            write!(w, ", P99: {}B", hist.value_at_percentile(99.0))?;
            writeln!(w, ", max: {}B", hist.max())?;
        }

        Ok(())
    }

    fn record_error(mut self, err: VerifyError, f: &Path, show_verbose: bool) -> Self {
        if show_verbose {
            if let Some(loc) = err.loc() {
                println!(
                    "\rerror: {}[{}:{}]: {}",
                    f.display(),
                    loc.line_begin,
                    loc.col_begin,
                    err
                );
            } else {
                println!("error: {}: {}", f.display(), err);
            }
        }

        self.passed = false;
        let e = self
            .error_histogram
            .entry(err.to_string())
            .or_insert_with(|| ErrorStat {
                count: 0,
                example: f.to_path_buf(),
            });
        e.count += 1;
        self
    }

    fn num_failed(&self) -> usize {
        self.error_histogram
            .values()
            .fold(0, |acc, ErrorStat { count, .. }| acc + count)
    }
}

fn verify_one_file(path: &Path, mode: &Mode) -> ProfileAcc {
    let mut acc = ProfileAcc::default();

    let content = match fs::read(path) {
        Ok(content) => content,
        Err(err) => {
            return acc.record_error(
                VerifyError::ReadError(err.to_string()),
                path,
                mode.common().show_verbose,
            );
        }
    };

    let files = match multifile::to_files(path, content) {
        Ok(files) => files,
        Err(err) => {
            return acc.record_error(
                VerifyError::MultifileError(err.to_string()),
                path,
                mode.common().show_verbose,
            );
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
            return acc.record_error(err, &path, mode.common().show_verbose);
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

    let common = mode.common();
    profile.report_final(duration, count, total, common.show_all)?;

    ensure!(
        profile.passed,
        "{} files failed to verify",
        profile.num_failed()
    );
    Ok(())
}

pub fn run(mut opts: Opts) -> anyhow::Result<()> {
    let num_threads = opts.mode.common().num_threads;

    opts.mode.setup();

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
