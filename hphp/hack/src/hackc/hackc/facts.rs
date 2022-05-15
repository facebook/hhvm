// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use anyhow::Result;
use clap::Parser;
use facts_rust::Facts;
use oxidized::relative_path::{Prefix, RelativePath};
use serde_json::{json, Value};

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
    let filenames = opts.files.gather_input_files()?;
    let mut file_to_facts = serde_json::Map::with_capacity(filenames.len());
    for path in filenames {
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
        let json = if opts.nohash {
            json!(facts)
        } else {
            facts.to_json_value(&text)
        };
        file_to_facts.insert(path.to_str().unwrap().to_owned(), json);
    }
    if multi {
        serde_json::to_writer(std::io::stdout(), &Value::Object(file_to_facts))?;
    } else if let Some((_, json)) = file_to_facts.into_iter().next() {
        serde_json::to_writer_pretty(std::io::stdout(), &json)?;
    }
    Ok(())
}
