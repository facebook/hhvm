// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use pos::RelativePath;
use std::fmt::Debug;
use std::marker::{Send, Sync};

#[derive(ToOcamlRep, FromOcamlRep)]
pub enum FileType {
    Disk(bstr::BString),
    Ide(bstr::BString),
}

///
pub trait FileProvider: Debug + Send + Sync {
    ///
    fn get(&self, file: RelativePath) -> Option<FileType>;

    ///
    fn get_contents(&self, file: RelativePath) -> bstr::BString;

    ///
    fn provide_file_for_tests(&self, file: RelativePath, contents: bstr::BString);

    ///
    fn provide_file_for_ide(&self, file: RelativePath, contents: bstr::BString);

    ///
    fn provide_file_hint(&self, file: RelativePath, file_type: FileType);

    ///
    fn remove_batch<I: Iterator<Item = RelativePath>>(&self, files: I);

    ///
    fn push_local_changes(&self);

    ///
    fn pop_local_changes(&self);
}
