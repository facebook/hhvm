// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::io::Write;

use anyhow::Result;
use clap::Args;
use facts::Facts;
use relative_path::Prefix;
use relative_path::RelativePath;
use serde_json::json;
use serde_json::Value;

/// Facts subcommand options
#[derive(Args, Debug, Default)]
pub(crate) struct Opts {
    #[command(flatten)]
    pub files: crate::FileOpts,
}

/// Write a JSON object to stdout containing Facts for each input file.
/// If input-file-list was specified or if there are multiple CLI files,
/// the result is a JSON object that maps filenames to facts. Otherwise
/// the result is just the single file's Facts.
pub(crate) fn extract_facts(
    hackc_opts: &crate::Opts,
    opts: Opts,
    w: &mut impl Write,
) -> Result<()> {
    // If --input-file-list, output a json wrapper object mapping
    // filenames to facts objects
    let is_batch = !hackc_opts.daemon && opts.files.is_batch_mode();
    let filenames = opts.files.gather_input_files()?;
    let mut file_to_facts = serde_json::Map::with_capacity(filenames.len());
    for path in filenames {
        let dp_opts = hackc_opts.decl_opts();

        // Parse decls
        let text = std::fs::read(&path)?;
        let filename = RelativePath::make(Prefix::Root, path.clone());
        let arena = bumpalo::Bump::new();
        let parsed_file =
            direct_decl_parser::parse_decls_for_bytecode(&dp_opts, filename, &text, &arena);

        // Decls to facts
        if parsed_file.has_first_pass_parse_errors {
            // Swallowing errors is bad.
            continue;
        }
        let facts = Facts::from_decls(&parsed_file);
        let json = json!(facts);
        file_to_facts.insert(path.to_str().unwrap().to_owned(), json);
    }
    if is_batch {
        serde_json::to_writer(w, &Value::Object(file_to_facts))?;
    } else if let Some((_, facts)) = file_to_facts.into_iter().next() {
        serde_json::to_writer_pretty(w, &facts)?;
    }
    Ok(())
}

pub(crate) fn run_flag(hackc_opts: &mut crate::Opts, w: &mut impl Write) -> Result<()> {
    let facts_opts = Opts {
        files: std::mem::take(&mut hackc_opts.files),
        ..Default::default()
    };
    extract_facts(hackc_opts, facts_opts, w)
}

pub(crate) fn daemon(hackc_opts: &mut crate::Opts) -> Result<()> {
    crate::daemon_loop(|path, w| {
        hackc_opts.files.filenames = vec![path];
        run_flag(hackc_opts, w)
    })
}
