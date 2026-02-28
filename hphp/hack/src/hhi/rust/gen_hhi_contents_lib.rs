// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Write;
use std::path::Path;
use std::path::PathBuf;

pub fn write_hhi_contents_file(
    out_filename: &Path,
    hhi_contents: &[(PathBuf, String)],
) -> std::io::Result<()> {
    let mut out_file = std::fs::File::create(out_filename)?;
    writeln!(out_file, "pub const HHI_CONTENTS: &[(&str, &str)] = &[")?;
    for (path, contents) in hhi_contents {
        writeln!(
            out_file,
            "    (\"{}\", r###\"{}\"###),",
            path.display(),
            contents
        )?;
    }
    writeln!(out_file, "];")?;
    Ok(())
}
