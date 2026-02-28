// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::diagnostics::Diagnostic;
use oxidized::warnings_saved_state::ErrorHash;

ocaml_ffi! {
    fn hash_diagnostic_for_saved_state(diagnostic: Diagnostic) -> ErrorHash {
        diagnostic.hash_for_saved_state()
    }
}
