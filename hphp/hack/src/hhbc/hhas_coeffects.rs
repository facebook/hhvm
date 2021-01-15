// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_string_utils_rust::strip_ns;
use naming_special_names_rust::{self as sn, coeffects as c, user_attributes as attr};
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use oxidized::{
    aast as a,
    aast_defs::{Hint, Hint_},
    ast_defs::Id,
};
use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, ToOcamlRep, FromOcamlRep)]
pub enum Ctx {
    Defaults,

    // Rx hierarchy
    RxLocal,
    RxShallow,
    Rx,
    Local,

    // Cipp hierarchy
    CippLocal,
    CippShallow,
    Cipp,
    CippGlobal,

    // Pure
    Pure,
}

impl fmt::Display for Ctx {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use Ctx::*;
        match self {
            Defaults => write!(f, "{}", c::DEFAULTS),
            RxLocal => write!(f, "{}", c::RX_LOCAL),
            RxShallow => write!(f, "{}", c::RX_SHALLOW),
            Rx => write!(f, "{}", c::RX),
            Local => write!(f, "{}", c::LOCAL),
            CippLocal => write!(f, "{}", c::CIPP_LOCAL),
            CippShallow => write!(f, "{}", c::CIPP_SHALLOW),
            Cipp => write!(f, "{}", c::CIPP),
            CippGlobal => write!(f, "{}", c::CIPP_GLOBAL),
            Pure => write!(f, "{}", c::PURE),
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
                .join(" "),
        )
    }

    pub fn from_ast<Ex, Fb, En, Hi>(
        ast_attrs: impl AsRef<[a::UserAttribute<Ex, Fb, En, Hi>]>,
        ctxs_opt: &Option<a::Contexts>,
    ) -> Self {
        let mut static_coeffects = vec![];
        let mut is_any_rx = false;
        let mut is_pure = false;

        // From attributes
        for attr in ast_attrs.as_ref() {
            match attr.name.1.as_str() {
                attr::NON_RX => static_coeffects.push(Ctx::Defaults),
                attr::LOCAL_REACTIVE => static_coeffects.push(Ctx::RxLocal),
                attr::SHALLOW_REACTIVE => static_coeffects.push(Ctx::RxShallow),
                attr::REACTIVE => static_coeffects.push(Ctx::Rx),
                attr::PURE => static_coeffects.push(Ctx::Pure),
                _ => {}
            }
            match attr.name.1.as_str() {
                attr::LOCAL_REACTIVE | attr::SHALLOW_REACTIVE | attr::REACTIVE => is_any_rx = true,
                attr::PURE => is_pure = true,
                _ => {}
            }
        }

        // From coeffect syntax
        if let Some(ctxs) = ctxs_opt {
            if ctxs.1.is_empty() {
                is_pure = true;
                static_coeffects.push(Ctx::Pure);
            }
            for ctx in &ctxs.1 {
                let Hint(_, h) = ctx;
                if let Hint_::Happly(Id(_, id), _) = &**h {
                    match strip_ns(id.as_str()) {
                        c::DEFAULTS => static_coeffects.push(Ctx::Defaults),
                        c::RX_LOCAL => static_coeffects.push(Ctx::RxLocal),
                        c::RX_SHALLOW => static_coeffects.push(Ctx::RxShallow),
                        c::RX => static_coeffects.push(Ctx::Rx),
                        c::LOCAL => static_coeffects.push(Ctx::Local),
                        c::CIPP_LOCAL => static_coeffects.push(Ctx::CippLocal),
                        c::CIPP_SHALLOW => static_coeffects.push(Ctx::CippShallow),
                        c::CIPP_GLOBAL => static_coeffects.push(Ctx::CippGlobal),
                        c::CIPP | c::CIPP_OF => static_coeffects.push(Ctx::Cipp),
                        _ => {}
                    }
                    if let c::RX_LOCAL | c::RX_SHALLOW | c::RX = strip_ns(id.as_str()) {
                        is_any_rx = true;
                    }
                }
            }
        }

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
            let Id(_, name) = &**sid;
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
