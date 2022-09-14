// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs;
use std::io;
//use std::io::Write;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Result;
use clap::Parser;
use compile::Profile;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;

use crate::compile::SingleFileOpts;
use crate::FileOpts;

#[derive(Parser, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    #[clap(flatten)]
    files: FileOpts,

    #[clap(flatten)]
    single_file_opts: SingleFileOpts,

    /// Skip emitting 'standard' builtins.
    #[clap(long)]
    no_builtins: bool,
}

pub fn run(opts: Opts) -> Result<()> {
    let files = opts.files.gather_input_files()?;

    let mut stdout = io::stdout();

    for path in files {
        let pre_alloc = bumpalo::Bump::default();
        let unit = compile_php_file(&pre_alloc, &path, &opts.single_file_opts)?;

        textual::textual_writer(&mut stdout, &path, &unit)?;
    }

    // if !opts.no_builtins {
    //     write!(&mut stdout, "{}", textual::BUILTIN_DECLS)?;
    // }

    Ok(())
}

fn compile_php_file<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    path: &'a Path,
    //content: Vec<u8>,
    single_file_opts: &'a SingleFileOpts,
    //profile: &mut Profile,
) -> Result<ir::Unit<'arena>> {
    let content = fs::read(path)?;

    let filepath = RelativePath::make(Prefix::Dummy, path.to_path_buf());
    let source_text = SourceText::make(RcOc::new(filepath.clone()), &content);
    let env = crate::compile::native_env(filepath, single_file_opts);
    let mut profile = Profile::default();
    let unit = compile::unit_from_text(alloc, source_text, &env, None, &mut profile)?;

    let ir = bc_to_ir::bc_to_ir(&unit, path);

    Ok(ir)
}
