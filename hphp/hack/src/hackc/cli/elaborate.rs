// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashSet;
use std::path::Path;
use std::sync::Arc;

use aast_parser::AastParser;
use anyhow::anyhow;
use anyhow::Context;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::typechecker_options::TypecheckerOptions;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use relative_path::Prefix;
use relative_path::RelativePath;

use crate::FileOpts;

#[derive(clap::Args, Debug)]
pub struct Opts {
    #[command(flatten)]
    files: FileOpts,
}

pub fn run(opts: Opts) -> anyhow::Result<()> {
    let files = opts.files.gather_input_files()?;
    let single_file = files.len() == 1;
    // Process all files, collect fatal errors
    let errs: Vec<anyhow::Error> = files
        .into_par_iter()
        .map(|path| process_one_file(single_file, &path, &opts))
        .filter_map(|result| result.err())
        .collect();
    if errs.is_empty() {
        Ok(())
    } else {
        for err in &errs {
            eprintln!("fatal error: {err}");
        }
        Err(anyhow!("encountered {} fatal errors", errs.len()))
    }
}

fn process_one_file(single_file: bool, path: &Path, _: &Opts) -> anyhow::Result<()> {
    let content =
        std::fs::read(path).with_context(|| format!("failed to read {}", path.display()))?;
    let rel_path = Arc::new(RelativePath::make(Prefix::Dummy, path.to_path_buf()));
    let source_text = IndexedSourceText::new(SourceText::make(Arc::clone(&rel_path), &content));
    let mut parser_result =
        AastParser::from_text(&Default::default(), &source_text, HashSet::default())
            .map_err(|e| anyhow!("failed to parse {}: {:#?}", path.display(), e))?;
    let tco = TypecheckerOptions::default();
    let errs = elab::elaborate_program(&tco, &rel_path, &mut parser_result.aast);
    print_parse_result(single_file, path, &parser_result, &errs)?;
    Ok(())
}

fn print_parse_result(
    single_file: bool,
    path: &Path,
    parser_result: &aast_parser::ParserResult,
    naming_errors: &[NamingPhaseError],
) -> std::io::Result<()> {
    use std::io::Write;
    if !parser_result.syntax_errors.is_empty()
        || !parser_result.lowerer_parsing_errors.is_empty()
        || !parser_result.errors.is_empty()
        || !parser_result.lint_errors.is_empty()
        || !naming_errors.is_empty()
    {
        let file_name = path.file_name().unwrap().to_string_lossy();
        let mut stdout = std::io::stdout().lock();
        let w = &mut stdout;
        if !single_file {
            writeln!(w, "{}", file_name)?;
        }
        write_errs(w, "Syntax", &parser_result.syntax_errors)?;
        write_errs(w, "Lowering", &parser_result.lowerer_parsing_errors)?;
        write_errs(w, "Parse", &parser_result.errors)?;
        write_errs(w, "Lint", &parser_result.lint_errors)?;
        write_errs(w, "Naming", naming_errors)?;
    }
    if single_file {
        println!("{:#?}", &parser_result.aast);
    }
    Ok(())
}

fn write_errs<W: std::io::Write, T: std::fmt::Debug>(
    w: &mut W,
    error_kind: &'static str,
    errs: &[T],
) -> std::io::Result<()> {
    if errs.is_empty() {
        return Ok(());
    }
    writeln!(w, "{error_kind} errors: {:#?}", errs)
}
