// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<f3c4fe583d9ec784fdec0c7fe08f51de>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Env {
    pub ns_uses: s_map::SMap<std::rc::Rc<String>>,
    pub class_uses: s_map::SMap<std::rc::Rc<String>>,
    pub record_def_uses: s_map::SMap<std::rc::Rc<String>>,
    pub fun_uses: s_map::SMap<std::rc::Rc<String>>,
    pub const_uses: s_map::SMap<std::rc::Rc<String>>,
    pub name: Option<std::rc::Rc<String>>,
    pub auto_ns_map: Vec<(std::rc::Rc<String>, std::rc::Rc<String>)>,
    pub is_codegen: bool,
    pub disable_xhp_element_mangling: bool,
}
