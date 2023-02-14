// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<87493ca3bbe5fde273e2fb21388f64a0>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use std::ops::ControlFlow::Break;

use oxidized::aast_defs::*;
use oxidized::ast_defs::*;
mod pass;
pub use pass::Pass;
pub use pass::Passes;
pub trait Transform<Cfg, Err> {
    #[inline(always)]
    fn transform(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        self.traverse(cfg, errs, pass);
    }
    #[inline(always)]
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
    }
}
impl<Cfg, Err> Transform<Cfg, Err> for () {}
impl<Cfg, Err> Transform<Cfg, Err> for bool {}
impl<Cfg, Err> Transform<Cfg, Err> for isize {}
impl<Cfg, Err> Transform<Cfg, Err> for String {}
impl<Cfg, Err> Transform<Cfg, Err> for bstr::BString {}
impl<Cfg, Err> Transform<Cfg, Err> for oxidized::pos::Pos {}
impl<Cfg, Err> Transform<Cfg, Err> for oxidized::file_info::Mode {}
impl<Cfg, Err> Transform<Cfg, Err> for oxidized::namespace_env::Env {}
impl<Cfg, Err, Ex> Transform<Cfg, Err> for oxidized::LocalIdMap<(Pos, Ex)> {}
impl<Cfg, Err, T> Transform<Cfg, Err> for &mut T
where
    T: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        (**self).transform(cfg, errs, &mut pass.clone())
    }
}
impl<Cfg, Err, T> Transform<Cfg, Err> for Box<T>
where
    T: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        (**self).transform(cfg, errs, &mut pass.clone())
    }
}
impl<Cfg, Err, L, R> Transform<Cfg, Err> for itertools::Either<L, R>
where
    L: Transform<Cfg, Err>,
    R: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        match self {
            Self::Left(x) => x.transform(cfg, errs, &mut pass.clone()),
            Self::Right(x) => x.transform(cfg, errs, &mut pass.clone()),
        }
    }
}
impl<Cfg, Err, T> Transform<Cfg, Err> for Vec<T>
where
    T: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        for x in self.iter_mut() {
            x.transform(cfg, errs, &mut pass.clone());
        }
    }
}
impl<Cfg, Err, T> Transform<Cfg, Err> for Option<T>
where
    T: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        match self {
            Some(x) => x.transform(cfg, errs, &mut pass.clone()),
            None => {}
        }
    }
}
impl<Cfg, Err, T> Transform<Cfg, Err> for oxidized::lazy::Lazy<T>
where
    T: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        self.0.transform(cfg, errs, &mut pass.clone())
    }
}
impl<Cfg, Err, K, V> Transform<Cfg, Err> for std::collections::BTreeMap<K, V>
where
    K: Transform<Cfg, Err>,
    V: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        for x in self.values_mut() {
            x.transform(cfg, errs, &mut pass.clone());
        }
    }
}
impl<Cfg, Err, T> Transform<Cfg, Err> for ocamlrep::rc::RcOc<T>
where
    T: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        if let Some(x) = ocamlrep::rc::RcOc::get_mut(self) {
            x.transform(cfg, errs, &mut pass.clone());
        }
    }
}
impl<Cfg, Err, T> Transform<Cfg, Err> for std::rc::Rc<T>
where
    T: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        if let Some(x) = std::rc::Rc::get_mut(self) {
            x.transform(cfg, errs, &mut pass.clone());
        }
    }
}
impl<Cfg, Err, T1, T2> Transform<Cfg, Err> for (T1, T2)
where
    T1: Transform<Cfg, Err>,
    T2: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        self.0.transform(cfg, errs, &mut pass.clone());
        self.1.transform(cfg, errs, &mut pass.clone());
    }
}
impl<Cfg, Err, T1, T2, T3> Transform<Cfg, Err> for (T1, T2, T3)
where
    T1: Transform<Cfg, Err>,
    T2: Transform<Cfg, Err>,
    T3: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        self.0.transform(cfg, errs, &mut pass.clone());
        self.1.transform(cfg, errs, &mut pass.clone());
        self.2.transform(cfg, errs, &mut pass.clone());
    }
}
impl<Cfg, Err, T1, T2, T3, T4> Transform<Cfg, Err> for (T1, T2, T3, T4)
where
    T1: Transform<Cfg, Err>,
    T2: Transform<Cfg, Err>,
    T3: Transform<Cfg, Err>,
    T4: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        self.0.transform(cfg, errs, &mut pass.clone());
        self.1.transform(cfg, errs, &mut pass.clone());
        self.2.transform(cfg, errs, &mut pass.clone());
        self.3.transform(cfg, errs, &mut pass.clone());
    }
}
impl<Cfg, Err, T1, T2, T3, T4, T5> Transform<Cfg, Err> for (T1, T2, T3, T4, T5)
where
    T1: Transform<Cfg, Err>,
    T2: Transform<Cfg, Err>,
    T3: Transform<Cfg, Err>,
    T4: Transform<Cfg, Err>,
    T5: Transform<Cfg, Err>,
{
    fn traverse(
        &mut self,
        cfg: &Cfg,
        errs: &mut Vec<Err>,
        pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
    ) {
        self.0.transform(cfg, errs, &mut pass.clone());
        self.1.transform(cfg, errs, &mut pass.clone());
        self.2.transform(cfg, errs, &mut pass.clone());
        self.3.transform(cfg, errs, &mut pass.clone());
        self.4.transform(cfg, errs, &mut pass.clone());
    }
}
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Lid {}
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Program<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_program_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_program_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Program(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Stmt<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_stmt_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_stmt_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Stmt(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Stmt_<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_stmt__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_stmt__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Stmt_::Fallthrough => {}
                Stmt_::Expr(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Break => {}
                Stmt_::Continue => {}
                Stmt_::Throw(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Return(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::YieldBreak => {}
                Stmt_::Awaitall(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::If(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Do(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::While(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Using(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::For(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Switch(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Foreach(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Try(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Stmt_::Noop => {}
                Stmt_::Block(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                _ => {}
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for EnvAnnot {}
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for UsingStmt<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_using_stmt_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_using_stmt_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                UsingStmt {
                    is_block_scoped: ref mut __binding_0,
                    has_await: ref mut __binding_1,
                    exprs: ref mut __binding_2,
                    block: ref mut __binding_3,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    { __binding_3.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for AsExpr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_as_expr_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_as_expr_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                AsExpr::AsV(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                AsExpr::AsKv(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                AsExpr::AwaitAsV(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                AsExpr::AwaitAsKv(
                    ref mut __binding_0,
                    ref mut __binding_1,
                    ref mut __binding_2,
                ) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Block<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_block_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_block_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Block(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ClassId<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_id_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_id_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassId(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ClassId_<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_id__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_id__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassId_::CIparent => {}
                ClassId_::CIself => {}
                ClassId_::CIstatic => {}
                ClassId_::CIexpr(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                ClassId_::CI(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Expr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_expr_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_expr_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Expr(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex> Transform<Cfg, Err> for CollectionTarg<Ex>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_collection_targ_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_collection_targ_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                CollectionTarg::CollectionTV(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                CollectionTarg::CollectionTKV(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for FunctionPtrId<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_function_ptr_id_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_function_ptr_id_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                FunctionPtrId::FPId(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                FunctionPtrId::FPClassConst(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ExpressionTree<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_expression_tree_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_expression_tree_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ExpressionTree {
                    hint: ref mut __binding_0,
                    splices: ref mut __binding_1,
                    function_pointers: ref mut __binding_2,
                    virtualized_expr: ref mut __binding_3,
                    runtime_expr: ref mut __binding_4,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    { __binding_4.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Expr_<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_expr__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_expr__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Expr_::Darray(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Varray(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Shape(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ValCollection(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::KeyValCollection(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                Expr_::Null => {}
                Expr_::This => {}
                Expr_::True => {}
                Expr_::False => {}
                Expr_::Omitted => {}
                Expr_::Invalid(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Id(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Lvar(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Dollardollar(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Clone(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ArrayGet(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ObjGet(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ClassGet(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ClassConst(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Call(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::FunctionPointer(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                Expr_::Int(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Float(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::String2(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::PrefixedString(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                Expr_::Yield(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Await(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ReadonlyExpr(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Tuple(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::List(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Cast(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Unop(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Binop(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Pipe(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Eif(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Is(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::As(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Upcast(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::New(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Efun(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Lfun(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Xml(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Import(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Collection(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ExpressionTree(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                Expr_::FunId(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::MethodId(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::MethodCaller(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::SmethodId(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::Pair(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::ETSplice(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Expr_::EnumClassLabel(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                Expr_::Hole(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                _ => {}
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for HoleSource {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_hole_source_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_hole_source_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                HoleSource::Typing => {}
                HoleSource::UnsafeCast(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                HoleSource::UnsafeNonnullCast => {}
                HoleSource::EnforcedCast(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ClassGetExpr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_get_expr_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_get_expr_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassGetExpr::CGstring(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                ClassGetExpr::CGexpr(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Case<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_case_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_case_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Case(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for DefaultCase<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_default_case_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_default_case_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                DefaultCase(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Catch<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_catch_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_catch_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Catch(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Field<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_field_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_field_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Field(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Afield<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_afield_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_afield_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Afield::AFvalue(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Afield::AFkvalue(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for XhpSimple<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_xhp_simple_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_xhp_simple_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                XhpSimple {
                    type_: ref mut __binding_1,
                    expr: ref mut __binding_2,
                    ..
                } => {
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for XhpAttribute<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_xhp_attribute_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_xhp_attribute_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                XhpAttribute::XhpSimple(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                XhpAttribute::XhpSpread(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for FunParam<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_fun_param_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_fun_param_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                FunParam {
                    annotation: ref mut __binding_0,
                    type_hint: ref mut __binding_1,
                    is_variadic: ref mut __binding_2,
                    name: ref mut __binding_4,
                    expr: ref mut __binding_5,
                    user_attributes: ref mut __binding_8,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_4.transform(cfg, errs, pass)
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    { __binding_8.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Fun_<Ex, En>
    where
        Ex: Default,
        En: Transform<Cfg, Err>,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_fun__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_fun__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Fun_ {
                    annotation: ref mut __binding_2,
                    ret: ref mut __binding_4,
                    tparams: ref mut __binding_5,
                    where_constraints: ref mut __binding_6,
                    params: ref mut __binding_7,
                    ctxs: ref mut __binding_8,
                    unsafe_ctxs: ref mut __binding_9,
                    body: ref mut __binding_10,
                    user_attributes: ref mut __binding_12,
                    external: ref mut __binding_13,
                    doc_comment: ref mut __binding_14,
                    ..
                } => {
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_fun__ret_top_down(__binding_4, cfg, errs) {
                            return;
                        }
                        __binding_4.transform(cfg, errs, pass);
                        in_pass.on_fld_fun__ret_bottom_up(__binding_4, cfg, errs);
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    {
                        __binding_6.transform(cfg, errs, pass)
                    }
                    {
                        __binding_7.transform(cfg, errs, pass)
                    }
                    {
                        __binding_8.transform(cfg, errs, pass)
                    }
                    {
                        __binding_9.transform(cfg, errs, pass)
                    }
                    {
                        __binding_10.transform(cfg, errs, pass)
                    }
                    {
                        __binding_12.transform(cfg, errs, pass)
                    }
                    {
                        __binding_13.transform(cfg, errs, pass)
                    }
                    { __binding_14.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Efun<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_efun_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_efun_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Efun {
                    fun: ref mut __binding_0,
                    use_: ref mut __binding_1,
                    closure_class_name: ref mut __binding_2,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for FuncBody<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_func_body_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_func_body_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                FuncBody {
                    fb_ast: ref mut __binding_0,
                } => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex> Transform<Cfg, Err> for TypeHint<Ex>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_type_hint_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_type_hint_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                TypeHint(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex> Transform<Cfg, Err> for Targ<Ex>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_targ_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_targ_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Targ(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for UserAttribute<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_user_attribute_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_user_attribute_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                UserAttribute {
                    name: ref mut __binding_0,
                    params: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for FileAttribute<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_file_attribute_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_file_attribute_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                FileAttribute {
                    user_attributes: ref mut __binding_0,
                    namespace: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Tparam<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_tparam_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_tparam_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Tparam {
                    name: ref mut __binding_1,
                    parameters: ref mut __binding_2,
                    constraints: ref mut __binding_3,
                    user_attributes: ref mut __binding_5,
                    ..
                } => {
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    { __binding_5.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for RequireKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for EmitId {}
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Class_<Ex, En>
    where
        Ex: Default,
        En: Transform<Cfg, Err>,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Class_ {
                    annotation: ref mut __binding_1,
                    final_: ref mut __binding_3,
                    is_xhp: ref mut __binding_4,
                    has_xhp_keyword: ref mut __binding_5,
                    name: ref mut __binding_7,
                    tparams: ref mut __binding_8,
                    extends: ref mut __binding_9,
                    uses: ref mut __binding_10,
                    xhp_attr_uses: ref mut __binding_11,
                    reqs: ref mut __binding_13,
                    implements: ref mut __binding_14,
                    where_constraints: ref mut __binding_15,
                    consts: ref mut __binding_16,
                    typeconsts: ref mut __binding_17,
                    vars: ref mut __binding_18,
                    methods: ref mut __binding_19,
                    xhp_children: ref mut __binding_20,
                    xhp_attrs: ref mut __binding_21,
                    namespace: ref mut __binding_22,
                    user_attributes: ref mut __binding_23,
                    file_attributes: ref mut __binding_24,
                    docs_url: ref mut __binding_25,
                    enum_: ref mut __binding_26,
                    doc_comment: ref mut __binding_27,
                    emit_id: ref mut __binding_28,
                    internal: ref mut __binding_29,
                    module: ref mut __binding_30,
                    ..
                } => {
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    {
                        __binding_4.transform(cfg, errs, pass)
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    {
                        __binding_7.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__tparams_top_down(__binding_8, cfg, errs)
                        {
                            return;
                        }
                        __binding_8.transform(cfg, errs, pass);
                        in_pass.on_fld_class__tparams_bottom_up(__binding_8, cfg, errs);
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__extends_top_down(__binding_9, cfg, errs)
                        {
                            return;
                        }
                        __binding_9.transform(cfg, errs, pass);
                        in_pass.on_fld_class__extends_bottom_up(__binding_9, cfg, errs);
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__uses_top_down(__binding_10, cfg, errs)
                        {
                            return;
                        }
                        __binding_10.transform(cfg, errs, pass);
                        in_pass.on_fld_class__uses_bottom_up(__binding_10, cfg, errs);
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__xhp_attr_uses_top_down(__binding_11, cfg, errs)
                        {
                            return;
                        }
                        __binding_11.transform(cfg, errs, pass);
                        in_pass.on_fld_class__xhp_attr_uses_bottom_up(__binding_11, cfg, errs);
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__reqs_top_down(__binding_13, cfg, errs)
                        {
                            return;
                        }
                        __binding_13.transform(cfg, errs, pass);
                        in_pass.on_fld_class__reqs_bottom_up(__binding_13, cfg, errs);
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__implements_top_down(__binding_14, cfg, errs)
                        {
                            return;
                        }
                        __binding_14.transform(cfg, errs, pass);
                        in_pass.on_fld_class__implements_bottom_up(__binding_14, cfg, errs);
                    }
                    {
                        __binding_15.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__consts_top_down(__binding_16, cfg, errs)
                        {
                            return;
                        }
                        __binding_16.transform(cfg, errs, pass);
                        in_pass.on_fld_class__consts_bottom_up(__binding_16, cfg, errs);
                    }
                    {
                        __binding_17.transform(cfg, errs, pass)
                    }
                    {
                        __binding_18.transform(cfg, errs, pass)
                    }
                    {
                        __binding_19.transform(cfg, errs, pass)
                    }
                    {
                        __binding_20.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__xhp_attrs_top_down(__binding_21, cfg, errs)
                        {
                            return;
                        }
                        __binding_21.transform(cfg, errs, pass);
                        in_pass.on_fld_class__xhp_attrs_bottom_up(__binding_21, cfg, errs);
                    }
                    {
                        __binding_22.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__user_attributes_top_down(__binding_23, cfg, errs)
                        {
                            return;
                        }
                        __binding_23.transform(cfg, errs, pass);
                        in_pass.on_fld_class__user_attributes_bottom_up(__binding_23, cfg, errs);
                    }
                    {
                        __binding_24.transform(cfg, errs, pass)
                    }
                    {
                        __binding_25.transform(cfg, errs, pass)
                    }
                    {
                        __binding_26.transform(cfg, errs, pass)
                    }
                    {
                        __binding_27.transform(cfg, errs, pass)
                    }
                    {
                        __binding_28.transform(cfg, errs, pass)
                    }
                    {
                        __binding_29.transform(cfg, errs, pass)
                    }
                    { __binding_30.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for XhpAttrTag {}
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for XhpAttr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_xhp_attr_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_xhp_attr_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                XhpAttr(
                    ref mut __binding_0,
                    ref mut __binding_1,
                    ref mut __binding_2,
                    ref mut __binding_3,
                ) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    { __binding_3.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ClassConstKind<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_const_kind_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_const_kind_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassConstKind::CCAbstract(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                ClassConstKind::CCConcrete(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ClassConst<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_const_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_const_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassConst {
                    user_attributes: ref mut __binding_0,
                    type_: ref mut __binding_1,
                    id: ref mut __binding_2,
                    kind: ref mut __binding_3,
                    doc_comment: ref mut __binding_5,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    { __binding_5.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ClassAbstractTypeconst {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_abstract_typeconst_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_abstract_typeconst_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassAbstractTypeconst {
                    as_constraint: ref mut __binding_0,
                    super_constraint: ref mut __binding_1,
                    default: ref mut __binding_2,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ClassConcreteTypeconst {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_concrete_typeconst_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_concrete_typeconst_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassConcreteTypeconst {
                    c_tc_type: ref mut __binding_0,
                } => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ClassTypeconst {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_typeconst_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_typeconst_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassTypeconst::TCAbstract(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                ClassTypeconst::TCConcrete(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ClassTypeconstDef<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_typeconst_def_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_typeconst_def_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassTypeconstDef {
                    user_attributes: ref mut __binding_0,
                    name: ref mut __binding_1,
                    kind: ref mut __binding_2,
                    doc_comment: ref mut __binding_4,
                    is_ctx: ref mut __binding_5,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_4.transform(cfg, errs, pass)
                    }
                    { __binding_5.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for XhpAttrInfo {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_xhp_attr_info_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_xhp_attr_info_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                XhpAttrInfo {
                    tag: ref mut __binding_1,
                    ..
                } => __binding_1.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ClassVar<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_class_var_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_class_var_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ClassVar {
                    final_: ref mut __binding_0,
                    xhp_attr: ref mut __binding_1,
                    abstract_: ref mut __binding_2,
                    readonly: ref mut __binding_3,
                    type_: ref mut __binding_5,
                    id: ref mut __binding_6,
                    expr: ref mut __binding_7,
                    user_attributes: ref mut __binding_8,
                    doc_comment: ref mut __binding_9,
                    is_promoted_variadic: ref mut __binding_10,
                    is_static: ref mut __binding_11,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class_var_type__top_down(__binding_5, cfg, errs)
                        {
                            return;
                        }
                        __binding_5.transform(cfg, errs, pass);
                        in_pass.on_fld_class_var_type__bottom_up(__binding_5, cfg, errs);
                    }
                    {
                        __binding_6.transform(cfg, errs, pass)
                    }
                    {
                        __binding_7.transform(cfg, errs, pass)
                    }
                    {
                        __binding_8.transform(cfg, errs, pass)
                    }
                    {
                        __binding_9.transform(cfg, errs, pass)
                    }
                    {
                        __binding_10.transform(cfg, errs, pass)
                    }
                    { __binding_11.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Method_<Ex, En>
    where
        Ex: Default,
        En: Transform<Cfg, Err>,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_method__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_method__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Method_ {
                    annotation: ref mut __binding_1,
                    final_: ref mut __binding_2,
                    abstract_: ref mut __binding_3,
                    static_: ref mut __binding_4,
                    readonly_this: ref mut __binding_5,
                    name: ref mut __binding_7,
                    tparams: ref mut __binding_8,
                    where_constraints: ref mut __binding_9,
                    params: ref mut __binding_10,
                    ctxs: ref mut __binding_11,
                    unsafe_ctxs: ref mut __binding_12,
                    body: ref mut __binding_13,
                    user_attributes: ref mut __binding_15,
                    ret: ref mut __binding_17,
                    external: ref mut __binding_18,
                    doc_comment: ref mut __binding_19,
                    ..
                } => {
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    {
                        __binding_4.transform(cfg, errs, pass)
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    {
                        __binding_7.transform(cfg, errs, pass)
                    }
                    {
                        __binding_8.transform(cfg, errs, pass)
                    }
                    {
                        __binding_9.transform(cfg, errs, pass)
                    }
                    {
                        __binding_10.transform(cfg, errs, pass)
                    }
                    {
                        __binding_11.transform(cfg, errs, pass)
                    }
                    {
                        __binding_12.transform(cfg, errs, pass)
                    }
                    {
                        __binding_13.transform(cfg, errs, pass)
                    }
                    {
                        __binding_15.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_method__ret_top_down(__binding_17, cfg, errs)
                        {
                            return;
                        }
                        __binding_17.transform(cfg, errs, pass);
                        in_pass.on_fld_method__ret_bottom_up(__binding_17, cfg, errs);
                    }
                    {
                        __binding_18.transform(cfg, errs, pass)
                    }
                    { __binding_19.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Typedef<Ex, En>
    where
        Ex: Default,
        En: Transform<Cfg, Err>,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_typedef_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_typedef_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Typedef {
                    annotation: ref mut __binding_0,
                    name: ref mut __binding_1,
                    tparams: ref mut __binding_2,
                    as_constraint: ref mut __binding_3,
                    super_constraint: ref mut __binding_4,
                    kind: ref mut __binding_5,
                    user_attributes: ref mut __binding_6,
                    file_attributes: ref mut __binding_7,
                    namespace: ref mut __binding_10,
                    emit_id: ref mut __binding_12,
                    is_ctx: ref mut __binding_13,
                    internal: ref mut __binding_14,
                    module: ref mut __binding_15,
                    docs_url: ref mut __binding_16,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    {
                        __binding_4.transform(cfg, errs, pass)
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    {
                        __binding_6.transform(cfg, errs, pass)
                    }
                    {
                        __binding_7.transform(cfg, errs, pass)
                    }
                    {
                        __binding_10.transform(cfg, errs, pass)
                    }
                    {
                        __binding_12.transform(cfg, errs, pass)
                    }
                    {
                        __binding_13.transform(cfg, errs, pass)
                    }
                    {
                        __binding_14.transform(cfg, errs, pass)
                    }
                    {
                        __binding_15.transform(cfg, errs, pass)
                    }
                    { __binding_16.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Gconst<Ex, En>
    where
        Ex: Default,
        En: Transform<Cfg, Err>,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_gconst_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_gconst_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Gconst {
                    annotation: ref mut __binding_0,
                    name: ref mut __binding_2,
                    type_: ref mut __binding_3,
                    value: ref mut __binding_4,
                    namespace: ref mut __binding_5,
                    emit_id: ref mut __binding_7,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_gconst_value_top_down(__binding_4, cfg, errs)
                        {
                            return;
                        }
                        __binding_4.transform(cfg, errs, pass);
                        in_pass.on_fld_gconst_value_bottom_up(__binding_4, cfg, errs);
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    { __binding_7.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for FunDef<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_fun_def_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_fun_def_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                FunDef {
                    namespace: ref mut __binding_0,
                    file_attributes: ref mut __binding_1,
                    name: ref mut __binding_3,
                    fun: ref mut __binding_4,
                    internal: ref mut __binding_5,
                    module: ref mut __binding_6,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    {
                        __binding_4.transform(cfg, errs, pass)
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    { __binding_6.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for ModuleDef<Ex, En>
    where
        Ex: Default,
        En: Transform<Cfg, Err>,
        Ex: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_module_def_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_module_def_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ModuleDef {
                    annotation: ref mut __binding_0,
                    user_attributes: ref mut __binding_2,
                    doc_comment: ref mut __binding_5,
                    exports: ref mut __binding_6,
                    imports: ref mut __binding_7,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_5.transform(cfg, errs, pass)
                    }
                    {
                        __binding_6.transform(cfg, errs, pass)
                    }
                    { __binding_7.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for PackageDef<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_package_def_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_package_def_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                PackageDef {
                    name: ref mut __binding_0,
                    user_attributes: ref mut __binding_1,
                    uses: ref mut __binding_2,
                    includes: ref mut __binding_3,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    { __binding_3.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for MdNameKind {}
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for Def<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_def_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_def_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Def::Fun(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::Class(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::Stmt(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::Typedef(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::Constant(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::Namespace(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::NamespaceUse(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::SetNamespaceEnv(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::FileAttributes(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::Module(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::Package(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Def::SetModule(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for NsKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ImportFlavor {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for XhpChild {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_xhp_child_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_xhp_child_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                XhpChild::ChildName(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                XhpChild::ChildList(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                XhpChild::ChildUnary(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                XhpChild::ChildBinary(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for XhpChildOp {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Hint {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_hint_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_hint_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Hint(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err, Ex, En> Transform<Cfg, Err> for UserAttributes<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Cfg, Err>,
        En: Transform<Cfg, Err>,
    {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_user_attributes_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_user_attributes_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                UserAttributes(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Contexts {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_contexts_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_contexts_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Contexts(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for HfParamInfo {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for HintFun {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_hint_fun_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_hint_fun_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                HintFun {
                    param_tys: ref mut __binding_1,
                    param_info: ref mut __binding_2,
                    variadic_ty: ref mut __binding_3,
                    ctxs: ref mut __binding_4,
                    return_ty: ref mut __binding_5,
                    ..
                } => {
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    {
                        __binding_2.transform(cfg, errs, pass)
                    }
                    {
                        __binding_3.transform(cfg, errs, pass)
                    }
                    {
                        __binding_4.transform(cfg, errs, pass)
                    }
                    {
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_hint_fun_return_ty_top_down(__binding_5, cfg, errs)
                        {
                            return;
                        }
                        __binding_5.transform(cfg, errs, pass);
                        in_pass.on_fld_hint_fun_return_ty_bottom_up(__binding_5, cfg, errs);
                    }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Hint_ {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_hint__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_hint__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Hint_::Hoption(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Hlike(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Hfun(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Htuple(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Happly(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                Hint_::Hshape(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Haccess(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                Hint_::Hsoft(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Hrefinement(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                Hint_::Hany => {}
                Hint_::Herr => {}
                Hint_::Hmixed => {}
                Hint_::Hnonnull => {}
                Hint_::Habstr(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                Hint_::HvecOrDict(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                Hint_::Hprim(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Hthis => {}
                Hint_::Hdynamic => {}
                Hint_::Hnothing => {}
                Hint_::Hunion(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Hintersection(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::HfunContext(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
                Hint_::Hvar(ref mut __binding_0) => __binding_0.transform(cfg, errs, pass),
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Refinement {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_refinement_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_refinement_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Refinement::Rctx(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
                Refinement::Rtype(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for TypeRefinement {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_type_refinement_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_type_refinement_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                TypeRefinement::TRexact(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                TypeRefinement::TRloose(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for TypeRefinementBounds {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_type_refinement_bounds_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_type_refinement_bounds_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                TypeRefinementBounds {
                    lower: ref mut __binding_0,
                    upper: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for CtxRefinement {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_ctx_refinement_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_ctx_refinement_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                CtxRefinement::CRexact(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
                CtxRefinement::CRloose(ref mut __binding_0) => {
                    __binding_0.transform(cfg, errs, pass)
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for CtxRefinementBounds {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_ctx_refinement_bounds_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_ctx_refinement_bounds_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                CtxRefinementBounds {
                    lower: ref mut __binding_0,
                    upper: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ShapeFieldInfo {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_shape_field_info_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_shape_field_info_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                ShapeFieldInfo {
                    optional: ref mut __binding_0,
                    hint: ref mut __binding_1,
                    ..
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for NastShapeInfo {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_nast_shape_info_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_nast_shape_info_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                NastShapeInfo {
                    allows_unknown_fields: ref mut __binding_0,
                    field_map: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for KvcKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for VcKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Enum_ {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_enum__top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_enum__bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Enum_ {
                    base: ref mut __binding_0,
                    constraint: ref mut __binding_1,
                    includes: ref mut __binding_2,
                } => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for WhereConstraintHint {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_where_constraint_hint_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_where_constraint_hint_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                WhereConstraintHint(
                    ref mut __binding_0,
                    ref mut __binding_1,
                    ref mut __binding_2,
                ) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    {
                        __binding_1.transform(cfg, errs, pass)
                    }
                    { __binding_2.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Id {
        fn transform(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            let mut in_pass = pass.clone();
            if let Break(..) = pass.on_ty_id_top_down(self, cfg, errs) {
                return;
            }
            self.traverse(cfg, errs, pass);
            in_pass.on_ty_id_bottom_up(self, cfg, errs);
        }
        fn traverse(
            &mut self,
            cfg: &Cfg,
            errs: &mut Vec<Err>,
            pass: &mut (impl Pass<Cfg = Cfg, Err = Err> + Clone),
        ) {
            match self {
                Id(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(cfg, errs, pass)
                    }
                    { __binding_1.transform(cfg, errs, pass) }
                }
            }
        }
    }
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ShapeFieldName {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Variance {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ConstraintKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Abstraction {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ClassishKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ParamKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ReadonlyKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for OgNullFlavor {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for PropOrMethod {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for FunKind {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Bop {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Uop {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Visibility {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for XhpEnumValue {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for Tprim {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for TypedefVisibility {}
};
const _: () = {
    impl<Cfg, Err> Transform<Cfg, Err> for ReifyKind {}
};
