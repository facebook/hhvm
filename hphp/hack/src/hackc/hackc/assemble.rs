// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::FileOpts;
use anyhow::{anyhow, Result};
use bumpalo::Bump;
use clap::Parser;
use hhbc::hackc_unit::HackCUnit;
use options::Options;
use oxidized::relative_path::{self, RelativePath};
use rayon::prelude::*;
use std::{
    fs::File,
    io::{stdout, Write},
    path::{Path, PathBuf},
    sync::Mutex,
};

#[derive(Parser, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    /// The input hhas file(s) to deserialize back to HackCUnit
    #[clap(flatten)]
    files: FileOpts,
}

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

/// Assembles the hhas within f to a HackCUnit. Currently just returns default
pub fn assemble<'arena>(
    _alloc: &'arena bumpalo::Bump,
    _f: &Path,
    _opts: &Opts,
) -> Result<HackCUnit<'arena>> {
    let _tr: Result<HackCUnit<'_>> = Ok(Default::default());
    todo!()
}

pub fn run(mut opts: Opts) -> Result<()> {
    //Create writer to output/stdout
    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(stdout())),
        Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
    };
    //May have specified multiple files
    let files = opts.files.gather_input_files()?;
    //Process each file
    files
        .into_par_iter()
        .map(|path| process_one_file(&path, &opts, &writer))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

/// Assemble the hhas in a given file to a HackCUnit. Then use bytecode printer
/// to write the hhas representation of that HCU to output
pub fn process_one_file(f: &Path, opts: &Opts, w: &SyncWrite) -> Result<()> {
    let alloc = Bump::default();
    let hcu = assemble(&alloc, f, opts)?;
    let filepath = RelativePath::make(relative_path::Prefix::Dummy, f.to_owned());
    let comp_options: Options = Default::default();
    //note: why not make a native_env based on the filepath
    //and then use its to_options -- why is to_options() private?
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
