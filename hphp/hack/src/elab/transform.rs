// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8f90f081c28763ba5924735ea4370fee>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
#![allow(unused_braces)]
#![allow(clippy::match_single_binding)]
use std::ops::ControlFlow::Break;

use oxidized::nast::*;

use crate::env::Env;
use crate::Pass;
pub trait Transform {
    #[inline(always)]
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        stack_limit::maybe_grow(|| self.traverse(env, pass));
    }
    #[inline(always)]
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {}
}
impl Transform for () {}
impl Transform for bool {}
impl Transform for isize {}
impl Transform for String {}
impl Transform for bstr::BString {}
impl Transform for oxidized::pos::Pos {}
impl Transform for oxidized::file_info::Mode {}
impl Transform for oxidized::namespace_env::Env {}
impl Transform for oxidized::LocalIdMap<(Pos, ())> {}
impl<T> Transform for &mut T
where
    T: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        (**self).transform(env, &mut pass.clone())
    }
}
impl<T> Transform for Box<T>
where
    T: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        (**self).transform(env, &mut pass.clone())
    }
}
impl<L, R> Transform for itertools::Either<L, R>
where
    L: Transform,
    R: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Self::Left(x) => x.transform(env, &mut pass.clone()),
            Self::Right(x) => x.transform(env, &mut pass.clone()),
        }
    }
}
impl<T> Transform for Vec<T>
where
    T: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        for x in self.iter_mut() {
            x.transform(env, &mut pass.clone());
        }
    }
}
impl<T> Transform for Option<T>
where
    T: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Some(x) => x.transform(env, &mut pass.clone()),
            None => {}
        }
    }
}
impl<T> Transform for oxidized::lazy::Lazy<T>
where
    T: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        self.0.transform(env, &mut pass.clone())
    }
}
impl<K, V> Transform for std::collections::BTreeMap<K, V>
where
    K: Transform,
    V: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        for x in self.values_mut() {
            x.transform(env, &mut pass.clone());
        }
    }
}
impl<T> Transform for std::sync::Arc<T>
where
    T: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        if let Some(x) = std::sync::Arc::get_mut(self) {
            x.transform(env, &mut pass.clone());
        }
    }
}
impl<T> Transform for std::rc::Rc<T>
where
    T: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        if let Some(x) = std::rc::Rc::get_mut(self) {
            x.transform(env, &mut pass.clone());
        }
    }
}
impl<T1, T2> Transform for (T1, T2)
where
    T1: Transform,
    T2: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        self.0.transform(env, &mut pass.clone());
        self.1.transform(env, &mut pass.clone());
    }
}
impl<T1, T2, T3> Transform for (T1, T2, T3)
where
    T1: Transform,
    T2: Transform,
    T3: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        self.0.transform(env, &mut pass.clone());
        self.1.transform(env, &mut pass.clone());
        self.2.transform(env, &mut pass.clone());
    }
}
impl<T1, T2, T3, T4> Transform for (T1, T2, T3, T4)
where
    T1: Transform,
    T2: Transform,
    T3: Transform,
    T4: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        self.0.transform(env, &mut pass.clone());
        self.1.transform(env, &mut pass.clone());
        self.2.transform(env, &mut pass.clone());
        self.3.transform(env, &mut pass.clone());
    }
}
impl<T1, T2, T3, T4, T5> Transform for (T1, T2, T3, T4, T5)
where
    T1: Transform,
    T2: Transform,
    T3: Transform,
    T4: Transform,
    T5: Transform,
{
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        self.0.transform(env, &mut pass.clone());
        self.1.transform(env, &mut pass.clone());
        self.2.transform(env, &mut pass.clone());
        self.3.transform(env, &mut pass.clone());
        self.4.transform(env, &mut pass.clone());
    }
}
const _: () = {
    impl Transform for Lid {}
};
impl Transform for Program {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_program_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_program_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Program(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for Stmt {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_stmt_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_stmt_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Stmt(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Stmt_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_stmt__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_stmt__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Stmt_::Noop => {}
            Stmt_::Fallthrough => {}
            Stmt_::Expr(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Break => {}
            Stmt_::Continue => {}
            Stmt_::Throw(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Return(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::YieldBreak => {}
            Stmt_::Awaitall(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Concurrent(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::If(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Do(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::While(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Using(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::For(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Switch(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Match(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Foreach(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::Try(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Stmt_::DeclareLocal(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Stmt_::Block(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            _ => {}
        }
    }
}
const _: () = {
    impl Transform for EnvAnnot {}
};
impl Transform for UsingStmt {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_using_stmt_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_using_stmt_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            UsingStmt {
                is_block_scoped: ref mut __binding_0,
                has_await: ref mut __binding_1,
                exprs: ref mut __binding_2,
                block: ref mut __binding_3,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                { __binding_3.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for AsExpr {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_as_expr_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_as_expr_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            AsExpr::AsV(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            AsExpr::AsKv(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            AsExpr::AwaitAsV(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            AsExpr::AwaitAsKv(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Block {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_block_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_block_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Block(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for FinallyBlock {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_finally_block_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_finally_block_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            FinallyBlock(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for StmtMatch {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_stmt_match_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_stmt_match_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            StmtMatch {
                expr: ref mut __binding_0,
                arms: ref mut __binding_1,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for StmtMatchArm {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_stmt_match_arm_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_stmt_match_arm_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            StmtMatchArm {
                pat: ref mut __binding_0,
                body: ref mut __binding_1,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Pattern {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_pattern_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_pattern_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Pattern::PVar(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Pattern::PRefinement(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for PatVar {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_pat_var_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_pat_var_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            PatVar {
                id: ref mut __binding_1,
                ..
            } => __binding_1.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for PatRefinement {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_pat_refinement_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_pat_refinement_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            PatRefinement {
                id: ref mut __binding_1,
                hint: ref mut __binding_2,
                ..
            } => {
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ClassId {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_id_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_id_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassId(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ClassId_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_id__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_id__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassId_::CIparent => {}
            ClassId_::CIself => {}
            ClassId_::CIstatic => {}
            ClassId_::CIexpr(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            ClassId_::CI(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for Expr {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_expr_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_expr_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Expr(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for CollectionTarg {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_collection_targ_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_collection_targ_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            CollectionTarg::CollectionTV(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            CollectionTarg::CollectionTKV(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for FunctionPtrId {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_function_ptr_id_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_function_ptr_id_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            FunctionPtrId::FPId(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            FunctionPtrId::FPClassConst(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ExpressionTree {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_expression_tree_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_expression_tree_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                { __binding_4.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Expr_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_expr__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_expr__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Expr_::Null => {}
            Expr_::True => {}
            Expr_::False => {}
            Expr_::Darray(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Varray(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Shape(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ValCollection(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::KeyValCollection(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::This => {}
            Expr_::Omitted => {}
            Expr_::Invalid(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Id(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Lvar(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Dollardollar(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::Clone(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ArrayGet(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ObjGet(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ClassGet(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ClassConst(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Call(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::FunctionPointer(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::Int(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Float(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::String2(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::PrefixedString(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::Yield(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Await(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ReadonlyExpr(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::Tuple(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::List(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Cast(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Unop(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Binop(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Pipe(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Eif(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Is(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::As(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Upcast(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::New(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Efun(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Lfun(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Xml(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Import(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Collection(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ExpressionTree(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::MethodCaller(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::Pair(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::ETSplice(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::EnumClassLabel(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Expr_::Hole(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Package(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Expr_::Nameof(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            _ => {}
        }
    }
}
impl Transform for HoleSource {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_hole_source_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_hole_source_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            HoleSource::Typing => {}
            HoleSource::UnsafeCast(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            HoleSource::UnsafeNonnullCast => {}
            HoleSource::EnforcedCast(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for Binop {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_binop_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_binop_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Binop {
                lhs: ref mut __binding_1,
                rhs: ref mut __binding_2,
                ..
            } => {
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_binop_lhs_top_down(env, __binding_1) {
                            return;
                        }
                        __binding_1.transform(env, &mut pass.clone());
                        in_pass.on_fld_binop_lhs_bottom_up(env, __binding_1);
                    }
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_binop_rhs_top_down(env, __binding_2) {
                            return;
                        }
                        __binding_2.transform(env, &mut pass.clone());
                        in_pass.on_fld_binop_rhs_bottom_up(env, __binding_2);
                    }
                }
            }
        }
    }
}
impl Transform for ClassGetExpr {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_get_expr_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_get_expr_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassGetExpr::CGstring(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            ClassGetExpr::CGexpr(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for Case {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_case_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_case_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Case(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for DefaultCase {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_default_case_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_default_case_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            DefaultCase(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Catch {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_catch_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_catch_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Catch(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Field {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_field_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_field_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Field(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Afield {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_afield_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_afield_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Afield::AFvalue(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Afield::AFkvalue(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for XhpSimple {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_xhp_simple_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_xhp_simple_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            XhpSimple {
                type_: ref mut __binding_1,
                expr: ref mut __binding_2,
                ..
            } => {
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for XhpAttribute {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_xhp_attribute_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_xhp_attribute_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            XhpAttribute::XhpSimple(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            XhpAttribute::XhpSpread(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for FunParam {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_fun_param_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_fun_param_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_4.transform(env, &mut pass.clone())
                }
                {
                    __binding_5.transform(env, &mut pass.clone())
                }
                { __binding_8.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Fun_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_fun__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_fun__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Fun_ {
                annotation: ref mut __binding_2,
                ret: ref mut __binding_4,
                params: ref mut __binding_5,
                ctxs: ref mut __binding_6,
                unsafe_ctxs: ref mut __binding_7,
                body: ref mut __binding_8,
                user_attributes: ref mut __binding_10,
                external: ref mut __binding_11,
                doc_comment: ref mut __binding_12,
                ..
            } => {
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_fun__ret_top_down(env, __binding_4) {
                            return;
                        }
                        __binding_4.transform(env, &mut pass.clone());
                        in_pass.on_fld_fun__ret_bottom_up(env, __binding_4);
                    }
                }
                {
                    __binding_5.transform(env, &mut pass.clone())
                }
                {
                    __binding_6.transform(env, &mut pass.clone())
                }
                {
                    __binding_7.transform(env, &mut pass.clone())
                }
                {
                    __binding_8.transform(env, &mut pass.clone())
                }
                {
                    __binding_10.transform(env, &mut pass.clone())
                }
                {
                    __binding_11.transform(env, &mut pass.clone())
                }
                { __binding_12.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for CaptureLid {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_capture_lid_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_capture_lid_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            CaptureLid(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Efun {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_efun_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_efun_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Efun {
                fun: ref mut __binding_0,
                use_: ref mut __binding_1,
                closure_class_name: ref mut __binding_2,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for FuncBody {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_func_body_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_func_body_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            FuncBody {
                fb_ast: ref mut __binding_0,
            } => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for TypeHint {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_type_hint_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_type_hint_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            TypeHint(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Targ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_targ_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_targ_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Targ(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for CallExpr {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_call_expr_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_call_expr_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            CallExpr {
                func: ref mut __binding_0,
                targs: ref mut __binding_1,
                args: ref mut __binding_2,
                unpacked_arg: ref mut __binding_3,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                { __binding_3.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for UserAttribute {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_user_attribute_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_user_attribute_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            UserAttribute {
                name: ref mut __binding_0,
                params: ref mut __binding_1,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for FileAttribute {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_file_attribute_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_file_attribute_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            FileAttribute {
                user_attributes: ref mut __binding_0,
                namespace: ref mut __binding_1,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Tparam {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_tparam_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_tparam_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Tparam {
                name: ref mut __binding_1,
                parameters: ref mut __binding_2,
                constraints: ref mut __binding_3,
                user_attributes: ref mut __binding_5,
                ..
            } => {
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                { __binding_5.transform(env, &mut pass.clone()) }
            }
        }
    }
}
const _: () = {
    impl Transform for RequireKind {}
};
const _: () = {
    impl Transform for EmitId {}
};
impl Transform for Class_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    __binding_4.transform(env, &mut pass.clone())
                }
                {
                    __binding_5.transform(env, &mut pass.clone())
                }
                {
                    __binding_7.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__tparams_top_down(env, __binding_8) {
                            return;
                        }
                        __binding_8.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__tparams_bottom_up(env, __binding_8);
                    }
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__extends_top_down(env, __binding_9) {
                            return;
                        }
                        __binding_9.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__extends_bottom_up(env, __binding_9);
                    }
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__uses_top_down(env, __binding_10) {
                            return;
                        }
                        __binding_10.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__uses_bottom_up(env, __binding_10);
                    }
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__xhp_attr_uses_top_down(env, __binding_11)
                        {
                            return;
                        }
                        __binding_11.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__xhp_attr_uses_bottom_up(env, __binding_11);
                    }
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__reqs_top_down(env, __binding_13) {
                            return;
                        }
                        __binding_13.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__reqs_bottom_up(env, __binding_13);
                    }
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__implements_top_down(env, __binding_14)
                        {
                            return;
                        }
                        __binding_14.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__implements_bottom_up(env, __binding_14);
                    }
                }
                {
                    __binding_15.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__consts_top_down(env, __binding_16) {
                            return;
                        }
                        __binding_16.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__consts_bottom_up(env, __binding_16);
                    }
                }
                {
                    __binding_17.transform(env, &mut pass.clone())
                }
                {
                    __binding_18.transform(env, &mut pass.clone())
                }
                {
                    __binding_19.transform(env, &mut pass.clone())
                }
                {
                    __binding_20.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class__xhp_attrs_top_down(env, __binding_21)
                        {
                            return;
                        }
                        __binding_21.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__xhp_attrs_bottom_up(env, __binding_21);
                    }
                }
                {
                    __binding_22.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class__user_attributes_top_down(env, __binding_23)
                        {
                            return;
                        }
                        __binding_23.transform(env, &mut pass.clone());
                        in_pass.on_fld_class__user_attributes_bottom_up(env, __binding_23);
                    }
                }
                {
                    __binding_24.transform(env, &mut pass.clone())
                }
                {
                    __binding_25.transform(env, &mut pass.clone())
                }
                {
                    __binding_26.transform(env, &mut pass.clone())
                }
                {
                    __binding_27.transform(env, &mut pass.clone())
                }
                {
                    __binding_28.transform(env, &mut pass.clone())
                }
                {
                    __binding_29.transform(env, &mut pass.clone())
                }
                { __binding_30.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ClassReq {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_req_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_req_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassReq(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
const _: () = {
    impl Transform for XhpAttrTag {}
};
impl Transform for XhpAttr {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_xhp_attr_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_xhp_attr_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            XhpAttr(
                ref mut __binding_0,
                ref mut __binding_1,
                ref mut __binding_2,
                ref mut __binding_3,
            ) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                { __binding_3.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ClassConstKind {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_const_kind_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_const_kind_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassConstKind::CCAbstract(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            ClassConstKind::CCConcrete(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for ClassConst {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_const_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_const_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                { __binding_5.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ClassAbstractTypeconst {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_abstract_typeconst_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_abstract_typeconst_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassAbstractTypeconst {
                as_constraint: ref mut __binding_0,
                super_constraint: ref mut __binding_1,
                default: ref mut __binding_2,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) =
                            pass.on_fld_class_abstract_typeconst_default_top_down(env, __binding_2)
                        {
                            return;
                        }
                        __binding_2.transform(env, &mut pass.clone());
                        in_pass.on_fld_class_abstract_typeconst_default_bottom_up(env, __binding_2);
                    }
                }
            }
        }
    }
}
impl Transform for ClassConcreteTypeconst {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_concrete_typeconst_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_concrete_typeconst_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassConcreteTypeconst {
                c_tc_type: ref mut __binding_0,
            } => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for ClassTypeconst {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_typeconst_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_typeconst_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ClassTypeconst::TCAbstract(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            ClassTypeconst::TCConcrete(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for ClassTypeconstDef {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_typeconst_def_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_typeconst_def_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_4.transform(env, &mut pass.clone())
                }
                { __binding_5.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for XhpAttrInfo {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_xhp_attr_info_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_xhp_attr_info_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            XhpAttrInfo {
                tag: ref mut __binding_1,
                ..
            } => __binding_1.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for ClassVar {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_class_var_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_class_var_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_class_var_type__top_down(env, __binding_5) {
                            return;
                        }
                        __binding_5.transform(env, &mut pass.clone());
                        in_pass.on_fld_class_var_type__bottom_up(env, __binding_5);
                    }
                }
                {
                    __binding_6.transform(env, &mut pass.clone())
                }
                {
                    __binding_7.transform(env, &mut pass.clone())
                }
                {
                    __binding_8.transform(env, &mut pass.clone())
                }
                {
                    __binding_9.transform(env, &mut pass.clone())
                }
                {
                    __binding_10.transform(env, &mut pass.clone())
                }
                { __binding_11.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Method_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_method__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_method__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    __binding_4.transform(env, &mut pass.clone())
                }
                {
                    __binding_5.transform(env, &mut pass.clone())
                }
                {
                    __binding_7.transform(env, &mut pass.clone())
                }
                {
                    __binding_8.transform(env, &mut pass.clone())
                }
                {
                    __binding_9.transform(env, &mut pass.clone())
                }
                {
                    __binding_10.transform(env, &mut pass.clone())
                }
                {
                    __binding_11.transform(env, &mut pass.clone())
                }
                {
                    __binding_12.transform(env, &mut pass.clone())
                }
                {
                    __binding_13.transform(env, &mut pass.clone())
                }
                {
                    __binding_15.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_method__ret_top_down(env, __binding_17) {
                            return;
                        }
                        __binding_17.transform(env, &mut pass.clone());
                        in_pass.on_fld_method__ret_bottom_up(env, __binding_17);
                    }
                }
                {
                    __binding_18.transform(env, &mut pass.clone())
                }
                { __binding_19.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Typedef {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_typedef_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_typedef_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                doc_comment: ref mut __binding_17,
                ..
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    __binding_4.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_typedef_kind_top_down(env, __binding_5) {
                            return;
                        }
                        __binding_5.transform(env, &mut pass.clone());
                        in_pass.on_fld_typedef_kind_bottom_up(env, __binding_5);
                    }
                }
                {
                    __binding_6.transform(env, &mut pass.clone())
                }
                {
                    __binding_7.transform(env, &mut pass.clone())
                }
                {
                    __binding_10.transform(env, &mut pass.clone())
                }
                {
                    __binding_12.transform(env, &mut pass.clone())
                }
                {
                    __binding_13.transform(env, &mut pass.clone())
                }
                {
                    __binding_14.transform(env, &mut pass.clone())
                }
                {
                    __binding_15.transform(env, &mut pass.clone())
                }
                {
                    __binding_16.transform(env, &mut pass.clone())
                }
                { __binding_17.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Gconst {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_gconst_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_gconst_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_gconst_value_top_down(env, __binding_4) {
                            return;
                        }
                        __binding_4.transform(env, &mut pass.clone());
                        in_pass.on_fld_gconst_value_bottom_up(env, __binding_4);
                    }
                }
                {
                    __binding_5.transform(env, &mut pass.clone())
                }
                { __binding_7.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for FunDef {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_fun_def_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_fun_def_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            FunDef {
                namespace: ref mut __binding_0,
                file_attributes: ref mut __binding_1,
                name: ref mut __binding_3,
                fun: ref mut __binding_4,
                internal: ref mut __binding_5,
                module: ref mut __binding_6,
                tparams: ref mut __binding_7,
                where_constraints: ref mut __binding_8,
                ..
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    __binding_4.transform(env, &mut pass.clone())
                }
                {
                    __binding_5.transform(env, &mut pass.clone())
                }
                {
                    __binding_6.transform(env, &mut pass.clone())
                }
                {
                    __binding_7.transform(env, &mut pass.clone())
                }
                { __binding_8.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ModuleDef {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_module_def_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_module_def_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ModuleDef {
                annotation: ref mut __binding_0,
                user_attributes: ref mut __binding_2,
                file_attributes: ref mut __binding_3,
                doc_comment: ref mut __binding_6,
                exports: ref mut __binding_7,
                imports: ref mut __binding_8,
                ..
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    __binding_6.transform(env, &mut pass.clone())
                }
                {
                    __binding_7.transform(env, &mut pass.clone())
                }
                { __binding_8.transform(env, &mut pass.clone()) }
            }
        }
    }
}
const _: () = {
    impl Transform for MdNameKind {}
};
impl Transform for Def {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_def_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_def_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Def::Fun(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::Class(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::Stmt(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::Typedef(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::Constant(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::Namespace(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::NamespaceUse(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::SetNamespaceEnv(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Def::FileAttributes(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Def::Module(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Def::SetModule(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
const _: () = {
    impl Transform for NsKind {}
};
const _: () = {
    impl Transform for ImportFlavor {}
};
impl Transform for XhpChild {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_xhp_child_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_xhp_child_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            XhpChild::ChildName(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            XhpChild::ChildList(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            XhpChild::ChildUnary(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            XhpChild::ChildBinary(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
const _: () = {
    impl Transform for XhpChildOp {}
};
impl Transform for Hint {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_hint_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_hint_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Hint(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for UserAttributes {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_user_attributes_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_user_attributes_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            UserAttributes(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for Contexts {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_contexts_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_contexts_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Contexts(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
const _: () = {
    impl Transform for HfParamInfo {}
};
impl Transform for HintFun {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_hint_fun_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_hint_fun_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
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
                    __binding_1.transform(env, &mut pass.clone())
                }
                {
                    __binding_2.transform(env, &mut pass.clone())
                }
                {
                    __binding_3.transform(env, &mut pass.clone())
                }
                {
                    __binding_4.transform(env, &mut pass.clone())
                }
                {
                    {
                        let pass = &mut pass.clone();
                        let mut in_pass = pass.clone();
                        if let Break(..) = pass.on_fld_hint_fun_return_ty_top_down(env, __binding_5)
                        {
                            return;
                        }
                        __binding_5.transform(env, &mut pass.clone());
                        in_pass.on_fld_hint_fun_return_ty_bottom_up(env, __binding_5);
                    }
                }
            }
        }
    }
}
impl Transform for Hint_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_hint__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_hint__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Hint_::Hprim(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Happly(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            Hint_::Hoption(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Hlike(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Hfun(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Htuple(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::HclassArgs(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Hshape(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Haccess(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            Hint_::Hsoft(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Hrefinement(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            Hint_::Hany => {}
            Hint_::Herr => {}
            Hint_::Hmixed => {}
            Hint_::Hwildcard => {}
            Hint_::Hnonnull => {}
            Hint_::Habstr(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            Hint_::HvecOrDict(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            Hint_::Hthis => {}
            Hint_::Hdynamic => {}
            Hint_::Hnothing => {}
            Hint_::Hunion(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
            Hint_::Hintersection(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Hint_::HfunContext(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            Hint_::Hvar(ref mut __binding_0) => __binding_0.transform(env, &mut pass.clone()),
        }
    }
}
impl Transform for Refinement {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_refinement_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_refinement_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Refinement::Rctx(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
            Refinement::Rtype(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for TypeRefinement {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_type_refinement_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_type_refinement_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            TypeRefinement::TRexact(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            TypeRefinement::TRloose(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for TypeRefinementBounds {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_type_refinement_bounds_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_type_refinement_bounds_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            TypeRefinementBounds {
                lower: ref mut __binding_0,
                upper: ref mut __binding_1,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for CtxRefinement {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_ctx_refinement_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_ctx_refinement_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            CtxRefinement::CRexact(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
            CtxRefinement::CRloose(ref mut __binding_0) => {
                __binding_0.transform(env, &mut pass.clone())
            }
        }
    }
}
impl Transform for CtxRefinementBounds {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_ctx_refinement_bounds_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_ctx_refinement_bounds_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            CtxRefinementBounds {
                lower: ref mut __binding_0,
                upper: ref mut __binding_1,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for ShapeFieldInfo {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_shape_field_info_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_shape_field_info_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            ShapeFieldInfo {
                optional: ref mut __binding_0,
                hint: ref mut __binding_1,
                ..
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for NastShapeInfo {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_nast_shape_info_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_nast_shape_info_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            NastShapeInfo {
                allows_unknown_fields: ref mut __binding_0,
                field_map: ref mut __binding_1,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
const _: () = {
    impl Transform for KvcKind {}
};
const _: () = {
    impl Transform for VcKind {}
};
impl Transform for Enum_ {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_enum__top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_enum__bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Enum_ {
                base: ref mut __binding_0,
                constraint: ref mut __binding_1,
                includes: ref mut __binding_2,
            } => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for WhereConstraintHint {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_where_constraint_hint_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_where_constraint_hint_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            WhereConstraintHint(ref mut __binding_0, ref mut __binding_1, ref mut __binding_2) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                {
                    __binding_1.transform(env, &mut pass.clone())
                }
                { __binding_2.transform(env, &mut pass.clone()) }
            }
        }
    }
}
impl Transform for Id {
    fn transform(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        let mut in_pass = pass.clone();
        if let Break(..) = pass.on_ty_id_top_down(env, self) {
            return;
        }
        stack_limit::maybe_grow(|| self.traverse(env, pass));
        in_pass.on_ty_id_bottom_up(env, self);
    }
    fn traverse(&mut self, env: &Env, pass: &mut (impl Pass + Clone)) {
        match self {
            Id(ref mut __binding_0, ref mut __binding_1) => {
                {
                    __binding_0.transform(env, &mut pass.clone())
                }
                { __binding_1.transform(env, &mut pass.clone()) }
            }
        }
    }
}
const _: () = {
    impl Transform for ShapeFieldName {}
};
const _: () = {
    impl Transform for Variance {}
};
const _: () = {
    impl Transform for ConstraintKind {}
};
const _: () = {
    impl Transform for Abstraction {}
};
const _: () = {
    impl Transform for ClassishKind {}
};
const _: () = {
    impl Transform for ParamKind {}
};
const _: () = {
    impl Transform for ReadonlyKind {}
};
const _: () = {
    impl Transform for OgNullFlavor {}
};
const _: () = {
    impl Transform for PropOrMethod {}
};
const _: () = {
    impl Transform for FunKind {}
};
const _: () = {
    impl Transform for Bop {}
};
const _: () = {
    impl Transform for Uop {}
};
const _: () = {
    impl Transform for Visibility {}
};
const _: () = {
    impl Transform for XhpEnumValue {}
};
const _: () = {
    impl Transform for Tprim {}
};
const _: () = {
    impl Transform for TypedefVisibility {}
};
const _: () = {
    impl Transform for ReifyKind {}
};
