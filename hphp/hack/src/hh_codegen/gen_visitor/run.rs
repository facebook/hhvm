// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use super::{
    context::Context, generator::Generator, node_impl_generator::*, node_trait_generator::*,
    type_params_generator::*, visitor_trait_generator::*,
};
use anyhow::Result;
use std::{
    fs::File,
    io::Read,
    path::{Path, PathBuf},
};
use structopt::StructOpt;

#[derive(Debug, StructOpt)]
pub struct Args {
    /// Rust files containing the types for which codegen will be performed.
    /// All types reachable from the given root type must be defined in one of
    /// the files provided as `--input` or `--extern-input`.
    #[structopt(short, long, parse(from_os_str))]
    input: Vec<PathBuf>,

    /// The root type of the AST. All types reachable from this type will be
    /// visited by the generated visitor.
    #[structopt(short, long)]
    root: String,

    /// The directory to which generated files will be written.
    #[structopt(short, long, parse(from_os_str))]
    output: PathBuf,
}

pub fn run(args: &Args) -> Result<Vec<(PathBuf, String)>> {
    let inputs = &args.input;
    let output_dir = &args.output;
    let root = &args.root;
    let files = inputs
        .iter()
        .map(|file| -> Result<(syn::File, &Path)> {
            let file_path = Path::new(file);
            let mut file = File::open(file)?;
            let mut src = String::new();
            file.read_to_string(&mut src)?;
            Ok((syn::parse_file(&src)?, file_path))
        })
        .collect::<Result<Vec<_>>>()?;

    let ctx = Context::new(files.as_slice(), root)?;

    let generators: Vec<Box<dyn Generator>> = vec![
        Box::new(TypeParamGenerator),
        Box::new(RefNodeTrait),
        Box::new(MutNodeTrait),
        Box::new(RefNodeImpl),
        Box::new(MutNodeImpl),
        Box::new(RefVisitorTrait),
        Box::new(MutVisitorTrait),
    ];

    Ok(generators
        .into_iter()
        .map(|g| {
            let code = g.gen(&ctx)?;
            let filepath = output_dir.join(g.filename());
            Ok((filepath, format!("{}", code)))
        })
        .collect::<Result<Vec<_>>>()?)
}
