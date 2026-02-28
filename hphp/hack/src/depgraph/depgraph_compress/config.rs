// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

use balanced_partition::BalanceConfig;

pub enum OptimizeConfig {
    None,
    Bisect(BalanceConfig),
    Copy(PathBuf),
}

impl Default for OptimizeConfig {
    fn default() -> Self {
        OptimizeConfig::Bisect(BalanceConfig::default())
    }
}

pub enum WriteConfig {
    /// Simple is experimental, and does not produce a usable output file.
    /// The intent is to produce something without bespoke compression
    /// (delta coding, varint, move to front) so we can try external compression
    /// programs on it.
    ///
    /// This will produce a file with a different magic number so you don't
    /// accidentally try to use it.
    Simple,

    /// Normal write mode, uses some bespoke compression then runs it through zstd.
    Zstd { compression_level: i32 },
}

impl Default for WriteConfig {
    fn default() -> Self {
        WriteConfig::Zstd {
            compression_level: 12,
        }
    }
}
