// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use decl_rust::direct_decl_parser::parse_decls;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep_ocamlpool::{ocaml_ffi, to_ocaml};
use oxidized::relative_path::RelativePath;

ocaml_ffi! {
    fn parse_decls_ffi(filename: RelativePath, text: Vec<u8>) -> Result<UnsafeOcamlPtr, String> {
        let arena = Bump::new();
        parse_decls(filename, &text, &arena)
            .map(|decls| unsafe { UnsafeOcamlPtr::new(to_ocaml(&decls)) })
    }
}
