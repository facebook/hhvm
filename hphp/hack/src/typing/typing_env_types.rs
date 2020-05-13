// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing_env_return_info;
use crate::typing_make_type::TypeBuilder;
use crate::typing_per_cont_env::{PerContEntry, TypingPerContEnv};
use arena_trait::Arena;
use decl_provider_rust::DeclProvider;
use ocamlrep::{Allocator, ToOcamlRep, Value};
use oxidized::pos::Pos as OwnedPos;
use oxidized::ToOxidized;
use oxidized::{relative_path, typechecker_options};
use oxidized_by_ref::pos::Pos;
use std::cell::RefCell;
use std::collections::{BTreeMap, BTreeSet};
use typing_ast_rust::typing_inference_env::InferenceEnv;

pub use typing_collections_rust::*;
pub use typing_defs_rust::typing_logic::{SubtypeProp, *};
pub use typing_defs_rust::{Ty, *};

pub struct Env<'a> {
    pub ident_counter: isize,
    pub function_pos: &'a Pos<'a>,
    pub fresh_typarams: SSet<'a>,
    pub lenv: LocalEnv<'a>,
    pub genv: Genv<'a>,
    pub log_levels: SMap<'a, isize>,
    pub inference_env: InferenceEnv<'a>,
}

impl<'a> Env<'a> {
    pub fn new<'b>(function_pos: &'b OwnedPos, genv: Genv<'a>) -> Self {
        let function_pos = Pos::from_oxidized_in(function_pos, genv.builder.bumpalo());
        Env {
            ident_counter: 0,
            function_pos,
            fresh_typarams: SSet::empty(),
            inference_env: InferenceEnv::new(genv.builder),
            lenv: LocalEnv::initial_local(genv.builder),
            genv,
            log_levels: SMap::empty(),
        }
    }

    pub fn builder(&self) -> &'a TypeBuilder<'a> {
        self.genv.builder
    }

    pub fn bld(&self) -> &'a TypeBuilder<'a> {
        self.builder()
    }

    pub fn provider(&self) -> &'a dyn DeclProvider {
        self.genv.provider
    }

    pub fn ast_pos(&self, pos: &OwnedPos) -> &'a Pos<'a> {
        Pos::from_oxidized_with_file_in(
            pos,
            self.function_pos.filename(),
            self.genv.builder.bumpalo(),
        )
    }

    pub fn set_function_pos(&mut self, pos: &OwnedPos) {
        self.function_pos = Pos::from_oxidized_in(pos, self.genv.builder.bumpalo());
    }

    pub fn set_return_type(&mut self, ty: Ty<'a>) {
        self.genv.return_info.type_.type_ = ty;
    }
}

impl<'a> ToOxidized for Env<'a> {
    type Target = oxidized::typing_env_types::Env;

    fn to_oxidized(&self) -> oxidized::typing_env_types::Env {
        let Env {
            ident_counter: _,
            function_pos,
            fresh_typarams: _,
            lenv,
            genv,
            log_levels: _,
            inference_env,
        } = self;
        oxidized::typing_env_types::Env {
            function_pos: function_pos.to_oxidized(),
            fresh_typarams: BTreeSet::new(), // TODO(hrust)
            lenv: lenv.to_oxidized(),
            genv: genv.to_oxidized(),
            decl_env: (),
            in_loop: false,
            in_try: false,
            in_case: false,
            inside_constructor: false,
            inside_ppl_class: false,
            global_tpenv: oxidized::type_parameter_env::TypeParameterEnv {
                tparams: BTreeMap::new(),
                consistent: true,
            },
            log_levels: BTreeMap::new(), // TODO(hrust)
            inference_env: inference_env.to_oxidized(),
            allow_wildcards: false,
            big_envs: RefCell::new(vec![]),
            pessimize: false,
        }
    }
}

impl ToOcamlRep for Env<'_> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        self.to_oxidized().to_ocamlrep(alloc)
    }
}

pub struct Genv<'a> {
    pub tcopt: typechecker_options::TypecheckerOptions,
    pub return_info: typing_env_return_info::TypingEnvReturnInfo<'a>,
    pub params: Map<'a, LocalId<'a>, (Ty<'a>, ParamMode)>,
    pub file: relative_path::RelativePath,
    pub builder: &'a TypeBuilder<'a>,
    pub provider: &'a dyn DeclProvider,
}

impl<'a> ToOxidized for Genv<'a> {
    type Target = oxidized::typing_env_types::Genv;

    fn to_oxidized(&self) -> oxidized::typing_env_types::Genv {
        // TODO(hrust) most fields of Genv
        oxidized::typing_env_types::Genv {
            tcopt: oxidized::global_options::GlobalOptions::default(),
            return_: oxidized::typing_env_return_info::TypingEnvReturnInfo {
                type_: oxidized::typing_defs::PossiblyEnforcedTy {
                    enforced: true,
                    type_: oxidized::typing_defs_core::Ty(
                        oxidized::typing_reason::Reason::Rnone,
                        Box::new(oxidized::typing_defs_core::Ty_::Tmixed),
                    ),
                },
                disposable: false,
                mutable: false,
                explicit: true,
                void_to_rx: false,
            },
            params: BTreeMap::new(),
            condition_types: oxidized::s_map::SMap::new(),
            parent: None,
            self_: None,
            static_: false,
            fun_kind: oxidized::ast_defs::FunKind::FSync,
            val_kind: oxidized::typing_defs::ValKind::Lval,
            fun_mutable: None,
            file: oxidized::relative_path::RelativePath::make(
                oxidized::relative_path::Prefix::Root,
                std::path::PathBuf::new(),
            ),
        }
    }
}

#[derive(Debug)]
pub struct LocalEnv<'a> {
    pub per_cont_env: TypingPerContEnv<'a>,
    // TODO(hrust): mutability
    // TODO(hrust): reactivity
    // TODO(hrust): using vars
}

impl<'a> LocalEnv<'a> {
    pub fn initial_local<A: Arena>(arena: &'a A) -> Self {
        LocalEnv {
            per_cont_env: TypingPerContEnv::initial_locals(arena, PerContEntry::empty()),
        }
    }
}

impl<'a> ToOxidized for LocalEnv<'a> {
    type Target = oxidized::typing_env_types::LocalEnv;

    fn to_oxidized(&self) -> oxidized::typing_env_types::LocalEnv {
        let LocalEnv { per_cont_env } = self;
        oxidized::typing_env_types::LocalEnv {
            per_cont_env: per_cont_env.to_oxidized(),
            local_mutability: BTreeMap::new(),
            local_reactive: oxidized::typing_env_types::Reactivity::Nonreactive,
            local_using_vars: BTreeSet::new(),
        }
    }
}

pub use oxidized_by_ref::local_id::LocalId;
