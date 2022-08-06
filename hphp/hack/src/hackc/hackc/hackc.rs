// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod assemble;
mod cmp_unit;
mod compile;
mod crc;
mod expr_trees;
mod facts;
mod parse;
mod profile;
mod util;
mod verify;

use std::io::BufRead;
use std::path::Path;
use std::path::PathBuf;

use ::compile::EnvFlags;
use ::compile::HHBCFlags;
use ::compile::NativeEnv;
use ::compile::ParserFlags;
use anyhow::Result;
use byte_unit::Byte;
use clap::Parser;
use hhvm_options::HhvmOptions;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::relative_path;
use oxidized::relative_path::RelativePath;

/// Hack Compiler
#[derive(Parser, Debug, Default)]
struct Opts {
    #[clap(subcommand)]
    command: Option<Command>,

    /// Runs in daemon mode for testing purposes. Do not rely on for production
    #[clap(long)]
    daemon: bool,

    #[clap(flatten)]
    flag_commands: FlagCommands,

    #[clap(flatten)]
    hhvm_options: HhvmOptions,

    #[clap(flatten)]
    files: FileOpts,

    /// Disable toplevel definition elaboration
    #[clap(long)]
    disable_toplevel_elaboration: bool,

    /// Mutate the program as if we're in the debugger repl
    #[clap(long)]
    for_debugger_eval: bool,

    #[clap(long, default_value("0"))]
    emit_class_pointers: i32,

    #[clap(long, default_value("0"))]
    check_int_overflow: i32,

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

    /// Controls systemlib specific logic
    #[clap(long)]
    is_systemlib: bool,

    /// [Experimental] Enable Types in Compilation
    #[clap(long)]
    types_in_compilation: bool,
}

/// Hack Compiler
#[derive(Parser, Debug, Default)]
struct FileOpts {
    /// Input file(s)
    filenames: Vec<PathBuf>,

    /// Read a list of files (one-per-line) from this file
    #[clap(long)]
    input_file_list: Option<PathBuf>,
}

#[derive(Parser, Debug)]
enum Command {
    /// Assemble HHAS file(s) into HackCUnit. Prints those HCUs' HHAS representation.
    Assemble(assemble::Opts),

    /// Compile one Hack source file or a list of files to HHAS
    Compile(compile::Opts),

    /// Compile Hack source files or directories and produce a single CRC per
    /// input file.
    Crc(crc::Opts),

    /// Print the source code with expression tree literals desugared.
    /// Best effort debugging tool.
    DesugarExprTrees(expr_trees::Opts),

    /// Compute facts for a set of files.
    Facts(facts::Opts),

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
    pub fn gather_input_files(&mut self) -> Result<Vec<PathBuf>> {
        use std::io::BufReader;
        let mut files: Vec<PathBuf> = Default::default();
        if let Some(list_path) = self.input_file_list.take() {
            for line in BufReader::new(std::fs::File::open(list_path)?).lines() {
                files.push(Path::new(&line?).to_path_buf());
            }
        }
        files.append(&mut self.filenames);
        Ok(files)
    }

    pub fn is_batch_mode(&self) -> bool {
        self.input_file_list.is_some() || self.filenames.len() > 1
    }
}

impl Opts {
    pub fn env_flags(&self) -> EnvFlags {
        let mut flags = EnvFlags::empty();
        if self.for_debugger_eval {
            flags |= EnvFlags::FOR_DEBUGGER_EVAL;
        }
        if self.disable_toplevel_elaboration {
            flags |= EnvFlags::DISABLE_TOPLEVEL_ELABORATION;
        }
        if self.is_systemlib {
            flags |= EnvFlags::IS_SYSTEMLIB;
        }
        if self.types_in_compilation {
            flags |= EnvFlags::TYPES_IN_COMPILATION;
        }
        flags
    }

    pub fn decl_opts(&self) -> DeclParserOptions {
        // TODO: share this logic with hackc_create_decl_parse_options()
        let config_opts = options::Options::from_configs(&[Self::AUTO_NAMESPACE_MAP]).unwrap();
        let auto_namespace_map = match config_opts.hhvm.aliased_namespaces.get().as_map() {
            Some(m) => m.iter().map(|(k, v)| (k.clone(), v.clone())).collect(),
            None => Vec::new(),
        };
        DeclParserOptions {
            auto_namespace_map,
            disable_xhp_element_mangling: false,
            interpret_soft_types_as_like_types: true,
            allow_new_attribute_syntax: true,
            enable_xhp_class_modifier: false,
            php5_compat_mode: true,
            hhvm_compat_mode: true,
            ..Default::default()
        }
    }

    pub fn native_env(&self, path: PathBuf) -> Result<NativeEnv<'_>> {
        let hhvm_options = &self.hhvm_options;
        let hhvm_config = hhvm_options.to_config()?;
        let parser_flags = ParserFlags::from_hhvm_config(&hhvm_config)?;
        let hhbc_flags = HHBCFlags::from_hhvm_config(&hhvm_config)?;
        Ok(NativeEnv {
            filepath: RelativePath::make(relative_path::Prefix::Dummy, path),
            aliased_namespaces: crate::Opts::AUTO_NAMESPACE_MAP,
            include_roots: crate::Opts::INCLUDE_ROOTS,
            hhbc_flags,
            parser_flags,
            flags: self.env_flags(),
            emit_class_pointers: self.emit_class_pointers,
            check_int_overflow: self.check_int_overflow,
        })
    }

    // TODO (T118266805): get these from nearest .hhconfig enclosing each file.
    pub(crate) const AUTO_NAMESPACE_MAP: &'static str = r#"{
            "hhvm.aliased_namespaces": {
                "global_value": {
                    "Async": "HH\\Lib\\Async",
                    "C": "FlibSL\\C",
                    "Dict": "FlibSL\\Dict",
                    "File": "HH\\Lib\\File",
                    "IO": "HH\\Lib\\IO",
                    "Keyset": "FlibSL\\Keyset",
                    "Locale": "FlibSL\\Locale",
                    "Math": "FlibSL\\Math",
                    "OS": "HH\\Lib\\OS",
                    "PHP": "FlibSL\\PHP",
                    "PseudoRandom": "FlibSL\\PseudoRandom",
                    "Regex": "FlibSL\\Regex",
                    "SecureRandom": "FlibSL\\SecureRandom",
                    "Str": "FlibSL\\Str",
                    "Vec": "FlibSL\\Vec"
                }
            }
        }"#;
    pub(crate) const INCLUDE_ROOTS: &'static str = "";
}

fn main() -> Result<()> {
    env_logger::init();
    let mut opts = Opts::parse();

    // Some subcommands need worker threads with larger than default stacks,
    // even when using Stacker. In particular, various derived traits (e.g. Drop)
    // on AAST nodes are inherently recursive.
    rayon::ThreadPoolBuilder::new()
        .num_threads(opts.num_threads)
        .stack_size(opts.stack_size.get_bytes().try_into()?)
        .build_global()
        .unwrap();

    match opts.command.take() {
        Some(Command::Assemble(opts)) => assemble::run(opts),
        Some(Command::Crc(opts)) => crc::run(opts),
        Some(Command::Parse(parse_opts)) => parse::run(parse_opts),
        Some(Command::ParseBench(bench_opts)) => parse::run_bench_command(bench_opts),
        Some(Command::Verify(opts)) => verify::run(opts),

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
