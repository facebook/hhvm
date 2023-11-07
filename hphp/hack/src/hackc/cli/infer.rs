// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs;
use std::io;
use std::path::Path;
use std::path::PathBuf;
use std::sync::Arc;

use anyhow::Result;
use clap::Args;
use compile::Profile;
use decl_provider::SelfProvider;
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

    /// (DEPRECATED) Attempt to keep going instead of panicking on unimplemented code. This is the
    /// default now and can be reversed via `--fail-fast` flag.
    #[clap(long)]
    keep_going: bool,

    /// Panic on unimplemented code instead of generating stub instructions.
    #[clap(long)]
    fail_fast: bool,

    /// Skip files that contain unimplemented code that can't be fully translated.
    #[clap(long)]
    skip_unimplemented: bool,

    /// Keep memoization attributes. The default is to remove them prior to the codegen as it
    /// simplifies the analysis.
    #[clap(long)]
    keep_memo: bool,

    /// Control the output of coeffects in textual files. This option can be set to hide the coeffects
    /// information and ease human reading of textual code.
    #[clap(long)]
    hide_static_coeffects: bool,
}

pub fn run(mut opts: Opts) -> Result<()> {
    textual::KEEP_GOING.store(!opts.fail_fast, std::sync::atomic::Ordering::Release);
    // Always unwrap concurrent blocks for infer use-cases. We can make it configurable later on if
    // needed.
    opts.single_file_opts.unwrap_concurrent = true;

    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(io::stdout())),
        Some(output_file) => Mutex::new(Box::new(fs::File::create(output_file)?)),
    };

    let files = opts.files.gather_input_files()?;

    writeln!(writer.lock(), "// TEXTUAL UNIT COUNT {}", files.len())?;

    if !opts.fail_fast || opts.skip_unimplemented {
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
        build_ir(&pre_alloc, path, &content, opts).and_then(|unit| {
            let mut output = Vec::new();
            textual::textual_writer(
                &mut output,
                path,
                unit,
                opts.no_builtins,
                opts.hide_static_coeffects,
            )?;
            Ok(output)
        })
    };

    if !opts.fail_fast || opts.skip_unimplemented {
        with_catch_panics(action)
    } else {
        action()
    }
}

fn build_ir<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    path: &'a Path,
    content: &[u8],
    opts: &'a Opts,
) -> Result<ir::Unit<'arena>> {
    let filepath = RelativePath::make(Prefix::Dummy, path.to_path_buf());
    let source_text = SourceText::make(Arc::new(filepath.clone()), content);
    let env = crate::compile::native_env(filepath, &opts.single_file_opts)?;
    let mut profile = Profile::default();
    let decl_arena = bumpalo::Bump::new();
    let decl_provider = SelfProvider::wrap_existing_provider(
        None,
        env.to_decl_parser_options(),
        source_text.clone(),
        &decl_arena,
    );
    let unit = compile::unit_from_text_with_opts(
        alloc,
        source_text,
        &env,
        decl_provider,
        &mut profile,
        &elab::CodegenOpts {
            textual_remove_memoize: !opts.keep_memo,
        },
    )?;
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
