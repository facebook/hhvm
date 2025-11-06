// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9a3643d525a4f2d460092f114710187d>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum FilePath {
    Dot,
    Slash {
        prefix: Box<FilePath>,
        segment: patt_string::PattString,
    },
    #[rust_to_ocaml(name = "Slash_opt")]
    SlashOpt {
        prefix: Box<FilePath>,
        segment: patt_string::PattString,
    },
}

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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum PattFile {
    As {
        lbl: patt_var::PattVar,
        patt: Box<PattFile>,
    },
    Name {
        allow_glob: bool,
        patt_file_path: Option<FilePath>,
        patt_file_name: patt_string::PattString,
        patt_file_extension: patt_string::PattString,
    },
    Wildcard,
    Invalid {
        errs: Vec<validation_err::ValidationErr>,
        patt: Box<PattFile>,
    },
}
