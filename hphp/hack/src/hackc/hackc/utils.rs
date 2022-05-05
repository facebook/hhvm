// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ::anyhow::{Context, Result};
use std::{
    fs::File,
    io::{self, BufRead, BufReader, Read},
    iter::Map,
    path::{Path, PathBuf},
    vec::IntoIter,
};

pub fn read_file_list(input_path: Option<&PathBuf>) -> Result<impl Iterator<Item = PathBuf>> {
    fn read_lines(r: impl Read) -> Result<Map<IntoIter<String>, fn(String) -> PathBuf>> {
        Ok(BufReader::new(r)
            .lines()
            .collect::<std::io::Result<Vec<_>>>()
            .context("could not read line from input file list")?
            .into_iter()
            .map(|l| PathBuf::from(l.trim())))
    }
    match input_path.as_ref() {
        None => read_lines(io::stdin()),
        Some(path) => read_lines(
            File::open(path)
                .with_context(|| format!("Could not open input file: {}", path.display()))?,
        ),
    }
}

pub fn read_file(filepath: &Path) -> Result<Vec<u8>> {
    let mut text: Vec<u8> = Vec::new();
    File::open(filepath)
        .with_context(|| format!("cannot open input file: {}", filepath.display()))?
        .read_to_end(&mut text)?;
    Ok(text)
}
