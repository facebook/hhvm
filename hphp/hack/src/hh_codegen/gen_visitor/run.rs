// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fs::File;
use std::io::Read;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Result;

use super::context::Context;
use super::generator::Generator;
use super::node_impl_generator::*;
use super::node_trait_generator::*;
use super::type_params_generator::*;
use super::visitor_trait_generator::*;
pub use crate::common::args::Args;

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

    generators
        .into_iter()
        .map(|g| {
            let code = g.gen(&ctx)?;
            let filepath = output_dir.join(g.filename());
            Ok((filepath, format!("{}", code)))
        })
        .collect::<Result<Vec<_>>>()
}
