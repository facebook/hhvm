// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use anyhow::Result;
use clap::Parser;
use facts_rust::Facts;
use oxidized::relative_path::{Prefix, RelativePath};
use serde_json::json;
use std::io::Write;

/// Facts subcommand options
#[derive(Parser, Debug, Default)]
pub(crate) struct Opts {
    #[clap(flatten)]
    pub files: crate::FileOpts,

    /// Exclude sha1sum from JSON results.
    #[clap(long)]
    pub nohash: bool,
}

/// Write a JSON object to stdout containing Facts for each input file.
/// If input-file-list was specified or if there are multiple CLI files,
/// the result is a JSON object will map filenames to facts. Otherwise
/// the result is just the single file's Facts.
pub(crate) fn extract_facts(hackc_opts: crate::Opts, mut opts: Opts) -> Result<()> {
    // If --input-file-list, output a json wrapper object mapping
    // filenames to facts objects
    let multi = opts.files.is_multi();
    let mut sep = "{";
    let filenames = opts.files.gather_input_files()?;
    for path in filenames {
        let mut w = std::io::stdout();
        let arena = bumpalo::Bump::new();
        let dp_opts = hackc_opts.decl_opts(&arena);

        // Parse decls
        let text = std::fs::read(&path)?;
        let filename = RelativePath::make(Prefix::Root, path.clone());
        let parsed_file = direct_decl_parser::parse_decls_without_reference_text(
            &dp_opts, filename, &text, &arena,
        );

        // Decls to facts
        if parsed_file.has_first_pass_parse_errors {
            // Swallowing errors is bad.
            continue;
        }
        let facts = Facts::from_decls(
            &parsed_file.decls,
            parsed_file.file_attributes,
            dp_opts.disable_xhp_element_mangling,
        );
        if multi {
            write!(w, "{}\"{}\":", sep, path.display())?;
        }
        if opts.nohash {
            // No pretty-print to save space.
            serde_json::to_writer(w, &json!(facts))?;
        } else {
            // Pretty-print for a user friendly CLI.
            w.write_all(facts.to_json(true, &text).as_bytes())?;
        }
        sep = ",";
    }
    if multi && sep != "{" {
        write!(std::io::stdout(), "}}")?;
    }
    Ok(())
}
