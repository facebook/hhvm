// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(box_patterns)]
#![feature(extract_if)]

#[macro_use]
extern crate rust_to_ocaml_attr;

pub use file_info;
pub use file_info::prim_defs;
pub use naming_types;
pub use pos::file_pos;
pub use pos::file_pos_large;
pub use pos::file_pos_small;
pub use pos::pos_span_raw;
pub use pos::pos_span_tiny;
pub use rc_pos as pos;

mod manual;

pub use manual::aast;
pub use manual::aast_defs_impl;
pub use manual::aast_impl;
pub use manual::ast_defs_impl;
pub use manual::custom_error_config_impl;
pub use manual::direct_decl_parser;
pub use manual::global_options_impl;
pub use manual::i_map;
pub use manual::i_set;
pub use manual::ident;
pub use manual::internal_type_set;
pub use manual::lazy;
pub use manual::local_id;
pub use manual::method_flags;
pub use manual::namespace_env_impl;
pub use manual::package_info_impl;
pub use manual::prop_flags;
pub use manual::s_map;
pub use manual::s_set;
pub use manual::saved_state_rollouts_impl;
pub use manual::scoured_comments_impl;
pub use manual::shallow_decl_defs_impl;
pub use manual::shape_map;
pub use manual::symbol_name;
pub use manual::t_shape_map;
pub use manual::tany_sentinel;
pub use manual::tast_collector_impl;
pub use manual::tast_hashes_impl;
pub use manual::typing_defs_flags;
pub use manual::typing_env;
pub use manual::typing_logic;
pub use manual::typing_reason_impl;
pub use manual::typing_set;

mod stubs;

pub use stubs::opaque_digest;

mod impl_gen;
pub use impl_gen::*;

pub mod aast_visitor;

mod asts;
pub use asts::ast;
pub use asts::nast;

mod gen;

pub use gen::aast_defs;
pub use gen::ast_defs;
pub use gen::custom_error;
pub use gen::custom_error_config;
pub use gen::decl_defs;
pub use gen::decl_parser_options;
pub use gen::decl_reference;
pub use gen::error_codes;
pub use gen::error_message;
pub use gen::errors;
pub use gen::full_fidelity_parser_env;
pub use gen::global_options;
pub use gen::map_reduce_ffi;
pub use gen::message;
pub use gen::name_context;
pub use gen::namespace_env;
pub use gen::naming_error;
pub use gen::naming_phase_error;
pub use gen::nast_check_error;
pub use gen::package;
pub use gen::package_info;
pub use gen::parser_options;
pub use gen::parsing_error;
pub use gen::patt_binding_ty;
pub use gen::patt_error;
pub use gen::patt_locl_ty;
pub use gen::patt_name;
pub use gen::patt_string;
pub use gen::patt_var;
pub use gen::pos_or_decl;
pub use gen::quickfix;
pub use gen::reason_collector;
pub use gen::saved_state_rollouts;
pub use gen::scoured_comments;
pub use gen::search_types;
pub use gen::shallow_decl_defs;
pub use gen::tast;
pub use gen::tast_collector;
pub use gen::tast_hashes;
pub use gen::tast_with_dynamic;
pub use gen::type_counter;
pub use gen::type_parameter_env;
pub use gen::typechecker_options;
pub use gen::typing_defs;
pub use gen::typing_defs_core;
pub use gen::typing_inference_env;
pub use gen::typing_kinding_defs;
pub use gen::typing_reason;
pub use gen::typing_tyvar_occurrences;
pub use gen::user_error;
pub use gen::validation_err;
pub use gen::xhp_attribute;
