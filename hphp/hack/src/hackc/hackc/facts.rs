// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::Opts;
use anyhow::Result;
use facts_rust::facts::Facts;
use oxidized::relative_path::{Prefix, RelativePath};
use std::io::Write;

/// Write a JSON object to stdout containing Facts for each input file.
/// If input-file-list was specified or if there are multiple CLI files,
/// the result is a JSON object will map filenames to facts. Otherwise
/// the result is just the single file's Facts.
pub(crate) fn extract_facts(mut opts: Opts) -> Result<()> {
    // If --input-file-list, output a json wrapper object mapping
    // filenames to facts objects
    let multi = opts.input_file_list.is_some() || opts.filenames.len() > 1;
    let mut sep = "{";
    let filenames = opts.gather_input_files()?;
    let mut w = std::io::stdout();
    for path in filenames {
        let arena = bumpalo::Bump::new();
        let dp_opts = opts.decl_opts(&arena);

        // Parse decls
        let text = std::fs::read(&path)?;
        let filename = RelativePath::make(Prefix::Root, path.clone());
        let parsed_file = direct_decl_parser::parse_decls_without_reference_text(
            &dp_opts, filename, &text, &arena,
        );

        // Decls to facts
        if parsed_file.has_first_pass_parse_errors {
            // XXX swallowing errors is bad.
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
        w.write_all(facts.to_json(&text).as_bytes())?;
        sep = ",";
    }
    if multi && sep != "{" {
        write!(w, "}}")?;
    }
    Ok(())
}
