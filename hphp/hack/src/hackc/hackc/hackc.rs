// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod expr_trees;

use anyhow::Result;
use clap::Parser;
use compile::EnvFlags;
use hhvm_options::HhvmOptions;
use std::{
    fs,
    io::{BufRead, BufReader},
    path::{Path, PathBuf},
};

/// Hack Compiler
#[derive(Parser, Debug)]
struct Opts {
    #[clap(flatten)]
    command: Command,

    #[clap(flatten)]
    hhvm_options: HhvmOptions,

    /// Disable toplevel definition elaboration
    #[clap(long)]
    disable_toplevel_elaboration: bool,

    /// Input file(s)
    filenames: Vec<PathBuf>,

    /// Mutate the program as if we're in the debugger repl
    #[clap(long)]
    for_debugger_eval: bool,

    /// Read a list of files (one-per-line) from this file
    #[clap(long)]
    input_file_list: Option<PathBuf>,
}

// Which command are we running? Every bool option here conflicts with
// every other one. Using bool opts for backward compatibility with
// hh_single_compile_cpp.
#[derive(Parser, Debug)]
struct Command {
    /// Compile source text to HackCUnit
    #[clap(long)]
    compile_and_print_unit: bool,

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

    /// Test FFIs
    #[clap(long)]
    test: bool,

    /// Compile file with decls from the same file available during compilation.
    #[clap(long)]
    test_compile_with_decls: bool,

    /// Verify decls ffi
    #[clap(long)]
    verify_decls_ffi: bool,
}

impl Opts {
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
}

fn test(_: Opts) -> Result<()> {
    unimplemented!()
}

fn verify_decls_ffi(_: Opts) -> Result<()> {
    unimplemented!()
}

fn compile_unit_from_text(_: Opts) -> Result<()> {
    unimplemented!()
}

fn daemon_mode(_: Opts) -> Result<()> {
    unimplemented!()
}

fn parse(_: Opts) -> Result<()> {
    unimplemented!()
}

fn extract_facts_from_decls(_: Opts) -> Result<()> {
    unimplemented!()
}

fn compile_from_text_with_same_file_decl(_: Opts) -> Result<()> {
    unimplemented!()
}

fn compile_from_text(_: Opts) -> Result<()> {
    unimplemented!()
}

fn main() -> Result<()> {
    let opts = Opts::parse();
    if opts.command.test {
        test(opts)
    } else if opts.command.verify_decls_ffi {
        verify_decls_ffi(opts)
    } else if opts.command.compile_and_print_unit {
        compile_unit_from_text(opts)
    } else if opts.command.daemon {
        daemon_mode(opts)
    } else if opts.command.parse {
        parse(opts)
    } else if opts.command.extract_facts_from_decls {
        extract_facts_from_decls(opts)
    } else if opts.command.test_compile_with_decls {
        compile_from_text_with_same_file_decl(opts)
    } else if opts.command.dump_desugared_expression_trees {
        expr_trees::dump_expr_trees(opts)
    } else {
        compile_from_text(opts)
    }
}
