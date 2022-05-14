// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::FileOpts;
use anyhow::Result;
use clap::Parser;
use compile::{EnvFlags, HHBCFlags, NativeEnv, ParserFlags, Profile};
use decl_provider::DeclProvider;
use multifile_rust as multifile;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::{self, RelativePath};
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use std::{
    fs::{self, File},
    io::{stdout, Write},
    path::{Path, PathBuf},
    sync::Mutex,
};

#[derive(Parser, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    #[clap(flatten)]
    files: FileOpts,

    #[clap(flatten)]
    single_file_opts: SingleFileOpts,
}

#[derive(Parser, Clone, Debug)]
pub(crate) struct SingleFileOpts {
    /// Disable toplevel definition elaboration
    #[clap(long)]
    pub(crate) disable_toplevel_elaboration: bool,

    /// The level of verbosity (can be set multiple times)
    #[clap(long = "verbose", parse(from_occurrences))]
    pub(crate) verbosity: isize,
}

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

pub fn run(mut opts: Opts) -> Result<()> {
    if opts.single_file_opts.verbosity > 1 {
        eprintln!("hackc compile options/flags: {:#?}", opts);
    }

    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(stdout())),
        Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
    };

    let files = opts.files.gather_input_files()?;

    // Collect a Vec first so we process all files - not just up to the first failure.
    files
        .into_par_iter()
        .map(|path| process_one_file(&path, &opts, &writer))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

/// Process a single physical file by breaking it into multiple logical
/// files (using multifile) and then processing each of those with
/// process_single_file().
///
/// If an error occurs then continue to process as much as possible,
/// returning the first error that occured.
fn process_one_file(f: &Path, opts: &Opts, w: &SyncWrite) -> Result<()> {
    let content = fs::read(f)?;
    let files = multifile::to_files(f, content)?;

    // Collect a Vec so we process all files - not just up to the first
    // failure.
    let results: Vec<Result<()>> = files
        .into_iter()
        .map(|(f, content)| {
            let f = f.as_ref();
            match process_single_file(
                &opts.single_file_opts,
                f.into(),
                content,
                &mut Profile::default(),
            ) {
                Err(e) => {
                    writeln!(w.lock().unwrap(), "Error in file {}: {}", f.display(), e)?;
                    Err(e)
                }
                Ok(output) => {
                    w.lock().unwrap().write_all(&output)?;
                    Ok(())
                }
            }
        })
        .collect();

    results.into_iter().collect()
}

pub(crate) fn process_single_file(
    opts: &SingleFileOpts,
    filepath: PathBuf,
    content: Vec<u8>,
    profile: &mut Profile,
) -> Result<Vec<u8>> {
    if opts.verbosity > 1 {
        eprintln!("processing file: {}", filepath.display());
    }
    let filepath = RelativePath::make(relative_path::Prefix::Dummy, filepath);
    let source_text = SourceText::make(RcOc::new(filepath.clone()), &content);
    let mut flags = EnvFlags::empty();
    flags.set(
        EnvFlags::DISABLE_TOPLEVEL_ELABORATION,
        opts.disable_toplevel_elaboration,
    );
    let hhbc_flags = HHBCFlags::EMIT_CLS_METH_POINTERS
        | HHBCFlags::EMIT_METH_CALLER_FUNC_POINTERS
        | HHBCFlags::FOLD_LAZY_CLASS_KEYS
        | HHBCFlags::LOG_EXTERN_COMPILER_PERF;
    let parser_flags = ParserFlags::ENABLE_ENUM_CLASSES;
    let native_env = NativeEnv {
        filepath,
        hhbc_flags,
        parser_flags,
        flags,
        ..Default::default()
    };
    let alloc = bumpalo::Bump::new();
    let mut output = Vec::new();
    compile::from_text(&alloc, &mut output, source_text, &native_env, None, profile)?;
    if opts.verbosity >= 1 {
        eprintln!("{}: {:#?}", native_env.filepath.path().display(), profile);
    }
    Ok(output)
}

pub(crate) fn compile_from_text(mut hackc_opts: crate::Opts) -> Result<()> {
    let files = hackc_opts.files.gather_input_files()?;
    for path in files {
        let source_text = fs::read(&path)?;
        let native_env = hackc_opts.native_env(path)?;
        let hhas = compile_impl(native_env, source_text, None)?;
        std::io::stdout().write_all(&hhas)?;
        println!(); // print an extra newline to match hh_single_compile_cpp
    }
    Ok(())
}

fn compile_impl<'decl>(
    native_env: NativeEnv<'_>,
    source_text: Vec<u8>,
    decl_provider: Option<&'decl dyn DeclProvider<'decl>>,
) -> Result<Vec<u8>> {
    let text = SourceText::make(
        ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
        &source_text,
    );
    let mut hhas = Vec::new();
    let arena = bumpalo::Bump::new();
    compile::from_text(
        &arena,
        &mut hhas,
        text,
        &native_env,
        decl_provider,
        &mut Default::default(), // profile
    )?;
    Ok(hhas)
}
