// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod expr_trees;
mod facts;

use anyhow::Result;
use clap::Parser;
use compile::EnvFlags;
use hhvm_options::HhvmOptions;
use oxidized_by_ref::decl_parser_options::DeclParserOptions;
use std::{
    fs,
    io::{BufRead, BufReader},
    path::{Path, PathBuf},
};

/// Hack Compiler
#[derive(Parser, Debug, Default)]
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
#[derive(Parser, Debug, Default)]
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

    pub fn decl_opts<'a>(&self, arena: &'a bumpalo::Bump) -> DeclParserOptions<'a> {
        // TODO (T118266805): get these from nearest .hhconfig enclosing each file.
        // For now these are all hardcoded in hh_single_compile_cpp, so hardcode
        // them here too.
        const AUTO_NAMESPACE_MAP: &str = r#"{
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

        // TODO: share this logic with hackc_create_decl_parse_options()
        let config_opts = options::Options::from_configs(&[AUTO_NAMESPACE_MAP], &[]).unwrap();
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
        facts::extract_facts(opts)
    } else if opts.command.test_compile_with_decls {
        compile_from_text_with_same_file_decl(opts)
    } else if opts.command.dump_desugared_expression_trees {
        expr_trees::dump_expr_trees(opts)
    } else {
        compile_from_text(opts)
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
