// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// In Ocaml, providers return Typing_defs.fun_elt, and Shallow_decl_defs.fun_elt is an alias to it, so
// this signature is accurate. In case of class declarations, Typing_defs.class_type is a separate type, which
// folds many shallow class declarations into one. We don't have this type / logic in Rust yet.
pub type FunDecl<'a> = oxidized_by_ref::shallow_decl_defs::FunElt<'a>;
pub type ClassDecl<'a> = oxidized_by_ref::shallow_decl_defs::ShallowClass<'a>;

pub trait DeclProvider {
    fn get_fun(&self, s: &str) -> Option<&FunDecl<'_>>;
    fn get_shallow_class(&self, s: &str) -> Option<&ClassDecl<'_>>;
}
