// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs::File;
use std::io::stdout;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;
use std::sync::Mutex;

use ::assemble as _;
use anyhow::anyhow;
use anyhow::Result;
use bumpalo::Bump;
use clap::Parser;
use options::Options;
use rayon::prelude::*;
use relative_path::RelativePath;

use crate::FileOpts;

#[derive(Parser, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    /// The input hhas file(s) to assemble back to hhbc::Unit
    #[clap(flatten)]
    files: FileOpts,
}

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

pub fn run(opts: Opts) -> Result<()> {
    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(stdout())),
        Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
    };
    let files = opts.files.gather_input_files()?;
    files
        .into_par_iter()
        .map(|path| process_one_file(&path, &writer))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

/// Assemble the hhas in a given file to a hhbc::Unit. Then use bytecode printer
/// to write the hhas representation of that HCU to output.
pub fn process_one_file(f: &Path, w: &SyncWrite) -> Result<()> {
    let alloc = Bump::default();
    let (hcu, fp) = assemble::assemble(&alloc, f)?;
    let filepath = RelativePath::make(relative_path::Prefix::Dummy, fp);
    let comp_options: Options = Default::default();
    let ctxt = bytecode_printer::Context::new(&comp_options, Some(&filepath), false);
    let mut output = Vec::new();
    match bytecode_printer::print_unit(&ctxt, &mut output, &hcu) {
        Err(e) => {
            eprintln!("Error bytecode_printing file {}: {}", f.display(), e);
            Err(anyhow!("bytecode_printer problem"))
        }
        Ok(_) => {
            w.lock().unwrap().write_all(&output)?;
            Ok(())
        }
    }
}
