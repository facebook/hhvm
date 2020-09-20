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
    fn parse_decls_ffi(filename: RelativePath, text: Vec<u8>) -> UnsafeOcamlPtr {
        let arena = Bump::new();
        let decls = parse_decls(filename, &text, &arena);
        // SAFETY: We immediately hand this pointer to the OCaml runtime.
        // The use of UnsafeOcamlPtr is necessary here because we cannot return
        // `decls`, since it borrows `arena`, which is destroyed at the end of
        // this function scope. Instead, we convert the decls to OCaml
        // ourselves, and return the pointer (the converted OCaml value does not
        // borrow the arena).
        unsafe { UnsafeOcamlPtr::new(to_ocaml(&decls)) }
    }
}
