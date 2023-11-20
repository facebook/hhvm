// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashSet;
use std::io::stdin;
use std::io::BufRead;
use std::io::BufReader;
use std::io::Write;
use std::path::PathBuf;
use std::sync::Arc;

use aast_parser::AastParser;
use anyhow::Context;
use anyhow::Result;
use clap::builder::TypedValueParser;
use clap::Args;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::parser_env::ParserEnv;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use relative_path::Prefix;
use relative_path::RelativePath;
use strum::Display;
use strum::EnumString;
use strum::EnumVariantNames;
use strum::VariantNames;

#[derive(Args, Clone, Debug)]
pub struct BenchOpts {
    #[clap(default_value_t, long, value_parser = clap::builder::PossibleValuesParser::new(ParserKind::VARIANTS).map(|s| s.parse::<ParserKind>().unwrap()))]
    parser: ParserKind,
}

#[derive(Clone, Copy, Debug, Display, EnumString, EnumVariantNames)]
enum ParserKind {
    Aast,
    Positioned,
    PositionedByRef,
    PositionedWithFullTrivia,
    DirectDecl,
}

impl Default for ParserKind {
    fn default() -> Self {
        Self::Positioned
    }
}

pub fn run_bench_command(bench_opts: BenchOpts) -> Result<()> {
    BufReader::new(stdin())
        .lines()
        .collect::<std::io::Result<Vec<_>>>()
        .context("could not read line from input file list")?
        .into_par_iter()
        .try_for_each(|line| {
            let path = PathBuf::from(line.trim());
            let content = std::fs::read(&path)?;
            let env = ParserEnv::default();
            let filepath = RelativePath::make(Prefix::Dummy, path);
            let source_text = SourceText::make(Arc::new(filepath.clone()), &content);
            match bench_opts.parser {
                ParserKind::PositionedWithFullTrivia => {
                    let stdout = std::io::stdout();
                    let w = stdout.lock();
                    let mut s = serde_json::Serializer::new(w);
                    let arena = bumpalo::Bump::new();
                    let src = IndexedSourceText::new(source_text);
                    positioned_full_trivia_parser::parse_script_to_json(&arena, &mut s, &src, env)?
                }
                ParserKind::PositionedByRef => {
                    let arena = bumpalo::Bump::new();
                    let (_, _, _) =
                        positioned_by_ref_parser::parse_script(&arena, &source_text, env);
                }
                ParserKind::Positioned => {
                    let (_, _, _) = positioned_parser::parse_script(&source_text, env);
                }
                ParserKind::DirectDecl => {
                    let arena = bumpalo::Bump::new();
                    let opts = Default::default();
                    let _ = direct_decl_parser::parse_decls_for_typechecking(
                        &opts, filepath, &content, &arena,
                    );
                }
                ParserKind::Aast => {
                    let indexed_source_text = IndexedSourceText::new(source_text);
                    let env = aast_parser::rust_aast_parser_types::Env::default();
                    let _ = AastParser::from_text(&env, &indexed_source_text, HashSet::default());
                }
            }
            Ok(())
        })
}

#[derive(Args, Debug)]
pub(crate) struct Opts {
    #[command(flatten)]
    pub files: crate::FileOpts,

    /// Print compact JSON instead of formatted & indented JSON.
    #[clap(long)]
    compact: bool,
}

fn parse(path: PathBuf, w: &mut impl Write, pretty: bool) -> Result<()> {
    let source_text = std::fs::read(&path)?;
    // TODO T118266805: These should come from config files.
    let env = ParserEnv {
        codegen: true,
        hhvm_compat_mode: true,
        php5_compat_mode: true,
        interpret_soft_types_as_like_types: true,
        ..Default::default()
    };
    let filepath = RelativePath::make(Prefix::Dummy, path);
    let source_text = SourceText::make(Arc::new(filepath), &source_text);
    let indexed_source = IndexedSourceText::new(source_text);
    let arena = bumpalo::Bump::new();
    let json = if pretty {
        let mut serializer = serde_json::Serializer::pretty(vec![]);
        positioned_full_trivia_parser::parse_script_to_json(
            &arena,
            &mut serializer,
            &indexed_source,
            env,
        )?;
        serializer.into_inner()
    } else {
        let mut serializer = serde_json::Serializer::new(vec![]);
        positioned_full_trivia_parser::parse_script_to_json(
            &arena,
            &mut serializer,
            &indexed_source,
            env,
        )?;
        serializer.into_inner()
    };
    w.write_all(&json)?;
    Ok(())
}

pub(crate) fn run(opts: Opts) -> Result<()> {
    let filenames = opts.files.gather_input_files()?;
    let mut w = std::io::stdout();
    for path in filenames {
        parse(path, &mut w, !opts.compact)?;
        w.write_all(b"\n")?;
        w.flush()?;
    }
    Ok(())
}
