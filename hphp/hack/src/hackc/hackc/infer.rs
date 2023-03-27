// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs;
use std::io;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Result;
use clap::Args;
use compile::Profile;
use decl_provider::SelfProvider;
use ocamlrep::rc::RcOc;
use parking_lot::Mutex;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use relative_path::Prefix;
use relative_path::RelativePath;

use crate::compile::SingleFileOpts;
use crate::util::SyncWrite;
use crate::FileOpts;

#[derive(Args, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    #[command(flatten)]
    files: FileOpts,

    #[command(flatten)]
    single_file_opts: SingleFileOpts,

    /// Skip emitting 'standard' builtins.
    #[clap(long)]
    no_builtins: bool,

    /// Attempt to keep going instead of panicking on unimplemented code.
    #[clap(long)]
    keep_going: bool,

    /// Skip files that can't be fully translated. Unlike `--keep-going` it won't emit dummy
    /// instructions for unimplemented code but will rather completely skip the file.
    #[clap(long)]
    skip_unimplemented: bool,
}

pub fn run(opts: Opts) -> Result<()> {
    textual::KEEP_GOING.store(opts.keep_going, std::sync::atomic::Ordering::Release);

    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(io::stdout())),
        Some(output_file) => Mutex::new(Box::new(fs::File::create(output_file)?)),
    };

    let files = opts.files.gather_input_files()?;

    writeln!(writer.lock(), "// TEXTUAL UNIT COUNT {}", files.len())?;

    if opts.keep_going || opts.skip_unimplemented {
        files
            .into_par_iter()
            .for_each(|path| match process_single_file(&path, &opts, &writer) {
                Ok(_) => (),
                Err(err) => {
                    eprintln!("Failed to compile {}: {}", path.display(), err)
                }
            })
    } else {
        files
            .into_par_iter()
            .try_for_each(|path| process_single_file(&path, &opts, &writer))?
    }

    Ok(())
}

fn process_single_file(path: &Path, opts: &Opts, writer: &SyncWrite) -> Result<()> {
    match convert_single_file(path, opts) {
        Ok(textual) => writer.lock().write_all(&textual).map_err(|e| e.into()),
        Err(err) => Err(err),
    }
}

fn convert_single_file(path: &Path, opts: &Opts) -> Result<Vec<u8>> {
    let content = fs::read(path)?;

    let action = || {
        let pre_alloc = bumpalo::Bump::default();
        build_ir(&pre_alloc, path, &content, &opts.single_file_opts).and_then(|unit| {
            let mut output = Vec::new();
            textual::textual_writer(&mut output, path, unit, opts.no_builtins)?;
            Ok(output)
        })
    };

    if opts.keep_going || opts.skip_unimplemented {
        with_catch_panics(action)
    } else {
        action()
    }
}

fn build_ir<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    path: &'a Path,
    content: &[u8],
    single_file_opts: &'a SingleFileOpts,
) -> Result<ir::Unit<'arena>> {
    let filepath = RelativePath::make(Prefix::Dummy, path.to_path_buf());
    let source_text = SourceText::make(RcOc::new(filepath.clone()), content);
    let env = crate::compile::native_env(filepath, single_file_opts);
    let mut profile = Profile::default();
    let decl_arena = bumpalo::Bump::new();
    let decl_provider = SelfProvider::wrap_existing_provider(
        None,
        env.to_decl_parser_options(),
        source_text.clone(),
        &decl_arena,
    );
    let unit = compile::unit_from_text(alloc, source_text, &env, decl_provider, &mut profile)?;
    let ir = bc_to_ir::bc_to_ir(&unit, path);

    Ok(ir)
}

fn with_catch_panics<R>(action: impl FnOnce() -> Result<R> + std::panic::UnwindSafe) -> Result<R> {
    match std::panic::catch_unwind(action) {
        Ok(result) => result,
        Err(e) => {
            let msg = panic_message::panic_message(&e).to_string();
            let err = anyhow::anyhow!(msg);
            Err(err)
        }
    }
}
