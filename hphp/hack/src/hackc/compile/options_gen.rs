// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::Deserialize;
use serde::Serialize;

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
#[repr(C)]
pub struct HhbcFlags {
    pub ltr_assign: bool,
    pub uvs: bool,
    pub log_extern_compiler_perf: bool,
    pub enable_intrinsics_extension: bool,
    pub emit_cls_meth_pointers: bool,
    pub fold_lazy_class_keys: bool,
    pub optimize_reified_param_checks: bool,
    pub stress_shallow_decl_deps: bool,
    pub stress_folded_decl_deps: bool,
    pub enable_native_enum_class_labels: bool,
    pub optimize_param_lifetimes: bool,
    pub optimize_local_lifetimes: bool,
    pub optimize_local_iterators: bool,
    pub optimize_is_type_checks: bool,
}

unsafe impl cxx::ExternType for HhbcFlags {
    type Id = cxx::type_id!("HPHP::hackc::HhbcFlags");
    type Kind = cxx::kind::Trivial;
}

impl Default for HhbcFlags {
    fn default() -> Self {
        Self {
            ltr_assign: false,
            uvs: false,
            log_extern_compiler_perf: false,
            enable_intrinsics_extension: false,
            emit_cls_meth_pointers: true,
            fold_lazy_class_keys: true,
            optimize_reified_param_checks: false,
            stress_shallow_decl_deps: false,
            stress_folded_decl_deps: false,
            enable_native_enum_class_labels: false,
            optimize_param_lifetimes: true,
            optimize_local_lifetimes: true,
            optimize_local_iterators: true,
            optimize_is_type_checks: true,
        }
    }
}
