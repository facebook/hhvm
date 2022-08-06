// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;

use anyhow::Result;
use bstr::BString;
use pos::RelativePath;

mod provider;
pub use provider::DiskProvider;

/// The interface through which the typechecker can access the contents of the
/// repository and HHI files.
///
/// The implementation may load file contents from the filesystem, or from a
/// cache in front of the filesystem, or from in-memory IDE buffers.
pub trait FileProvider: Debug + Send + Sync {
    /// Return the contents of the given file. May return `Ok("")` if the file
    /// can't be found in the underlying store, or may return a `std::io::Error`
    /// with `ErrorKind::NotFound` (wrapped in `anyhow::Error`), depending on
    /// the use case.
    fn get(&self, file: RelativePath) -> Result<BString>;
}
