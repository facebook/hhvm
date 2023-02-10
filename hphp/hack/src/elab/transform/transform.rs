// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<84101817d995b58574f7b2215ac7e181>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use std::ops::ControlFlow::Break;

use oxidized::aast_defs::*;
use oxidized::ast_defs::*;
mod pass;
pub use pass::Pass;
pub trait Transform<Ctx: Clone, Err> {
    #[inline(always)]
    fn transform(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        self.traverse(ctx, errs, top_down, bottom_up);
    }
    #[inline(always)]
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
    }
}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for () {}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for bool {}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for isize {}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for String {}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for bstr::BString {}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for oxidized::pos::Pos {}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for oxidized::file_info::Mode {}
impl<Ctx: Clone, Err> Transform<Ctx, Err> for oxidized::namespace_env::Env {}
impl<Ctx: Clone, Err, Ex> Transform<Ctx, Err> for oxidized::LocalIdMap<(Pos, Ex)> {}
impl<Ctx, Err, T> Transform<Ctx, Err> for &mut T
where
    Ctx: Clone,
    T: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        (**self).transform(&mut ctx.clone(), errs, top_down, bottom_up)
    }
}
impl<Ctx, Err, T> Transform<Ctx, Err> for Box<T>
where
    Ctx: Clone,
    T: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        (**self).transform(&mut ctx.clone(), errs, top_down, bottom_up)
    }
}
impl<Ctx, Err, L, R> Transform<Ctx, Err> for itertools::Either<L, R>
where
    Ctx: Clone,
    L: Transform<Ctx, Err>,
    R: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        match self {
            Self::Left(x) => x.transform(&mut ctx.clone(), errs, top_down, bottom_up),
            Self::Right(x) => x.transform(&mut ctx.clone(), errs, top_down, bottom_up),
        }
    }
}
impl<Ctx, Err, T> Transform<Ctx, Err> for Vec<T>
where
    Ctx: Clone,
    T: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        for x in self.iter_mut() {
            x.transform(&mut ctx.clone(), errs, top_down, bottom_up);
        }
    }
}
impl<Ctx, Err, T> Transform<Ctx, Err> for Option<T>
where
    Ctx: Clone,
    T: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        match self {
            Some(x) => x.transform(&mut ctx.clone(), errs, top_down, bottom_up),
            None => {}
        }
    }
}
impl<Ctx, Err, T> Transform<Ctx, Err> for oxidized::lazy::Lazy<T>
where
    Ctx: Clone,
    T: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        self.0
            .transform(&mut ctx.clone(), errs, top_down, bottom_up)
    }
}
impl<Ctx, Err, K, V> Transform<Ctx, Err> for std::collections::BTreeMap<K, V>
where
    Ctx: Clone,
    K: Transform<Ctx, Err>,
    V: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        for x in self.values_mut() {
            x.transform(&mut ctx.clone(), errs, top_down, bottom_up);
        }
    }
}
impl<Ctx, Err, T> Transform<Ctx, Err> for ocamlrep::rc::RcOc<T>
where
    Ctx: Clone,
    T: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        if let Some(x) = ocamlrep::rc::RcOc::get_mut(self) {
            x.transform(&mut ctx.clone(), errs, top_down, bottom_up);
        }
    }
}
impl<Ctx, Err, T> Transform<Ctx, Err> for std::rc::Rc<T>
where
    Ctx: Clone,
    T: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        if let Some(x) = std::rc::Rc::get_mut(self) {
            x.transform(&mut ctx.clone(), errs, top_down, bottom_up);
        }
    }
}
impl<Ctx, Err, T1, T2> Transform<Ctx, Err> for (T1, T2)
where
    Ctx: Clone,
    T1: Transform<Ctx, Err>,
    T2: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        self.0
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.1
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
    }
}
impl<Ctx, Err, T1, T2, T3> Transform<Ctx, Err> for (T1, T2, T3)
where
    Ctx: Clone,
    T1: Transform<Ctx, Err>,
    T2: Transform<Ctx, Err>,
    T3: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        self.0
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.1
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.2
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
    }
}
impl<Ctx, Err, T1, T2, T3, T4> Transform<Ctx, Err> for (T1, T2, T3, T4)
where
    Ctx: Clone,
    T1: Transform<Ctx, Err>,
    T2: Transform<Ctx, Err>,
    T3: Transform<Ctx, Err>,
    T4: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        self.0
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.1
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.2
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.3
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
    }
}
impl<Ctx, Err, T1, T2, T3, T4, T5> Transform<Ctx, Err> for (T1, T2, T3, T4, T5)
where
    Ctx: Clone,
    T1: Transform<Ctx, Err>,
    T2: Transform<Ctx, Err>,
    T3: Transform<Ctx, Err>,
    T4: Transform<Ctx, Err>,
    T5: Transform<Ctx, Err>,
{
    fn traverse(
        &mut self,
        ctx: &mut Ctx,
        errs: &mut Vec<Err>,
        top_down: &impl Pass<Ctx = Ctx, Err = Err>,
        bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
    ) {
        self.0
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.1
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.2
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.3
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
        self.4
            .transform(&mut ctx.clone(), errs, top_down, bottom_up);
    }
}
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Lid {}
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Program<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_program(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_program(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Program(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Stmt<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_stmt(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_stmt(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Stmt(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Stmt_<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_stmt_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_stmt_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Stmt_::Fallthrough => {}
                Stmt_::Expr(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Break => {}
                Stmt_::Continue => {}
                Stmt_::Throw(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Return(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::YieldBreak => {}
                Stmt_::Awaitall(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::If(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Do(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::While(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Using(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::For(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Switch(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Foreach(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Try(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Stmt_::Noop => {}
                Stmt_::Block(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                _ => {}
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for EnvAnnot {}
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for UsingStmt<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_using_stmt(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_using_stmt(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                UsingStmt {
                    is_block_scoped: ref mut __binding_0,
                    has_await: ref mut __binding_1,
                    exprs: ref mut __binding_2,
                    block: ref mut __binding_3,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_3.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for AsExpr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_as_expr(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_as_expr(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                AsExpr::AsV(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                AsExpr::AsKv(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                AsExpr::AwaitAsV(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                AsExpr::AwaitAsKv(
                    ref mut __binding_0,
                    ref mut __binding_1,
                    ref mut __binding_2,
                ) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Block<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_block(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_block(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Block(ref mut __binding_0) => __binding_0.transform(ctx, errs, top_down, bottom_up),
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ClassId<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_id(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_id(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ClassId(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ClassId_<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_id_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_id_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ClassId_::CIparent => {}
                ClassId_::CIself => {}
                ClassId_::CIstatic => {}
                ClassId_::CIexpr(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                ClassId_::CI(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Expr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_expr(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_expr(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Expr(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex> Transform<Ctx, Err> for CollectionTarg<Ex>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_collection_targ(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_collection_targ(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                CollectionTarg::CollectionTV(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                CollectionTarg::CollectionTKV(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for FunctionPtrId<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_function_ptr_id(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_function_ptr_id(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                FunctionPtrId::FPId(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                FunctionPtrId::FPClassConst(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ExpressionTree<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_expression_tree(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_expression_tree(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_4.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Expr_<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_expr_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_expr_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Expr_::Darray(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Varray(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Shape(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ValCollection(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::KeyValCollection(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Null => {}
                Expr_::This => {}
                Expr_::True => {}
                Expr_::False => {}
                Expr_::Omitted => {}
                Expr_::Invalid(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Id(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Lvar(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Dollardollar(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Clone(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ArrayGet(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ObjGet(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ClassGet(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ClassConst(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Call(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::FunctionPointer(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Int(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Float(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::String2(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::PrefixedString(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Yield(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Await(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ReadonlyExpr(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Tuple(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::List(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Cast(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Unop(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Binop(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Pipe(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Eif(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Is(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::As(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Upcast(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::New(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Efun(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Lfun(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Xml(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Import(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Collection(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ExpressionTree(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::FunId(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::MethodId(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::MethodCaller(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::SmethodId(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Pair(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::ETSplice(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::EnumClassLabel(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Expr_::Hole(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                _ => {}
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for HoleSource {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_hole_source(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_hole_source(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                HoleSource::Typing => {}
                HoleSource::UnsafeCast(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                HoleSource::UnsafeNonnullCast => {}
                HoleSource::EnforcedCast(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ClassGetExpr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_get_expr(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_get_expr(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ClassGetExpr::CGstring(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                ClassGetExpr::CGexpr(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Case<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_case(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_case(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Case(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for DefaultCase<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_default_case(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_default_case(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                DefaultCase(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Catch<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_catch(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_catch(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Catch(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Field<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_field(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_field(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Field(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Afield<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_afield(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_afield(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Afield::AFvalue(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Afield::AFkvalue(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for XhpSimple<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_xhp_simple(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_xhp_simple(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                XhpSimple {
                    type_: ref mut __binding_1,
                    expr: ref mut __binding_2,
                    ..
                } => {
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for XhpAttribute<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_xhp_attribute(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_xhp_attribute(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                XhpAttribute::XhpSimple(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                XhpAttribute::XhpSpread(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for FunParam<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_fun_param(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_fun_param(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_4.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_8.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Fun_<Ex, En>
    where
        Ex: Default,
        En: Transform<Ctx, Err>,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_fun_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_fun_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_fun__ret(__binding_4, ctx, errs) {
                            return;
                        }
                        __binding_4.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_fun__ret(__binding_4, &mut in_ctx, errs);
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_6.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_7.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_8.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_9.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_10.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_12.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_13.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_14.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Efun<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_efun(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_efun(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Efun {
                    fun: ref mut __binding_0,
                    use_: ref mut __binding_1,
                    closure_class_name: ref mut __binding_2,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for FuncBody<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_func_body(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_func_body(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                FuncBody {
                    fb_ast: ref mut __binding_0,
                } => __binding_0.transform(ctx, errs, top_down, bottom_up),
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex> Transform<Ctx, Err> for TypeHint<Ex>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_type_hint(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_type_hint(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                TypeHint(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex> Transform<Ctx, Err> for Targ<Ex>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_targ(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_targ(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Targ(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for UserAttribute<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_user_attribute(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_user_attribute(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                UserAttribute {
                    name: ref mut __binding_0,
                    params: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for FileAttribute<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_file_attribute(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_file_attribute(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                FileAttribute {
                    user_attributes: ref mut __binding_0,
                    namespace: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Tparam<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_tparam(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_tparam(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_5.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for RequireKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for EmitId {}
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Class_<Ex, En>
    where
        Ex: Default,
        En: Transform<Ctx, Err>,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_4.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_7.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_class__tparams(__binding_8, ctx, errs) {
                            return;
                        }
                        __binding_8.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__tparams(__binding_8, &mut in_ctx, errs);
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_class__extends(__binding_9, ctx, errs) {
                            return;
                        }
                        __binding_9.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__extends(__binding_9, &mut in_ctx, errs);
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_class__uses(__binding_10, ctx, errs) {
                            return;
                        }
                        __binding_10.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__uses(__binding_10, &mut in_ctx, errs);
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) =
                            top_down.on_fld_class__xhp_attr_uses(__binding_11, ctx, errs)
                        {
                            return;
                        }
                        __binding_11.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__xhp_attr_uses(__binding_11, &mut in_ctx, errs);
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_class__reqs(__binding_13, ctx, errs) {
                            return;
                        }
                        __binding_13.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__reqs(__binding_13, &mut in_ctx, errs);
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) =
                            top_down.on_fld_class__implements(__binding_14, ctx, errs)
                        {
                            return;
                        }
                        __binding_14.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__implements(__binding_14, &mut in_ctx, errs);
                    }
                    {
                        __binding_15.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_class__consts(__binding_16, ctx, errs) {
                            return;
                        }
                        __binding_16.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__consts(__binding_16, &mut in_ctx, errs);
                    }
                    {
                        __binding_17.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_18.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_19.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_20.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_class__xhp_attrs(__binding_21, ctx, errs)
                        {
                            return;
                        }
                        __binding_21.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__xhp_attrs(__binding_21, &mut in_ctx, errs);
                    }
                    {
                        __binding_22.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) =
                            top_down.on_fld_class__user_attributes(__binding_23, ctx, errs)
                        {
                            return;
                        }
                        __binding_23.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class__user_attributes(__binding_23, &mut in_ctx, errs);
                    }
                    {
                        __binding_24.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_25.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_26.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_27.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_28.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_29.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_30.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for XhpAttrTag {}
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for XhpAttr<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_xhp_attr(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_xhp_attr(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                XhpAttr(
                    ref mut __binding_0,
                    ref mut __binding_1,
                    ref mut __binding_2,
                    ref mut __binding_3,
                ) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_3.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ClassConstKind<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_const_kind(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_const_kind(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ClassConstKind::CCAbstract(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                ClassConstKind::CCConcrete(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ClassConst<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_const(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_const(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_5.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ClassAbstractTypeconst {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_abstract_typeconst(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_abstract_typeconst(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ClassAbstractTypeconst {
                    as_constraint: ref mut __binding_0,
                    super_constraint: ref mut __binding_1,
                    default: ref mut __binding_2,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ClassConcreteTypeconst {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_concrete_typeconst(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_concrete_typeconst(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ClassConcreteTypeconst {
                    c_tc_type: ref mut __binding_0,
                } => __binding_0.transform(ctx, errs, top_down, bottom_up),
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ClassTypeconst {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_typeconst(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_typeconst(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ClassTypeconst::TCAbstract(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                ClassTypeconst::TCConcrete(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ClassTypeconstDef<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_typeconst_def(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_typeconst_def(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_4.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_5.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for XhpAttrInfo {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_xhp_attr_info(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_xhp_attr_info(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                XhpAttrInfo {
                    tag: ref mut __binding_1,
                    ..
                } => __binding_1.transform(ctx, errs, top_down, bottom_up),
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ClassVar<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_class_var(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_class_var(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_class_var_type_(__binding_5, ctx, errs) {
                            return;
                        }
                        __binding_5.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_class_var_type_(__binding_5, &mut in_ctx, errs);
                    }
                    {
                        __binding_6.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_7.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_8.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_9.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_10.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_11.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Method_<Ex, En>
    where
        Ex: Default,
        En: Transform<Ctx, Err>,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_method_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_method_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_4.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_7.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_8.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_9.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_10.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_11.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_12.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_13.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_15.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_method__ret(__binding_17, ctx, errs) {
                            return;
                        }
                        __binding_17.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_method__ret(__binding_17, &mut in_ctx, errs);
                    }
                    {
                        __binding_18.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_19.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Typedef<Ex, En>
    where
        Ex: Default,
        En: Transform<Ctx, Err>,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_typedef(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_typedef(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_4.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_6.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_7.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_10.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_12.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_13.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_14.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_15.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_16.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Gconst<Ex, En>
    where
        Ex: Default,
        En: Transform<Ctx, Err>,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_gconst(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_gconst(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) = top_down.on_fld_gconst_value(__binding_4, ctx, errs) {
                            return;
                        }
                        __binding_4.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_gconst_value(__binding_4, &mut in_ctx, errs);
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_7.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for FunDef<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_fun_def(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_fun_def(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_4.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_6.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for ModuleDef<Ex, En>
    where
        Ex: Default,
        En: Transform<Ctx, Err>,
        Ex: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_module_def(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_module_def(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_5.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_6.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_7.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for MdNameKind {}
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for Def<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_def(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_def(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Def::Fun(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::Class(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::Stmt(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::Typedef(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::Constant(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::Namespace(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::NamespaceUse(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::SetNamespaceEnv(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::FileAttributes(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::Module(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Def::SetModule(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for NsKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ImportFlavor {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for XhpChild {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_xhp_child(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_xhp_child(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                XhpChild::ChildName(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                XhpChild::ChildList(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                XhpChild::ChildUnary(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                XhpChild::ChildBinary(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for XhpChildOp {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Hint {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_hint(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_hint(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Hint(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err, Ex, En> Transform<Ctx, Err> for UserAttributes<Ex, En>
    where
        Ex: Default,
        Ex: Transform<Ctx, Err>,
        En: Transform<Ctx, Err>,
    {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_user_attributes(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_user_attributes(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                UserAttributes(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Contexts {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_contexts(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_contexts(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Contexts(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for HfParamInfo {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for HintFun {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_hint_fun(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_hint_fun(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
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
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_2.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_3.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_4.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        let mut in_ctx = ctx.clone();
                        if let Break(..) =
                            top_down.on_fld_hint_fun_return_ty(__binding_5, ctx, errs)
                        {
                            return;
                        }
                        __binding_5.transform(ctx, errs, top_down, bottom_up);
                        bottom_up.on_fld_hint_fun_return_ty(__binding_5, &mut in_ctx, errs);
                    }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Hint_ {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_hint_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_hint_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Hint_::Hoption(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Hlike(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Hfun(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Htuple(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Happly(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                Hint_::Hshape(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Haccess(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                Hint_::Hsoft(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Hrefinement(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                Hint_::Hany => {}
                Hint_::Herr => {}
                Hint_::Hmixed => {}
                Hint_::Hnonnull => {}
                Hint_::Habstr(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                Hint_::HvecOrDict(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                Hint_::Hprim(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Hthis => {}
                Hint_::Hdynamic => {}
                Hint_::Hnothing => {}
                Hint_::Hunion(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Hintersection(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::HfunContext(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                Hint_::Hvar(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Refinement {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_refinement(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_refinement(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Refinement::Rctx(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
                Refinement::Rtype(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for TypeRefinement {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_type_refinement(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_type_refinement(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                TypeRefinement::TRexact(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                TypeRefinement::TRloose(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for TypeRefinementBounds {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_type_refinement_bounds(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_type_refinement_bounds(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                TypeRefinementBounds {
                    lower: ref mut __binding_0,
                    upper: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for CtxRefinement {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_ctx_refinement(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_ctx_refinement(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                CtxRefinement::CRexact(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
                CtxRefinement::CRloose(ref mut __binding_0) => {
                    __binding_0.transform(ctx, errs, top_down, bottom_up)
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for CtxRefinementBounds {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_ctx_refinement_bounds(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_ctx_refinement_bounds(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                CtxRefinementBounds {
                    lower: ref mut __binding_0,
                    upper: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ShapeFieldInfo {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_shape_field_info(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_shape_field_info(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                ShapeFieldInfo {
                    optional: ref mut __binding_0,
                    hint: ref mut __binding_1,
                    ..
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for NastShapeInfo {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_nast_shape_info(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_nast_shape_info(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                NastShapeInfo {
                    allows_unknown_fields: ref mut __binding_0,
                    field_map: ref mut __binding_1,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for KvcKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for VcKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Enum_ {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_enum_(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_enum_(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Enum_ {
                    base: ref mut __binding_0,
                    constraint: ref mut __binding_1,
                    includes: ref mut __binding_2,
                } => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for WhereConstraintHint {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_where_constraint_hint(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_where_constraint_hint(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                WhereConstraintHint(
                    ref mut __binding_0,
                    ref mut __binding_1,
                    ref mut __binding_2,
                ) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    {
                        __binding_1.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_2.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Id {
        fn transform(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            let mut in_ctx = ctx.clone();
            if let Break(..) = top_down.on_ty_id(self, ctx, errs) {
                return;
            }
            self.traverse(ctx, errs, top_down, bottom_up);
            bottom_up.on_ty_id(self, &mut in_ctx, errs);
        }
        fn traverse(
            &mut self,
            ctx: &mut Ctx,
            errs: &mut Vec<Err>,
            top_down: &impl Pass<Ctx = Ctx, Err = Err>,
            bottom_up: &impl Pass<Ctx = Ctx, Err = Err>,
        ) {
            match self {
                Id(ref mut __binding_0, ref mut __binding_1) => {
                    {
                        __binding_0.transform(ctx, errs, top_down, bottom_up)
                    }
                    { __binding_1.transform(ctx, errs, top_down, bottom_up) }
                }
            }
        }
    }
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ShapeFieldName {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Variance {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ConstraintKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Abstraction {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ClassishKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ParamKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ReadonlyKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for OgNullFlavor {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for PropOrMethod {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for FunKind {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Bop {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Uop {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Visibility {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for XhpEnumValue {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for Tprim {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for TypedefVisibility {}
};
const _: () = {
    impl<Ctx: Clone, Err> Transform<Ctx, Err> for ReifyKind {}
};
