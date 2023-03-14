// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<19855cebf995ece0cef1283455852094>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

pub use crate::error_codes::Parsing as Error_code;
#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum ParsingError {
    #[rust_to_ocaml(name = "Fixme_format")]
    FixmeFormat(pos::Pos),
    #[rust_to_ocaml(name = "Hh_ignore_comment")]
    HhIgnoreComment(pos::Pos),
    #[rust_to_ocaml(name = "Parsing_error")]
    ParsingError {
        pos: pos::Pos,
        msg: String,
        quickfixes: Vec<quickfix::Quickfix<pos::Pos>>,
    },
    #[rust_to_ocaml(name = "Xhp_parsing_error")]
    XhpParsingError { pos: pos::Pos, msg: String },
    #[rust_to_ocaml(name = "Package_config_error")]
    PackageConfigError { pos: pos::Pos, msg: String },
}
