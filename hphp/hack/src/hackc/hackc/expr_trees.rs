// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use anyhow::Result;
use clap::Parser;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;

use crate::FileOpts;

#[derive(Parser, Debug, Default)]
pub(crate) struct Opts {
    #[clap(flatten)]
    pub files: FileOpts,
}

pub(crate) fn desugar_expr_trees(hackc_opts: &crate::Opts, mut opts: Opts) -> Result<()> {
    for path in opts.files.gather_input_files()? {
        compile::dump_expr_tree::desugar_and_print(
            RelativePath::make(Prefix::Dummy, path),
            hackc_opts.env_flags(),
        );
    }
    Ok(())
}
