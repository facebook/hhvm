use std::path::Path;

use aast_parser::AastParser;
use anyhow::anyhow;
use anyhow::Result;
use clap::Args;
use log::info;
use ocamlrep::rc::RcOc;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use relative_path::Prefix;
use relative_path::RelativePath;

use crate::FileOpts;

#[derive(Args, Debug)]
pub struct Opts {
    #[command(flatten)]
    files: FileOpts,
}

pub fn run(opts: Opts) -> Result<()> {
    // Collect a Vec first so we process all files - not just up to the
    // first failure.
    let files = &opts.files.filenames;
    files
        .into_par_iter()
        .map(|path| process_one_file(path, &opts))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

pub fn process_one_file(path: &Path, _: &Opts) -> Result<()> {
    let content = std::fs::read(path)?;
    let filepath = RelativePath::make(Prefix::Dummy, path.to_path_buf());
    let filename = path.file_name().unwrap().to_str().unwrap();
    let source_text = SourceText::make(RcOc::new(filepath), &content);
    let indexed_source_text = IndexedSourceText::new(source_text);
    let env = aast_parser::rust_aast_parser_types::Env::default();

    info!("elaborating {filename:#?}");
    match AastParser::from_text(&env, &indexed_source_text) {
        Err(error) => Err(anyhow!("parse failure {path:#?}: {error:#?}"))?,
        Ok(mut parse_result) => {
            // TODO: print errors in parse result?

            info!("invoking 'elaborate_program' on {filename:#?}");
            let _errs = elab::elaborate_program(&mut parse_result.aast);

            // TODO: print elaboaration errors (`errs`).
            println!("{:#?}", &parse_result.aast);
            Ok(())
        }
    }
}
