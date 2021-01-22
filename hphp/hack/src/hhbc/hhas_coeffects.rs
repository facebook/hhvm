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
    WriteProps,

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
            WriteProps => write!(f, "{}", c::WRITE_PROPS),
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
    fun_param: Vec<usize>,
    cc_param: Vec<(usize, String)>,
    cc_this: Vec<String>,
    is_any_rx: bool,
    is_pure: bool,
}

impl HhasCoeffects {
    fn vec_to_string<T, F: Fn(&T) -> String>(v: &[T], f: F) -> Option<String> {
        if v.is_empty() {
            return None;
        }
        Some(v.iter().map(|x| f(x)).collect::<Vec<String>>().join(" "))
    }

    pub fn coeffects_to_hhas(coeffects: &Self) -> Vec<String> {
        let mut results = vec![];
        if let Some(str) =
            HhasCoeffects::vec_to_string(coeffects.get_static_coeffects(), |c| c.to_string())
        {
            results.push(format!(".coeffects_static {};", str));
        }
        if let Some(str) =
            HhasCoeffects::vec_to_string(coeffects.get_fun_param(), |c| c.to_string())
        {
            results.push(format!(".coeffects_fun_param {};", str));
        }
        if let Some(str) =
            HhasCoeffects::vec_to_string(coeffects.get_cc_param(), |c| format!("{} {}", c.0, c.1))
        {
            results.push(format!(".coeffects_cc_param {};", str));
        }
        if let Some(str) = HhasCoeffects::vec_to_string(coeffects.get_cc_this(), |c| c.to_string())
        {
            results.push(format!(".coeffects_cc_this {};", str));
        }
        results
    }

    pub fn from_ast<Ex, Fb, En, Hi>(
        ast_attrs: impl AsRef<[a::UserAttribute<Ex, Fb, En, Hi>]>,
        ctxs_opt: &Option<a::Contexts>,
        params: impl AsRef<[a::FunParam<Ex, Fb, En, Hi>]>,
    ) -> Self {
        let mut static_coeffects = vec![];
        let mut fun_param = vec![];
        let mut cc_param = vec![];
        let mut cc_this = vec![];
        let mut is_any_rx = false;
        let mut is_pure = false;

        let get_arg_pos = |name: &String| -> usize {
            if let Some(pos) = params.as_ref().iter().position(|x| x.name == *name) {
                pos
            } else {
                panic!("Invalid context");
            }
        };

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
                match &**h {
                    Hint_::Happly(Id(_, id), _) => {
                        match strip_ns(id.as_str()) {
                            c::DEFAULTS => static_coeffects.push(Ctx::Defaults),
                            c::RX_LOCAL => static_coeffects.push(Ctx::RxLocal),
                            c::RX_SHALLOW => static_coeffects.push(Ctx::RxShallow),
                            c::RX => static_coeffects.push(Ctx::Rx),
                            c::WRITE_PROPS => static_coeffects.push(Ctx::WriteProps),
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
                    Hint_::HfunContext(name) => fun_param.push(get_arg_pos(name)),
                    Hint_::Haccess(Hint(_, hint), sids) if sids.len() == 1 => {
                        let Id(_, sid_name) = &sids[0];
                        match &**hint {
                            Hint_::Happly(Id(_, id), _)
                                if strip_ns(id.as_str()) == sn::typehints::THIS =>
                            {
                                cc_this.push(sid_name.clone());
                            }
                            Hint_::Hvar(name) => {
                                let pos = get_arg_pos(name);
                                cc_param.push((pos, sid_name.clone()));
                            }
                            _ => {}
                        }
                    }
                    _ => {}
                }
            }
        }

        Self {
            static_coeffects,
            fun_param,
            cc_param,
            cc_this,
            is_any_rx,
            is_pure,
        }
    }

    pub fn get_static_coeffects(&self) -> &[Ctx] {
        self.static_coeffects.as_slice()
    }

    pub fn get_fun_param(&self) -> &[usize] {
        self.fun_param.as_slice()
    }

    pub fn get_cc_param(&self) -> &[(usize, String)] {
        self.cc_param.as_slice()
    }

    pub fn get_cc_this(&self) -> &[String] {
        self.cc_this.as_slice()
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
