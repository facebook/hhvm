// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::marker::PhantomData;

use bumpalo::Bump;
use ocamlrep::Allocator;
use ocamlrep::OpaqueValue;
use ocamlrep::ToOcamlRep;
use oxidized::aast;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::Params;
use oxidized::aast_visitor::VisitorMut;
use pos::ToOxidized;
use ty::local;
use ty::local::Ty;
use ty::reason::Reason;

use crate::inference_env::InferenceEnv;

#[derive(Clone, Debug)]
pub struct SavedEnv<R: Reason> {
    pub inf_env: InferenceEnv<R>,
}

pub type Program<R> = aast::Program<Ty<R>, SavedEnv<R>>;
pub type Def<R> = aast::Def<Ty<R>, SavedEnv<R>>;
pub type Expr<R> = aast::Expr<Ty<R>, SavedEnv<R>>;
pub type Expr_<R> = aast::Expr_<Ty<R>, SavedEnv<R>>;
pub type Stmt<R> = aast::Stmt<Ty<R>, SavedEnv<R>>;
pub type Stmt_<R> = aast::Stmt_<Ty<R>, SavedEnv<R>>;
pub type Block<R> = aast::Block<Ty<R>, SavedEnv<R>>;
pub type Class_<R> = aast::Class_<Ty<R>, SavedEnv<R>>;
pub type ClassId<R> = aast::ClassId<Ty<R>, SavedEnv<R>>;
pub type TypeHint<R> = aast::TypeHint<Ty<R>>;
pub type Targ<R> = aast::Targ<Ty<R>>;
pub type ClassGetExpr<R> = aast::ClassGetExpr<Ty<R>, SavedEnv<R>>;
pub type ClassTypeconstDef<R> = aast::ClassTypeconstDef<Ty<R>, SavedEnv<R>>;
pub type UserAttribute<R> = aast::UserAttribute<Ty<R>, SavedEnv<R>>;
pub type Fun_<R> = aast::Fun_<Ty<R>, SavedEnv<R>>;
pub type FileAttribute<R> = aast::FileAttribute<Ty<R>, SavedEnv<R>>;
pub type FunDef<R> = aast::FunDef<Ty<R>, SavedEnv<R>>;
pub type FunParam<R> = aast::FunParam<Ty<R>, SavedEnv<R>>;
pub type FuncBody<R> = aast::FuncBody<Ty<R>, SavedEnv<R>>;
pub type Method_<R> = aast::Method_<Ty<R>, SavedEnv<R>>;
pub type ClassVar<R> = aast::ClassVar<Ty<R>, SavedEnv<R>>;
pub type ClassConst<R> = aast::ClassConst<Ty<R>, SavedEnv<R>>;
pub type Tparam<R> = aast::Tparam<Ty<R>, SavedEnv<R>>;
pub type Typedef<R> = aast::Typedef<Ty<R>, SavedEnv<R>>;
pub type Gconst<R> = aast::Gconst<Ty<R>, SavedEnv<R>>;

impl<R: Reason> ToOcamlRep for SavedEnv<R> {
    fn to_ocamlrep<'a, A: Allocator>(&'a self, alloc: &'a A) -> OpaqueValue<'a> {
        // This implementation of `to_ocamlrep` (which allocates in an arena,
        // converts to OCaml, then drops the arena) violates a `ToOcamlRep`
        // requirement: we may not drop values after passing them to `alloc.add`
        // or invoking `to_ocamlrep`. See comment on impl of ToOcamlRep for Ty.
        // We must take care not to use `ocamlrep::Allocator::add_root` on
        // values containing this type.
        let bump = Bump::new();
        let SavedEnv { inf_env } = self;
        let saved_env = oxidized_by_ref::tast::SavedEnv {
            tcopt: oxidized_by_ref::global_options::GlobalOptions::default_ref(),
            inference_env: bump.alloc(inf_env.to_oxidized(&bump)),
            tpenv: bump.alloc(oxidized_by_ref::type_parameter_env::TypeParameterEnv {
                tparams: Default::default(),
                consistent: false,
            }),
            condition_types: Default::default(),
            fun_tast_info: None,
        };
        // SAFETY: Transmute away the lifetime to allow the arena-allocated
        // value to be converted to OCaml. Won't break type safety in Rust, but
        // will produce broken OCaml values if used with `add_root` (see comment
        // on impl of ToOcamlRep for Ty).
        let saved_env = unsafe {
            std::mem::transmute::<
                &'_ oxidized_by_ref::tast::SavedEnv<'_>,
                &'a oxidized_by_ref::tast::SavedEnv<'a>,
            >(&saved_env)
        };
        saved_env.to_ocamlrep(alloc)
    }
}

pub struct TastExpander<R: Reason> {
    env: Option<SavedEnv<R>>,
}

impl<R: Reason> TastExpander<R> {
    fn new() -> Self {
        Self { env: None }
    }

    pub fn expand_program(program: &mut Program<R>) {
        Self::new().visit_program(&mut (), program).unwrap()
    }

    pub fn set_env(&mut self, env: SavedEnv<R>) {
        self.env = Some(env);
    }

    pub fn env(&mut self) -> &mut SavedEnv<R> {
        self.env.as_mut().unwrap()
    }
    fn exp_ty_mut(&mut self, ty: &mut Ty<R>) {
        *ty = self.env().inf_env.resolve_ty(ty);

        use local::Ty_::*;
        let r = ty.reason().clone();
        match &**ty {
            Tnonnull | Tvar(..) | Tprim(..) | Tany => {}
            Tfun(ft) => *ty = Ty::fun(r, self.exp_fun_type(ft)),
            Tgeneric(n, args) => *ty = Ty::generic(r, *n, self.exp_tys(args)),
            Tclass(n, e, args) => *ty = Ty::class(r, n.clone(), *e, self.exp_tys(args)),
            Tunion(args) => *ty = Ty::union(r, self.exp_tys(args)),
            Tintersection(args) => *ty = Ty::intersection(r, self.exp_tys(args)),
            Toption(arg) => *ty = Ty::option(r, self.exp_ty(arg)),
        }
    }

    #[must_use]
    fn exp_ty(&mut self, ty: &Ty<R>) -> Ty<R> {
        let mut ty = ty.clone();
        self.exp_ty_mut(&mut ty);
        ty
    }

    fn exp_tys(&mut self, tys: &[Ty<R>]) -> Vec<Ty<R>> {
        tys.iter().map(|ty| self.exp_ty(ty)).collect()
    }

    fn exp_fun_type(&mut self, ft: &local::FunType<R>) -> local::FunType<R> {
        let mut ft = ft.clone();
        let local::FunType {
            tparams: _,
            params,
            ret,
            flags: _,
        } = &mut ft;
        params.iter_mut().for_each(|p| self.exp_fun_param_mut(p));
        self.exp_ty_mut(ret);
        ft
    }

    fn exp_fun_param_mut(&mut self, fp: &mut local::FunParam<R>) {
        self.exp_ty_mut(&mut fp.ty);
    }
}

pub struct TastExpanderParams<R>(PhantomData<R>);

impl<R: Reason> Params for TastExpanderParams<R> {
    type Context = ();
    type Error = ();
    type Ex = Ty<R>;
    type En = SavedEnv<R>;
}

impl<'node, R: Reason> VisitorMut<'node> for TastExpander<R> {
    type Params = TastExpanderParams<R>;

    fn object(&mut self) -> &mut dyn VisitorMut<'node, Params = Self::Params> {
        self
    }

    fn visit_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut aast::Expr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        self.exp_ty_mut(&mut p.0);
        p.recurse(c, self.object())
    }

    fn visit_class_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut aast::ClassId<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        self.exp_ty_mut(&mut p.0);
        p.recurse(c, self.object())
    }

    fn visit_ex(
        &mut self,
        _c: &mut <Self::Params as Params>::Context,
        p: &'node mut <Self::Params as Params>::Ex,
    ) -> Result<(), <Self::Params as Params>::Error> {
        self.exp_ty_mut(p);
        Ok(())
    }

    fn visit_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut aast::Def<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        let env = match p {
            aast::Def::Fun(box f) => f.fun.annotation.clone(),
            aast::Def::Class(box c) => c.annotation.clone(),
            p => rupro_todo!(AST, "{:?}", p),
        };
        self.set_env(env);
        p.recurse(c, self.object())
    }

    fn visit_fun_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut aast::Fun_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        self.set_env(p.annotation.clone());
        p.recurse(c, self.object())
    }

    fn visit_method_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut aast::Method_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        self.set_env(p.annotation.clone());
        p.recurse(c, self.object())
    }
}
