// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a8eb089002a3e0f194e5bc63d206977c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C)]
pub struct DeclParserOptions {
    pub auto_namespace_map: Vec<(String, String)>,
    pub disable_xhp_element_mangling: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub enable_xhp_class_modifier: bool,
    pub everything_sdt: bool,
    pub php5_compat_mode: bool,
    pub hhvm_compat_mode: bool,
    pub keep_user_attributes: bool,
    pub include_assignment_values: bool,
    /// Stack size to for the parallel workers
    pub stack_size: isize,
    pub deregister_php_stdlib: bool,
    pub package_info: package_info::PackageInfo,
    pub package_v2: bool,
    pub package_v2_support_multifile_tests: bool,
    pub enable_class_pointer_hint: bool,
    pub disallow_non_annotated_memoize: bool,
    pub treat_non_annotated_memoize_as_kbic: bool,
}
