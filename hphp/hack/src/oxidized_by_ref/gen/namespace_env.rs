// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<5539deb9881e395be073a7821d306e27>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct Env<'a> {
    pub ns_uses: s_map::SMap<'a, &'a str>,
    pub class_uses: s_map::SMap<'a, &'a str>,
    pub record_def_uses: s_map::SMap<'a, &'a str>,
    pub fun_uses: s_map::SMap<'a, &'a str>,
    pub const_uses: s_map::SMap<'a, &'a str>,
    pub name: Option<&'a str>,
    pub auto_ns_map: &'a [(&'a str, &'a str)],
    pub is_codegen: bool,
    pub disable_xhp_element_mangling: bool,
}
