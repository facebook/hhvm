// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use anyhow::Result;

pub fn textual_writer(
    _w: &mut dyn std::io::Write,
    _path: &Path,
    _unit: &ir::Unit<'_>,
) -> Result<()> {
    todo!();
}
