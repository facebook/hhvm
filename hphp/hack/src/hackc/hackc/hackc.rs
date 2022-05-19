// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod compile;
mod crc;
mod expr_trees;
mod facts;
mod parse;

use ::compile::{EnvFlags, HHBCFlags, NativeEnv, ParserFlags};
use anyhow::Result;
use clap::Parser;
use hhvm_options::HhvmOptions;
use oxidized::relative_path::{self, RelativePath};
use oxidized_by_ref::decl_parser_options::DeclParserOptions;
use std::{
    fs,
    io::{BufRead, BufReader},
    path::{Path, PathBuf},
};

/// Hack Compiler
#[derive(Parser, Debug, Default)]
struct Opts {
    #[clap(subcommand)]
    command: Option<Command>,

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

    /// Number of parallel worker threads. If 0, use num-cpu worker threads.
    #[clap(long, default_value("0"))]
    num_threads: usize,
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
    /// Compile one Hack source file or a list of files to HHAS
    Compile(compile::Opts),

    /// Compile Hack source files or directories and produce a single CRC per
    /// input file.
    Crc(crc::Opts),

    /// Compute facts for a set of files.
    Facts(facts::Opts),

    /// Parse many files whose filenames are read from stdin
    Parse(parse::Opts),
}

// Which command are we running? Every bool option here conflicts with
// every other one. Using bool opts for backward compatibility with
// hh_single_compile_cpp.
#[derive(Parser, Debug, Default)]
struct FlagCommands {
    /// Runs in daemon mode for testing purposes. Do not rely on for production
    #[clap(long)]
    daemon: bool,

    /// Print the source code with expression tree literals desugared.
    /// Best effort debugging tool.
    #[clap(long)]
    dump_desugared_expression_trees: bool,

    /// Parse decls from source text, transform them into facts, and print the facts
    /// in JSON format.
    #[clap(long)]
    extract_facts_from_decls: bool,

    /// Render the source text parse tree
    #[clap(long)]
    parse: bool,

    /// Compile file with decls from the same file available during compilation.
    #[clap(long)]
    test_compile_with_decls: bool,

    /// Verify decls ffi
    #[clap(long)]
    verify_decls_ffi: bool,
}

impl FileOpts {
    pub fn gather_input_files(&mut self) -> Result<Vec<PathBuf>> {
        let mut files: Vec<PathBuf> = Default::default();
        if let Some(list_path) = self.input_file_list.take() {
            for line in BufReader::new(fs::File::open(list_path)?).lines() {
                files.push(Path::new(&line?).to_path_buf());
            }
        }
        files.append(&mut self.filenames);
        Ok(files)
    }

    pub fn is_multi(&self) -> bool {
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
        flags
    }

    pub fn decl_opts<'a>(&self, arena: &'a bumpalo::Bump) -> DeclParserOptions<'a> {
        // TODO: share this logic with hackc_create_decl_parse_options()
        let config_opts = options::Options::from_configs(&[Self::AUTO_NAMESPACE_MAP]).unwrap();
        let auto_namespace_map = match config_opts.hhvm.aliased_namespaces.get().as_map() {
            Some(m) => bumpalo::collections::Vec::from_iter_in(
                m.iter().map(|(k, v)| {
                    (
                        arena.alloc_str(k.as_str()) as &str,
                        arena.alloc_str(v.as_str()) as &str,
                    )
                }),
                arena,
            )
            .into_bump_slice(),
            None => &[],
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
    // For now these are all hardcoded in hh_single_compile_cpp, so hardcode
    // them here too.
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

fn verify_decls_ffi(_: Opts) -> Result<()> {
    unimplemented!()
}

fn daemon_mode(_: Opts) -> Result<()> {
    unimplemented!()
}

fn parse(_: Opts) -> Result<()> {
    unimplemented!()
}

fn compile_from_text_with_same_file_decl(_: Opts) -> Result<()> {
    unimplemented!()
}

fn main() -> Result<()> {
    env_logger::init();
    let mut opts = Opts::parse();

    let builder = rayon::ThreadPoolBuilder::new().num_threads(opts.num_threads);
    let builder = if matches!(&opts.command, Some(Command::Crc(_))) {
        builder.stack_size(32 * 1024 * 1024)
    } else {
        builder
    };
    builder.build_global().unwrap();

    match opts.command.take() {
        Some(Command::Compile(opts)) => compile::run(opts),
        Some(Command::Crc(opts)) => crc::run(opts),
        Some(Command::Parse(opts)) => parse::run(opts),
        Some(Command::Facts(facts_opts)) => facts::extract_facts(opts, facts_opts),
        None => {
            if opts.flag_commands.verify_decls_ffi {
                verify_decls_ffi(opts)
            } else if opts.flag_commands.daemon {
                daemon_mode(opts)
            } else if opts.flag_commands.parse {
                parse(opts)
            } else if opts.flag_commands.extract_facts_from_decls {
                let facts_opts = facts::Opts {
                    files: std::mem::take(&mut opts.files),
                    ..Default::default()
                };
                facts::extract_facts(opts, facts_opts)
            } else if opts.flag_commands.test_compile_with_decls {
                compile_from_text_with_same_file_decl(opts)
            } else if opts.flag_commands.dump_desugared_expression_trees {
                expr_trees::dump_expr_trees(opts)
            } else {
                compile::compile_from_text(opts)
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Just make sure json parsing produces a proper list.
    /// If the alias map length changes, keep this test in sync.
    #[test]
    fn test_auto_namespace_map() {
        let arena = bumpalo::Bump::new();
        let dp_opts = Opts::default().decl_opts(&arena);
        assert_eq!(dp_opts.auto_namespace_map.len(), 15);
    }
}
