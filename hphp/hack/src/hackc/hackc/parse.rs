// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use aast_parser::AastParser;
use anyhow::{Context, Result};
use clap::Parser;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::{Prefix, RelativePath};
use parser_core_types::{
    indexed_source_text::IndexedSourceText, parser_env::ParserEnv, source_text::SourceText,
};
use rayon::prelude::*;
use std::{
    io::{stdin, BufRead, BufReader},
    path::PathBuf,
};
use strum::VariantNames;
use strum_macros::{Display, EnumString, EnumVariantNames};

#[derive(Parser, Clone, Debug)]
pub struct BenchOpts {
    #[clap(default_value_t, long, possible_values(ParserKind::VARIANTS))]
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
            let source_text = SourceText::make(RcOc::new(filepath.clone()), &content);
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
                    let _ = direct_decl_parser::parse_decls(&opts, filepath, &content, &arena);
                }
                ParserKind::Aast => {
                    let indexed_source_text = IndexedSourceText::new(source_text);
                    let env = aast_parser::rust_aast_parser_types::Env::default();
                    let _ = AastParser::from_text(&env, &indexed_source_text);
                }
            }
            Ok(())
        })
}

#[derive(Parser, Clone, Debug)]
pub struct Opts {}

pub(crate) fn run(hackc_opts: &mut crate::Opts, _: Opts) -> Result<()> {
    let filenames = hackc_opts.files.gather_input_files()?;
    for path in filenames {
        let source_text = std::fs::read(&path)?;
        // TODO T118266805: These should come from config files.
        let env = ParserEnv {
            codegen: true,
            hhvm_compat_mode: true,
            php5_compat_mode: true,
            allow_new_attribute_syntax: true,
            disallow_fun_and_cls_meth_pseudo_funcs: true,
            interpret_soft_types_as_like_types: true,
            ..Default::default()
        };
        let filepath = RelativePath::make(Prefix::Dummy, path);
        let source_text = SourceText::make(RcOc::new(filepath), &source_text);
        let indexed_source = IndexedSourceText::new(source_text);
        let arena = bumpalo::Bump::new();
        let mut serializer = serde_json::Serializer::new(vec![]);
        positioned_full_trivia_parser::parse_script_to_json(
            &arena,
            &mut serializer,
            &indexed_source,
            env,
        )?;
        let json = serializer.into_inner();
        crate::daemon_print(hackc_opts, &json)?;
    }
    Ok(())
}
