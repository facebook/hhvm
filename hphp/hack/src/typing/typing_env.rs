// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing_env_return_info;
pub use crate::typing_env_types::*;
use crate::typing_local_types::Local;
use crate::typing_per_cont_env::{PerContEntry, TypingContKey};

use decl_provider_rust as decl_provider;
use oxidized::pos::Pos;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::ident::Ident;
use oxidized_by_ref::shallow_decl_defs::{ShallowClass, ShallowMethod};
use typing_collections_rust::Map;
use typing_defs_rust as typing_defs;
use typing_defs_rust::typing_make_type::TypeBuilder;
use typing_heap_rust::Class;

pub fn empty_global_env<'a>(
    builder: &'a TypeBuilder<'a>,
    provider: &'a dyn decl_provider::DeclProvider,
    file: RelativePath,
) -> Genv<'a> {
    Genv {
        file,
        tcopt: oxidized::global_options::GlobalOptions::default(),
        params: Map::empty(),
        return_info: typing_env_return_info::TypingEnvReturnInfo {
            explicit: false,
            mutable: false,
            void_to_rx: false,
            disposable: false,
            type_: typing_defs::PossiblyEnforcedTy {
                enforced: false,
                type_: builder.void(builder.mk_rnone()),
            },
        },
        builder,
        provider,
    }
}

impl<'a> Env<'a> {
    pub fn ident(&mut self) -> Ident {
        let x = self.ident_counter;
        self.ident_counter += 1;
        return x;
    }

    pub fn set_param(&mut self, id: LocalId<'a>, param: (Ty<'a>, ParamMode)) {
        self.genv.params = self.genv.params.add(self.builder(), id, param)
    }

    pub fn set_local(&mut self, x: LocalId<'a>, new_type: Ty<'a>) {
        let new_type = match new_type.get_node() {
            Ty_::Tunion(tys) if tys.len() == 1 => *tys.first().unwrap(),
            _ => new_type,
        };
        if let Some(next_cont) = self.next_cont_opt() {
            let expr_id = match next_cont.local_types.get(&x) {
                None => self.ident(),
                Some(Local(_, y)) => *y,
            };
            let local = Local(new_type, expr_id);
            self.set_local_(x, local);
        }
    }

    fn set_local_(&mut self, x: LocalId<'a>, ty: Local<'a>) {
        let bld = self.bld();
        let per_cont_env = self
            .lenv
            .per_cont_env
            .add_to_cont(bld, TypingContKey::Next, x, ty);
        self.lenv.per_cont_env = per_cont_env;
    }

    pub fn get_local_check_defined(&self, p: &'a Pos, x: LocalId<'a>) -> Ty<'a> {
        let (_flag, ty) = self.get_local_in_next_continuation(Some(p), x);
        ty
    }

    fn get_local_in_next_continuation(
        &self,
        error_if_undef_at_pos: Option<&'a Pos>,
        x: LocalId<'a>,
    ) -> (bool, Ty<'a>) {
        let next_cont = self.next_cont_opt();
        self.get_local_ty_in_ctx(error_if_undef_at_pos, x, next_cont)
    }

    fn get_local_ty_in_ctx(
        &self,
        error_if_undef_at_pos: Option<&'a Pos>,
        x: LocalId<'a>,
        ctx: Option<&'a PerContEntry<'a>>,
    ) -> (bool, Ty<'a>) {
        match self.get_local_in_ctx(error_if_undef_at_pos, x, ctx) {
            None => {
                let bld = self.bld();
                let ty = bld.any(bld.mk_rnone());
                (true, ty)
            }
            Some(Local(x, _)) => (true, x),
        }
    }

    fn get_local_in_ctx(
        &self,
        _error_if_undef_at_pos: Option<&'a Pos>,
        x: LocalId<'a>,
        ctx: Option<&'a PerContEntry<'a>>,
    ) -> Option<Local<'a>> {
        let bld = self.bld();
        match ctx {
            None => {
                // If the continuation is absent, we are in dead code so the
                // variable should have type nothing.
                Some(Local(bld.nothing(bld.mk_rnone()), 0))
            }
            Some(ctx) => {
                let lcl = ctx.local_types.find(&x).copied();
                // TODO(hrust): error if not found
                lcl
            }
        }
    }

    pub fn set_local_expr_id(&mut self, x: LocalId<'a>, new_eid: Ident) {
        if let Some(next_cont) = self.next_cont_opt() {
            match next_cont.local_types.get(&x) {
                Some(Local(ty, eid)) if *eid != new_eid => {
                    let bld = self.bld();
                    let local = Local(*ty, new_eid);
                    let per_cont_env =
                        self.lenv
                            .per_cont_env
                            .add_to_cont(bld, TypingContKey::Next, x, local);
                    self.lenv.per_cont_env = per_cont_env;
                }
                _ => (),
            }
        }
    }

    pub fn get_local_expr_id(&self, x: LocalId<'a>) -> Option<Ident> {
        match self.next_cont_opt() {
            None => {
                /* dead code */
                None
            }
            Some(next_cont) => {
                let lcl = next_cont.local_types.get(&x);
                lcl.map(|Local(_, x)| *x)
            }
        }
    }

    fn next_cont_opt(&self) -> Option<&'a PerContEntry<'a>> {
        self.lenv.per_cont_env.get_cont_option(TypingContKey::Next)
    }

    pub fn get_construct<'b>(
        &mut self,
        class: &'b ShallowClass<'b>,
    ) -> &'b Option<ShallowMethod<'b>> {
        // TODO(hrust) add dependencies
        &class.constructor
    }

    pub fn get_member<'b>(
        &self,
        class: &'b ShallowClass<'b>,
        member_id: &str,
        is_method: bool,
    ) -> Option<&'b ShallowMethod<'b>> {
        // TODO(hrust) add dependencies
        if is_method {
            Class::get_method(class, member_id)
        } else {
            unimplemented!()
        }
    }

    pub fn tany(&self, r: &'a Reason<'a>) -> Ty<'a> {
        // TODO(hrust) check dynamic view
        self.bld().any(r)
    }
}
