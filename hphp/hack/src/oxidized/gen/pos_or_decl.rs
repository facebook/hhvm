// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<1d19381463e2809e48adf54cbe3ff3fa>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

pub use crate::pos::map;
#[allow(unused_imports)]
use crate::*;

/// There are two kinds of positions: AST positions provided by the parser,
/// which are fully qualified with filename, line and column ranges, and
/// decl positions, which may be compressed and need resolving before being
/// used or printed.
///
/// AST positions are represented by Pos.t. Decl positions don't have their own
/// type yet but may have in the future.
/// This type is for either of these positions.
/// It's used in the decl heap and in places that can take any kind of positions,
/// e.g. error secondary positions.
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
pub type PosOrDecl = pos::Pos;

/// The decl and file of a position.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C)]
pub struct Ctx {
    pub decl: Option<decl_reference::DeclReference>,
    pub file: relative_path::RelativePath,
}
