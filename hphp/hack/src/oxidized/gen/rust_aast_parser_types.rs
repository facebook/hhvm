// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<08a1f9d7d028830bf37e8178ad3595c9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

use crate::aast;
use crate::parser_options;
use crate::pos;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Env {
    pub is_hh_file: bool,
    pub codegen: bool,
    pub php5_compat_mode: bool,
    pub elaborate_namespaces: bool,
    pub include_line_comments: bool,
    pub keep_errors: bool,
    pub quick_mode: bool,
    pub show_all_errors: bool,
    pub lower_coroutines: bool,
    pub fail_open: bool,
    pub parser_options: parser_options::ParserOptions,
    pub hacksperimental: bool,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Result {
    pub aast: aast::Program<pos::Pos, (), (), ()>,
}
