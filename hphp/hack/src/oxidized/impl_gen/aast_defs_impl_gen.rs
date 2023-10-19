// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ec5ba5b5964a4f6c1f34f29e390fd079>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use crate::aast_defs::*;
use crate::ast_defs;
use crate::LocalIdMap;
impl<Ex, En> Stmt_<Ex, En> {
    pub fn mk_fallthrough() -> Self {
        Stmt_::Fallthrough
    }
    pub fn mk_expr(p0: Expr<Ex, En>) -> Self {
        Stmt_::Expr(Box::new(p0))
    }
    pub fn mk_break() -> Self {
        Stmt_::Break
    }
    pub fn mk_continue() -> Self {
        Stmt_::Continue
    }
    pub fn mk_throw(p0: Expr<Ex, En>) -> Self {
        Stmt_::Throw(Box::new(p0))
    }
    pub fn mk_return(p0: Option<Expr<Ex, En>>) -> Self {
        Stmt_::Return(Box::new(p0))
    }
    pub fn mk_yield_break() -> Self {
        Stmt_::YieldBreak
    }
    pub fn mk_awaitall(p0: Vec<(Lid, Expr<Ex, En>)>, p1: Block<Ex, En>) -> Self {
        Stmt_::Awaitall(Box::new((p0, p1)))
    }
    pub fn mk_concurrent(p0: Block<Ex, En>) -> Self {
        Stmt_::Concurrent(p0)
    }
    pub fn mk_if(p0: Expr<Ex, En>, p1: Block<Ex, En>, p2: Block<Ex, En>) -> Self {
        Stmt_::If(Box::new((p0, p1, p2)))
    }
    pub fn mk_do(p0: Block<Ex, En>, p1: Expr<Ex, En>) -> Self {
        Stmt_::Do(Box::new((p0, p1)))
    }
    pub fn mk_while(p0: Expr<Ex, En>, p1: Block<Ex, En>) -> Self {
        Stmt_::While(Box::new((p0, p1)))
    }
    pub fn mk_using(p0: UsingStmt<Ex, En>) -> Self {
        Stmt_::Using(Box::new(p0))
    }
    pub fn mk_for(
        p0: Vec<Expr<Ex, En>>,
        p1: Option<Expr<Ex, En>>,
        p2: Vec<Expr<Ex, En>>,
        p3: Block<Ex, En>,
    ) -> Self {
        Stmt_::For(Box::new((p0, p1, p2, p3)))
    }
    pub fn mk_switch(
        p0: Expr<Ex, En>,
        p1: Vec<Case<Ex, En>>,
        p2: Option<DefaultCase<Ex, En>>,
    ) -> Self {
        Stmt_::Switch(Box::new((p0, p1, p2)))
    }
    pub fn mk_match(p0: StmtMatch<Ex, En>) -> Self {
        Stmt_::Match(Box::new(p0))
    }
    pub fn mk_foreach(p0: Expr<Ex, En>, p1: AsExpr<Ex, En>, p2: Block<Ex, En>) -> Self {
        Stmt_::Foreach(Box::new((p0, p1, p2)))
    }
    pub fn mk_try(p0: Block<Ex, En>, p1: Vec<Catch<Ex, En>>, p2: FinallyBlock<Ex, En>) -> Self {
        Stmt_::Try(Box::new((p0, p1, p2)))
    }
    pub fn mk_noop() -> Self {
        Stmt_::Noop
    }
    pub fn mk_declare_local(p0: Lid, p1: Hint, p2: Option<Expr<Ex, En>>) -> Self {
        Stmt_::DeclareLocal(Box::new((p0, p1, p2)))
    }
    pub fn mk_block(p0: Option<Vec<Lid>>, p1: Block<Ex, En>) -> Self {
        Stmt_::Block(Box::new((p0, p1)))
    }
    pub fn mk_markup(p0: Pstring) -> Self {
        Stmt_::Markup(Box::new(p0))
    }
    pub fn mk_assert_env(p0: EnvAnnot, p1: LocalIdMap<(Pos, Ex)>) -> Self {
        Stmt_::AssertEnv(Box::new((p0, p1)))
    }
    pub fn is_fallthrough(&self) -> bool {
        match self {
            Stmt_::Fallthrough => true,
            _ => false,
        }
    }
    pub fn is_expr(&self) -> bool {
        match self {
            Stmt_::Expr(..) => true,
            _ => false,
        }
    }
    pub fn is_break(&self) -> bool {
        match self {
            Stmt_::Break => true,
            _ => false,
        }
    }
    pub fn is_continue(&self) -> bool {
        match self {
            Stmt_::Continue => true,
            _ => false,
        }
    }
    pub fn is_throw(&self) -> bool {
        match self {
            Stmt_::Throw(..) => true,
            _ => false,
        }
    }
    pub fn is_return(&self) -> bool {
        match self {
            Stmt_::Return(..) => true,
            _ => false,
        }
    }
    pub fn is_yield_break(&self) -> bool {
        match self {
            Stmt_::YieldBreak => true,
            _ => false,
        }
    }
    pub fn is_awaitall(&self) -> bool {
        match self {
            Stmt_::Awaitall(..) => true,
            _ => false,
        }
    }
    pub fn is_concurrent(&self) -> bool {
        match self {
            Stmt_::Concurrent(..) => true,
            _ => false,
        }
    }
    pub fn is_if(&self) -> bool {
        match self {
            Stmt_::If(..) => true,
            _ => false,
        }
    }
    pub fn is_do(&self) -> bool {
        match self {
            Stmt_::Do(..) => true,
            _ => false,
        }
    }
    pub fn is_while(&self) -> bool {
        match self {
            Stmt_::While(..) => true,
            _ => false,
        }
    }
    pub fn is_using(&self) -> bool {
        match self {
            Stmt_::Using(..) => true,
            _ => false,
        }
    }
    pub fn is_for(&self) -> bool {
        match self {
            Stmt_::For(..) => true,
            _ => false,
        }
    }
    pub fn is_switch(&self) -> bool {
        match self {
            Stmt_::Switch(..) => true,
            _ => false,
        }
    }
    pub fn is_match(&self) -> bool {
        match self {
            Stmt_::Match(..) => true,
            _ => false,
        }
    }
    pub fn is_foreach(&self) -> bool {
        match self {
            Stmt_::Foreach(..) => true,
            _ => false,
        }
    }
    pub fn is_try(&self) -> bool {
        match self {
            Stmt_::Try(..) => true,
            _ => false,
        }
    }
    pub fn is_noop(&self) -> bool {
        match self {
            Stmt_::Noop => true,
            _ => false,
        }
    }
    pub fn is_declare_local(&self) -> bool {
        match self {
            Stmt_::DeclareLocal(..) => true,
            _ => false,
        }
    }
    pub fn is_block(&self) -> bool {
        match self {
            Stmt_::Block(..) => true,
            _ => false,
        }
    }
    pub fn is_markup(&self) -> bool {
        match self {
            Stmt_::Markup(..) => true,
            _ => false,
        }
    }
    pub fn is_assert_env(&self) -> bool {
        match self {
            Stmt_::AssertEnv(..) => true,
            _ => false,
        }
    }
    pub fn as_expr(&self) -> Option<&Expr<Ex, En>> {
        match self {
            Stmt_::Expr(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_throw(&self) -> Option<&Expr<Ex, En>> {
        match self {
            Stmt_::Throw(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_return(&self) -> Option<&Option<Expr<Ex, En>>> {
        match self {
            Stmt_::Return(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_awaitall(&self) -> Option<(&Vec<(Lid, Expr<Ex, En>)>, &Block<Ex, En>)> {
        match self {
            Stmt_::Awaitall(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_concurrent(&self) -> Option<&Block<Ex, En>> {
        match self {
            Stmt_::Concurrent(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_if(&self) -> Option<(&Expr<Ex, En>, &Block<Ex, En>, &Block<Ex, En>)> {
        match self {
            Stmt_::If(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_do(&self) -> Option<(&Block<Ex, En>, &Expr<Ex, En>)> {
        match self {
            Stmt_::Do(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_while(&self) -> Option<(&Expr<Ex, En>, &Block<Ex, En>)> {
        match self {
            Stmt_::While(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_using(&self) -> Option<&UsingStmt<Ex, En>> {
        match self {
            Stmt_::Using(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_for(
        &self,
    ) -> Option<(
        &Vec<Expr<Ex, En>>,
        &Option<Expr<Ex, En>>,
        &Vec<Expr<Ex, En>>,
        &Block<Ex, En>,
    )> {
        match self {
            Stmt_::For(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3)),
            _ => None,
        }
    }
    pub fn as_switch(
        &self,
    ) -> Option<(
        &Expr<Ex, En>,
        &Vec<Case<Ex, En>>,
        &Option<DefaultCase<Ex, En>>,
    )> {
        match self {
            Stmt_::Switch(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_match(&self) -> Option<&StmtMatch<Ex, En>> {
        match self {
            Stmt_::Match(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_foreach(&self) -> Option<(&Expr<Ex, En>, &AsExpr<Ex, En>, &Block<Ex, En>)> {
        match self {
            Stmt_::Foreach(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_try(&self) -> Option<(&Block<Ex, En>, &Vec<Catch<Ex, En>>, &FinallyBlock<Ex, En>)> {
        match self {
            Stmt_::Try(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_declare_local(&self) -> Option<(&Lid, &Hint, &Option<Expr<Ex, En>>)> {
        match self {
            Stmt_::DeclareLocal(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_block(&self) -> Option<(&Option<Vec<Lid>>, &Block<Ex, En>)> {
        match self {
            Stmt_::Block(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_markup(&self) -> Option<&Pstring> {
        match self {
            Stmt_::Markup(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_assert_env(&self) -> Option<(&EnvAnnot, &LocalIdMap<(Pos, Ex)>)> {
        match self {
            Stmt_::AssertEnv(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_expr_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            Stmt_::Expr(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_throw_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            Stmt_::Throw(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_return_mut(&mut self) -> Option<&mut Option<Expr<Ex, En>>> {
        match self {
            Stmt_::Return(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_awaitall_mut(
        &mut self,
    ) -> Option<(&mut Vec<(Lid, Expr<Ex, En>)>, &mut Block<Ex, En>)> {
        match self {
            Stmt_::Awaitall(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_concurrent_mut(&mut self) -> Option<&mut Block<Ex, En>> {
        match self {
            Stmt_::Concurrent(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_if_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, En>, &mut Block<Ex, En>, &mut Block<Ex, En>)> {
        match self {
            Stmt_::If(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_do_mut(&mut self) -> Option<(&mut Block<Ex, En>, &mut Expr<Ex, En>)> {
        match self {
            Stmt_::Do(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_while_mut(&mut self) -> Option<(&mut Expr<Ex, En>, &mut Block<Ex, En>)> {
        match self {
            Stmt_::While(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_using_mut(&mut self) -> Option<&mut UsingStmt<Ex, En>> {
        match self {
            Stmt_::Using(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_for_mut(
        &mut self,
    ) -> Option<(
        &mut Vec<Expr<Ex, En>>,
        &mut Option<Expr<Ex, En>>,
        &mut Vec<Expr<Ex, En>>,
        &mut Block<Ex, En>,
    )> {
        match self {
            Stmt_::For(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3)),
            _ => None,
        }
    }
    pub fn as_switch_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, En>,
        &mut Vec<Case<Ex, En>>,
        &mut Option<DefaultCase<Ex, En>>,
    )> {
        match self {
            Stmt_::Switch(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_match_mut(&mut self) -> Option<&mut StmtMatch<Ex, En>> {
        match self {
            Stmt_::Match(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_foreach_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, En>, &mut AsExpr<Ex, En>, &mut Block<Ex, En>)> {
        match self {
            Stmt_::Foreach(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_try_mut(
        &mut self,
    ) -> Option<(
        &mut Block<Ex, En>,
        &mut Vec<Catch<Ex, En>>,
        &mut FinallyBlock<Ex, En>,
    )> {
        match self {
            Stmt_::Try(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_declare_local_mut(
        &mut self,
    ) -> Option<(&mut Lid, &mut Hint, &mut Option<Expr<Ex, En>>)> {
        match self {
            Stmt_::DeclareLocal(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_block_mut(&mut self) -> Option<(&mut Option<Vec<Lid>>, &mut Block<Ex, En>)> {
        match self {
            Stmt_::Block(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_markup_mut(&mut self) -> Option<&mut Pstring> {
        match self {
            Stmt_::Markup(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_assert_env_mut(&mut self) -> Option<(&mut EnvAnnot, &mut LocalIdMap<(Pos, Ex)>)> {
        match self {
            Stmt_::AssertEnv(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_expr_into(self) -> Option<Expr<Ex, En>> {
        match self {
            Stmt_::Expr(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_throw_into(self) -> Option<Expr<Ex, En>> {
        match self {
            Stmt_::Throw(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_return_into(self) -> Option<Option<Expr<Ex, En>>> {
        match self {
            Stmt_::Return(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_awaitall_into(self) -> Option<(Vec<(Lid, Expr<Ex, En>)>, Block<Ex, En>)> {
        match self {
            Stmt_::Awaitall(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_concurrent_into(self) -> Option<Block<Ex, En>> {
        match self {
            Stmt_::Concurrent(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_if_into(self) -> Option<(Expr<Ex, En>, Block<Ex, En>, Block<Ex, En>)> {
        match self {
            Stmt_::If(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_do_into(self) -> Option<(Block<Ex, En>, Expr<Ex, En>)> {
        match self {
            Stmt_::Do(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_while_into(self) -> Option<(Expr<Ex, En>, Block<Ex, En>)> {
        match self {
            Stmt_::While(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_using_into(self) -> Option<UsingStmt<Ex, En>> {
        match self {
            Stmt_::Using(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_for_into(
        self,
    ) -> Option<(
        Vec<Expr<Ex, En>>,
        Option<Expr<Ex, En>>,
        Vec<Expr<Ex, En>>,
        Block<Ex, En>,
    )> {
        match self {
            Stmt_::For(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3)),
            _ => None,
        }
    }
    pub fn as_switch_into(
        self,
    ) -> Option<(Expr<Ex, En>, Vec<Case<Ex, En>>, Option<DefaultCase<Ex, En>>)> {
        match self {
            Stmt_::Switch(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_match_into(self) -> Option<StmtMatch<Ex, En>> {
        match self {
            Stmt_::Match(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_foreach_into(self) -> Option<(Expr<Ex, En>, AsExpr<Ex, En>, Block<Ex, En>)> {
        match self {
            Stmt_::Foreach(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_try_into(self) -> Option<(Block<Ex, En>, Vec<Catch<Ex, En>>, FinallyBlock<Ex, En>)> {
        match self {
            Stmt_::Try(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_declare_local_into(self) -> Option<(Lid, Hint, Option<Expr<Ex, En>>)> {
        match self {
            Stmt_::DeclareLocal(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_block_into(self) -> Option<(Option<Vec<Lid>>, Block<Ex, En>)> {
        match self {
            Stmt_::Block(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_markup_into(self) -> Option<Pstring> {
        match self {
            Stmt_::Markup(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_assert_env_into(self) -> Option<(EnvAnnot, LocalIdMap<(Pos, Ex)>)> {
        match self {
            Stmt_::AssertEnv(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
}
impl EnvAnnot {
    pub fn mk_join() -> Self {
        EnvAnnot::Join
    }
    pub fn mk_refinement() -> Self {
        EnvAnnot::Refinement
    }
    pub fn is_join(&self) -> bool {
        match self {
            EnvAnnot::Join => true,
            _ => false,
        }
    }
    pub fn is_refinement(&self) -> bool {
        match self {
            EnvAnnot::Refinement => true,
            _ => false,
        }
    }
}
impl<Ex, En> AsExpr<Ex, En> {
    pub fn mk_as_v(p0: Expr<Ex, En>) -> Self {
        AsExpr::AsV(p0)
    }
    pub fn mk_as_kv(p0: Expr<Ex, En>, p1: Expr<Ex, En>) -> Self {
        AsExpr::AsKv(p0, p1)
    }
    pub fn mk_await_as_v(p0: Pos, p1: Expr<Ex, En>) -> Self {
        AsExpr::AwaitAsV(p0, p1)
    }
    pub fn mk_await_as_kv(p0: Pos, p1: Expr<Ex, En>, p2: Expr<Ex, En>) -> Self {
        AsExpr::AwaitAsKv(p0, p1, p2)
    }
    pub fn is_as_v(&self) -> bool {
        match self {
            AsExpr::AsV(..) => true,
            _ => false,
        }
    }
    pub fn is_as_kv(&self) -> bool {
        match self {
            AsExpr::AsKv(..) => true,
            _ => false,
        }
    }
    pub fn is_await_as_v(&self) -> bool {
        match self {
            AsExpr::AwaitAsV(..) => true,
            _ => false,
        }
    }
    pub fn is_await_as_kv(&self) -> bool {
        match self {
            AsExpr::AwaitAsKv(..) => true,
            _ => false,
        }
    }
    pub fn as_as_v(&self) -> Option<&Expr<Ex, En>> {
        match self {
            AsExpr::AsV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_as_kv(&self) -> Option<(&Expr<Ex, En>, &Expr<Ex, En>)> {
        match self {
            AsExpr::AsKv(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_v(&self) -> Option<(&Pos, &Expr<Ex, En>)> {
        match self {
            AsExpr::AwaitAsV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_kv(&self) -> Option<(&Pos, &Expr<Ex, En>, &Expr<Ex, En>)> {
        match self {
            AsExpr::AwaitAsKv(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
    pub fn as_as_v_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            AsExpr::AsV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_as_kv_mut(&mut self) -> Option<(&mut Expr<Ex, En>, &mut Expr<Ex, En>)> {
        match self {
            AsExpr::AsKv(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_v_mut(&mut self) -> Option<(&mut Pos, &mut Expr<Ex, En>)> {
        match self {
            AsExpr::AwaitAsV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_kv_mut(
        &mut self,
    ) -> Option<(&mut Pos, &mut Expr<Ex, En>, &mut Expr<Ex, En>)> {
        match self {
            AsExpr::AwaitAsKv(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
    pub fn as_as_v_into(self) -> Option<Expr<Ex, En>> {
        match self {
            AsExpr::AsV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_as_kv_into(self) -> Option<(Expr<Ex, En>, Expr<Ex, En>)> {
        match self {
            AsExpr::AsKv(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_v_into(self) -> Option<(Pos, Expr<Ex, En>)> {
        match self {
            AsExpr::AwaitAsV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_kv_into(self) -> Option<(Pos, Expr<Ex, En>, Expr<Ex, En>)> {
        match self {
            AsExpr::AwaitAsKv(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
}
impl Pattern {
    pub fn mk_pvar(p0: PatVar) -> Self {
        Pattern::PVar(Box::new(p0))
    }
    pub fn mk_prefinement(p0: PatRefinement) -> Self {
        Pattern::PRefinement(Box::new(p0))
    }
    pub fn is_pvar(&self) -> bool {
        match self {
            Pattern::PVar(..) => true,
            _ => false,
        }
    }
    pub fn is_prefinement(&self) -> bool {
        match self {
            Pattern::PRefinement(..) => true,
            _ => false,
        }
    }
    pub fn as_pvar(&self) -> Option<&PatVar> {
        match self {
            Pattern::PVar(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_prefinement(&self) -> Option<&PatRefinement> {
        match self {
            Pattern::PRefinement(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_pvar_mut(&mut self) -> Option<&mut PatVar> {
        match self {
            Pattern::PVar(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_prefinement_mut(&mut self) -> Option<&mut PatRefinement> {
        match self {
            Pattern::PRefinement(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_pvar_into(self) -> Option<PatVar> {
        match self {
            Pattern::PVar(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_prefinement_into(self) -> Option<PatRefinement> {
        match self {
            Pattern::PRefinement(p0) => Some(*p0),
            _ => None,
        }
    }
}
impl<Ex, En> ClassId_<Ex, En> {
    pub fn mk_ciparent() -> Self {
        ClassId_::CIparent
    }
    pub fn mk_ciself() -> Self {
        ClassId_::CIself
    }
    pub fn mk_cistatic() -> Self {
        ClassId_::CIstatic
    }
    pub fn mk_ciexpr(p0: Expr<Ex, En>) -> Self {
        ClassId_::CIexpr(p0)
    }
    pub fn mk_ci(p0: ClassName) -> Self {
        ClassId_::CI(p0)
    }
    pub fn is_ciparent(&self) -> bool {
        match self {
            ClassId_::CIparent => true,
            _ => false,
        }
    }
    pub fn is_ciself(&self) -> bool {
        match self {
            ClassId_::CIself => true,
            _ => false,
        }
    }
    pub fn is_cistatic(&self) -> bool {
        match self {
            ClassId_::CIstatic => true,
            _ => false,
        }
    }
    pub fn is_ciexpr(&self) -> bool {
        match self {
            ClassId_::CIexpr(..) => true,
            _ => false,
        }
    }
    pub fn is_ci(&self) -> bool {
        match self {
            ClassId_::CI(..) => true,
            _ => false,
        }
    }
    pub fn as_ciexpr(&self) -> Option<&Expr<Ex, En>> {
        match self {
            ClassId_::CIexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ci(&self) -> Option<&ClassName> {
        match self {
            ClassId_::CI(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ciexpr_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            ClassId_::CIexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ci_mut(&mut self) -> Option<&mut ClassName> {
        match self {
            ClassId_::CI(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ciexpr_into(self) -> Option<Expr<Ex, En>> {
        match self {
            ClassId_::CIexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ci_into(self) -> Option<ClassName> {
        match self {
            ClassId_::CI(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex> CollectionTarg<Ex> {
    pub fn mk_collectiontv(p0: Targ<Ex>) -> Self {
        CollectionTarg::CollectionTV(p0)
    }
    pub fn mk_collectiontkv(p0: Targ<Ex>, p1: Targ<Ex>) -> Self {
        CollectionTarg::CollectionTKV(p0, p1)
    }
    pub fn is_collectiontv(&self) -> bool {
        match self {
            CollectionTarg::CollectionTV(..) => true,
            _ => false,
        }
    }
    pub fn is_collectiontkv(&self) -> bool {
        match self {
            CollectionTarg::CollectionTKV(..) => true,
            _ => false,
        }
    }
    pub fn as_collectiontv(&self) -> Option<&Targ<Ex>> {
        match self {
            CollectionTarg::CollectionTV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_collectiontkv(&self) -> Option<(&Targ<Ex>, &Targ<Ex>)> {
        match self {
            CollectionTarg::CollectionTKV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_collectiontv_mut(&mut self) -> Option<&mut Targ<Ex>> {
        match self {
            CollectionTarg::CollectionTV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_collectiontkv_mut(&mut self) -> Option<(&mut Targ<Ex>, &mut Targ<Ex>)> {
        match self {
            CollectionTarg::CollectionTKV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_collectiontv_into(self) -> Option<Targ<Ex>> {
        match self {
            CollectionTarg::CollectionTV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_collectiontkv_into(self) -> Option<(Targ<Ex>, Targ<Ex>)> {
        match self {
            CollectionTarg::CollectionTKV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl<Ex, En> FunctionPtrId<Ex, En> {
    pub fn mk_fpid(p0: Sid) -> Self {
        FunctionPtrId::FPId(p0)
    }
    pub fn mk_fpclass_const(p0: ClassId<Ex, En>, p1: Pstring) -> Self {
        FunctionPtrId::FPClassConst(p0, p1)
    }
    pub fn is_fpid(&self) -> bool {
        match self {
            FunctionPtrId::FPId(..) => true,
            _ => false,
        }
    }
    pub fn is_fpclass_const(&self) -> bool {
        match self {
            FunctionPtrId::FPClassConst(..) => true,
            _ => false,
        }
    }
    pub fn as_fpid(&self) -> Option<&Sid> {
        match self {
            FunctionPtrId::FPId(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fpclass_const(&self) -> Option<(&ClassId<Ex, En>, &Pstring)> {
        match self {
            FunctionPtrId::FPClassConst(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_fpid_mut(&mut self) -> Option<&mut Sid> {
        match self {
            FunctionPtrId::FPId(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fpclass_const_mut(&mut self) -> Option<(&mut ClassId<Ex, En>, &mut Pstring)> {
        match self {
            FunctionPtrId::FPClassConst(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_fpid_into(self) -> Option<Sid> {
        match self {
            FunctionPtrId::FPId(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fpclass_const_into(self) -> Option<(ClassId<Ex, En>, Pstring)> {
        match self {
            FunctionPtrId::FPClassConst(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl<Ex, En> Expr_<Ex, En> {
    pub fn mk_darray(
        p0: Option<(Targ<Ex>, Targ<Ex>)>,
        p1: Vec<(Expr<Ex, En>, Expr<Ex, En>)>,
    ) -> Self {
        Expr_::Darray(Box::new((p0, p1)))
    }
    pub fn mk_varray(p0: Option<Targ<Ex>>, p1: Vec<Expr<Ex, En>>) -> Self {
        Expr_::Varray(Box::new((p0, p1)))
    }
    pub fn mk_shape(p0: Vec<(ast_defs::ShapeFieldName, Expr<Ex, En>)>) -> Self {
        Expr_::Shape(p0)
    }
    pub fn mk_val_collection(
        p0: (Pos, VcKind),
        p1: Option<Targ<Ex>>,
        p2: Vec<Expr<Ex, En>>,
    ) -> Self {
        Expr_::ValCollection(Box::new((p0, p1, p2)))
    }
    pub fn mk_key_val_collection(
        p0: (Pos, KvcKind),
        p1: Option<(Targ<Ex>, Targ<Ex>)>,
        p2: Vec<Field<Ex, En>>,
    ) -> Self {
        Expr_::KeyValCollection(Box::new((p0, p1, p2)))
    }
    pub fn mk_null() -> Self {
        Expr_::Null
    }
    pub fn mk_this() -> Self {
        Expr_::This
    }
    pub fn mk_true() -> Self {
        Expr_::True
    }
    pub fn mk_false() -> Self {
        Expr_::False
    }
    pub fn mk_omitted() -> Self {
        Expr_::Omitted
    }
    pub fn mk_invalid(p0: Option<Expr<Ex, En>>) -> Self {
        Expr_::Invalid(Box::new(p0))
    }
    pub fn mk_id(p0: Sid) -> Self {
        Expr_::Id(Box::new(p0))
    }
    pub fn mk_lvar(p0: Lid) -> Self {
        Expr_::Lvar(Box::new(p0))
    }
    pub fn mk_dollardollar(p0: Lid) -> Self {
        Expr_::Dollardollar(Box::new(p0))
    }
    pub fn mk_clone(p0: Expr<Ex, En>) -> Self {
        Expr_::Clone(Box::new(p0))
    }
    pub fn mk_array_get(p0: Expr<Ex, En>, p1: Option<Expr<Ex, En>>) -> Self {
        Expr_::ArrayGet(Box::new((p0, p1)))
    }
    pub fn mk_obj_get(
        p0: Expr<Ex, En>,
        p1: Expr<Ex, En>,
        p2: OgNullFlavor,
        p3: PropOrMethod,
    ) -> Self {
        Expr_::ObjGet(Box::new((p0, p1, p2, p3)))
    }
    pub fn mk_class_get(p0: ClassId<Ex, En>, p1: ClassGetExpr<Ex, En>, p2: PropOrMethod) -> Self {
        Expr_::ClassGet(Box::new((p0, p1, p2)))
    }
    pub fn mk_class_const(p0: ClassId<Ex, En>, p1: Pstring) -> Self {
        Expr_::ClassConst(Box::new((p0, p1)))
    }
    pub fn mk_call(p0: CallExpr<Ex, En>) -> Self {
        Expr_::Call(Box::new(p0))
    }
    pub fn mk_function_pointer(p0: FunctionPtrId<Ex, En>, p1: Vec<Targ<Ex>>) -> Self {
        Expr_::FunctionPointer(Box::new((p0, p1)))
    }
    pub fn mk_int(p0: String) -> Self {
        Expr_::Int(p0)
    }
    pub fn mk_float(p0: String) -> Self {
        Expr_::Float(p0)
    }
    pub fn mk_string(p0: bstr::BString) -> Self {
        Expr_::String(p0)
    }
    pub fn mk_string2(p0: Vec<Expr<Ex, En>>) -> Self {
        Expr_::String2(p0)
    }
    pub fn mk_prefixed_string(p0: String, p1: Expr<Ex, En>) -> Self {
        Expr_::PrefixedString(Box::new((p0, p1)))
    }
    pub fn mk_yield(p0: Afield<Ex, En>) -> Self {
        Expr_::Yield(Box::new(p0))
    }
    pub fn mk_await(p0: Expr<Ex, En>) -> Self {
        Expr_::Await(Box::new(p0))
    }
    pub fn mk_readonly_expr(p0: Expr<Ex, En>) -> Self {
        Expr_::ReadonlyExpr(Box::new(p0))
    }
    pub fn mk_tuple(p0: Vec<Expr<Ex, En>>) -> Self {
        Expr_::Tuple(p0)
    }
    pub fn mk_list(p0: Vec<Expr<Ex, En>>) -> Self {
        Expr_::List(p0)
    }
    pub fn mk_cast(p0: Hint, p1: Expr<Ex, En>) -> Self {
        Expr_::Cast(Box::new((p0, p1)))
    }
    pub fn mk_unop(p0: ast_defs::Uop, p1: Expr<Ex, En>) -> Self {
        Expr_::Unop(Box::new((p0, p1)))
    }
    pub fn mk_binop(p0: Binop<Ex, En>) -> Self {
        Expr_::Binop(Box::new(p0))
    }
    pub fn mk_pipe(p0: Lid, p1: Expr<Ex, En>, p2: Expr<Ex, En>) -> Self {
        Expr_::Pipe(Box::new((p0, p1, p2)))
    }
    pub fn mk_eif(p0: Expr<Ex, En>, p1: Option<Expr<Ex, En>>, p2: Expr<Ex, En>) -> Self {
        Expr_::Eif(Box::new((p0, p1, p2)))
    }
    pub fn mk_is(p0: Expr<Ex, En>, p1: Hint) -> Self {
        Expr_::Is(Box::new((p0, p1)))
    }
    pub fn mk_as(p0: Expr<Ex, En>, p1: Hint, p2: bool) -> Self {
        Expr_::As(Box::new((p0, p1, p2)))
    }
    pub fn mk_upcast(p0: Expr<Ex, En>, p1: Hint) -> Self {
        Expr_::Upcast(Box::new((p0, p1)))
    }
    pub fn mk_new(
        p0: ClassId<Ex, En>,
        p1: Vec<Targ<Ex>>,
        p2: Vec<Expr<Ex, En>>,
        p3: Option<Expr<Ex, En>>,
        p4: Ex,
    ) -> Self {
        Expr_::New(Box::new((p0, p1, p2, p3, p4)))
    }
    pub fn mk_efun(p0: Efun<Ex, En>) -> Self {
        Expr_::Efun(Box::new(p0))
    }
    pub fn mk_lfun(p0: Fun_<Ex, En>, p1: Vec<CaptureLid<Ex>>) -> Self {
        Expr_::Lfun(Box::new((p0, p1)))
    }
    pub fn mk_xml(p0: ClassName, p1: Vec<XhpAttribute<Ex, En>>, p2: Vec<Expr<Ex, En>>) -> Self {
        Expr_::Xml(Box::new((p0, p1, p2)))
    }
    pub fn mk_import(p0: ImportFlavor, p1: Expr<Ex, En>) -> Self {
        Expr_::Import(Box::new((p0, p1)))
    }
    pub fn mk_collection(
        p0: ClassName,
        p1: Option<CollectionTarg<Ex>>,
        p2: Vec<Afield<Ex, En>>,
    ) -> Self {
        Expr_::Collection(Box::new((p0, p1, p2)))
    }
    pub fn mk_expression_tree(p0: ExpressionTree<Ex, En>) -> Self {
        Expr_::ExpressionTree(Box::new(p0))
    }
    pub fn mk_lplaceholder(p0: Pos) -> Self {
        Expr_::Lplaceholder(Box::new(p0))
    }
    pub fn mk_method_caller(p0: ClassName, p1: Pstring) -> Self {
        Expr_::MethodCaller(Box::new((p0, p1)))
    }
    pub fn mk_pair(p0: Option<(Targ<Ex>, Targ<Ex>)>, p1: Expr<Ex, En>, p2: Expr<Ex, En>) -> Self {
        Expr_::Pair(Box::new((p0, p1, p2)))
    }
    pub fn mk_etsplice(p0: Expr<Ex, En>) -> Self {
        Expr_::ETSplice(Box::new(p0))
    }
    pub fn mk_enum_class_label(p0: Option<ClassName>, p1: String) -> Self {
        Expr_::EnumClassLabel(Box::new((p0, p1)))
    }
    pub fn mk_hole(p0: Expr<Ex, En>, p1: Ex, p2: Ex, p3: HoleSource) -> Self {
        Expr_::Hole(Box::new((p0, p1, p2, p3)))
    }
    pub fn mk_package(p0: Sid) -> Self {
        Expr_::Package(Box::new(p0))
    }
    pub fn mk_nameof(p0: ClassId<Ex, En>) -> Self {
        Expr_::Nameof(Box::new(p0))
    }
    pub fn is_darray(&self) -> bool {
        match self {
            Expr_::Darray(..) => true,
            _ => false,
        }
    }
    pub fn is_varray(&self) -> bool {
        match self {
            Expr_::Varray(..) => true,
            _ => false,
        }
    }
    pub fn is_shape(&self) -> bool {
        match self {
            Expr_::Shape(..) => true,
            _ => false,
        }
    }
    pub fn is_val_collection(&self) -> bool {
        match self {
            Expr_::ValCollection(..) => true,
            _ => false,
        }
    }
    pub fn is_key_val_collection(&self) -> bool {
        match self {
            Expr_::KeyValCollection(..) => true,
            _ => false,
        }
    }
    pub fn is_null(&self) -> bool {
        match self {
            Expr_::Null => true,
            _ => false,
        }
    }
    pub fn is_this(&self) -> bool {
        match self {
            Expr_::This => true,
            _ => false,
        }
    }
    pub fn is_true(&self) -> bool {
        match self {
            Expr_::True => true,
            _ => false,
        }
    }
    pub fn is_false(&self) -> bool {
        match self {
            Expr_::False => true,
            _ => false,
        }
    }
    pub fn is_omitted(&self) -> bool {
        match self {
            Expr_::Omitted => true,
            _ => false,
        }
    }
    pub fn is_invalid(&self) -> bool {
        match self {
            Expr_::Invalid(..) => true,
            _ => false,
        }
    }
    pub fn is_id(&self) -> bool {
        match self {
            Expr_::Id(..) => true,
            _ => false,
        }
    }
    pub fn is_lvar(&self) -> bool {
        match self {
            Expr_::Lvar(..) => true,
            _ => false,
        }
    }
    pub fn is_dollardollar(&self) -> bool {
        match self {
            Expr_::Dollardollar(..) => true,
            _ => false,
        }
    }
    pub fn is_clone(&self) -> bool {
        match self {
            Expr_::Clone(..) => true,
            _ => false,
        }
    }
    pub fn is_array_get(&self) -> bool {
        match self {
            Expr_::ArrayGet(..) => true,
            _ => false,
        }
    }
    pub fn is_obj_get(&self) -> bool {
        match self {
            Expr_::ObjGet(..) => true,
            _ => false,
        }
    }
    pub fn is_class_get(&self) -> bool {
        match self {
            Expr_::ClassGet(..) => true,
            _ => false,
        }
    }
    pub fn is_class_const(&self) -> bool {
        match self {
            Expr_::ClassConst(..) => true,
            _ => false,
        }
    }
    pub fn is_call(&self) -> bool {
        match self {
            Expr_::Call(..) => true,
            _ => false,
        }
    }
    pub fn is_function_pointer(&self) -> bool {
        match self {
            Expr_::FunctionPointer(..) => true,
            _ => false,
        }
    }
    pub fn is_int(&self) -> bool {
        match self {
            Expr_::Int(..) => true,
            _ => false,
        }
    }
    pub fn is_float(&self) -> bool {
        match self {
            Expr_::Float(..) => true,
            _ => false,
        }
    }
    pub fn is_string(&self) -> bool {
        match self {
            Expr_::String(..) => true,
            _ => false,
        }
    }
    pub fn is_string2(&self) -> bool {
        match self {
            Expr_::String2(..) => true,
            _ => false,
        }
    }
    pub fn is_prefixed_string(&self) -> bool {
        match self {
            Expr_::PrefixedString(..) => true,
            _ => false,
        }
    }
    pub fn is_yield(&self) -> bool {
        match self {
            Expr_::Yield(..) => true,
            _ => false,
        }
    }
    pub fn is_await(&self) -> bool {
        match self {
            Expr_::Await(..) => true,
            _ => false,
        }
    }
    pub fn is_readonly_expr(&self) -> bool {
        match self {
            Expr_::ReadonlyExpr(..) => true,
            _ => false,
        }
    }
    pub fn is_tuple(&self) -> bool {
        match self {
            Expr_::Tuple(..) => true,
            _ => false,
        }
    }
    pub fn is_list(&self) -> bool {
        match self {
            Expr_::List(..) => true,
            _ => false,
        }
    }
    pub fn is_cast(&self) -> bool {
        match self {
            Expr_::Cast(..) => true,
            _ => false,
        }
    }
    pub fn is_unop(&self) -> bool {
        match self {
            Expr_::Unop(..) => true,
            _ => false,
        }
    }
    pub fn is_binop(&self) -> bool {
        match self {
            Expr_::Binop(..) => true,
            _ => false,
        }
    }
    pub fn is_pipe(&self) -> bool {
        match self {
            Expr_::Pipe(..) => true,
            _ => false,
        }
    }
    pub fn is_eif(&self) -> bool {
        match self {
            Expr_::Eif(..) => true,
            _ => false,
        }
    }
    pub fn is_is(&self) -> bool {
        match self {
            Expr_::Is(..) => true,
            _ => false,
        }
    }
    pub fn is_as(&self) -> bool {
        match self {
            Expr_::As(..) => true,
            _ => false,
        }
    }
    pub fn is_upcast(&self) -> bool {
        match self {
            Expr_::Upcast(..) => true,
            _ => false,
        }
    }
    pub fn is_new(&self) -> bool {
        match self {
            Expr_::New(..) => true,
            _ => false,
        }
    }
    pub fn is_efun(&self) -> bool {
        match self {
            Expr_::Efun(..) => true,
            _ => false,
        }
    }
    pub fn is_lfun(&self) -> bool {
        match self {
            Expr_::Lfun(..) => true,
            _ => false,
        }
    }
    pub fn is_xml(&self) -> bool {
        match self {
            Expr_::Xml(..) => true,
            _ => false,
        }
    }
    pub fn is_import(&self) -> bool {
        match self {
            Expr_::Import(..) => true,
            _ => false,
        }
    }
    pub fn is_collection(&self) -> bool {
        match self {
            Expr_::Collection(..) => true,
            _ => false,
        }
    }
    pub fn is_expression_tree(&self) -> bool {
        match self {
            Expr_::ExpressionTree(..) => true,
            _ => false,
        }
    }
    pub fn is_lplaceholder(&self) -> bool {
        match self {
            Expr_::Lplaceholder(..) => true,
            _ => false,
        }
    }
    pub fn is_method_caller(&self) -> bool {
        match self {
            Expr_::MethodCaller(..) => true,
            _ => false,
        }
    }
    pub fn is_pair(&self) -> bool {
        match self {
            Expr_::Pair(..) => true,
            _ => false,
        }
    }
    pub fn is_etsplice(&self) -> bool {
        match self {
            Expr_::ETSplice(..) => true,
            _ => false,
        }
    }
    pub fn is_enum_class_label(&self) -> bool {
        match self {
            Expr_::EnumClassLabel(..) => true,
            _ => false,
        }
    }
    pub fn is_hole(&self) -> bool {
        match self {
            Expr_::Hole(..) => true,
            _ => false,
        }
    }
    pub fn is_package(&self) -> bool {
        match self {
            Expr_::Package(..) => true,
            _ => false,
        }
    }
    pub fn is_nameof(&self) -> bool {
        match self {
            Expr_::Nameof(..) => true,
            _ => false,
        }
    }
    pub fn as_darray(
        &self,
    ) -> Option<(
        &Option<(Targ<Ex>, Targ<Ex>)>,
        &Vec<(Expr<Ex, En>, Expr<Ex, En>)>,
    )> {
        match self {
            Expr_::Darray(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_varray(&self) -> Option<(&Option<Targ<Ex>>, &Vec<Expr<Ex, En>>)> {
        match self {
            Expr_::Varray(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_shape(&self) -> Option<&Vec<(ast_defs::ShapeFieldName, Expr<Ex, En>)>> {
        match self {
            Expr_::Shape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_val_collection(
        &self,
    ) -> Option<(&(Pos, VcKind), &Option<Targ<Ex>>, &Vec<Expr<Ex, En>>)> {
        match self {
            Expr_::ValCollection(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_key_val_collection(
        &self,
    ) -> Option<(
        &(Pos, KvcKind),
        &Option<(Targ<Ex>, Targ<Ex>)>,
        &Vec<Field<Ex, En>>,
    )> {
        match self {
            Expr_::KeyValCollection(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_invalid(&self) -> Option<&Option<Expr<Ex, En>>> {
        match self {
            Expr_::Invalid(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_id(&self) -> Option<&Sid> {
        match self {
            Expr_::Id(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_lvar(&self) -> Option<&Lid> {
        match self {
            Expr_::Lvar(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_dollardollar(&self) -> Option<&Lid> {
        match self {
            Expr_::Dollardollar(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_clone(&self) -> Option<&Expr<Ex, En>> {
        match self {
            Expr_::Clone(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_array_get(&self) -> Option<(&Expr<Ex, En>, &Option<Expr<Ex, En>>)> {
        match self {
            Expr_::ArrayGet(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_obj_get(
        &self,
    ) -> Option<(&Expr<Ex, En>, &Expr<Ex, En>, &OgNullFlavor, &PropOrMethod)> {
        match self {
            Expr_::ObjGet(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3)),
            _ => None,
        }
    }
    pub fn as_class_get(&self) -> Option<(&ClassId<Ex, En>, &ClassGetExpr<Ex, En>, &PropOrMethod)> {
        match self {
            Expr_::ClassGet(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_class_const(&self) -> Option<(&ClassId<Ex, En>, &Pstring)> {
        match self {
            Expr_::ClassConst(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_call(&self) -> Option<&CallExpr<Ex, En>> {
        match self {
            Expr_::Call(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_function_pointer(&self) -> Option<(&FunctionPtrId<Ex, En>, &Vec<Targ<Ex>>)> {
        match self {
            Expr_::FunctionPointer(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_int(&self) -> Option<&String> {
        match self {
            Expr_::Int(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_float(&self) -> Option<&String> {
        match self {
            Expr_::Float(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_string(&self) -> Option<&bstr::BString> {
        match self {
            Expr_::String(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_string2(&self) -> Option<&Vec<Expr<Ex, En>>> {
        match self {
            Expr_::String2(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_prefixed_string(&self) -> Option<(&String, &Expr<Ex, En>)> {
        match self {
            Expr_::PrefixedString(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_yield(&self) -> Option<&Afield<Ex, En>> {
        match self {
            Expr_::Yield(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_await(&self) -> Option<&Expr<Ex, En>> {
        match self {
            Expr_::Await(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_readonly_expr(&self) -> Option<&Expr<Ex, En>> {
        match self {
            Expr_::ReadonlyExpr(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_tuple(&self) -> Option<&Vec<Expr<Ex, En>>> {
        match self {
            Expr_::Tuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_list(&self) -> Option<&Vec<Expr<Ex, En>>> {
        match self {
            Expr_::List(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cast(&self) -> Option<(&Hint, &Expr<Ex, En>)> {
        match self {
            Expr_::Cast(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_unop(&self) -> Option<(&ast_defs::Uop, &Expr<Ex, En>)> {
        match self {
            Expr_::Unop(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_binop(&self) -> Option<&Binop<Ex, En>> {
        match self {
            Expr_::Binop(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_pipe(&self) -> Option<(&Lid, &Expr<Ex, En>, &Expr<Ex, En>)> {
        match self {
            Expr_::Pipe(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_eif(&self) -> Option<(&Expr<Ex, En>, &Option<Expr<Ex, En>>, &Expr<Ex, En>)> {
        match self {
            Expr_::Eif(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_is(&self) -> Option<(&Expr<Ex, En>, &Hint)> {
        match self {
            Expr_::Is(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_as(&self) -> Option<(&Expr<Ex, En>, &Hint, &bool)> {
        match self {
            Expr_::As(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_upcast(&self) -> Option<(&Expr<Ex, En>, &Hint)> {
        match self {
            Expr_::Upcast(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_new(
        &self,
    ) -> Option<(
        &ClassId<Ex, En>,
        &Vec<Targ<Ex>>,
        &Vec<Expr<Ex, En>>,
        &Option<Expr<Ex, En>>,
        &Ex,
    )> {
        match self {
            Expr_::New(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3, &p0.4)),
            _ => None,
        }
    }
    pub fn as_efun(&self) -> Option<&Efun<Ex, En>> {
        match self {
            Expr_::Efun(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_lfun(&self) -> Option<(&Fun_<Ex, En>, &Vec<CaptureLid<Ex>>)> {
        match self {
            Expr_::Lfun(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_xml(&self) -> Option<(&ClassName, &Vec<XhpAttribute<Ex, En>>, &Vec<Expr<Ex, En>>)> {
        match self {
            Expr_::Xml(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_import(&self) -> Option<(&ImportFlavor, &Expr<Ex, En>)> {
        match self {
            Expr_::Import(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_collection(
        &self,
    ) -> Option<(
        &ClassName,
        &Option<CollectionTarg<Ex>>,
        &Vec<Afield<Ex, En>>,
    )> {
        match self {
            Expr_::Collection(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_expression_tree(&self) -> Option<&ExpressionTree<Ex, En>> {
        match self {
            Expr_::ExpressionTree(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_lplaceholder(&self) -> Option<&Pos> {
        match self {
            Expr_::Lplaceholder(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_method_caller(&self) -> Option<(&ClassName, &Pstring)> {
        match self {
            Expr_::MethodCaller(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_pair(&self) -> Option<(&Option<(Targ<Ex>, Targ<Ex>)>, &Expr<Ex, En>, &Expr<Ex, En>)> {
        match self {
            Expr_::Pair(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_etsplice(&self) -> Option<&Expr<Ex, En>> {
        match self {
            Expr_::ETSplice(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_enum_class_label(&self) -> Option<(&Option<ClassName>, &String)> {
        match self {
            Expr_::EnumClassLabel(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_hole(&self) -> Option<(&Expr<Ex, En>, &Ex, &Ex, &HoleSource)> {
        match self {
            Expr_::Hole(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3)),
            _ => None,
        }
    }
    pub fn as_package(&self) -> Option<&Sid> {
        match self {
            Expr_::Package(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_nameof(&self) -> Option<&ClassId<Ex, En>> {
        match self {
            Expr_::Nameof(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_darray_mut(
        &mut self,
    ) -> Option<(
        &mut Option<(Targ<Ex>, Targ<Ex>)>,
        &mut Vec<(Expr<Ex, En>, Expr<Ex, En>)>,
    )> {
        match self {
            Expr_::Darray(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_varray_mut(&mut self) -> Option<(&mut Option<Targ<Ex>>, &mut Vec<Expr<Ex, En>>)> {
        match self {
            Expr_::Varray(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_shape_mut(&mut self) -> Option<&mut Vec<(ast_defs::ShapeFieldName, Expr<Ex, En>)>> {
        match self {
            Expr_::Shape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_val_collection_mut(
        &mut self,
    ) -> Option<(
        &mut (Pos, VcKind),
        &mut Option<Targ<Ex>>,
        &mut Vec<Expr<Ex, En>>,
    )> {
        match self {
            Expr_::ValCollection(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_key_val_collection_mut(
        &mut self,
    ) -> Option<(
        &mut (Pos, KvcKind),
        &mut Option<(Targ<Ex>, Targ<Ex>)>,
        &mut Vec<Field<Ex, En>>,
    )> {
        match self {
            Expr_::KeyValCollection(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_invalid_mut(&mut self) -> Option<&mut Option<Expr<Ex, En>>> {
        match self {
            Expr_::Invalid(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_id_mut(&mut self) -> Option<&mut Sid> {
        match self {
            Expr_::Id(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_lvar_mut(&mut self) -> Option<&mut Lid> {
        match self {
            Expr_::Lvar(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_dollardollar_mut(&mut self) -> Option<&mut Lid> {
        match self {
            Expr_::Dollardollar(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_clone_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            Expr_::Clone(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_array_get_mut(&mut self) -> Option<(&mut Expr<Ex, En>, &mut Option<Expr<Ex, En>>)> {
        match self {
            Expr_::ArrayGet(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_obj_get_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, En>,
        &mut Expr<Ex, En>,
        &mut OgNullFlavor,
        &mut PropOrMethod,
    )> {
        match self {
            Expr_::ObjGet(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3)),
            _ => None,
        }
    }
    pub fn as_class_get_mut(
        &mut self,
    ) -> Option<(
        &mut ClassId<Ex, En>,
        &mut ClassGetExpr<Ex, En>,
        &mut PropOrMethod,
    )> {
        match self {
            Expr_::ClassGet(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_class_const_mut(&mut self) -> Option<(&mut ClassId<Ex, En>, &mut Pstring)> {
        match self {
            Expr_::ClassConst(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_call_mut(&mut self) -> Option<&mut CallExpr<Ex, En>> {
        match self {
            Expr_::Call(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_function_pointer_mut(
        &mut self,
    ) -> Option<(&mut FunctionPtrId<Ex, En>, &mut Vec<Targ<Ex>>)> {
        match self {
            Expr_::FunctionPointer(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_int_mut(&mut self) -> Option<&mut String> {
        match self {
            Expr_::Int(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_float_mut(&mut self) -> Option<&mut String> {
        match self {
            Expr_::Float(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_string_mut(&mut self) -> Option<&mut bstr::BString> {
        match self {
            Expr_::String(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_string2_mut(&mut self) -> Option<&mut Vec<Expr<Ex, En>>> {
        match self {
            Expr_::String2(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_prefixed_string_mut(&mut self) -> Option<(&mut String, &mut Expr<Ex, En>)> {
        match self {
            Expr_::PrefixedString(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_yield_mut(&mut self) -> Option<&mut Afield<Ex, En>> {
        match self {
            Expr_::Yield(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_await_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            Expr_::Await(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_readonly_expr_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            Expr_::ReadonlyExpr(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_tuple_mut(&mut self) -> Option<&mut Vec<Expr<Ex, En>>> {
        match self {
            Expr_::Tuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_list_mut(&mut self) -> Option<&mut Vec<Expr<Ex, En>>> {
        match self {
            Expr_::List(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cast_mut(&mut self) -> Option<(&mut Hint, &mut Expr<Ex, En>)> {
        match self {
            Expr_::Cast(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_unop_mut(&mut self) -> Option<(&mut ast_defs::Uop, &mut Expr<Ex, En>)> {
        match self {
            Expr_::Unop(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_binop_mut(&mut self) -> Option<&mut Binop<Ex, En>> {
        match self {
            Expr_::Binop(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_pipe_mut(&mut self) -> Option<(&mut Lid, &mut Expr<Ex, En>, &mut Expr<Ex, En>)> {
        match self {
            Expr_::Pipe(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_eif_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, En>,
        &mut Option<Expr<Ex, En>>,
        &mut Expr<Ex, En>,
    )> {
        match self {
            Expr_::Eif(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_is_mut(&mut self) -> Option<(&mut Expr<Ex, En>, &mut Hint)> {
        match self {
            Expr_::Is(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_as_mut(&mut self) -> Option<(&mut Expr<Ex, En>, &mut Hint, &mut bool)> {
        match self {
            Expr_::As(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_upcast_mut(&mut self) -> Option<(&mut Expr<Ex, En>, &mut Hint)> {
        match self {
            Expr_::Upcast(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_new_mut(
        &mut self,
    ) -> Option<(
        &mut ClassId<Ex, En>,
        &mut Vec<Targ<Ex>>,
        &mut Vec<Expr<Ex, En>>,
        &mut Option<Expr<Ex, En>>,
        &mut Ex,
    )> {
        match self {
            Expr_::New(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3, &mut p0.4)),
            _ => None,
        }
    }
    pub fn as_efun_mut(&mut self) -> Option<&mut Efun<Ex, En>> {
        match self {
            Expr_::Efun(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_lfun_mut(&mut self) -> Option<(&mut Fun_<Ex, En>, &mut Vec<CaptureLid<Ex>>)> {
        match self {
            Expr_::Lfun(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_xml_mut(
        &mut self,
    ) -> Option<(
        &mut ClassName,
        &mut Vec<XhpAttribute<Ex, En>>,
        &mut Vec<Expr<Ex, En>>,
    )> {
        match self {
            Expr_::Xml(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_import_mut(&mut self) -> Option<(&mut ImportFlavor, &mut Expr<Ex, En>)> {
        match self {
            Expr_::Import(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_collection_mut(
        &mut self,
    ) -> Option<(
        &mut ClassName,
        &mut Option<CollectionTarg<Ex>>,
        &mut Vec<Afield<Ex, En>>,
    )> {
        match self {
            Expr_::Collection(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_expression_tree_mut(&mut self) -> Option<&mut ExpressionTree<Ex, En>> {
        match self {
            Expr_::ExpressionTree(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_lplaceholder_mut(&mut self) -> Option<&mut Pos> {
        match self {
            Expr_::Lplaceholder(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_method_caller_mut(&mut self) -> Option<(&mut ClassName, &mut Pstring)> {
        match self {
            Expr_::MethodCaller(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_pair_mut(
        &mut self,
    ) -> Option<(
        &mut Option<(Targ<Ex>, Targ<Ex>)>,
        &mut Expr<Ex, En>,
        &mut Expr<Ex, En>,
    )> {
        match self {
            Expr_::Pair(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_etsplice_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            Expr_::ETSplice(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_enum_class_label_mut(&mut self) -> Option<(&mut Option<ClassName>, &mut String)> {
        match self {
            Expr_::EnumClassLabel(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_hole_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, En>, &mut Ex, &mut Ex, &mut HoleSource)> {
        match self {
            Expr_::Hole(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3)),
            _ => None,
        }
    }
    pub fn as_package_mut(&mut self) -> Option<&mut Sid> {
        match self {
            Expr_::Package(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_nameof_mut(&mut self) -> Option<&mut ClassId<Ex, En>> {
        match self {
            Expr_::Nameof(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_darray_into(
        self,
    ) -> Option<(
        Option<(Targ<Ex>, Targ<Ex>)>,
        Vec<(Expr<Ex, En>, Expr<Ex, En>)>,
    )> {
        match self {
            Expr_::Darray(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_varray_into(self) -> Option<(Option<Targ<Ex>>, Vec<Expr<Ex, En>>)> {
        match self {
            Expr_::Varray(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_shape_into(self) -> Option<Vec<(ast_defs::ShapeFieldName, Expr<Ex, En>)>> {
        match self {
            Expr_::Shape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_val_collection_into(
        self,
    ) -> Option<((Pos, VcKind), Option<Targ<Ex>>, Vec<Expr<Ex, En>>)> {
        match self {
            Expr_::ValCollection(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_key_val_collection_into(
        self,
    ) -> Option<(
        (Pos, KvcKind),
        Option<(Targ<Ex>, Targ<Ex>)>,
        Vec<Field<Ex, En>>,
    )> {
        match self {
            Expr_::KeyValCollection(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_invalid_into(self) -> Option<Option<Expr<Ex, En>>> {
        match self {
            Expr_::Invalid(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_id_into(self) -> Option<Sid> {
        match self {
            Expr_::Id(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_lvar_into(self) -> Option<Lid> {
        match self {
            Expr_::Lvar(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_dollardollar_into(self) -> Option<Lid> {
        match self {
            Expr_::Dollardollar(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_clone_into(self) -> Option<Expr<Ex, En>> {
        match self {
            Expr_::Clone(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_array_get_into(self) -> Option<(Expr<Ex, En>, Option<Expr<Ex, En>>)> {
        match self {
            Expr_::ArrayGet(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_obj_get_into(
        self,
    ) -> Option<(Expr<Ex, En>, Expr<Ex, En>, OgNullFlavor, PropOrMethod)> {
        match self {
            Expr_::ObjGet(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3)),
            _ => None,
        }
    }
    pub fn as_class_get_into(
        self,
    ) -> Option<(ClassId<Ex, En>, ClassGetExpr<Ex, En>, PropOrMethod)> {
        match self {
            Expr_::ClassGet(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_class_const_into(self) -> Option<(ClassId<Ex, En>, Pstring)> {
        match self {
            Expr_::ClassConst(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_call_into(self) -> Option<CallExpr<Ex, En>> {
        match self {
            Expr_::Call(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_function_pointer_into(self) -> Option<(FunctionPtrId<Ex, En>, Vec<Targ<Ex>>)> {
        match self {
            Expr_::FunctionPointer(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_int_into(self) -> Option<String> {
        match self {
            Expr_::Int(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_float_into(self) -> Option<String> {
        match self {
            Expr_::Float(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_string_into(self) -> Option<bstr::BString> {
        match self {
            Expr_::String(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_string2_into(self) -> Option<Vec<Expr<Ex, En>>> {
        match self {
            Expr_::String2(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_prefixed_string_into(self) -> Option<(String, Expr<Ex, En>)> {
        match self {
            Expr_::PrefixedString(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_yield_into(self) -> Option<Afield<Ex, En>> {
        match self {
            Expr_::Yield(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_await_into(self) -> Option<Expr<Ex, En>> {
        match self {
            Expr_::Await(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_readonly_expr_into(self) -> Option<Expr<Ex, En>> {
        match self {
            Expr_::ReadonlyExpr(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_tuple_into(self) -> Option<Vec<Expr<Ex, En>>> {
        match self {
            Expr_::Tuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_list_into(self) -> Option<Vec<Expr<Ex, En>>> {
        match self {
            Expr_::List(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cast_into(self) -> Option<(Hint, Expr<Ex, En>)> {
        match self {
            Expr_::Cast(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_unop_into(self) -> Option<(ast_defs::Uop, Expr<Ex, En>)> {
        match self {
            Expr_::Unop(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_binop_into(self) -> Option<Binop<Ex, En>> {
        match self {
            Expr_::Binop(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_pipe_into(self) -> Option<(Lid, Expr<Ex, En>, Expr<Ex, En>)> {
        match self {
            Expr_::Pipe(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_eif_into(self) -> Option<(Expr<Ex, En>, Option<Expr<Ex, En>>, Expr<Ex, En>)> {
        match self {
            Expr_::Eif(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_is_into(self) -> Option<(Expr<Ex, En>, Hint)> {
        match self {
            Expr_::Is(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_as_into(self) -> Option<(Expr<Ex, En>, Hint, bool)> {
        match self {
            Expr_::As(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_upcast_into(self) -> Option<(Expr<Ex, En>, Hint)> {
        match self {
            Expr_::Upcast(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_new_into(
        self,
    ) -> Option<(
        ClassId<Ex, En>,
        Vec<Targ<Ex>>,
        Vec<Expr<Ex, En>>,
        Option<Expr<Ex, En>>,
        Ex,
    )> {
        match self {
            Expr_::New(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3, (*p0).4)),
            _ => None,
        }
    }
    pub fn as_efun_into(self) -> Option<Efun<Ex, En>> {
        match self {
            Expr_::Efun(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_lfun_into(self) -> Option<(Fun_<Ex, En>, Vec<CaptureLid<Ex>>)> {
        match self {
            Expr_::Lfun(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_xml_into(self) -> Option<(ClassName, Vec<XhpAttribute<Ex, En>>, Vec<Expr<Ex, En>>)> {
        match self {
            Expr_::Xml(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_import_into(self) -> Option<(ImportFlavor, Expr<Ex, En>)> {
        match self {
            Expr_::Import(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_collection_into(
        self,
    ) -> Option<(ClassName, Option<CollectionTarg<Ex>>, Vec<Afield<Ex, En>>)> {
        match self {
            Expr_::Collection(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_expression_tree_into(self) -> Option<ExpressionTree<Ex, En>> {
        match self {
            Expr_::ExpressionTree(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_lplaceholder_into(self) -> Option<Pos> {
        match self {
            Expr_::Lplaceholder(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_method_caller_into(self) -> Option<(ClassName, Pstring)> {
        match self {
            Expr_::MethodCaller(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_pair_into(
        self,
    ) -> Option<(Option<(Targ<Ex>, Targ<Ex>)>, Expr<Ex, En>, Expr<Ex, En>)> {
        match self {
            Expr_::Pair(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_etsplice_into(self) -> Option<Expr<Ex, En>> {
        match self {
            Expr_::ETSplice(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_enum_class_label_into(self) -> Option<(Option<ClassName>, String)> {
        match self {
            Expr_::EnumClassLabel(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_hole_into(self) -> Option<(Expr<Ex, En>, Ex, Ex, HoleSource)> {
        match self {
            Expr_::Hole(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3)),
            _ => None,
        }
    }
    pub fn as_package_into(self) -> Option<Sid> {
        match self {
            Expr_::Package(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_nameof_into(self) -> Option<ClassId<Ex, En>> {
        match self {
            Expr_::Nameof(p0) => Some(*p0),
            _ => None,
        }
    }
}
impl HoleSource {
    pub fn mk_typing() -> Self {
        HoleSource::Typing
    }
    pub fn mk_unsafe_cast(p0: Vec<Hint>) -> Self {
        HoleSource::UnsafeCast(p0)
    }
    pub fn mk_unsafe_nonnull_cast() -> Self {
        HoleSource::UnsafeNonnullCast
    }
    pub fn mk_enforced_cast(p0: Vec<Hint>) -> Self {
        HoleSource::EnforcedCast(p0)
    }
    pub fn is_typing(&self) -> bool {
        match self {
            HoleSource::Typing => true,
            _ => false,
        }
    }
    pub fn is_unsafe_cast(&self) -> bool {
        match self {
            HoleSource::UnsafeCast(..) => true,
            _ => false,
        }
    }
    pub fn is_unsafe_nonnull_cast(&self) -> bool {
        match self {
            HoleSource::UnsafeNonnullCast => true,
            _ => false,
        }
    }
    pub fn is_enforced_cast(&self) -> bool {
        match self {
            HoleSource::EnforcedCast(..) => true,
            _ => false,
        }
    }
    pub fn as_unsafe_cast(&self) -> Option<&Vec<Hint>> {
        match self {
            HoleSource::UnsafeCast(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_enforced_cast(&self) -> Option<&Vec<Hint>> {
        match self {
            HoleSource::EnforcedCast(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_unsafe_cast_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            HoleSource::UnsafeCast(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_enforced_cast_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            HoleSource::EnforcedCast(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_unsafe_cast_into(self) -> Option<Vec<Hint>> {
        match self {
            HoleSource::UnsafeCast(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_enforced_cast_into(self) -> Option<Vec<Hint>> {
        match self {
            HoleSource::EnforcedCast(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, En> ClassGetExpr<Ex, En> {
    pub fn mk_cgstring(p0: Pstring) -> Self {
        ClassGetExpr::CGstring(p0)
    }
    pub fn mk_cgexpr(p0: Expr<Ex, En>) -> Self {
        ClassGetExpr::CGexpr(p0)
    }
    pub fn is_cgstring(&self) -> bool {
        match self {
            ClassGetExpr::CGstring(..) => true,
            _ => false,
        }
    }
    pub fn is_cgexpr(&self) -> bool {
        match self {
            ClassGetExpr::CGexpr(..) => true,
            _ => false,
        }
    }
    pub fn as_cgstring(&self) -> Option<&Pstring> {
        match self {
            ClassGetExpr::CGstring(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cgexpr(&self) -> Option<&Expr<Ex, En>> {
        match self {
            ClassGetExpr::CGexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cgstring_mut(&mut self) -> Option<&mut Pstring> {
        match self {
            ClassGetExpr::CGstring(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cgexpr_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            ClassGetExpr::CGexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cgstring_into(self) -> Option<Pstring> {
        match self {
            ClassGetExpr::CGstring(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cgexpr_into(self) -> Option<Expr<Ex, En>> {
        match self {
            ClassGetExpr::CGexpr(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, En> GenCase<Ex, En> {
    pub fn mk_case(p0: Case<Ex, En>) -> Self {
        GenCase::Case(p0)
    }
    pub fn mk_default(p0: DefaultCase<Ex, En>) -> Self {
        GenCase::Default(p0)
    }
    pub fn is_case(&self) -> bool {
        match self {
            GenCase::Case(..) => true,
            _ => false,
        }
    }
    pub fn is_default(&self) -> bool {
        match self {
            GenCase::Default(..) => true,
            _ => false,
        }
    }
    pub fn as_case(&self) -> Option<&Case<Ex, En>> {
        match self {
            GenCase::Case(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_default(&self) -> Option<&DefaultCase<Ex, En>> {
        match self {
            GenCase::Default(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_case_mut(&mut self) -> Option<&mut Case<Ex, En>> {
        match self {
            GenCase::Case(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_default_mut(&mut self) -> Option<&mut DefaultCase<Ex, En>> {
        match self {
            GenCase::Default(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_case_into(self) -> Option<Case<Ex, En>> {
        match self {
            GenCase::Case(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_default_into(self) -> Option<DefaultCase<Ex, En>> {
        match self {
            GenCase::Default(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, En> Afield<Ex, En> {
    pub fn mk_afvalue(p0: Expr<Ex, En>) -> Self {
        Afield::AFvalue(p0)
    }
    pub fn mk_afkvalue(p0: Expr<Ex, En>, p1: Expr<Ex, En>) -> Self {
        Afield::AFkvalue(p0, p1)
    }
    pub fn is_afvalue(&self) -> bool {
        match self {
            Afield::AFvalue(..) => true,
            _ => false,
        }
    }
    pub fn is_afkvalue(&self) -> bool {
        match self {
            Afield::AFkvalue(..) => true,
            _ => false,
        }
    }
    pub fn as_afvalue(&self) -> Option<&Expr<Ex, En>> {
        match self {
            Afield::AFvalue(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_afkvalue(&self) -> Option<(&Expr<Ex, En>, &Expr<Ex, En>)> {
        match self {
            Afield::AFkvalue(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_afvalue_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            Afield::AFvalue(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_afkvalue_mut(&mut self) -> Option<(&mut Expr<Ex, En>, &mut Expr<Ex, En>)> {
        match self {
            Afield::AFkvalue(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_afvalue_into(self) -> Option<Expr<Ex, En>> {
        match self {
            Afield::AFvalue(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_afkvalue_into(self) -> Option<(Expr<Ex, En>, Expr<Ex, En>)> {
        match self {
            Afield::AFkvalue(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl<Ex, En> XhpAttribute<Ex, En> {
    pub fn mk_xhp_simple(p0: XhpSimple<Ex, En>) -> Self {
        XhpAttribute::XhpSimple(p0)
    }
    pub fn mk_xhp_spread(p0: Expr<Ex, En>) -> Self {
        XhpAttribute::XhpSpread(p0)
    }
    pub fn is_xhp_simple(&self) -> bool {
        match self {
            XhpAttribute::XhpSimple(..) => true,
            _ => false,
        }
    }
    pub fn is_xhp_spread(&self) -> bool {
        match self {
            XhpAttribute::XhpSpread(..) => true,
            _ => false,
        }
    }
    pub fn as_xhp_simple(&self) -> Option<&XhpSimple<Ex, En>> {
        match self {
            XhpAttribute::XhpSimple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_spread(&self) -> Option<&Expr<Ex, En>> {
        match self {
            XhpAttribute::XhpSpread(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_simple_mut(&mut self) -> Option<&mut XhpSimple<Ex, En>> {
        match self {
            XhpAttribute::XhpSimple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_spread_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            XhpAttribute::XhpSpread(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_simple_into(self) -> Option<XhpSimple<Ex, En>> {
        match self {
            XhpAttribute::XhpSimple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_spread_into(self) -> Option<Expr<Ex, En>> {
        match self {
            XhpAttribute::XhpSpread(p0) => Some(p0),
            _ => None,
        }
    }
}
impl RequireKind {
    pub fn mk_require_extends() -> Self {
        RequireKind::RequireExtends
    }
    pub fn mk_require_implements() -> Self {
        RequireKind::RequireImplements
    }
    pub fn mk_require_class() -> Self {
        RequireKind::RequireClass
    }
    pub fn is_require_extends(&self) -> bool {
        match self {
            RequireKind::RequireExtends => true,
            _ => false,
        }
    }
    pub fn is_require_implements(&self) -> bool {
        match self {
            RequireKind::RequireImplements => true,
            _ => false,
        }
    }
    pub fn is_require_class(&self) -> bool {
        match self {
            RequireKind::RequireClass => true,
            _ => false,
        }
    }
}
impl EmitId {
    pub fn mk_emit_id(p0: isize) -> Self {
        EmitId::EmitId(p0)
    }
    pub fn mk_anonymous() -> Self {
        EmitId::Anonymous
    }
    pub fn is_emit_id(&self) -> bool {
        match self {
            EmitId::EmitId(..) => true,
            _ => false,
        }
    }
    pub fn is_anonymous(&self) -> bool {
        match self {
            EmitId::Anonymous => true,
            _ => false,
        }
    }
    pub fn as_emit_id(&self) -> Option<&isize> {
        match self {
            EmitId::EmitId(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_emit_id_mut(&mut self) -> Option<&mut isize> {
        match self {
            EmitId::EmitId(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_emit_id_into(self) -> Option<isize> {
        match self {
            EmitId::EmitId(p0) => Some(p0),
            _ => None,
        }
    }
}
impl XhpAttrTag {
    pub fn mk_required() -> Self {
        XhpAttrTag::Required
    }
    pub fn mk_late_init() -> Self {
        XhpAttrTag::LateInit
    }
    pub fn is_required(&self) -> bool {
        match self {
            XhpAttrTag::Required => true,
            _ => false,
        }
    }
    pub fn is_late_init(&self) -> bool {
        match self {
            XhpAttrTag::LateInit => true,
            _ => false,
        }
    }
}
impl<Ex, En> ClassConstKind<Ex, En> {
    pub fn mk_ccabstract(p0: Option<Expr<Ex, En>>) -> Self {
        ClassConstKind::CCAbstract(p0)
    }
    pub fn mk_ccconcrete(p0: Expr<Ex, En>) -> Self {
        ClassConstKind::CCConcrete(p0)
    }
    pub fn is_ccabstract(&self) -> bool {
        match self {
            ClassConstKind::CCAbstract(..) => true,
            _ => false,
        }
    }
    pub fn is_ccconcrete(&self) -> bool {
        match self {
            ClassConstKind::CCConcrete(..) => true,
            _ => false,
        }
    }
    pub fn as_ccabstract(&self) -> Option<&Option<Expr<Ex, En>>> {
        match self {
            ClassConstKind::CCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccconcrete(&self) -> Option<&Expr<Ex, En>> {
        match self {
            ClassConstKind::CCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccabstract_mut(&mut self) -> Option<&mut Option<Expr<Ex, En>>> {
        match self {
            ClassConstKind::CCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccconcrete_mut(&mut self) -> Option<&mut Expr<Ex, En>> {
        match self {
            ClassConstKind::CCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccabstract_into(self) -> Option<Option<Expr<Ex, En>>> {
        match self {
            ClassConstKind::CCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccconcrete_into(self) -> Option<Expr<Ex, En>> {
        match self {
            ClassConstKind::CCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
}
impl ClassTypeconst {
    pub fn mk_tcabstract(p0: ClassAbstractTypeconst) -> Self {
        ClassTypeconst::TCAbstract(p0)
    }
    pub fn mk_tcconcrete(p0: ClassConcreteTypeconst) -> Self {
        ClassTypeconst::TCConcrete(p0)
    }
    pub fn is_tcabstract(&self) -> bool {
        match self {
            ClassTypeconst::TCAbstract(..) => true,
            _ => false,
        }
    }
    pub fn is_tcconcrete(&self) -> bool {
        match self {
            ClassTypeconst::TCConcrete(..) => true,
            _ => false,
        }
    }
    pub fn as_tcabstract(&self) -> Option<&ClassAbstractTypeconst> {
        match self {
            ClassTypeconst::TCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_tcconcrete(&self) -> Option<&ClassConcreteTypeconst> {
        match self {
            ClassTypeconst::TCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_tcabstract_mut(&mut self) -> Option<&mut ClassAbstractTypeconst> {
        match self {
            ClassTypeconst::TCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_tcconcrete_mut(&mut self) -> Option<&mut ClassConcreteTypeconst> {
        match self {
            ClassTypeconst::TCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_tcabstract_into(self) -> Option<ClassAbstractTypeconst> {
        match self {
            ClassTypeconst::TCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_tcconcrete_into(self) -> Option<ClassConcreteTypeconst> {
        match self {
            ClassTypeconst::TCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
}
impl MdNameKind {
    pub fn mk_mdname_global(p0: Pos) -> Self {
        MdNameKind::MDNameGlobal(p0)
    }
    pub fn mk_mdname_prefix(p0: Sid) -> Self {
        MdNameKind::MDNamePrefix(p0)
    }
    pub fn mk_mdname_exact(p0: Sid) -> Self {
        MdNameKind::MDNameExact(p0)
    }
    pub fn is_mdname_global(&self) -> bool {
        match self {
            MdNameKind::MDNameGlobal(..) => true,
            _ => false,
        }
    }
    pub fn is_mdname_prefix(&self) -> bool {
        match self {
            MdNameKind::MDNamePrefix(..) => true,
            _ => false,
        }
    }
    pub fn is_mdname_exact(&self) -> bool {
        match self {
            MdNameKind::MDNameExact(..) => true,
            _ => false,
        }
    }
    pub fn as_mdname_global(&self) -> Option<&Pos> {
        match self {
            MdNameKind::MDNameGlobal(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_prefix(&self) -> Option<&Sid> {
        match self {
            MdNameKind::MDNamePrefix(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_exact(&self) -> Option<&Sid> {
        match self {
            MdNameKind::MDNameExact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_global_mut(&mut self) -> Option<&mut Pos> {
        match self {
            MdNameKind::MDNameGlobal(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_prefix_mut(&mut self) -> Option<&mut Sid> {
        match self {
            MdNameKind::MDNamePrefix(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_exact_mut(&mut self) -> Option<&mut Sid> {
        match self {
            MdNameKind::MDNameExact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_global_into(self) -> Option<Pos> {
        match self {
            MdNameKind::MDNameGlobal(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_prefix_into(self) -> Option<Sid> {
        match self {
            MdNameKind::MDNamePrefix(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_mdname_exact_into(self) -> Option<Sid> {
        match self {
            MdNameKind::MDNameExact(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, En> Def<Ex, En> {
    pub fn mk_fun(p0: FunDef<Ex, En>) -> Self {
        Def::Fun(Box::new(p0))
    }
    pub fn mk_class(p0: Class_<Ex, En>) -> Self {
        Def::Class(Box::new(p0))
    }
    pub fn mk_stmt(p0: Stmt<Ex, En>) -> Self {
        Def::Stmt(Box::new(p0))
    }
    pub fn mk_typedef(p0: Typedef<Ex, En>) -> Self {
        Def::Typedef(Box::new(p0))
    }
    pub fn mk_constant(p0: Gconst<Ex, En>) -> Self {
        Def::Constant(Box::new(p0))
    }
    pub fn mk_namespace(p0: Sid, p1: Vec<Def<Ex, En>>) -> Self {
        Def::Namespace(Box::new((p0, p1)))
    }
    pub fn mk_namespace_use(p0: Vec<(NsKind, Sid, Sid)>) -> Self {
        Def::NamespaceUse(p0)
    }
    pub fn mk_set_namespace_env(p0: Nsenv) -> Self {
        Def::SetNamespaceEnv(Box::new(p0))
    }
    pub fn mk_file_attributes(p0: FileAttribute<Ex, En>) -> Self {
        Def::FileAttributes(Box::new(p0))
    }
    pub fn mk_module(p0: ModuleDef<Ex, En>) -> Self {
        Def::Module(Box::new(p0))
    }
    pub fn mk_set_module(p0: Sid) -> Self {
        Def::SetModule(Box::new(p0))
    }
    pub fn is_fun(&self) -> bool {
        match self {
            Def::Fun(..) => true,
            _ => false,
        }
    }
    pub fn is_class(&self) -> bool {
        match self {
            Def::Class(..) => true,
            _ => false,
        }
    }
    pub fn is_stmt(&self) -> bool {
        match self {
            Def::Stmt(..) => true,
            _ => false,
        }
    }
    pub fn is_typedef(&self) -> bool {
        match self {
            Def::Typedef(..) => true,
            _ => false,
        }
    }
    pub fn is_constant(&self) -> bool {
        match self {
            Def::Constant(..) => true,
            _ => false,
        }
    }
    pub fn is_namespace(&self) -> bool {
        match self {
            Def::Namespace(..) => true,
            _ => false,
        }
    }
    pub fn is_namespace_use(&self) -> bool {
        match self {
            Def::NamespaceUse(..) => true,
            _ => false,
        }
    }
    pub fn is_set_namespace_env(&self) -> bool {
        match self {
            Def::SetNamespaceEnv(..) => true,
            _ => false,
        }
    }
    pub fn is_file_attributes(&self) -> bool {
        match self {
            Def::FileAttributes(..) => true,
            _ => false,
        }
    }
    pub fn is_module(&self) -> bool {
        match self {
            Def::Module(..) => true,
            _ => false,
        }
    }
    pub fn is_set_module(&self) -> bool {
        match self {
            Def::SetModule(..) => true,
            _ => false,
        }
    }
    pub fn as_fun(&self) -> Option<&FunDef<Ex, En>> {
        match self {
            Def::Fun(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_class(&self) -> Option<&Class_<Ex, En>> {
        match self {
            Def::Class(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_stmt(&self) -> Option<&Stmt<Ex, En>> {
        match self {
            Def::Stmt(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_typedef(&self) -> Option<&Typedef<Ex, En>> {
        match self {
            Def::Typedef(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_constant(&self) -> Option<&Gconst<Ex, En>> {
        match self {
            Def::Constant(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_namespace(&self) -> Option<(&Sid, &Vec<Def<Ex, En>>)> {
        match self {
            Def::Namespace(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_namespace_use(&self) -> Option<&Vec<(NsKind, Sid, Sid)>> {
        match self {
            Def::NamespaceUse(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_set_namespace_env(&self) -> Option<&Nsenv> {
        match self {
            Def::SetNamespaceEnv(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_file_attributes(&self) -> Option<&FileAttribute<Ex, En>> {
        match self {
            Def::FileAttributes(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_module(&self) -> Option<&ModuleDef<Ex, En>> {
        match self {
            Def::Module(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_set_module(&self) -> Option<&Sid> {
        match self {
            Def::SetModule(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_fun_mut(&mut self) -> Option<&mut FunDef<Ex, En>> {
        match self {
            Def::Fun(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_class_mut(&mut self) -> Option<&mut Class_<Ex, En>> {
        match self {
            Def::Class(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_stmt_mut(&mut self) -> Option<&mut Stmt<Ex, En>> {
        match self {
            Def::Stmt(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_typedef_mut(&mut self) -> Option<&mut Typedef<Ex, En>> {
        match self {
            Def::Typedef(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_constant_mut(&mut self) -> Option<&mut Gconst<Ex, En>> {
        match self {
            Def::Constant(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_namespace_mut(&mut self) -> Option<(&mut Sid, &mut Vec<Def<Ex, En>>)> {
        match self {
            Def::Namespace(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_namespace_use_mut(&mut self) -> Option<&mut Vec<(NsKind, Sid, Sid)>> {
        match self {
            Def::NamespaceUse(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_set_namespace_env_mut(&mut self) -> Option<&mut Nsenv> {
        match self {
            Def::SetNamespaceEnv(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_file_attributes_mut(&mut self) -> Option<&mut FileAttribute<Ex, En>> {
        match self {
            Def::FileAttributes(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_module_mut(&mut self) -> Option<&mut ModuleDef<Ex, En>> {
        match self {
            Def::Module(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_set_module_mut(&mut self) -> Option<&mut Sid> {
        match self {
            Def::SetModule(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_fun_into(self) -> Option<FunDef<Ex, En>> {
        match self {
            Def::Fun(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_class_into(self) -> Option<Class_<Ex, En>> {
        match self {
            Def::Class(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_stmt_into(self) -> Option<Stmt<Ex, En>> {
        match self {
            Def::Stmt(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_typedef_into(self) -> Option<Typedef<Ex, En>> {
        match self {
            Def::Typedef(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_constant_into(self) -> Option<Gconst<Ex, En>> {
        match self {
            Def::Constant(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_namespace_into(self) -> Option<(Sid, Vec<Def<Ex, En>>)> {
        match self {
            Def::Namespace(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_namespace_use_into(self) -> Option<Vec<(NsKind, Sid, Sid)>> {
        match self {
            Def::NamespaceUse(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_set_namespace_env_into(self) -> Option<Nsenv> {
        match self {
            Def::SetNamespaceEnv(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_file_attributes_into(self) -> Option<FileAttribute<Ex, En>> {
        match self {
            Def::FileAttributes(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_module_into(self) -> Option<ModuleDef<Ex, En>> {
        match self {
            Def::Module(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_set_module_into(self) -> Option<Sid> {
        match self {
            Def::SetModule(p0) => Some(*p0),
            _ => None,
        }
    }
}
impl NsKind {
    pub fn mk_nsnamespace() -> Self {
        NsKind::NSNamespace
    }
    pub fn mk_nsclass() -> Self {
        NsKind::NSClass
    }
    pub fn mk_nsclass_and_namespace() -> Self {
        NsKind::NSClassAndNamespace
    }
    pub fn mk_nsfun() -> Self {
        NsKind::NSFun
    }
    pub fn mk_nsconst() -> Self {
        NsKind::NSConst
    }
    pub fn is_nsnamespace(&self) -> bool {
        match self {
            NsKind::NSNamespace => true,
            _ => false,
        }
    }
    pub fn is_nsclass(&self) -> bool {
        match self {
            NsKind::NSClass => true,
            _ => false,
        }
    }
    pub fn is_nsclass_and_namespace(&self) -> bool {
        match self {
            NsKind::NSClassAndNamespace => true,
            _ => false,
        }
    }
    pub fn is_nsfun(&self) -> bool {
        match self {
            NsKind::NSFun => true,
            _ => false,
        }
    }
    pub fn is_nsconst(&self) -> bool {
        match self {
            NsKind::NSConst => true,
            _ => false,
        }
    }
}
impl ImportFlavor {
    pub fn mk_include() -> Self {
        ImportFlavor::Include
    }
    pub fn mk_require() -> Self {
        ImportFlavor::Require
    }
    pub fn mk_include_once() -> Self {
        ImportFlavor::IncludeOnce
    }
    pub fn mk_require_once() -> Self {
        ImportFlavor::RequireOnce
    }
    pub fn is_include(&self) -> bool {
        match self {
            ImportFlavor::Include => true,
            _ => false,
        }
    }
    pub fn is_require(&self) -> bool {
        match self {
            ImportFlavor::Require => true,
            _ => false,
        }
    }
    pub fn is_include_once(&self) -> bool {
        match self {
            ImportFlavor::IncludeOnce => true,
            _ => false,
        }
    }
    pub fn is_require_once(&self) -> bool {
        match self {
            ImportFlavor::RequireOnce => true,
            _ => false,
        }
    }
}
impl XhpChild {
    pub fn mk_child_name(p0: Sid) -> Self {
        XhpChild::ChildName(p0)
    }
    pub fn mk_child_list(p0: Vec<XhpChild>) -> Self {
        XhpChild::ChildList(p0)
    }
    pub fn mk_child_unary(p0: XhpChild, p1: XhpChildOp) -> Self {
        XhpChild::ChildUnary(Box::new(p0), p1)
    }
    pub fn mk_child_binary(p0: XhpChild, p1: XhpChild) -> Self {
        XhpChild::ChildBinary(Box::new(p0), Box::new(p1))
    }
    pub fn is_child_name(&self) -> bool {
        match self {
            XhpChild::ChildName(..) => true,
            _ => false,
        }
    }
    pub fn is_child_list(&self) -> bool {
        match self {
            XhpChild::ChildList(..) => true,
            _ => false,
        }
    }
    pub fn is_child_unary(&self) -> bool {
        match self {
            XhpChild::ChildUnary(..) => true,
            _ => false,
        }
    }
    pub fn is_child_binary(&self) -> bool {
        match self {
            XhpChild::ChildBinary(..) => true,
            _ => false,
        }
    }
    pub fn as_child_name(&self) -> Option<&Sid> {
        match self {
            XhpChild::ChildName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_list(&self) -> Option<&Vec<XhpChild>> {
        match self {
            XhpChild::ChildList(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_unary(&self) -> Option<(&XhpChild, &XhpChildOp)> {
        match self {
            XhpChild::ChildUnary(p0, p1) => Some((&p0, p1)),
            _ => None,
        }
    }
    pub fn as_child_binary(&self) -> Option<(&XhpChild, &XhpChild)> {
        match self {
            XhpChild::ChildBinary(p0, p1) => Some((&p0, &p1)),
            _ => None,
        }
    }
    pub fn as_child_name_mut(&mut self) -> Option<&mut Sid> {
        match self {
            XhpChild::ChildName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_list_mut(&mut self) -> Option<&mut Vec<XhpChild>> {
        match self {
            XhpChild::ChildList(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_unary_mut(&mut self) -> Option<(&mut XhpChild, &mut XhpChildOp)> {
        match self {
            XhpChild::ChildUnary(p0, p1) => Some((p0.as_mut(), p1)),
            _ => None,
        }
    }
    pub fn as_child_binary_mut(&mut self) -> Option<(&mut XhpChild, &mut XhpChild)> {
        match self {
            XhpChild::ChildBinary(p0, p1) => Some((p0.as_mut(), p1.as_mut())),
            _ => None,
        }
    }
    pub fn as_child_name_into(self) -> Option<Sid> {
        match self {
            XhpChild::ChildName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_list_into(self) -> Option<Vec<XhpChild>> {
        match self {
            XhpChild::ChildList(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_child_unary_into(self) -> Option<(XhpChild, XhpChildOp)> {
        match self {
            XhpChild::ChildUnary(p0, p1) => Some((*p0, p1)),
            _ => None,
        }
    }
    pub fn as_child_binary_into(self) -> Option<(XhpChild, XhpChild)> {
        match self {
            XhpChild::ChildBinary(p0, p1) => Some((*p0, *p1)),
            _ => None,
        }
    }
}
impl XhpChildOp {
    pub fn mk_child_star() -> Self {
        XhpChildOp::ChildStar
    }
    pub fn mk_child_plus() -> Self {
        XhpChildOp::ChildPlus
    }
    pub fn mk_child_question() -> Self {
        XhpChildOp::ChildQuestion
    }
    pub fn is_child_star(&self) -> bool {
        match self {
            XhpChildOp::ChildStar => true,
            _ => false,
        }
    }
    pub fn is_child_plus(&self) -> bool {
        match self {
            XhpChildOp::ChildPlus => true,
            _ => false,
        }
    }
    pub fn is_child_question(&self) -> bool {
        match self {
            XhpChildOp::ChildQuestion => true,
            _ => false,
        }
    }
}
impl Hint_ {
    pub fn mk_hoption(p0: Hint) -> Self {
        Hint_::Hoption(p0)
    }
    pub fn mk_hlike(p0: Hint) -> Self {
        Hint_::Hlike(p0)
    }
    pub fn mk_hfun(p0: HintFun) -> Self {
        Hint_::Hfun(p0)
    }
    pub fn mk_htuple(p0: Vec<Hint>) -> Self {
        Hint_::Htuple(p0)
    }
    pub fn mk_happly(p0: ClassName, p1: Vec<Hint>) -> Self {
        Hint_::Happly(p0, p1)
    }
    pub fn mk_hclass_args(p0: Hint) -> Self {
        Hint_::HclassArgs(p0)
    }
    pub fn mk_hshape(p0: NastShapeInfo) -> Self {
        Hint_::Hshape(p0)
    }
    pub fn mk_haccess(p0: Hint, p1: Vec<Sid>) -> Self {
        Hint_::Haccess(p0, p1)
    }
    pub fn mk_hsoft(p0: Hint) -> Self {
        Hint_::Hsoft(p0)
    }
    pub fn mk_hrefinement(p0: Hint, p1: Vec<Refinement>) -> Self {
        Hint_::Hrefinement(p0, p1)
    }
    pub fn mk_hany() -> Self {
        Hint_::Hany
    }
    pub fn mk_herr() -> Self {
        Hint_::Herr
    }
    pub fn mk_hmixed() -> Self {
        Hint_::Hmixed
    }
    pub fn mk_hwildcard() -> Self {
        Hint_::Hwildcard
    }
    pub fn mk_hnonnull() -> Self {
        Hint_::Hnonnull
    }
    pub fn mk_habstr(p0: String, p1: Vec<Hint>) -> Self {
        Hint_::Habstr(p0, p1)
    }
    pub fn mk_hvec_or_dict(p0: Option<Hint>, p1: Hint) -> Self {
        Hint_::HvecOrDict(p0, p1)
    }
    pub fn mk_hprim(p0: Tprim) -> Self {
        Hint_::Hprim(p0)
    }
    pub fn mk_hthis() -> Self {
        Hint_::Hthis
    }
    pub fn mk_hdynamic() -> Self {
        Hint_::Hdynamic
    }
    pub fn mk_hnothing() -> Self {
        Hint_::Hnothing
    }
    pub fn mk_hunion(p0: Vec<Hint>) -> Self {
        Hint_::Hunion(p0)
    }
    pub fn mk_hintersection(p0: Vec<Hint>) -> Self {
        Hint_::Hintersection(p0)
    }
    pub fn mk_hfun_context(p0: String) -> Self {
        Hint_::HfunContext(p0)
    }
    pub fn mk_hvar(p0: String) -> Self {
        Hint_::Hvar(p0)
    }
    pub fn is_hoption(&self) -> bool {
        match self {
            Hint_::Hoption(..) => true,
            _ => false,
        }
    }
    pub fn is_hlike(&self) -> bool {
        match self {
            Hint_::Hlike(..) => true,
            _ => false,
        }
    }
    pub fn is_hfun(&self) -> bool {
        match self {
            Hint_::Hfun(..) => true,
            _ => false,
        }
    }
    pub fn is_htuple(&self) -> bool {
        match self {
            Hint_::Htuple(..) => true,
            _ => false,
        }
    }
    pub fn is_happly(&self) -> bool {
        match self {
            Hint_::Happly(..) => true,
            _ => false,
        }
    }
    pub fn is_hclass_args(&self) -> bool {
        match self {
            Hint_::HclassArgs(..) => true,
            _ => false,
        }
    }
    pub fn is_hshape(&self) -> bool {
        match self {
            Hint_::Hshape(..) => true,
            _ => false,
        }
    }
    pub fn is_haccess(&self) -> bool {
        match self {
            Hint_::Haccess(..) => true,
            _ => false,
        }
    }
    pub fn is_hsoft(&self) -> bool {
        match self {
            Hint_::Hsoft(..) => true,
            _ => false,
        }
    }
    pub fn is_hrefinement(&self) -> bool {
        match self {
            Hint_::Hrefinement(..) => true,
            _ => false,
        }
    }
    pub fn is_hany(&self) -> bool {
        match self {
            Hint_::Hany => true,
            _ => false,
        }
    }
    pub fn is_herr(&self) -> bool {
        match self {
            Hint_::Herr => true,
            _ => false,
        }
    }
    pub fn is_hmixed(&self) -> bool {
        match self {
            Hint_::Hmixed => true,
            _ => false,
        }
    }
    pub fn is_hwildcard(&self) -> bool {
        match self {
            Hint_::Hwildcard => true,
            _ => false,
        }
    }
    pub fn is_hnonnull(&self) -> bool {
        match self {
            Hint_::Hnonnull => true,
            _ => false,
        }
    }
    pub fn is_habstr(&self) -> bool {
        match self {
            Hint_::Habstr(..) => true,
            _ => false,
        }
    }
    pub fn is_hvec_or_dict(&self) -> bool {
        match self {
            Hint_::HvecOrDict(..) => true,
            _ => false,
        }
    }
    pub fn is_hprim(&self) -> bool {
        match self {
            Hint_::Hprim(..) => true,
            _ => false,
        }
    }
    pub fn is_hthis(&self) -> bool {
        match self {
            Hint_::Hthis => true,
            _ => false,
        }
    }
    pub fn is_hdynamic(&self) -> bool {
        match self {
            Hint_::Hdynamic => true,
            _ => false,
        }
    }
    pub fn is_hnothing(&self) -> bool {
        match self {
            Hint_::Hnothing => true,
            _ => false,
        }
    }
    pub fn is_hunion(&self) -> bool {
        match self {
            Hint_::Hunion(..) => true,
            _ => false,
        }
    }
    pub fn is_hintersection(&self) -> bool {
        match self {
            Hint_::Hintersection(..) => true,
            _ => false,
        }
    }
    pub fn is_hfun_context(&self) -> bool {
        match self {
            Hint_::HfunContext(..) => true,
            _ => false,
        }
    }
    pub fn is_hvar(&self) -> bool {
        match self {
            Hint_::Hvar(..) => true,
            _ => false,
        }
    }
    pub fn as_hoption(&self) -> Option<&Hint> {
        match self {
            Hint_::Hoption(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hlike(&self) -> Option<&Hint> {
        match self {
            Hint_::Hlike(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun(&self) -> Option<&HintFun> {
        match self {
            Hint_::Hfun(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_htuple(&self) -> Option<&Vec<Hint>> {
        match self {
            Hint_::Htuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_happly(&self) -> Option<(&ClassName, &Vec<Hint>)> {
        match self {
            Hint_::Happly(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hclass_args(&self) -> Option<&Hint> {
        match self {
            Hint_::HclassArgs(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hshape(&self) -> Option<&NastShapeInfo> {
        match self {
            Hint_::Hshape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_haccess(&self) -> Option<(&Hint, &Vec<Sid>)> {
        match self {
            Hint_::Haccess(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hsoft(&self) -> Option<&Hint> {
        match self {
            Hint_::Hsoft(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hrefinement(&self) -> Option<(&Hint, &Vec<Refinement>)> {
        match self {
            Hint_::Hrefinement(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_habstr(&self) -> Option<(&String, &Vec<Hint>)> {
        match self {
            Hint_::Habstr(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hvec_or_dict(&self) -> Option<(&Option<Hint>, &Hint)> {
        match self {
            Hint_::HvecOrDict(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hprim(&self) -> Option<&Tprim> {
        match self {
            Hint_::Hprim(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hunion(&self) -> Option<&Vec<Hint>> {
        match self {
            Hint_::Hunion(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hintersection(&self) -> Option<&Vec<Hint>> {
        match self {
            Hint_::Hintersection(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun_context(&self) -> Option<&String> {
        match self {
            Hint_::HfunContext(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hvar(&self) -> Option<&String> {
        match self {
            Hint_::Hvar(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hoption_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::Hoption(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hlike_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::Hlike(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun_mut(&mut self) -> Option<&mut HintFun> {
        match self {
            Hint_::Hfun(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_htuple_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            Hint_::Htuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_happly_mut(&mut self) -> Option<(&mut ClassName, &mut Vec<Hint>)> {
        match self {
            Hint_::Happly(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hclass_args_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::HclassArgs(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hshape_mut(&mut self) -> Option<&mut NastShapeInfo> {
        match self {
            Hint_::Hshape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_haccess_mut(&mut self) -> Option<(&mut Hint, &mut Vec<Sid>)> {
        match self {
            Hint_::Haccess(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hsoft_mut(&mut self) -> Option<&mut Hint> {
        match self {
            Hint_::Hsoft(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hrefinement_mut(&mut self) -> Option<(&mut Hint, &mut Vec<Refinement>)> {
        match self {
            Hint_::Hrefinement(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_habstr_mut(&mut self) -> Option<(&mut String, &mut Vec<Hint>)> {
        match self {
            Hint_::Habstr(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hvec_or_dict_mut(&mut self) -> Option<(&mut Option<Hint>, &mut Hint)> {
        match self {
            Hint_::HvecOrDict(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hprim_mut(&mut self) -> Option<&mut Tprim> {
        match self {
            Hint_::Hprim(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hunion_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            Hint_::Hunion(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hintersection_mut(&mut self) -> Option<&mut Vec<Hint>> {
        match self {
            Hint_::Hintersection(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun_context_mut(&mut self) -> Option<&mut String> {
        match self {
            Hint_::HfunContext(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hvar_mut(&mut self) -> Option<&mut String> {
        match self {
            Hint_::Hvar(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hoption_into(self) -> Option<Hint> {
        match self {
            Hint_::Hoption(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hlike_into(self) -> Option<Hint> {
        match self {
            Hint_::Hlike(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun_into(self) -> Option<HintFun> {
        match self {
            Hint_::Hfun(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_htuple_into(self) -> Option<Vec<Hint>> {
        match self {
            Hint_::Htuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_happly_into(self) -> Option<(ClassName, Vec<Hint>)> {
        match self {
            Hint_::Happly(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hclass_args_into(self) -> Option<Hint> {
        match self {
            Hint_::HclassArgs(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hshape_into(self) -> Option<NastShapeInfo> {
        match self {
            Hint_::Hshape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_haccess_into(self) -> Option<(Hint, Vec<Sid>)> {
        match self {
            Hint_::Haccess(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hsoft_into(self) -> Option<Hint> {
        match self {
            Hint_::Hsoft(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hrefinement_into(self) -> Option<(Hint, Vec<Refinement>)> {
        match self {
            Hint_::Hrefinement(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_habstr_into(self) -> Option<(String, Vec<Hint>)> {
        match self {
            Hint_::Habstr(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hvec_or_dict_into(self) -> Option<(Option<Hint>, Hint)> {
        match self {
            Hint_::HvecOrDict(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_hprim_into(self) -> Option<Tprim> {
        match self {
            Hint_::Hprim(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hunion_into(self) -> Option<Vec<Hint>> {
        match self {
            Hint_::Hunion(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hintersection_into(self) -> Option<Vec<Hint>> {
        match self {
            Hint_::Hintersection(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hfun_context_into(self) -> Option<String> {
        match self {
            Hint_::HfunContext(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_hvar_into(self) -> Option<String> {
        match self {
            Hint_::Hvar(p0) => Some(p0),
            _ => None,
        }
    }
}
impl Refinement {
    pub fn mk_rctx(p0: Sid, p1: CtxRefinement) -> Self {
        Refinement::Rctx(p0, p1)
    }
    pub fn mk_rtype(p0: Sid, p1: TypeRefinement) -> Self {
        Refinement::Rtype(p0, p1)
    }
    pub fn is_rctx(&self) -> bool {
        match self {
            Refinement::Rctx(..) => true,
            _ => false,
        }
    }
    pub fn is_rtype(&self) -> bool {
        match self {
            Refinement::Rtype(..) => true,
            _ => false,
        }
    }
    pub fn as_rctx(&self) -> Option<(&Sid, &CtxRefinement)> {
        match self {
            Refinement::Rctx(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_rtype(&self) -> Option<(&Sid, &TypeRefinement)> {
        match self {
            Refinement::Rtype(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_rctx_mut(&mut self) -> Option<(&mut Sid, &mut CtxRefinement)> {
        match self {
            Refinement::Rctx(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_rtype_mut(&mut self) -> Option<(&mut Sid, &mut TypeRefinement)> {
        match self {
            Refinement::Rtype(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_rctx_into(self) -> Option<(Sid, CtxRefinement)> {
        match self {
            Refinement::Rctx(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_rtype_into(self) -> Option<(Sid, TypeRefinement)> {
        match self {
            Refinement::Rtype(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl TypeRefinement {
    pub fn mk_trexact(p0: Hint) -> Self {
        TypeRefinement::TRexact(p0)
    }
    pub fn mk_trloose(p0: TypeRefinementBounds) -> Self {
        TypeRefinement::TRloose(p0)
    }
    pub fn is_trexact(&self) -> bool {
        match self {
            TypeRefinement::TRexact(..) => true,
            _ => false,
        }
    }
    pub fn is_trloose(&self) -> bool {
        match self {
            TypeRefinement::TRloose(..) => true,
            _ => false,
        }
    }
    pub fn as_trexact(&self) -> Option<&Hint> {
        match self {
            TypeRefinement::TRexact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_trloose(&self) -> Option<&TypeRefinementBounds> {
        match self {
            TypeRefinement::TRloose(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_trexact_mut(&mut self) -> Option<&mut Hint> {
        match self {
            TypeRefinement::TRexact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_trloose_mut(&mut self) -> Option<&mut TypeRefinementBounds> {
        match self {
            TypeRefinement::TRloose(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_trexact_into(self) -> Option<Hint> {
        match self {
            TypeRefinement::TRexact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_trloose_into(self) -> Option<TypeRefinementBounds> {
        match self {
            TypeRefinement::TRloose(p0) => Some(p0),
            _ => None,
        }
    }
}
impl CtxRefinement {
    pub fn mk_crexact(p0: Hint) -> Self {
        CtxRefinement::CRexact(p0)
    }
    pub fn mk_crloose(p0: CtxRefinementBounds) -> Self {
        CtxRefinement::CRloose(p0)
    }
    pub fn is_crexact(&self) -> bool {
        match self {
            CtxRefinement::CRexact(..) => true,
            _ => false,
        }
    }
    pub fn is_crloose(&self) -> bool {
        match self {
            CtxRefinement::CRloose(..) => true,
            _ => false,
        }
    }
    pub fn as_crexact(&self) -> Option<&Hint> {
        match self {
            CtxRefinement::CRexact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_crloose(&self) -> Option<&CtxRefinementBounds> {
        match self {
            CtxRefinement::CRloose(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_crexact_mut(&mut self) -> Option<&mut Hint> {
        match self {
            CtxRefinement::CRexact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_crloose_mut(&mut self) -> Option<&mut CtxRefinementBounds> {
        match self {
            CtxRefinement::CRloose(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_crexact_into(self) -> Option<Hint> {
        match self {
            CtxRefinement::CRexact(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_crloose_into(self) -> Option<CtxRefinementBounds> {
        match self {
            CtxRefinement::CRloose(p0) => Some(p0),
            _ => None,
        }
    }
}
impl KvcKind {
    pub fn mk_map() -> Self {
        KvcKind::Map
    }
    pub fn mk_imm_map() -> Self {
        KvcKind::ImmMap
    }
    pub fn mk_dict() -> Self {
        KvcKind::Dict
    }
    pub fn is_map(&self) -> bool {
        match self {
            KvcKind::Map => true,
            _ => false,
        }
    }
    pub fn is_imm_map(&self) -> bool {
        match self {
            KvcKind::ImmMap => true,
            _ => false,
        }
    }
    pub fn is_dict(&self) -> bool {
        match self {
            KvcKind::Dict => true,
            _ => false,
        }
    }
}
impl VcKind {
    pub fn mk_vector() -> Self {
        VcKind::Vector
    }
    pub fn mk_imm_vector() -> Self {
        VcKind::ImmVector
    }
    pub fn mk_vec() -> Self {
        VcKind::Vec
    }
    pub fn mk_set() -> Self {
        VcKind::Set
    }
    pub fn mk_imm_set() -> Self {
        VcKind::ImmSet
    }
    pub fn mk_keyset() -> Self {
        VcKind::Keyset
    }
    pub fn is_vector(&self) -> bool {
        match self {
            VcKind::Vector => true,
            _ => false,
        }
    }
    pub fn is_imm_vector(&self) -> bool {
        match self {
            VcKind::ImmVector => true,
            _ => false,
        }
    }
    pub fn is_vec(&self) -> bool {
        match self {
            VcKind::Vec => true,
            _ => false,
        }
    }
    pub fn is_set(&self) -> bool {
        match self {
            VcKind::Set => true,
            _ => false,
        }
    }
    pub fn is_imm_set(&self) -> bool {
        match self {
            VcKind::ImmSet => true,
            _ => false,
        }
    }
    pub fn is_keyset(&self) -> bool {
        match self {
            VcKind::Keyset => true,
            _ => false,
        }
    }
}
