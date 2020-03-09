// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use super::{
    context::Context, generator::Generator, node_impl_generator::*, node_trait_generator::*,
    type_params_generator::*, visitor_trait_generator::*,
};
use crate::common::*;
use clap::ArgMatches;
use std::{
    fs::File,
    io::Read,
    path::{Path, PathBuf},
};

pub fn run(m: &ArgMatches) -> Result<Vec<(PathBuf, String)>> {
    let inputs = m.values_of("input").ok_or("missing input files")?;
    let output_dir = Path::new(m.value_of("output").ok_or("missing output path")?);
    let root = m.value_of("root").ok_or("missing root")?;
    let files = inputs
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
