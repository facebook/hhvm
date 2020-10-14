// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod context;
mod node_impl_generator;
mod visitor_trait_generator;

use std::path::{Path, PathBuf};

use anyhow::Result;
use structopt::StructOpt;

use context::Context;

#[derive(Debug, StructOpt)]
pub struct Args {
    /// Rust files containing the types for which codegen will be performed.
    /// All types reachable from the given root type must be defined in one of
    /// the files provided as `--input` or `--extern-input`.
    #[structopt(short, long, parse(from_os_str))]
    input: Vec<PathBuf>,

    /// Rust files containing the types for which codegen will be performed.
    /// All types reachable from the given root type must be defined in on of
    /// the files provided as `--input` or `--extern-input`.
    #[structopt(short, long, parse(from_os_str))]
    extern_input: Vec<PathBuf>,

    /// The root type of the AST. All types reachable from this type will be
    /// visited by the generated visitor.
    #[structopt(short, long)]
    root: String,

    /// The directory to which generated files will be written.
    #[structopt(short, long, parse(from_os_str))]
    output: PathBuf,
}

pub fn run(args: &Args) -> Result<Vec<(PathBuf, String)>> {
    let files = parse_all(&args.input)?;
    let extern_files = parse_all(&args.extern_input)?;

    let ctx = Context::new(files.as_slice(), extern_files.as_slice(), &args.root)?;

    let results = vec![
        ("node_impl_gen.rs", node_impl_generator::gen(&ctx)),
        ("visitor.rs", visitor_trait_generator::gen(&ctx)),
    ];
    Ok(results
        .iter()
        .map(|(filename, source)| (args.output.join(filename), source.to_string()))
        .collect())
}

fn parse_all<'a>(files: &'a [PathBuf]) -> Result<Vec<(&'a Path, Vec<syn::Item>)>> {
    files
        .iter()
        .map(|filename| -> Result<(&Path, Vec<syn::Item>)> {
            let src = std::fs::read_to_string(&filename)?;
            let file = syn::parse_file(&src)?;
            Ok((&filename, file.items.into_iter().collect()))
        })
        .collect()
}
