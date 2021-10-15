// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Pair, Slice, Str, Triple};
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_hhbc_string_utils::strip_ns;
use naming_special_names_rust::{self as sn, coeffects as c};
use oxidized::{
    aast as a,
    aast_defs::{Hint, Hint_},
    ast_defs::Id,
};
use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Ctx {
    Defaults,

    // Shared
    WriteThisProps,
    WriteProps,

    // Rx hierarchy
    RxLocal,
    RxShallow,
    Rx,

    // Policied hierarchy
    PoliciedOf,
    PoliciedLocal,
    PoliciedShallow,
    Policied,

    Controlled,

    ReadGlobals,
    Globals,

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
            WriteThisProps => write!(f, "{}", c::WRITE_THIS_PROPS),
            WriteProps => write!(f, "{}", c::WRITE_PROPS),
            PoliciedOf => write!(f, "{}", c::POLICIED_OF),
            PoliciedLocal => write!(f, "{}", c::POLICIED_LOCAL),
            PoliciedShallow => write!(f, "{}", c::POLICIED_SHALLOW),
            Policied => write!(f, "{}", c::POLICIED),
            Pure => write!(f, "{}", c::PURE),
            Controlled => write!(f, "{}", c::CONTROLLED),
            ReadGlobals => write!(f, "{}", c::READ_GLOBALS),
            Globals => write!(f, "{}", c::GLOBALS),
        }
    }
}

#[derive(Debug)]
#[repr(C)]
pub struct HhasCtxConstant<'arena> {
    pub name: Str<'arena>,
    pub coeffects: Pair<Slice<'arena, Ctx>, Slice<'arena, Str<'arena>>>,
    pub is_abstract: bool,
}

#[derive(Clone, Debug, Default)]
#[repr(C)]
pub struct HhasCoeffects<'arena> {
    static_coeffects: Slice<'arena, Ctx>,
    unenforced_static_coeffects: Slice<'arena, Str<'arena>>,
    fun_param: Slice<'arena, usize>,
    cc_param: Slice<'arena, Pair<usize, Str<'arena>>>,
    cc_this: Slice<'arena, Slice<'arena, Str<'arena>>>,
    cc_reified: Slice<'arena, Triple<bool, usize, Slice<'arena, Str<'arena>>>>,
    closure_parent_scope: bool,
    generator_this: bool,
    caller: bool,
}

impl<'arena> HhasCoeffects<'arena> {
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
                c.as_str().to_string()
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
        if let Some(str) = HhasCoeffects::vec_to_string(coeffects.get_cc_param(), |c| {
            format!("{} {}", c.0, c.1.as_str())
        }) {
            results.push(format!(".coeffects_cc_param {};", str));
        }
        for v in coeffects.get_cc_this() {
            match HhasCoeffects::vec_to_string(v.as_ref(), |c| c.as_str().to_string()) {
                Some(str) => results.push(format!(".coeffects_cc_this {};", str)),
                None => panic!("Not possible"),
            }
        }
        for v in coeffects.get_cc_reified() {
            match HhasCoeffects::vec_to_string(v.2.as_ref(), |c| c.as_str().to_string()) {
                Some(str) => results.push(format!(
                    ".coeffects_cc_reified {}{} {};",
                    if v.0 { "isClass " } else { "" },
                    v.1,
                    str
                )),
                None => panic!("Not possible"),
            }
        }
        if coeffects.is_closure_parent_scope() {
            results.push(".coeffects_closure_parent_scope;".to_string());
        }
        if coeffects.generator_this() {
            results.push(".coeffects_generator_this;".to_string());
        }
        if coeffects.caller() {
            results.push(".coeffects_caller;".to_string());
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
                c::WRITE_THIS_PROPS => Some(Ctx::WriteThisProps),
                c::WRITE_PROPS => Some(Ctx::WriteProps),
                c::POLICIED_OF => Some(Ctx::PoliciedOf),
                c::POLICIED_LOCAL => Some(Ctx::PoliciedLocal),
                c::POLICIED_SHALLOW => Some(Ctx::PoliciedShallow),
                c::POLICIED => Some(Ctx::Policied),
                c::CONTROLLED => Some(Ctx::Controlled),
                c::GLOBALS => Some(Ctx::Globals),
                c::READ_GLOBALS => Some(Ctx::ReadGlobals),
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
                PoliciedLocal => PoliciedShallow,
                _ => *c,
            })
        }
        result
    }

    pub fn from_ctx_constant(hint: &Hint) -> (Vec<Ctx>, Vec<String>) {
        let mut static_coeffects = vec![];
        let mut unenforced_static_coeffects = vec![];
        let Hint(_, h) = hint;
        match &**h {
            Hint_::Hintersection(hl) if hl.is_empty() => static_coeffects.push(Ctx::Pure),
            Hint_::Hintersection(hl) => {
                for h in hl {
                    let Hint(_, h_inner) = h;
                    match &**h_inner {
                        Hint_::Happly(Id(_, id), _) => {
                            if let Some(c) = HhasCoeffects::from_type_static(h) {
                                static_coeffects.push(c)
                            } else {
                                unenforced_static_coeffects.push(strip_ns(id.as_str()).to_string());
                            }
                        }
                        _ => {}
                    }
                }
            }
            _ => {}
        }
        (static_coeffects, unenforced_static_coeffects)
    }

    pub fn from_ast<Ex, En>(
        alloc: &'arena bumpalo::Bump,
        ctxs_opt: &Option<a::Contexts>,
        params: impl AsRef<[a::FunParam<Ex, En>]>,
        fun_tparams: impl AsRef<[a::Tparam<Ex, En>]>,
        cls_tparams: impl AsRef<[a::Tparam<Ex, En>]>,
    ) -> Self {
        let mut static_coeffects = vec![];
        let mut unenforced_static_coeffects: Vec<Str<'arena>> = vec![];
        let mut fun_param = vec![];
        let mut cc_param: Vec<Pair<usize, Str<'arena>>> = vec![];
        let mut cc_this: Vec<Slice<'arena, Str<'arena>>> = vec![];
        let mut cc_reified: Vec<Triple<bool, usize, Slice<'arena, Str<'arena>>>> = vec![];

        let get_arg_pos = |name: &String| -> usize {
            if let Some(pos) = params.as_ref().iter().position(|x| x.name == *name) {
                pos
            } else {
                panic!("Invalid context");
            }
        };

        let is_reified_tparam = |name: &str, is_class: bool| -> Option<usize> {
            let tparam = if is_class {
                cls_tparams.as_ref()
            } else {
                fun_tparams.as_ref()
            };
            tparam
                .iter()
                .position(|tp| tp.reified == a::ReifyKind::Reified && tp.name.1 == name)
        };

        // From coeffect syntax
        if let Some(ctxs) = ctxs_opt {
            if ctxs.1.is_empty() {
                static_coeffects.push(Ctx::Pure);
            }
            for ctx in &ctxs.1 {
                let Hint(_, h) = ctx;
                match &**h {
                    Hint_::Happly(Id(_, id), _) => {
                        if let Some(c) = HhasCoeffects::from_type_static(ctx) {
                            static_coeffects.push(c)
                        } else {
                            unenforced_static_coeffects
                                .push(alloc.alloc_str(strip_ns(id.as_str())).into());
                        }
                    }
                    Hint_::HfunContext(name) => fun_param.push(get_arg_pos(name)),
                    Hint_::Haccess(Hint(_, hint), sids) => match &**hint {
                        Hint_::Happly(Id(_, id), _) if !sids.is_empty() => {
                            if strip_ns(id.as_str()) == sn::typehints::THIS {
                                cc_this.push(Slice::from_vec(
                                    alloc,
                                    sids.into_iter()
                                        .map(|Id(_, id)| alloc.alloc_str(id).into())
                                        .collect(),
                                ));
                            } else if let Some(idx) = is_reified_tparam(id.as_str(), false) {
                                cc_reified.push(
                                    (
                                        false,
                                        idx,
                                        Slice::from_vec(
                                            alloc,
                                            sids.into_iter()
                                                .map(|Id(_, id)| alloc.alloc_str(id).into())
                                                .collect(),
                                        ),
                                    )
                                        .into(),
                                );
                            } else if let Some(idx) = is_reified_tparam(id.as_str(), true) {
                                cc_reified.push(
                                    (
                                        true,
                                        idx,
                                        Slice::from_vec(
                                            alloc,
                                            sids.into_iter()
                                                .map(|Id(_, id)| alloc.alloc_str(id).into())
                                                .collect(),
                                        ),
                                    )
                                        .into(),
                                );
                            }
                        }
                        Hint_::Hvar(name) if sids.len() == 1 => {
                            let pos = get_arg_pos(name);
                            let Id(_, sid_name) = &sids[0];
                            cc_param.push((pos, alloc.alloc_str(sid_name).into()).into());
                        }
                        _ => {}
                    },
                    _ => {}
                }
            }
        }

        // If there are no static coeffects but there are coeffect rules, then
        // the static coeffects are pure
        if static_coeffects.is_empty()
            && (!fun_param.is_empty()
                || !cc_param.is_empty()
                || !cc_this.is_empty()
                || !cc_reified.is_empty())
        {
            static_coeffects.push(Ctx::Pure);
        }

        Self {
            static_coeffects: Slice::from_vec(alloc, static_coeffects),
            unenforced_static_coeffects: Slice::from_vec(alloc, unenforced_static_coeffects),
            fun_param: Slice::from_vec(alloc, fun_param),
            cc_param: Slice::from_vec(alloc, cc_param),
            cc_this: Slice::from_vec(alloc, cc_this),
            cc_reified: Slice::from_vec(alloc, cc_reified),
            ..HhasCoeffects::default()
        }
    }

    pub fn pure(alloc: &'arena bumpalo::Bump) -> Self {
        Self {
            static_coeffects: Slice::from_vec(alloc, vec![Ctx::Pure]),
            ..HhasCoeffects::default()
        }
    }

    pub fn inherit_to_child_closure(&self, alloc: &'arena bumpalo::Bump) -> Self {
        let static_coeffects = HhasCoeffects::local_to_shallow(self.get_static_coeffects());
        if self.has_coeffect_rules() {
            Self {
                static_coeffects: Slice::from_vec(alloc, static_coeffects),
                closure_parent_scope: true,
                ..HhasCoeffects::default()
            }
        } else {
            Self {
                static_coeffects: Slice::from_vec(alloc, static_coeffects),
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

    pub fn with_caller(&self, alloc: &'arena bumpalo::Bump) -> Self {
        Self {
            static_coeffects: Slice::from_vec(alloc, vec![Ctx::Pure]),
            unenforced_static_coeffects: Slice::empty(),
            caller: true,
            ..self.clone()
        }
    }

    pub fn get_static_coeffects(&self) -> &[Ctx] {
        self.static_coeffects.as_ref()
    }

    pub fn get_unenforced_static_coeffects(&self) -> &[Str<'arena>] {
        self.unenforced_static_coeffects.as_ref()
    }

    pub fn get_fun_param(&self) -> &[usize] {
        self.fun_param.as_ref()
    }

    pub fn get_cc_param(&self) -> &[Pair<usize, Str<'arena>>] {
        self.cc_param.as_ref()
    }

    pub fn get_cc_this(&self) -> &[Slice<'arena, Str<'arena>>] {
        self.cc_this.as_ref()
    }

    pub fn get_cc_reified(&self) -> &[Triple<bool, usize, Slice<'arena, Str<'arena>>>] {
        self.cc_reified.as_ref()
    }

    pub fn generator_this(&self) -> bool {
        self.generator_this
    }

    pub fn caller(&self) -> bool {
        self.caller
    }

    fn has_coeffect_rules(&self) -> bool {
        !self.fun_param.is_empty()
            || !self.cc_param.is_empty()
            || !self.cc_this.is_empty()
            || !self.cc_reified.is_empty()
            || self.closure_parent_scope
            || self.generator_this
            || self.caller
    }

    pub fn has_coeffects_local(&self) -> bool {
        self.has_coeffect_rules() && !self.generator_this()
    }

    pub fn is_closure_parent_scope(&self) -> bool {
        self.closure_parent_scope
    }

    pub fn is_86caller(&self) -> bool {
        !self.has_coeffect_rules()
            && self.static_coeffects.len() == 0
            && self.unenforced_static_coeffects.len() == 1
            && self.unenforced_static_coeffects.as_ref()[0]
                == string_utils::coeffects::CALLER.into()
    }
}

#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_ctx_constant<'arena>(
    _: HhasCtxConstant<'arena>,
) {
    unimplemented!()
}

#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_coeffects<'arena>(
    _: HhasCoeffects<'arena>,
) {
    unimplemented!()
}
