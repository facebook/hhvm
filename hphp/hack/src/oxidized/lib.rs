// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ord;
use std::collections::{BTreeMap, BTreeSet};

mod manual;

pub use manual::aast_defs_impl;
pub use manual::aast_impl;
pub use manual::ast;
pub use manual::ast_defs_impl;
pub use manual::decl_env;
pub use manual::doc_comment;
pub use manual::file_info_impl;
pub use manual::file_pos_large;
pub use manual::file_pos_small;
pub use manual::global_options_impl;
pub use manual::i_map;
pub use manual::i_set;
pub use manual::ident_impl;
pub use manual::internal_type_set;
pub use manual::local_id;
pub use manual::namespace_env_impl;
pub use manual::phase_map;
pub use manual::pos;
pub use manual::relative_path;
pub use manual::s_map;
pub use manual::s_set;
pub use manual::scoured_comments_impl;
pub use manual::shape_map;
pub use manual::tany_sentinel;
pub use manual::tast_impl;
pub use manual::ty_impl;
pub use manual::typing_continuations;
pub use manual::typing_defs_flags;
pub use manual::typing_env_return_info;
pub use manual::typing_logic;
pub use manual::typing_reason_impl;
pub use manual::typing_set;
pub use manual::uast;

mod stubs;

pub use stubs::lazy;
pub use stubs::opaque_digest;

mod impl_gen;
pub use impl_gen::*;

pub mod aast_visitor;

mod gen;

pub use gen::aast;
pub use gen::aast_defs;
pub use gen::ast_defs;
pub use gen::decl_defs;
pub use gen::direct_decl_parser;
pub use gen::error_codes;
pub use gen::errors;
pub use gen::file_info;
pub use gen::full_fidelity_parser_env;
pub use gen::global_options;
pub use gen::ident;
pub use gen::namespace_env;
pub use gen::naming_types;
pub use gen::nast;
pub use gen::parser_options;
pub use gen::prim_defs;
pub use gen::scoured_comments;
pub use gen::shallow_decl_defs;
pub use gen::tast;
pub use gen::type_parameter_env;
pub use gen::typechecker_options;
pub use gen::typing_cont_key;
pub use gen::typing_defs;
pub use gen::typing_defs_core;
pub use gen::typing_env_types;
pub use gen::typing_fake_members;
pub use gen::typing_inference_env;
pub use gen::typing_local_types;
pub use gen::typing_mutability_env;
pub use gen::typing_per_cont_env;
pub use gen::typing_reason;
pub use gen::typing_tyvar_occurrences;

pub trait ToOxidized {
    type Target;

    fn to_oxidized(&self) -> Self::Target;
}

impl ToOxidized for &str {
    type Target = String;

    fn to_oxidized(&self) -> Self::Target {
        self.to_string()
    }
}

impl<T: ToOxidized> ToOxidized for Option<&T> {
    type Target = Option<T::Target>;

    fn to_oxidized(&self) -> Option<T::Target> {
        self.as_ref().map(|x| x.to_oxidized())
    }
}

impl<Tk: ToOxidized, T: ToOxidized> ToOxidized for arena_collections::map::Map<'_, Tk, &T>
where
    Tk::Target: Ord,
{
    type Target = BTreeMap<Tk::Target, T::Target>;

    fn to_oxidized(&self) -> Self::Target {
        self.into_iter()
            .map(|(k, v)| (k.to_oxidized(), v.to_oxidized()))
            .collect()
    }
}

impl<T: ToOxidized> ToOxidized for arena_collections::set::Set<'_, T>
where
    T::Target: Ord,
{
    type Target = BTreeSet<T::Target>;

    fn to_oxidized(&self) -> Self::Target {
        self.into_iter().map(|x| x.to_oxidized()).collect()
    }
}

impl<Ta: ToOxidized, Tb: ToOxidized> ToOxidized for (Ta, Tb) {
    type Target = (Ta::Target, Tb::Target);

    fn to_oxidized(&self) -> Self::Target {
        let (a, b) = self;
        (a.to_oxidized(), b.to_oxidized())
    }
}

impl<Ta: ToOxidized, Tb: ToOxidized, Tc: ToOxidized> ToOxidized for (Ta, Tb, Tc) {
    type Target = (Ta::Target, Tb::Target, Tc::Target);

    fn to_oxidized(&self) -> Self::Target {
        let (a, b, c) = self;
        (a.to_oxidized(), b.to_oxidized(), c.to_oxidized())
    }
}

impl<Ta: ToOxidized, Tb: ToOxidized, Tc: ToOxidized, Td: ToOxidized> ToOxidized
    for (Ta, Tb, Tc, Td)
{
    type Target = (Ta::Target, Tb::Target, Tc::Target, Td::Target);

    fn to_oxidized(&self) -> Self::Target {
        let (a, b, c, d) = self;
        (
            a.to_oxidized(),
            b.to_oxidized(),
            c.to_oxidized(),
            d.to_oxidized(),
        )
    }
}

pub trait Oxide: Clone {}

impl Oxide for isize {}
impl Oxide for ast_defs::Id {}

impl<T: Oxide> ToOxidized for T {
    type Target = Self;
    fn to_oxidized(&self) -> Self::Target {
        self.clone()
    }
}
