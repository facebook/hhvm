// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// In Ocaml, providers return Typing_defs.fun_elt, and Shallow_decl_defs.fun_elt is an alias to it, so
// this signature is accurate. In case of class declarations, Typing_defs.class_type is a separate type, which
// folds many shallow class declarations into one. We don't have this type / logic in Rust yet.
pub type FunDecl = oxidized::shallow_decl_defs::FunElt;
pub type ClassDecl = oxidized::shallow_decl_defs::ShallowClass;

pub trait DeclProvider {
    fn get_fun(&self, s: &str) -> Option<&FunDecl>;
    fn get_class(&self, s: &str) -> Option<&ClassDecl>;
    fn get_ancestor<'a>(&self, t: &'a ClassDecl, s: &str) -> Option<&'a oxidized::typing_defs::Ty>;
}
