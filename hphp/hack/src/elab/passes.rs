// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod elab_as_expr;
pub mod elab_block;
pub mod elab_class_id;
pub mod elab_class_vars;
pub mod elab_const_expr;
pub mod elab_defs;
pub mod elab_dynamic_class_name;
pub mod elab_enum_class;
pub mod elab_expr_call_call_user_func;
pub mod elab_expr_call_hh_invariant;
pub mod elab_expr_call_hh_meth_caller;
pub mod elab_expr_collection;
pub mod elab_expr_import;
pub mod elab_expr_lvar;
pub mod elab_expr_tuple;
pub mod elab_func_body;
pub mod elab_hint_haccess;
pub mod elab_hint_happly;
pub mod elab_hint_hsoft;
pub mod elab_hint_retonly;
pub mod elab_hint_this;
pub mod elab_hint_wildcard;
pub mod elab_hkt;
pub mod elab_shape_field_name;
pub mod elab_user_attributes;
pub mod guard_invalid;
pub mod validate_class_consistent_construct;
pub mod validate_class_req;
pub mod validate_expr_call_echo;
pub mod validate_expr_cast;
