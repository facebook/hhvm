// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen
use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// These severity levels are based on those provided by Arcanist:
/// - Error: when it fires, it will require confirmation
/// - Warning: when it fires, it shows up visibly before landing
/// - Advice: Similar to warning, but implies lesser severity
/// - Disabled: Hidden from most UI, but it is useful for telemetry
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
    #[rust_to_ocaml(name = "Lint_disabled")]
    LintDisabled,
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
    pub check_status: Option<aast_defs::CheckStatus>,
}
