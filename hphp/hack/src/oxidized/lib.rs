// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod manual;

pub use manual::aast_defs_impl;
pub use manual::aast_impl;
pub use manual::ast;
pub use manual::doc_comment;
pub use manual::file_info_impl;
pub use manual::file_pos_large;
pub use manual::file_pos_small;
pub use manual::global_options_impl;
pub use manual::i_map;
pub use manual::i_set;
pub use manual::infer_missing;
pub use manual::local_id;
pub use manual::namespace_env_impl;
pub use manual::pos;
pub use manual::relative_path;
pub use manual::s_map;
pub use manual::s_set;
pub use manual::scoured_comments_impl;
pub use manual::shape_map;
pub use manual::tany_sentinel;

mod stubs;

pub use stubs::errors;
pub use stubs::lazy;
pub use stubs::opaque_digest;
pub use stubs::sequence;

mod impl_gen;
pub use impl_gen::*;

pub mod aast_visitor;

mod gen;

pub use gen::aast;
pub use gen::aast_defs;
pub use gen::ast_defs;
pub use gen::decl_defs;
pub use gen::direct_decl_parser;
pub use gen::file_info;
pub use gen::full_fidelity_parser_env;
pub use gen::global_options;
pub use gen::ident;
pub use gen::namespace_env;
pub use gen::nast;
pub use gen::parser_options;
pub use gen::prim_defs;
pub use gen::scoured_comments;
pub use gen::shallow_decl_defs;
pub use gen::typechecker_options;
pub use gen::typing_defs;
pub use gen::typing_reason;
