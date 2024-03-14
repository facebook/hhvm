// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Vector;
use hhbc_string_utils::strip_ns;
use naming_special_names_rust::coeffects as c;
use naming_special_names_rust::coeffects::Ctx;
use naming_special_names_rust::{self as sn};
use oxidized::aast as a;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::ast_defs::Id;
use serde::Serialize;

use crate::StringId;

#[derive(Debug, Clone, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct CtxConstant {
    pub name: StringId,
    pub recognized: Vector<StringId>,
    pub unrecognized: Vector<StringId>,
    pub is_abstract: bool,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct CcParam {
    pub index: u32,
    pub ctx_name: StringId,
}

#[derive(Clone, Debug, Default, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct CcThis {
    pub types: Vector<StringId>,
}

#[derive(Clone, Debug, Default, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct CcReified {
    pub is_class: bool,
    pub index: u32,
    pub types: Vector<StringId>,
}

#[derive(Clone, Debug, Default, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Coeffects {
    pub static_coeffects: Vector<Ctx>,
    pub unenforced_static_coeffects: Vector<StringId>,
    pub fun_param: Vector<u32>,
    pub cc_param: Vector<CcParam>,
    pub cc_this: Vector<CcThis>,
    pub cc_reified: Vector<CcReified>,
    pub closure_parent_scope: bool,
    pub generator_this: bool,
    pub caller: bool,
}

impl Coeffects {
    pub fn new(
        static_coeffects: Vec<Ctx>,
        unenforced_static_coeffects: Vec<StringId>,
        fun_param: Vec<u32>,
        cc_param: Vec<CcParam>,
        cc_this: Vec<CcThis>,
        cc_reified: Vec<CcReified>,
        closure_parent_scope: bool,
        generator_this: bool,
        caller: bool,
    ) -> Self {
        Self {
            static_coeffects: static_coeffects.into(),
            unenforced_static_coeffects: unenforced_static_coeffects.into(),
            fun_param: fun_param.into(),
            cc_param: cc_param.into(),
            cc_this: cc_this.into(),
            cc_reified: cc_reified.into(),
            closure_parent_scope,
            generator_this,
            caller,
        }
    }

    pub fn from_static_coeffects(scs: Vec<Ctx>) -> Coeffects {
        Coeffects {
            static_coeffects: scs.into(),
            ..Default::default()
        }
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
                c::ZONED_WITH => Some(Ctx::ZonedWith),
                c::ZONED_LOCAL => Some(Ctx::ZonedLocal),
                c::ZONED_SHALLOW => Some(Ctx::ZonedShallow),
                c::ZONED => Some(Ctx::Zoned),
                c::LEAK_SAFE_LOCAL => Some(Ctx::LeakSafeLocal),
                c::LEAK_SAFE_SHALLOW => Some(Ctx::LeakSafeShallow),
                c::LEAK_SAFE => Some(Ctx::LeakSafe),
                c::GLOBALS => Some(Ctx::Globals),
                c::READ_GLOBALS => Some(Ctx::ReadGlobals),
                _ => None,
            },
            _ => None,
        }
    }

    pub fn local_to_shallow(coeffects: &[Ctx]) -> Vec<Ctx> {
        coeffects
            .iter()
            .map(|c| match c {
                Ctx::RxLocal => Ctx::RxShallow,
                Ctx::ZonedLocal => Ctx::ZonedShallow,
                Ctx::LeakSafeLocal => Ctx::LeakSafeShallow,
                _ => *c,
            })
            .collect()
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
                            if let Some(c) = Coeffects::from_type_static(h) {
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
        ctxs_opt: Option<&a::Contexts>,
        params: impl AsRef<[a::FunParam<Ex, En>]>,
        fun_tparams: impl AsRef<[a::Tparam<Ex, En>]>,
        cls_tparams: impl AsRef<[a::Tparam<Ex, En>]>,
    ) -> Self {
        let mut static_coeffects = vec![];
        let mut unenforced_static_coeffects = vec![];
        let mut fun_param = vec![];
        let mut cc_param: Vec<CcParam> = vec![];
        let mut cc_this: Vec<CcThis> = vec![];
        let mut cc_reified: Vec<CcReified> = vec![];

        let get_arg_pos = |name: &String| -> u32 {
            if let Some(pos) = params.as_ref().iter().position(|x| x.name == *name) {
                pos as u32
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
                        if let Some(c) = Coeffects::from_type_static(ctx) {
                            static_coeffects.push(c)
                        } else {
                            unenforced_static_coeffects.push(crate::intern(strip_ns(id.as_str())));
                        }
                    }
                    Hint_::HfunContext(name) => fun_param.push(get_arg_pos(name)),
                    Hint_::Haccess(Hint(_, hint), sids) => match &**hint {
                        Hint_::Happly(Id(_, id), _) if !sids.is_empty() => {
                            if strip_ns(id.as_str()) == sn::typehints::THIS {
                                cc_this.push(CcThis {
                                    types: Vec::from_iter(
                                        sids.iter().map(|Id(_, id)| crate::intern(id)),
                                    )
                                    .into(),
                                });
                            } else if let Some(idx) = is_reified_tparam(id.as_str(), false) {
                                cc_reified.push(CcReified {
                                    is_class: false,
                                    index: idx as u32,
                                    types: Vec::from_iter(
                                        sids.iter().map(|Id(_, id)| crate::intern(id)),
                                    )
                                    .into(),
                                });
                            } else if let Some(idx) = is_reified_tparam(id.as_str(), true) {
                                cc_reified.push(CcReified {
                                    is_class: true,
                                    index: idx as u32,
                                    types: Vec::from_iter(
                                        sids.iter().map(|Id(_, id)| crate::intern(id)),
                                    )
                                    .into(),
                                });
                            }
                        }
                        Hint_::Hvar(name) if sids.len() == 1 => {
                            let index = get_arg_pos(name);
                            let Id(_, sid_name) = &sids[0];
                            let ctx_name = crate::intern(sid_name);
                            cc_param.push(CcParam { index, ctx_name });
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
            static_coeffects: static_coeffects.into(),
            unenforced_static_coeffects: unenforced_static_coeffects.into(),
            fun_param: fun_param.into(),
            cc_param: cc_param.into(),
            cc_this: cc_this.into(),
            cc_reified: cc_reified.into(),
            ..Coeffects::default()
        }
    }

    pub fn pure() -> Self {
        Self {
            static_coeffects: vec![Ctx::Pure].into(),
            ..Coeffects::default()
        }
    }

    pub fn with_backdoor(&self) -> Self {
        Self {
            unenforced_static_coeffects: vec![crate::intern(c::BACKDOOR)].into(),
            ..self.clone()
        }
    }

    pub fn with_backdoor_globals_leak_safe(&self) -> Self {
        Self {
            unenforced_static_coeffects: vec![crate::intern(c::BACKDOOR_GLOBALS_LEAK_SAFE)].into(),
            ..self.clone()
        }
    }

    pub fn inherit_to_child_closure(&self) -> Self {
        let static_coeffects = Coeffects::local_to_shallow(self.get_static_coeffects());
        if self.has_coeffect_rules() {
            Self {
                static_coeffects: static_coeffects.into(),
                closure_parent_scope: true,
                ..Coeffects::default()
            }
        } else {
            Self {
                static_coeffects: static_coeffects.into(),
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

    pub fn with_caller(&self) -> Self {
        Self {
            static_coeffects: vec![Ctx::Pure].into(),
            unenforced_static_coeffects: vec![].into(),
            caller: true,
            ..self.clone()
        }
    }

    /// Does not erase any static_coeffects or unenforced_static_coeffects
    pub fn untouch_with_caller(&self) -> Self {
        Self {
            caller: true,
            ..self.clone()
        }
    }

    pub fn get_static_coeffects(&self) -> &[Ctx] {
        self.static_coeffects.as_ref()
    }

    pub fn get_unenforced_static_coeffects(&self) -> &[StringId] {
        self.unenforced_static_coeffects.as_ref()
    }

    pub fn get_fun_param(&self) -> &[u32] {
        self.fun_param.as_ref()
    }

    pub fn get_cc_param(&self) -> &[CcParam] {
        self.cc_param.as_ref()
    }

    pub fn get_cc_this(&self) -> &[CcThis] {
        self.cc_this.as_ref()
    }

    pub fn get_cc_reified(&self) -> &[CcReified] {
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
            && self.static_coeffects.is_empty()
            && self.unenforced_static_coeffects.len() == 1
            && self.unenforced_static_coeffects.as_ref()[0].as_str()
                == hhbc_string_utils::coeffects::CALLER
    }
}
