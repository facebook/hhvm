// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::io::Write;
use std::path::PathBuf;

use anyhow::Result;
use clap::Args;
use direct_decl_parser::ParsedFile;
use hash::IndexMap;
use relative_path::Prefix;
use relative_path::RelativePath;

/// Decls subcommand options
#[derive(Args, Debug, Default)]
pub(crate) struct Opts {
    #[command(flatten)]
    pub files: crate::FileOpts,
}

/// Write a serialized object to stdout containing Decls for each input file.
pub(crate) fn binary_decls(hackc_opts: crate::Opts, opts: Opts) -> Result<()> {
    let filenames = opts.files.gather_input_files()?;
    let dp_opts = hackc_opts.decl_opts();
    let mut parsed_files: IndexMap<PathBuf, ParsedFile<'_>> = Default::default();
    let arena = bumpalo::Bump::new();
    for path in filenames {
        // Parse decls
        let text = std::fs::read(&path)?;
        let filename = RelativePath::make(Prefix::Root, path.clone());
        let parsed_file =
            direct_decl_parser::parse_decls_for_bytecode(&dp_opts, filename, &text, &arena);
        parsed_files.insert(path.to_path_buf(), parsed_file);
    }
    let mut data = Vec::new();
    decl_provider::serialize_batch_decls(&mut data, &parsed_files)?;
    std::io::stdout().write_all(&data)?;
    Ok(())
}

pub(crate) fn json_decls(hackc_opts: crate::Opts, opts: Opts) -> Result<()> {
    let filenames = opts.files.gather_input_files()?;
    let dp_opts = hackc_opts.decl_opts();
    for path in filenames {
        // Parse decls
        let text = std::fs::read(&path)?;
        let arena = bumpalo::Bump::new();
        let filename = RelativePath::make(Prefix::Root, path.clone());
        let parsed_file =
            direct_decl_parser::parse_decls_for_bytecode(&dp_opts, filename, &text, &arena);
        serde_json::to_writer_pretty(&mut std::io::stdout(), &parsed_file)?;
    }
    Ok(())
}
