// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Pair, Slice, Str, Triple};
use hhbc_string_utils::strip_ns;
use naming_special_names_rust::{self as sn, coeffects as c, coeffects::Ctx};
use oxidized::{
    aast as a,
    aast_defs::{Hint, Hint_},
    ast_defs::Id,
};

#[derive(Debug)]
#[repr(C)]
pub struct HhasCtxConstant<'arena> {
    pub name: Str<'arena>,
    pub recognized: Slice<'arena, Str<'arena>>,
    pub unrecognized: Slice<'arena, Str<'arena>>,
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
                c::POLICIED_OF | c::ZONED_WITH => Some(Ctx::ZonedWith),
                c::POLICIED_LOCAL | c::ZONED_LOCAL => Some(Ctx::ZonedLocal),
                c::POLICIED_SHALLOW | c::ZONED_SHALLOW => Some(Ctx::ZonedShallow),
                c::POLICIED | c::ZONED => Some(Ctx::Zoned),
                c::CONTROLLED | c::LEAK_SAFE => Some(Ctx::LeakSafe),
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
        ctxs_opt: Option<&a::Contexts>,
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
                                    sids.iter()
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
                                            sids.iter()
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
                                            sids.iter()
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

    pub fn with_backdoor(&self, alloc: &'arena bumpalo::Bump) -> Self {
        Self {
            unenforced_static_coeffects: Slice::from_vec(alloc, vec![Str::from(c::BACKDOOR)]),
            ..self.clone()
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
            && self.static_coeffects.is_empty()
            && self.unenforced_static_coeffects.len() == 1
            && self.unenforced_static_coeffects.as_ref()[0]
                == hhbc_string_utils::coeffects::CALLER.into()
    }
}
