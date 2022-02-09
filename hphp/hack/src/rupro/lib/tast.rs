// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing_defs::Ty;

use bumpalo::Bump;
use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};

#[derive(Clone, Debug)]
pub struct SavedEnv;

pub type Program<R> = oxidized::aast::Program<Ty<R>, SavedEnv>;
pub type Def<R> = oxidized::aast::Def<Ty<R>, SavedEnv>;
pub type Expr<R> = oxidized::aast::Expr<Ty<R>, SavedEnv>;
pub type Expr_<R> = oxidized::aast::Expr_<Ty<R>, SavedEnv>;
pub type Stmt<R> = oxidized::aast::Stmt<Ty<R>, SavedEnv>;
pub type Stmt_<R> = oxidized::aast::Stmt_<Ty<R>, SavedEnv>;
pub type Block<R> = oxidized::aast::Block<Ty<R>, SavedEnv>;
pub type Class_<R> = oxidized::aast::Class_<Ty<R>, SavedEnv>;
pub type ClassId<R> = oxidized::aast::ClassId<Ty<R>, SavedEnv>;
pub type TypeHint<R> = oxidized::aast::TypeHint<Ty<R>>;
pub type Targ<R> = oxidized::aast::Targ<Ty<R>>;
pub type ClassGetExpr<R> = oxidized::aast::ClassGetExpr<Ty<R>, SavedEnv>;
pub type ClassTypeconstDef<R> = oxidized::aast::ClassTypeconstDef<Ty<R>, SavedEnv>;
pub type UserAttribute<R> = oxidized::aast::UserAttribute<Ty<R>, SavedEnv>;
pub type Fun_<R> = oxidized::aast::Fun_<Ty<R>, SavedEnv>;
pub type FileAttribute<R> = oxidized::aast::FileAttribute<Ty<R>, SavedEnv>;
pub type FunDef<R> = oxidized::aast::FunDef<Ty<R>, SavedEnv>;
pub type FunParam<R> = oxidized::aast::FunParam<Ty<R>, SavedEnv>;
pub type FuncBody<R> = oxidized::aast::FuncBody<Ty<R>, SavedEnv>;
pub type Method_<R> = oxidized::aast::Method_<Ty<R>, SavedEnv>;
pub type ClassVar<R> = oxidized::aast::ClassVar<Ty<R>, SavedEnv>;
pub type ClassConst<R> = oxidized::aast::ClassConst<Ty<R>, SavedEnv>;
pub type Tparam<R> = oxidized::aast::Tparam<Ty<R>, SavedEnv>;
pub type Typedef<R> = oxidized::aast::Typedef<Ty<R>, SavedEnv>;
pub type Gconst<R> = oxidized::aast::Gconst<Ty<R>, SavedEnv>;

impl ToOcamlRep for SavedEnv {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        let bump = Bump::new();
        let saved_env = oxidized_by_ref::tast::SavedEnv {
            tcopt: oxidized_by_ref::global_options::GlobalOptions::default_ref(),
            inference_env: bump.alloc(oxidized_by_ref::typing_inference_env::TypingInferenceEnv {
                tvenv: Default::default(),
                tyvars_stack: &[],
                subtype_prop: bump.alloc(oxidized_by_ref::typing_logic::SubtypeProp::Disj(&[])),
                tyvar_occurrences: bump.alloc(
                    oxidized_by_ref::typing_tyvar_occurrences::TypingTyvarOccurrences {
                        tyvar_occurrences: Default::default(),
                        tyvars_in_tyvar: Default::default(),
                    },
                ),
                allow_solve_globals: false,
            }),
            tpenv: bump.alloc(oxidized_by_ref::type_parameter_env::TypeParameterEnv {
                tparams: Default::default(),
                consistent: false,
            }),
            condition_types: Default::default(),
            pessimize: false,
            fun_tast_info: None,
        };
        saved_env.to_ocamlrep(alloc)
    }
}
