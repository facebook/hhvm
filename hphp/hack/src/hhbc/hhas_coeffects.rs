// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_string_utils_rust::strip_ns;
use naming_special_names_rust::{self as sn, coeffects as c};
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

    // Shared
    WriteProps,

    // Rx hierarchy
    RxLocal,
    RxShallow,
    Rx,

    // Policied hierarchy
    PoliciedOfLocal,
    PoliciedOfShallow,
    PoliciedOf,
    PoliciedLocal,
    PoliciedShallow,
    Policied,

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
            PoliciedOfLocal => write!(f, "{}", c::POLICIED_OF_LOCAL),
            PoliciedOfShallow => write!(f, "{}", c::POLICIED_OF_SHALLOW),
            PoliciedOf => write!(f, "{}", c::POLICIED_OF),
            PoliciedLocal => write!(f, "{}", c::POLICIED_LOCAL),
            PoliciedShallow => write!(f, "{}", c::POLICIED_SHALLOW),
            Policied => write!(f, "{}", c::POLICIED),
            Pure => write!(f, "{}", c::PURE),
        }
    }
}

#[derive(Debug)]
pub struct HhasCtxConstant {
    pub name: String,
    pub coeffects: Vec<Ctx>,
    pub is_abstract: bool,
}

#[derive(Clone, Debug, Default, ToOcamlRep, FromOcamlRep)]
pub struct HhasCoeffects {
    static_coeffects: Vec<Ctx>,
    unenforced_static_coeffects: Vec<String>,
    fun_param: Vec<usize>,
    cc_param: Vec<(usize, String)>,
    cc_this: Vec<String>,
    is_any_rx: bool,
    is_pure: bool,
    closure_inherit_from_parent: bool,
    generator_this: bool,
}

impl HhasCoeffects {
    pub fn vec_to_string<T, F: Fn(&T) -> String>(v: &[T], f: F) -> Option<String> {
        if v.is_empty() {
            return None;
        }
        Some(v.iter().map(|x| f(x)).collect::<Vec<String>>().join(" "))
    }

    pub fn coeffects_to_hhas(coeffects: &Self) -> Vec<String> {
        let mut results = vec![];
        let static_coeffect =
            HhasCoeffects::vec_to_string(coeffects.get_static_coeffects(), |c| c.to_string());
        let unenforced_static_coeffects =
            HhasCoeffects::vec_to_string(coeffects.get_unenforced_static_coeffects(), |c| {
                c.to_string()
            });
        match (static_coeffect, unenforced_static_coeffects) {
            (None, None) => {}
            (Some(s), None) | (None, Some(s)) => results.push(format!(".coeffects_static {};", s)),
            (Some(s1), Some(s2)) => results.push(format!(".coeffects_static {} {};", s1, s2)),
        };
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
        if coeffects.is_closure_inherit_from_parent() {
            results.push(".coeffects_closure_inherit_from_parent;".to_string());
        }
        if coeffects.generator_this() {
            results.push(".coeffects_generator_this;".to_string());
        }
        results
    }

    fn from_type_static(hint: &Hint) -> Option<Ctx> {
        let Hint(_, h) = hint;
        match &**h {
            Hint_::Happly(Id(_, id), _) => match strip_ns(id.as_str()) {
                c::DEFAULTS => Some(Ctx::Defaults),
                c::RX_LOCAL => Some(Ctx::RxLocal),
                c::RX_SHALLOW => Some(Ctx::RxShallow),
                c::RX => Some(Ctx::Rx),
                c::WRITE_PROPS => Some(Ctx::WriteProps),
                c::POLICIED_OF_LOCAL => Some(Ctx::PoliciedOfLocal),
                c::POLICIED_OF_SHALLOW => Some(Ctx::PoliciedOfShallow),
                c::POLICIED_OF => Some(Ctx::PoliciedOf),
                c::POLICIED_LOCAL => Some(Ctx::PoliciedLocal),
                c::POLICIED_SHALLOW => Some(Ctx::PoliciedShallow),
                c::POLICIED => Some(Ctx::Policied),
                _ => None,
            },
            _ => None,
        }
    }

    pub fn local_to_shallow(coeffects: &[Ctx]) -> Vec<Ctx> {
        use Ctx::*;
        let mut result = vec![];
        for c in coeffects.iter() {
            result.push(match c {
                RxLocal => RxShallow,
                PoliciedOfLocal => PoliciedOfShallow,
                PoliciedLocal => PoliciedShallow,
                _ => *c,
            })
        }
        result
    }

    pub fn from_ctx_constant(hint: &Hint) -> Vec<Ctx> {
        let Hint(_, h) = hint;
        match &**h {
            Hint_::Hintersection(hl) if hl.is_empty() => vec![Ctx::Pure],
            Hint_::Hintersection(hl) => {
                let mut result = vec![];
                for h in hl {
                    if let Some(c) = HhasCoeffects::from_type_static(h) {
                        result.push(c);
                    }
                }
                result
            }
            _ => vec![],
        }
    }

    pub fn from_ast<Ex, Fb, En, Hi>(
        ctxs_opt: &Option<a::Contexts>,
        params: impl AsRef<[a::FunParam<Ex, Fb, En, Hi>]>,
    ) -> Self {
        let mut static_coeffects = vec![];
        let mut unenforced_static_coeffects = vec![];
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
                        if let Some(c) = HhasCoeffects::from_type_static(ctx) {
                            static_coeffects.push(c)
                        } else {
                            unenforced_static_coeffects.push(strip_ns(id.as_str()).to_string());
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

        // If there are no static coeffects but there are coeffect rules, then
        // the static coeffects are pure
        if static_coeffects.is_empty()
            && (!fun_param.is_empty() || !cc_param.is_empty() || !cc_this.is_empty())
        {
            static_coeffects.push(Ctx::Pure);
        }

        Self {
            static_coeffects,
            unenforced_static_coeffects,
            fun_param,
            cc_param,
            cc_this,
            is_any_rx,
            is_pure,
            ..HhasCoeffects::default()
        }
    }

    pub fn inherit_to_child_closure(&self) -> Self {
        let static_coeffects = HhasCoeffects::local_to_shallow(self.get_static_coeffects());
        if self.has_coeffect_rules() {
            Self {
                static_coeffects,
                closure_inherit_from_parent: true,
                ..HhasCoeffects::default()
            }
        } else {
            Self {
                static_coeffects,
                ..self.clone()
            }
        }
    }

    pub fn with_gen_coeffect(&self) -> Self {
        Self {
            generator_this: true,
            ..self.clone()
        }
    }

    pub fn get_static_coeffects(&self) -> &[Ctx] {
        self.static_coeffects.as_slice()
    }

    pub fn get_unenforced_static_coeffects(&self) -> &[String] {
        self.unenforced_static_coeffects.as_slice()
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

    pub fn generator_this(&self) -> bool {
        self.generator_this
    }

    fn has_coeffect_rules(&self) -> bool {
        !self.fun_param.is_empty()
            || !self.cc_param.is_empty()
            || !self.cc_this.is_empty()
            || self.closure_inherit_from_parent
            || self.generator_this
    }

    pub fn has_coeffects_local(&self) -> bool {
        self.has_coeffect_rules() && !self.generator_this()
    }

    pub fn is_closure_inherit_from_parent(&self) -> bool {
        self.closure_inherit_from_parent
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
