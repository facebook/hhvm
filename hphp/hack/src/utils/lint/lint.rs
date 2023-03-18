// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh
use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// These severity levels are based on those provided by Arcanist. "Advice"
/// means notify the user of the lint without requiring confirmation if the lint
/// is benign; "Warning" will raise a confirmation prompt if the lint applies to
/// a line that was changed in the given diff; and "Error" will always raise a
/// confirmation prompt, regardless of where the lint occurs in the file.
#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(u8)]
pub enum Severity {
    #[rust_to_ocaml(name = "Lint_error")]
    LintError,
    #[rust_to_ocaml(name = "Lint_warning")]
    LintWarning,
    #[rust_to_ocaml(name = "Lint_advice")]
    LintAdvice,
}
impl TrivialDrop for Severity {}
arena_deserializer::impl_deserialize_in_arena!(Severity);

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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C)]
pub struct LintsCore<Pos> {
    pub code: isize,
    pub severity: Severity,
    #[rust_to_ocaml(attr = "opaque")]
    pub pos: Pos,
    pub message: String,
    /// Normally, lint warnings and lint advice only get shown by arcanist if the
    /// lines they are raised on overlap with lines changed in a diff. This
    /// flag bypasses that behavior
    pub bypass_changed_lines: bool,
    pub autofix: Option<(String, pos::Pos)>,
    pub check_status: Option<tast::CheckStatus>,
}
