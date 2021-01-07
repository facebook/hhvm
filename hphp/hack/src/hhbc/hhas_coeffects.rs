// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::{self as sn, user_attributes::*};
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use oxidized::{aast as a, ast_defs};
use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, ToOcamlRep, FromOcamlRep)]
pub enum Ctx {
    Defaults,

    // Rx hierarchy
    RxLocal,
    RxShallow,
    Rx,

    // Cipp hierarchy
    //CippLocal,
    //CippShallow,
    //Cipp,
    //CippGlobal,
    Pure,
}

impl fmt::Display for Ctx {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use Ctx::*;
        match self {
            Defaults => write!(f, "defaults"),
            RxLocal => write!(f, "rx_local"),
            RxShallow => write!(f, "rx_shallow"),
            Rx => write!(f, "rx"),
            Pure => write!(f, "pure"),
        }
    }
}

#[derive(Clone, Debug, Default, ToOcamlRep, FromOcamlRep)]
pub struct HhasCoeffects {
    static_coeffects: Vec<Ctx>,
    //TODO: Add FUN_ARG, CC_ARG, CC_THIS vectors
    is_any_rx: bool,
    is_pure: bool,
}

impl HhasCoeffects {
    pub fn static_coeffects_to_string(coeffects: &Self) -> Option<String> {
        if coeffects.get_static_coeffects().is_empty() {
            return None;
        }
        Some(
            coeffects
                .get_static_coeffects()
                .iter()
                .map(|c| c.to_string())
                .collect::<Vec<String>>()
                .join(", "),
        )
    }

    pub fn from_ast<Ex, Fb, En, Hi>(
        ast_attrs: impl AsRef<[a::UserAttribute<Ex, Fb, En, Hi>]>,
    ) -> Self {
        let mut static_coeffects = vec![];
        let mut is_any_rx = false;
        let mut is_pure = false;

        let ast_attrs = ast_attrs.as_ref();
        let attrs_contain = |name| ast_attrs.iter().any(|attr| attr.name.1 == name);

        if attrs_contain(NON_RX) {
            static_coeffects.push(Ctx::Defaults);
        }
        if attrs_contain(LOCAL_REACTIVE) {
            is_any_rx = true;
            static_coeffects.push(Ctx::RxLocal);
        }
        if attrs_contain(SHALLOW_REACTIVE) {
            is_any_rx = true;
            static_coeffects.push(Ctx::RxShallow);
        }
        if attrs_contain(REACTIVE) {
            is_any_rx = true;
            static_coeffects.push(Ctx::Rx);
        }
        if attrs_contain(PURE) {
            is_pure = true;
            static_coeffects.push(Ctx::Pure);
        }

        // TODO: Add coeffects from ctxs

        Self {
            static_coeffects,
            is_any_rx,
            is_pure,
        }
    }

    pub fn get_static_coeffects(&self) -> &[Ctx] {
        self.static_coeffects.as_slice()
    }

    pub fn is_any_rx(&self) -> bool {
        self.is_any_rx
    }

    pub fn is_any_rx_or_pure(&self) -> bool {
        self.is_any_rx() || self.is_pure
    }
}

pub fn halves_of_is_enabled_body<Ex, Fb, En, Hi>(
    body: &a::FuncBody<Ex, Fb, En, Hi>,
) -> Option<(&a::Block<Ex, Fb, En, Hi>, &a::Block<Ex, Fb, En, Hi>)> {
    use a::*;
    if let [Stmt(_, Stmt_::If(if_))] = body.ast.as_slice() {
        if let (Expr(_, Expr_::Id(sid)), enabled, disabled) = &**if_ {
            let ast_defs::Id(_, name) = &**sid;
            return if name != sn::rx::IS_ENABLED {
                None
            } else {
                match disabled.as_slice() {
                    [] | [Stmt(_, Stmt_::Noop)] => None,
                    _ => Some((enabled, disabled)),
                }
            };
        }
    }
    None
}
