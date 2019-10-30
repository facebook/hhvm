// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_rust::direct_decl_parser::parse_decls;
use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::direct_decl_parser::Decls;
use oxidized::relative_path::RelativePath;

ocaml_ffi! {
    fn parse_decls_ffi(filename: RelativePath, text: String) -> Result<Decls, String> {
        parse_decls(filename, &text)
    }
}
