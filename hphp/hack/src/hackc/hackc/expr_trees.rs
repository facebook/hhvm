// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::Opts;
use anyhow::Result;
use oxidized::relative_path::{Prefix, RelativePath};

pub(crate) fn dump_expr_trees(mut opts: Opts) -> Result<()> {
    for path in opts.files.gather_input_files()? {
        compile::dump_expr_tree::desugar_and_print(
            RelativePath::make(Prefix::Dummy, path),
            opts.env_flags(),
        );
    }
    Ok(())
}
