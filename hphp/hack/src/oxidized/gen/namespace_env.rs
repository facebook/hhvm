// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ce04f57e718456ef9f03019c4d0f6bac>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use crate::parser_options;
use crate::s_map;

#[derive(Clone, Debug)]
pub struct Env {
    pub ns_uses: s_map::SMap<String>,
    pub class_uses: s_map::SMap<String>,
    pub fun_uses: s_map::SMap<String>,
    pub const_uses: s_map::SMap<String>,
    pub name: Option<String>,
    pub popt: parser_options::ParserOptions,
}
