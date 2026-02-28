// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::path::PathBuf;

use anyhow::Result;
use clap::Parser;
use direct_decl_parser::DeclParserOptions;
use hh_config::HhConfig;
use relative_path::Prefix;
use relative_path::RelativePath;

#[derive(Parser)]
struct Opts {
    path: PathBuf,

    #[clap(long)]
    root: Option<PathBuf>,
    #[clap(long, short, action)]
    for_typecheck: bool,
}

fn main() -> Result<()> {
    let opts = Opts::parse();
    let dp_opts = match opts.root {
        Some(root) => {
            let hh_config = HhConfig::from_root(root, &Default::default())?;
            DeclParserOptions::from_parser_options(&hh_config.opts.po)
        }
        None => Default::default(),
    };
    let text = std::fs::read(&opts.path)?;
    let rpath = RelativePath::make(Prefix::Tmp, opts.path);
    if opts.for_typecheck {
        println!(
            "{:#?}",
            direct_decl_parser::parse_decls_for_typechecking(&dp_opts, rpath.clone(), &text,)
        );
    } else {
        println!(
            "{:#?}",
            direct_decl_parser::parse_decls_for_bytecode(&dp_opts, rpath.clone(), &text,)
        );
    }
    Ok(())
}
