// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_guess_repo_root(start: PathBuf) -> Option<PathBuf> {
        repo_root::guess_root_from(&start).ok()
    }
}
