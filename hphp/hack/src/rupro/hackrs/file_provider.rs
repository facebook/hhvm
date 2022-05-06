// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use pos::RelativePath;
use std::fmt::Debug;
use std::marker::{Send, Sync};

mod provider;
pub use provider::PlainFileProvider;

#[derive(Clone, Debug, ToOcamlRep, FromOcamlRep)]
pub enum FileType {
    Disk(bstr::BString),
    Ide(bstr::BString),
}

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    IoError(#[from] std::io::Error),
}

/// Acts as a sort of caching facade which is filled on-demand as contents are
/// needed. The "cache" is (or rather, might be) filled by loading from the file
/// system if the file isn't open in the IDE (otherwise use the IDE contents).
/// That is, any IDE version of a file takes precedence over the file system's.
// note(sf, 2022-04-28): c.f. hphp/hack/src/providers/file_provider.ml
pub trait FileProvider: Debug + Send + Sync {
    /// Lookup a path.
    fn get(&self, file: RelativePath) -> Option<FileType>;

    /// If a cached entry for the provided path exists then return its contents.
    /// If not, try to read the contents from disk. Reading from disk might
    /// produce an `Error::IoError(_)` result.
    fn get_contents(&self, file: RelativePath) -> Result<bstr::BString>;

    /// Register `file` as a disk file containing `contents`.
    fn provide_file_for_tests(&self, file: RelativePath, contents: bstr::BString);

    /// Register `file` as an IDE file containing `contents`.
    fn provide_file_for_ide(&self, file: RelativePath, contents: bstr::BString);

    /// If `file_type` is an IDE file, register it. If it is a disk file, do
    /// nothing.
    fn provide_file_hint(&self, file: RelativePath, file_type: FileType);

    /// Associate each path in `files` with `None`.
    fn remove_batch(&self, files: &[RelativePath]);

    /// Save the current set of changes on the stack then clear it.
    fn push_local_changes(&self);

    /// Clear the current set of changes.
    fn pop_local_changes(&self);
}
