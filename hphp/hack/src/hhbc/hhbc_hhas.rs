// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_program_rust::HhasProgram;
use oxidized::relative_path::RelativePath;

pub fn to_segments(
    _path: Option<&RelativePath>,
    _dump_symbol_refs: bool,
    _program: HhasProgram,
) -> Vec<String> {
    //TODO(hrust):
    vec![]
}
