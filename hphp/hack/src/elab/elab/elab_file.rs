// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use aast_parser::AastParser;
use anyhow::anyhow;
use anyhow::Result;
use log::info;
use ocamlrep::rc::RcOc;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use relative_path::Prefix;
use relative_path::RelativePath;

pub fn elab_one_file<Ctx, Err, Pass>(
    path: &Path,
    top_down: &Pass,
    bottom_up: &Pass,
) -> Result<String>
where
    Ctx: Clone + Default,
    Pass: transform::Pass<Ctx = Ctx, Err = Err>,
{
    let content = std::fs::read(path)?;
    let filepath = RelativePath::make(Prefix::Dummy, path.to_path_buf());
    let filename = path.file_name().unwrap().to_str().unwrap();

    let source_text = SourceText::make(RcOc::new(filepath), &content);
    let indexed_source_text = IndexedSourceText::new(source_text);
    let env = aast_parser::rust_aast_parser_types::Env::default();
    match AastParser::from_text(&env, &indexed_source_text) {
        Err(error) => Err(anyhow!("parse failure {path:#?}: {error:#?}"))?,
        Ok(mut parse_result) => {
            info!("parsing {filename:#?}");
            let program = &mut parse_result.aast;
            let before = format!("{program:#?}");

            let mut ctx = Ctx::default();
            let mut errs = Vec::default();

            info!("calling 'tranform_ty_program' on {filename:#?}");
            transform::transform_ty_program(program, &mut ctx, &mut errs, top_down, bottom_up);
            if !errs.is_empty() {
                println!(
                    "{} errors encountered elaborating {filename:#?}",
                    errs.len()
                );
            }
            let after = format!("{program:#?}");

            info!("calculating diff for {filename:#?}");
            let diff = similar::TextDiff::from_lines(&before, &after)
                .unified_diff()
                .context_radius(10)
                .header("before", "after")
                .to_string();
            if diff.is_empty() {
                info!("calling 'transform_ty_program' on {filename:#?} had no effect");
            }
            Ok(diff)
        }
    }
}
