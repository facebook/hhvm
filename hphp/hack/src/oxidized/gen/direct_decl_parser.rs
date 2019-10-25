// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6fb98cfc139ecd0d8101ab2c504a4994>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

use crate::s_map;
use crate::shallow_decl_defs;
use crate::typing_defs;

#[derive(Clone, Debug, OcamlRep)]
pub struct Decls {
    pub classes: s_map::SMap<shallow_decl_defs::ShallowClass>,
    pub funs: s_map::SMap<typing_defs::DeclFunType>,
    pub typedefs: s_map::SMap<typing_defs::TypedefType>,
    pub consts: s_map::SMap<typing_defs::DeclTy>,
}
