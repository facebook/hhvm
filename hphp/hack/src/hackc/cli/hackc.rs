// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod asm;
mod asm_ir;
mod compile;
mod crc;
mod decls;
mod elaborate;
mod expr_trees;
mod facts;
mod infer;
mod parse;
mod profile;
mod util;
mod verify;

use std::io::BufRead;
use std::path::Path;
use std::path::PathBuf;
use std::time::Instant;

use ::compile::EnvFlags;
use ::compile::NativeEnv;
use anyhow::Result;
use byte_unit::Byte;
use clap::Args;
use clap::Parser;
use hhvm_options::HhvmOptions;
use log::info;
use options::Hhvm;
use options::ParserOptions;
use oxidized::decl_parser_options::DeclParserOptions;

use crate::profile::DurationEx;

/// Hack Compiler
#[derive(Parser, Debug, Default)]
#[command(author = "hphp_hphpi")]
struct Opts {
    #[clap(subcommand)]
    command: Option<Command>,

    /// Runs in daemon mode for testing purposes. Do not rely on for production
    #[clap(long)]
    daemon: bool,

    #[command(flatten)]
    flag_commands: FlagCommands,

    #[command(flatten)]
    hhvm_options: HhvmOptions,

    #[command(flatten)]
    files: FileOpts,

    #[command(flatten)]
    pub(crate) env_flags: EnvFlags,

    /// Number of parallel worker threads for subcommands that support parallelism,
    /// otherwise ignored. If 0, use available parallelism, typically num-cpus.
    #[clap(long, default_value("0"))]
    num_threads: usize,

    /// Stack size to use for parallel worker threads. Supports unit suffixes like KB, MiB, etc.
    #[clap(long, default_value("32 MiB"))]
    stack_size: Byte,

    /// Instead of printing the unit, print a list of the decls requested during compilation.
    /// (only used by --test-compile-with-decls)
    #[clap(long)]
    pub(crate) log_decls_requested: bool,

    /// Use serialized decl instead of decl pointer as the decl provider API
    /// (only used by --test-compile-with-decls)
    #[clap(long)]
    pub(crate) use_serialized_decls: bool,

    #[clap(long)]
    pub(crate) naming_table: Option<PathBuf>,

    #[clap(long)]
    pub(crate) naming_table_root: Option<PathBuf>,

    /// [Experimental] Enable Type-Directed Bytecode Compilation
    #[clap(long)]
    type_directed: bool,
}

/// Hack Compiler
#[derive(Args, Debug, Default)]
struct FileOpts {
    /// Input file(s)
    filenames: Vec<PathBuf>,

    /// Read a list of files (one-per-line) from this file
    #[clap(long)]
    input_file_list: Option<PathBuf>,

    /// change to directory DIR
    #[clap(long, short = 'C')]
    directory: Option<PathBuf>,
}

#[derive(Parser, Debug)]
enum Command {
    /// Assemble HHAS file(s) into Unit. Prints those HCUs' HHAS representation.
    Assemble(asm::Opts),

    /// Assemble IR assembly file(s) into Unit and print it.
    AssembleIr(asm_ir::Opts),

    /// Compute bincode-serialized decls for a set of Hack source files.
    BinaryDecls(decls::Opts),

    /// Compile one Hack source file or a list of files to HHAS
    Compile(compile::Opts),

    /// Elaborate one Hack source file or a list of files.
    Elaborate(elaborate::Opts),

    /// Compile Hack source files or directories and produce a single CRC per
    /// input file.
    Crc(crc::Opts),

    /// Compute JSON-serialized decls for a set of Hack source files.
    JsonDecls(decls::Opts),

    /// Print the source code with expression tree literals desugared.
    /// Best effort debugging tool.
    DesugarExprTrees(expr_trees::Opts),

    /// Compute JSON facts for a set of Hack source files.
    Facts(facts::Opts),

    /// Emit Infer SIL.
    CompileInfer(infer::Opts),

    /// Render the source text parse tree for each given file.
    Parse(parse::Opts),

    /// Parse many files whose filenames are read from stdin, discard parser output.
    ParseBench(parse::BenchOpts),

    /// Compile Hack source files or directories and check for compilation errors.
    Verify(verify::Opts),
}

/// Which command are we running? Using bool opts for compatibility with test harnesses.
/// New commands should be defined as subcommands using the Command enum.
#[derive(Parser, Debug, Default)]
struct FlagCommands {
    /// Parse decls from source text, transform them into facts, and print the facts
    /// in JSON format.
    #[clap(long)]
    extract_facts_from_decls: bool,

    /// Compile file with decls from the same file available during compilation.
    #[clap(long)]
    test_compile_with_decls: bool,
}

impl FileOpts {
    pub fn gather_input_files(&self) -> Result<Vec<PathBuf>> {
        use std::io::BufReader;
        let mut files: Vec<PathBuf> = Default::default();
        if let Some(list_path) = self.input_file_list.as_ref() {
            for line in BufReader::new(std::fs::File::open(list_path)?).lines() {
                let line = line?;
                let name = if let Some(directory) = self.directory.as_ref() {
                    directory.join(Path::new(&line))
                } else {
                    PathBuf::from(line)
                };
                files.push(name);
            }
        }
        for name in &self.filenames {
            let name = if let Some(directory) = self.directory.as_ref() {
                directory.join(name)
            } else {
                name.to_path_buf()
            };
            files.push(name);
        }
        Ok(files)
    }

    pub fn collect_input_files(&self, num_threads: usize) -> Result<Vec<PathBuf>> {
        info!("Collecting files");
        let gather_t = Instant::now();
        let files = self.gather_input_files()?;
        let files = crate::util::collect_files(&files, None, num_threads)?;
        let gather_t = gather_t.elapsed();
        info!("{} files found in {}", files.len(), gather_t.display());
        Ok(files)
    }

    pub fn is_batch_mode(&self) -> bool {
        self.input_file_list.is_some() || self.filenames.len() > 1
    }
}

impl Opts {
    pub fn decl_opts(&self) -> DeclParserOptions {
        DeclParserOptions {
            auto_namespace_map: compile::auto_namespace_map().collect(),
            disable_xhp_element_mangling: false,
            interpret_soft_types_as_like_types: true,
            allow_new_attribute_syntax: true,
            enable_xhp_class_modifier: false,
            php5_compat_mode: true,
            hhvm_compat_mode: true,
            keep_user_attributes: true,
            ..Default::default()
        }
    }

    pub fn native_env(&self, path: PathBuf) -> Result<NativeEnv> {
        let hhvm_options = &self.hhvm_options;
        let hhvm_config = hhvm_options.to_config()?;
        let parser_options = ParserOptions {
            po_auto_namespace_map: compile::auto_namespace_map().collect(),
            ..hhvm_config::parser_options(&hhvm_config)?
        };
        let hhbc_flags = hhvm_config::hhbc_flags(&hhvm_config)?;
        Ok(NativeEnv {
            filepath: relative_path::RelativePath::make(relative_path::Prefix::Dummy, path),
            hhvm: Hhvm {
                include_roots: Default::default(),
                renamable_functions: Default::default(),
                non_interceptable_functions: Default::default(),
                parser_options,
                jit_enable_rename_function: hhvm_config::jit_enable_rename_function(&hhvm_config)?,
            },
            flags: self.env_flags.clone(),
            hhbc_flags,
        })
    }
}

/// Facebook main: Perform some Facebook-specific initialization.
#[cfg(fbcode_build)]
#[cli::main("hackc", error_logging)]
fn main(_fb: fbinit::FacebookInit, opts: Opts) -> Result<()> {
    // tracing-log is EXTREMELY SLOW. Running a verify over www is about 10x
    // slower using tracing-log vs env_logger!
    // tracing_log::LogTracer::init()?;
    env_logger::init();

    hackc_main(opts)
}

/// non-Facebook main
#[cfg(not(fbcode_build))]
fn main() -> Result<()> {
    // In non-FB mode we convert tracing into logging (using tracing's 'log'
    // feature) - so init logs like normal.
    env_logger::init();
    let opts = Opts::parse();
    hackc_main(opts)
}

fn hackc_main(mut opts: Opts) -> Result<()> {
    // Some subcommands need worker threads with larger than default stacks,
    // even when using Stacker. In particular, various derived traits (e.g. Drop)
    // on AAST nodes are inherently recursive.
    rayon::ThreadPoolBuilder::new()
        .num_threads(opts.num_threads)
        .stack_size(opts.stack_size.get_bytes().try_into()?)
        .build_global()
        .unwrap();

    match opts.command.take() {
        Some(Command::Assemble(opts)) => asm::run(opts),
        Some(Command::AssembleIr(opts)) => asm_ir::run(opts),
        Some(Command::BinaryDecls(decls_opts)) => decls::binary_decls(opts, decls_opts),
        Some(Command::JsonDecls(decls_opts)) => decls::json_decls(opts, decls_opts),
        Some(Command::CompileInfer(opts)) => infer::run(opts),
        Some(Command::Crc(opts)) => crc::run(opts),
        Some(Command::Parse(parse_opts)) => parse::run(parse_opts),
        Some(Command::ParseBench(bench_opts)) => parse::run_bench_command(bench_opts),
        Some(Command::Verify(opts)) => verify::run(opts),
        Some(Command::Elaborate(opts)) => elaborate::run(opts),

        // Expr trees
        Some(Command::DesugarExprTrees(et_opts)) => expr_trees::desugar_expr_trees(&opts, et_opts),

        // Facts
        Some(Command::Facts(facts_opts)) => {
            facts::extract_facts(&opts, facts_opts, &mut std::io::stdout())
        }
        None if opts.daemon && opts.flag_commands.extract_facts_from_decls => {
            facts::daemon(&mut opts)
        }
        None if opts.flag_commands.extract_facts_from_decls => {
            facts::run_flag(&mut opts, &mut std::io::stdout())
        }

        // Test Decls-in-Compilation
        None if opts.daemon && opts.flag_commands.test_compile_with_decls => {
            compile::test_decl_compile_daemon(&mut opts)
        }
        None if opts.flag_commands.test_compile_with_decls => {
            compile::test_decl_compile(&mut opts, &mut std::io::stdout())
        }

        // Compile to hhas
        Some(Command::Compile(mut opts)) => compile::run(&mut opts),
        None if opts.daemon => compile::daemon(&mut opts),
        None => compile::compile_from_text(&mut opts, &mut std::io::stdout()),
    }
}

/// In daemon mode, hackc blocks waiting for a filename on stdin.
/// Then, using the originally invoked options, dispatches that file to be compiled.
fn daemon_loop(mut f: impl FnMut(PathBuf, &mut Vec<u8>) -> Result<()>) -> Result<()> {
    use std::io::Write;
    for line in std::io::stdin().lock().lines() {
        let mut buf = Vec::new();
        f(Path::new(&line?).to_path_buf(), &mut buf)?;
        // Account for utf-8 encoding and text streams with the python test runner:
        // https://stackoverflow.com/questions/3586923/counting-unicode-characters-in-c
        let mut w = std::io::stdout();
        let num_chars = buf.iter().filter(|&b| (b & 0xc0) != 0x80).count() + 1;
        writeln!(w, "{num_chars}")?;
        w.write_all(&buf)?;
        w.write_all(b"\n")?;
        w.flush()?;
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Just make sure json parsing produces a proper list.
    /// If the alias map length changes, keep this test in sync.
    #[test]
    fn test_auto_namespace_map() {
        let dp_opts = Opts::default().decl_opts();
        assert_eq!(dp_opts.auto_namespace_map.len(), 15);
    }
}
