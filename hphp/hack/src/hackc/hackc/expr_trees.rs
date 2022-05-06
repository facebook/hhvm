// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::Opts;
use anyhow::Result;
use oxidized::relative_path::{Prefix, RelativePath};

pub(crate) fn dump_expr_trees(mut opts: Opts) -> Result<()> {
    for path in opts.files.gather_input_files()? {
        let env: compile::Env<&str> = compile::Env {
            filepath: RelativePath::make(Prefix::Dummy, path),
            flags: opts.env_flags(),
            config_jsons: Default::default(),
            config_list: Default::default(),
        };
        compile::dump_expr_tree::desugar_and_print(&env);
    }
    Ok(())
}
