// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Result;
use clap::Parser;

/// Hack Compiler
#[derive(Parser, Debug)]
struct Opts {
    #[clap(flatten)]
    command: Command,
}

// Which command are we running? Every bool option here conflicts with
// every other one. Using bool opts for backward compatibility with
// hh_single_compile_cpp.
#[derive(Parser, Debug)]
struct Command {
    /// Compile source text to HackCUnit
    #[clap(long, exclusive(true))]
    compile_and_print_unit: bool,

    /// Runs in daemon mode for testing purposes. Do not rely on for production
    #[clap(long, exclusive(true))]
    daemon: bool,

    /// Print the source code with expression tree literals desugared.
    /// Best effort debugging tool.
    #[clap(long, exclusive(true))]
    dump_desugared_expression_trees: bool,

    /// Parse decls from source text, transform them into facts, and print the facts
    /// in JSON format.
    #[clap(long, exclusive(true))]
    extract_facts_from_decls: bool,

    /// Render the source text parse tree
    #[clap(long, exclusive(true))]
    parse: bool,

    /// Test FFIs
    #[clap(long, exclusive(true))]
    test: bool,

    /// Compile file with decls from the same file available during compilation.
    #[clap(long, exclusive(true))]
    test_compile_with_decls: bool,

    /// Verify decls ffi
    #[clap(long, exclusive(true))]
    verify_decls_ffi: bool,
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

fn dump_expr_trees(_: Opts) -> Result<()> {
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
        dump_expr_trees(opts)
    } else {
        compile_from_text(opts)
    }
}
