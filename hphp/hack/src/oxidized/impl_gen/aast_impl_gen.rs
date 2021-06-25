// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<4b50a3363fe9f8b98afc2502e04907a8>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use crate::aast::*;
use crate::ast_defs;
use crate::LocalIdMap;
impl<Ex, Fb, En, Hi> Stmt_<Ex, Fb, En, Hi> {
    pub fn mk_fallthrough() -> Self {
        Stmt_::Fallthrough
    }
    pub fn mk_expr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Expr(Box::new(p0))
    }
    pub fn mk_break() -> Self {
        Stmt_::Break
    }
    pub fn mk_continue() -> Self {
        Stmt_::Continue
    }
    pub fn mk_throw(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Throw(Box::new(p0))
    }
    pub fn mk_return(p0: Option<Expr<Ex, Fb, En, Hi>>) -> Self {
        Stmt_::Return(Box::new(p0))
    }
    pub fn mk_yield_break() -> Self {
        Stmt_::YieldBreak
    }
    pub fn mk_awaitall(
        p0: Vec<(Option<Lid>, Expr<Ex, Fb, En, Hi>)>,
        p1: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::Awaitall(Box::new((p0, p1)))
    }
    pub fn mk_if(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Block<Ex, Fb, En, Hi>,
        p2: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::If(Box::new((p0, p1, p2)))
    }
    pub fn mk_do(p0: Block<Ex, Fb, En, Hi>, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Do(Box::new((p0, p1)))
    }
    pub fn mk_while(p0: Expr<Ex, Fb, En, Hi>, p1: Block<Ex, Fb, En, Hi>) -> Self {
        Stmt_::While(Box::new((p0, p1)))
    }
    pub fn mk_using(p0: UsingStmt<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Using(Box::new(p0))
    }
    pub fn mk_for(
        p0: Vec<Expr<Ex, Fb, En, Hi>>,
        p1: Option<Expr<Ex, Fb, En, Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
        p3: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::For(Box::new((p0, p1, p2, p3)))
    }
    pub fn mk_switch(p0: Expr<Ex, Fb, En, Hi>, p1: Vec<Case<Ex, Fb, En, Hi>>) -> Self {
        Stmt_::Switch(Box::new((p0, p1)))
    }
    pub fn mk_foreach(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: AsExpr<Ex, Fb, En, Hi>,
        p2: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::Foreach(Box::new((p0, p1, p2)))
    }
    pub fn mk_try(
        p0: Block<Ex, Fb, En, Hi>,
        p1: Vec<Catch<Ex, Fb, En, Hi>>,
        p2: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::Try(Box::new((p0, p1, p2)))
    }
    pub fn mk_noop() -> Self {
        Stmt_::Noop
    }
    pub fn mk_block(p0: Block<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Block(p0)
    }
    pub fn mk_markup(p0: Pstring) -> Self {
        Stmt_::Markup(Box::new(p0))
    }
    pub fn mk_assert_env(p0: EnvAnnot, p1: LocalIdMap<Ex>) -> Self {
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
    pub fn as_expr(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Expr(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_throw(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Throw(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_return(&self) -> Option<&Option<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Stmt_::Return(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_awaitall(
        &self,
    ) -> Option<(
        &Vec<(Option<Lid>, Expr<Ex, Fb, En, Hi>)>,
        &Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Awaitall(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_if(
        &self,
    ) -> Option<(
        &Expr<Ex, Fb, En, Hi>,
        &Block<Ex, Fb, En, Hi>,
        &Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::If(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_do(&self) -> Option<(&Block<Ex, Fb, En, Hi>, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Stmt_::Do(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_while(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Block<Ex, Fb, En, Hi>)> {
        match self {
            Stmt_::While(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_using(&self) -> Option<&UsingStmt<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Using(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_for(
        &self,
    ) -> Option<(
        &Vec<Expr<Ex, Fb, En, Hi>>,
        &Option<Expr<Ex, Fb, En, Hi>>,
        &Vec<Expr<Ex, Fb, En, Hi>>,
        &Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::For(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3)),
            _ => None,
        }
    }
    pub fn as_switch(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Vec<Case<Ex, Fb, En, Hi>>)> {
        match self {
            Stmt_::Switch(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_foreach(
        &self,
    ) -> Option<(
        &Expr<Ex, Fb, En, Hi>,
        &AsExpr<Ex, Fb, En, Hi>,
        &Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Foreach(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_try(
        &self,
    ) -> Option<(
        &Block<Ex, Fb, En, Hi>,
        &Vec<Catch<Ex, Fb, En, Hi>>,
        &Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Try(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_block(&self) -> Option<&Block<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Block(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_markup(&self) -> Option<&Pstring> {
        match self {
            Stmt_::Markup(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_assert_env(&self) -> Option<(&EnvAnnot, &LocalIdMap<Ex>)> {
        match self {
            Stmt_::AssertEnv(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_expr_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Expr(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_throw_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Throw(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_return_mut(&mut self) -> Option<&mut Option<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Stmt_::Return(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_awaitall_mut(
        &mut self,
    ) -> Option<(
        &mut Vec<(Option<Lid>, Expr<Ex, Fb, En, Hi>)>,
        &mut Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Awaitall(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_if_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Block<Ex, Fb, En, Hi>,
        &mut Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::If(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_do_mut(&mut self) -> Option<(&mut Block<Ex, Fb, En, Hi>, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            Stmt_::Do(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_while_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Block<Ex, Fb, En, Hi>)> {
        match self {
            Stmt_::While(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_using_mut(&mut self) -> Option<&mut UsingStmt<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Using(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_for_mut(
        &mut self,
    ) -> Option<(
        &mut Vec<Expr<Ex, Fb, En, Hi>>,
        &mut Option<Expr<Ex, Fb, En, Hi>>,
        &mut Vec<Expr<Ex, Fb, En, Hi>>,
        &mut Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::For(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3)),
            _ => None,
        }
    }
    pub fn as_switch_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Vec<Case<Ex, Fb, En, Hi>>)> {
        match self {
            Stmt_::Switch(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_foreach_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, Fb, En, Hi>,
        &mut AsExpr<Ex, Fb, En, Hi>,
        &mut Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Foreach(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_try_mut(
        &mut self,
    ) -> Option<(
        &mut Block<Ex, Fb, En, Hi>,
        &mut Vec<Catch<Ex, Fb, En, Hi>>,
        &mut Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Try(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_block_mut(&mut self) -> Option<&mut Block<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Block(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_markup_mut(&mut self) -> Option<&mut Pstring> {
        match self {
            Stmt_::Markup(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_assert_env_mut(&mut self) -> Option<(&mut EnvAnnot, &mut LocalIdMap<Ex>)> {
        match self {
            Stmt_::AssertEnv(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_expr_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Expr(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_throw_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Throw(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_return_into(self) -> Option<Option<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Stmt_::Return(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_awaitall_into(
        self,
    ) -> Option<(
        Vec<(Option<Lid>, Expr<Ex, Fb, En, Hi>)>,
        Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Awaitall(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_if_into(
        self,
    ) -> Option<(
        Expr<Ex, Fb, En, Hi>,
        Block<Ex, Fb, En, Hi>,
        Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::If(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_do_into(self) -> Option<(Block<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Stmt_::Do(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_while_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Block<Ex, Fb, En, Hi>)> {
        match self {
            Stmt_::While(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_using_into(self) -> Option<UsingStmt<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Using(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_for_into(
        self,
    ) -> Option<(
        Vec<Expr<Ex, Fb, En, Hi>>,
        Option<Expr<Ex, Fb, En, Hi>>,
        Vec<Expr<Ex, Fb, En, Hi>>,
        Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::For(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3)),
            _ => None,
        }
    }
    pub fn as_switch_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Vec<Case<Ex, Fb, En, Hi>>)> {
        match self {
            Stmt_::Switch(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_foreach_into(
        self,
    ) -> Option<(
        Expr<Ex, Fb, En, Hi>,
        AsExpr<Ex, Fb, En, Hi>,
        Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Foreach(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_try_into(
        self,
    ) -> Option<(
        Block<Ex, Fb, En, Hi>,
        Vec<Catch<Ex, Fb, En, Hi>>,
        Block<Ex, Fb, En, Hi>,
    )> {
        match self {
            Stmt_::Try(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_block_into(self) -> Option<Block<Ex, Fb, En, Hi>> {
        match self {
            Stmt_::Block(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_markup_into(self) -> Option<Pstring> {
        match self {
            Stmt_::Markup(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_assert_env_into(self) -> Option<(EnvAnnot, LocalIdMap<Ex>)> {
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
impl<Ex, Fb, En, Hi> AsExpr<Ex, Fb, En, Hi> {
    pub fn mk_as_v(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        AsExpr::AsV(p0)
    }
    pub fn mk_as_kv(p0: Expr<Ex, Fb, En, Hi>, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        AsExpr::AsKv(p0, p1)
    }
    pub fn mk_await_as_v(p0: Pos, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        AsExpr::AwaitAsV(p0, p1)
    }
    pub fn mk_await_as_kv(p0: Pos, p1: Expr<Ex, Fb, En, Hi>, p2: Expr<Ex, Fb, En, Hi>) -> Self {
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
    pub fn as_as_v(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            AsExpr::AsV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_as_kv(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AsKv(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_v(&self) -> Option<(&Pos, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AwaitAsV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_kv(&self) -> Option<(&Pos, &Expr<Ex, Fb, En, Hi>, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AwaitAsKv(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
    pub fn as_as_v_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            AsExpr::AsV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_as_kv_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AsKv(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_v_mut(&mut self) -> Option<(&mut Pos, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AwaitAsV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_kv_mut(
        &mut self,
    ) -> Option<(
        &mut Pos,
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            AsExpr::AwaitAsKv(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
    pub fn as_as_v_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            AsExpr::AsV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_as_kv_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AsKv(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_v_into(self) -> Option<(Pos, Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AwaitAsV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_await_as_kv_into(self) -> Option<(Pos, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)> {
        match self {
            AsExpr::AwaitAsKv(p0, p1, p2) => Some((p0, p1, p2)),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> ClassId_<Ex, Fb, En, Hi> {
    pub fn mk_ciparent() -> Self {
        ClassId_::CIparent
    }
    pub fn mk_ciself() -> Self {
        ClassId_::CIself
    }
    pub fn mk_cistatic() -> Self {
        ClassId_::CIstatic
    }
    pub fn mk_ciexpr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        ClassId_::CIexpr(p0)
    }
    pub fn mk_ci(p0: Sid) -> Self {
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
    pub fn as_ciexpr(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            ClassId_::CIexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ci(&self) -> Option<&Sid> {
        match self {
            ClassId_::CI(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ciexpr_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            ClassId_::CIexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ci_mut(&mut self) -> Option<&mut Sid> {
        match self {
            ClassId_::CI(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ciexpr_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            ClassId_::CIexpr(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ci_into(self) -> Option<Sid> {
        match self {
            ClassId_::CI(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Hi> CollectionTarg<Hi> {
    pub fn mk_collectiontv(p0: Targ<Hi>) -> Self {
        CollectionTarg::CollectionTV(p0)
    }
    pub fn mk_collectiontkv(p0: Targ<Hi>, p1: Targ<Hi>) -> Self {
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
    pub fn as_collectiontv(&self) -> Option<&Targ<Hi>> {
        match self {
            CollectionTarg::CollectionTV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_collectiontkv(&self) -> Option<(&Targ<Hi>, &Targ<Hi>)> {
        match self {
            CollectionTarg::CollectionTKV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_collectiontv_mut(&mut self) -> Option<&mut Targ<Hi>> {
        match self {
            CollectionTarg::CollectionTV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_collectiontkv_mut(&mut self) -> Option<(&mut Targ<Hi>, &mut Targ<Hi>)> {
        match self {
            CollectionTarg::CollectionTKV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_collectiontv_into(self) -> Option<Targ<Hi>> {
        match self {
            CollectionTarg::CollectionTV(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_collectiontkv_into(self) -> Option<(Targ<Hi>, Targ<Hi>)> {
        match self {
            CollectionTarg::CollectionTKV(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> FunctionPtrId<Ex, Fb, En, Hi> {
    pub fn mk_fpid(p0: Sid) -> Self {
        FunctionPtrId::FPId(p0)
    }
    pub fn mk_fpclass_const(p0: ClassId<Ex, Fb, En, Hi>, p1: Pstring) -> Self {
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
    pub fn as_fpclass_const(&self) -> Option<(&ClassId<Ex, Fb, En, Hi>, &Pstring)> {
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
    pub fn as_fpclass_const_mut(&mut self) -> Option<(&mut ClassId<Ex, Fb, En, Hi>, &mut Pstring)> {
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
    pub fn as_fpclass_const_into(self) -> Option<(ClassId<Ex, Fb, En, Hi>, Pstring)> {
        match self {
            FunctionPtrId::FPClassConst(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> Expr_<Ex, Fb, En, Hi> {
    pub fn mk_darray(
        p0: Option<(Targ<Hi>, Targ<Hi>)>,
        p1: Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
    ) -> Self {
        Expr_::Darray(Box::new((p0, p1)))
    }
    pub fn mk_varray(p0: Option<Targ<Hi>>, p1: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::Varray(Box::new((p0, p1)))
    }
    pub fn mk_shape(p0: Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En, Hi>)>) -> Self {
        Expr_::Shape(p0)
    }
    pub fn mk_val_collection(
        p0: VcKind,
        p1: Option<Targ<Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::ValCollection(Box::new((p0, p1, p2)))
    }
    pub fn mk_key_val_collection(
        p0: KvcKind,
        p1: Option<(Targ<Hi>, Targ<Hi>)>,
        p2: Vec<Field<Ex, Fb, En, Hi>>,
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
    pub fn mk_id(p0: Sid) -> Self {
        Expr_::Id(Box::new(p0))
    }
    pub fn mk_lvar(p0: Lid) -> Self {
        Expr_::Lvar(Box::new(p0))
    }
    pub fn mk_dollardollar(p0: Lid) -> Self {
        Expr_::Dollardollar(Box::new(p0))
    }
    pub fn mk_clone(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Clone(Box::new(p0))
    }
    pub fn mk_array_get(p0: Expr<Ex, Fb, En, Hi>, p1: Option<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::ArrayGet(Box::new((p0, p1)))
    }
    pub fn mk_obj_get(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Expr<Ex, Fb, En, Hi>,
        p2: OgNullFlavor,
        p3: bool,
    ) -> Self {
        Expr_::ObjGet(Box::new((p0, p1, p2, p3)))
    }
    pub fn mk_class_get(
        p0: ClassId<Ex, Fb, En, Hi>,
        p1: ClassGetExpr<Ex, Fb, En, Hi>,
        p2: bool,
    ) -> Self {
        Expr_::ClassGet(Box::new((p0, p1, p2)))
    }
    pub fn mk_class_const(p0: ClassId<Ex, Fb, En, Hi>, p1: Pstring) -> Self {
        Expr_::ClassConst(Box::new((p0, p1)))
    }
    pub fn mk_call(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Vec<Targ<Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
        p3: Option<Expr<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::Call(Box::new((p0, p1, p2, p3)))
    }
    pub fn mk_function_pointer(p0: FunctionPtrId<Ex, Fb, En, Hi>, p1: Vec<Targ<Hi>>) -> Self {
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
    pub fn mk_string2(p0: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::String2(p0)
    }
    pub fn mk_prefixed_string(p0: String, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::PrefixedString(Box::new((p0, p1)))
    }
    pub fn mk_yield(p0: Afield<Ex, Fb, En, Hi>) -> Self {
        Expr_::Yield(Box::new(p0))
    }
    pub fn mk_await(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Await(Box::new(p0))
    }
    pub fn mk_readonly_expr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::ReadonlyExpr(Box::new(p0))
    }
    pub fn mk_tuple(p0: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::Tuple(p0)
    }
    pub fn mk_list(p0: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::List(p0)
    }
    pub fn mk_cast(p0: Hint, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Cast(Box::new((p0, p1)))
    }
    pub fn mk_unop(p0: ast_defs::Uop, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Unop(Box::new((p0, p1)))
    }
    pub fn mk_binop(p0: ast_defs::Bop, p1: Expr<Ex, Fb, En, Hi>, p2: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Binop(Box::new((p0, p1, p2)))
    }
    pub fn mk_pipe(p0: Lid, p1: Expr<Ex, Fb, En, Hi>, p2: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Pipe(Box::new((p0, p1, p2)))
    }
    pub fn mk_eif(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Option<Expr<Ex, Fb, En, Hi>>,
        p2: Expr<Ex, Fb, En, Hi>,
    ) -> Self {
        Expr_::Eif(Box::new((p0, p1, p2)))
    }
    pub fn mk_is(p0: Expr<Ex, Fb, En, Hi>, p1: Hint) -> Self {
        Expr_::Is(Box::new((p0, p1)))
    }
    pub fn mk_as(p0: Expr<Ex, Fb, En, Hi>, p1: Hint, p2: bool) -> Self {
        Expr_::As(Box::new((p0, p1, p2)))
    }
    pub fn mk_new(
        p0: ClassId<Ex, Fb, En, Hi>,
        p1: Vec<Targ<Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
        p3: Option<Expr<Ex, Fb, En, Hi>>,
        p4: Ex,
    ) -> Self {
        Expr_::New(Box::new((p0, p1, p2, p3, p4)))
    }
    pub fn mk_record(p0: Sid, p1: Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>) -> Self {
        Expr_::Record(Box::new((p0, p1)))
    }
    pub fn mk_efun(p0: Fun_<Ex, Fb, En, Hi>, p1: Vec<Lid>) -> Self {
        Expr_::Efun(Box::new((p0, p1)))
    }
    pub fn mk_lfun(p0: Fun_<Ex, Fb, En, Hi>, p1: Vec<Lid>) -> Self {
        Expr_::Lfun(Box::new((p0, p1)))
    }
    pub fn mk_xml(
        p0: Sid,
        p1: Vec<XhpAttribute<Ex, Fb, En, Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::Xml(Box::new((p0, p1, p2)))
    }
    pub fn mk_callconv(p0: ast_defs::ParamKind, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Callconv(Box::new((p0, p1)))
    }
    pub fn mk_import(p0: ImportFlavor, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Import(Box::new((p0, p1)))
    }
    pub fn mk_collection(
        p0: Sid,
        p1: Option<CollectionTarg<Hi>>,
        p2: Vec<Afield<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::Collection(Box::new((p0, p1, p2)))
    }
    pub fn mk_expression_tree(p0: ExpressionTree<Ex, Fb, En, Hi>) -> Self {
        Expr_::ExpressionTree(Box::new(p0))
    }
    pub fn mk_lplaceholder(p0: Pos) -> Self {
        Expr_::Lplaceholder(Box::new(p0))
    }
    pub fn mk_fun_id(p0: Sid) -> Self {
        Expr_::FunId(Box::new(p0))
    }
    pub fn mk_method_id(p0: Expr<Ex, Fb, En, Hi>, p1: Pstring) -> Self {
        Expr_::MethodId(Box::new((p0, p1)))
    }
    pub fn mk_method_caller(p0: Sid, p1: Pstring) -> Self {
        Expr_::MethodCaller(Box::new((p0, p1)))
    }
    pub fn mk_smethod_id(p0: ClassId<Ex, Fb, En, Hi>, p1: Pstring) -> Self {
        Expr_::SmethodId(Box::new((p0, p1)))
    }
    pub fn mk_pair(
        p0: Option<(Targ<Hi>, Targ<Hi>)>,
        p1: Expr<Ex, Fb, En, Hi>,
        p2: Expr<Ex, Fb, En, Hi>,
    ) -> Self {
        Expr_::Pair(Box::new((p0, p1, p2)))
    }
    pub fn mk_etsplice(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::ETSplice(Box::new(p0))
    }
    pub fn mk_enum_class_label(p0: Option<Sid>, p1: String) -> Self {
        Expr_::EnumClassLabel(Box::new((p0, p1)))
    }
    pub fn mk_hole(p0: Expr<Ex, Fb, En, Hi>, p1: Hi, p2: Hi, p3: HoleSource) -> Self {
        Expr_::Hole(Box::new((p0, p1, p2, p3)))
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
    pub fn is_new(&self) -> bool {
        match self {
            Expr_::New(..) => true,
            _ => false,
        }
    }
    pub fn is_record(&self) -> bool {
        match self {
            Expr_::Record(..) => true,
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
    pub fn is_callconv(&self) -> bool {
        match self {
            Expr_::Callconv(..) => true,
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
    pub fn is_fun_id(&self) -> bool {
        match self {
            Expr_::FunId(..) => true,
            _ => false,
        }
    }
    pub fn is_method_id(&self) -> bool {
        match self {
            Expr_::MethodId(..) => true,
            _ => false,
        }
    }
    pub fn is_method_caller(&self) -> bool {
        match self {
            Expr_::MethodCaller(..) => true,
            _ => false,
        }
    }
    pub fn is_smethod_id(&self) -> bool {
        match self {
            Expr_::SmethodId(..) => true,
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
    pub fn as_darray(
        &self,
    ) -> Option<(
        &Option<(Targ<Hi>, Targ<Hi>)>,
        &Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
    )> {
        match self {
            Expr_::Darray(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_varray(&self) -> Option<(&Option<Targ<Hi>>, &Vec<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::Varray(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_shape(&self) -> Option<&Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En, Hi>)>> {
        match self {
            Expr_::Shape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_val_collection(
        &self,
    ) -> Option<(&VcKind, &Option<Targ<Hi>>, &Vec<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::ValCollection(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_key_val_collection(
        &self,
    ) -> Option<(
        &KvcKind,
        &Option<(Targ<Hi>, Targ<Hi>)>,
        &Vec<Field<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::KeyValCollection(p0) => Some((&p0.0, &p0.1, &p0.2)),
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
    pub fn as_clone(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Clone(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_array_get(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Option<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::ArrayGet(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_obj_get(
        &self,
    ) -> Option<(
        &Expr<Ex, Fb, En, Hi>,
        &Expr<Ex, Fb, En, Hi>,
        &OgNullFlavor,
        &bool,
    )> {
        match self {
            Expr_::ObjGet(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3)),
            _ => None,
        }
    }
    pub fn as_class_get(
        &self,
    ) -> Option<(
        &ClassId<Ex, Fb, En, Hi>,
        &ClassGetExpr<Ex, Fb, En, Hi>,
        &bool,
    )> {
        match self {
            Expr_::ClassGet(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_class_const(&self) -> Option<(&ClassId<Ex, Fb, En, Hi>, &Pstring)> {
        match self {
            Expr_::ClassConst(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_call(
        &self,
    ) -> Option<(
        &Expr<Ex, Fb, En, Hi>,
        &Vec<Targ<Hi>>,
        &Vec<Expr<Ex, Fb, En, Hi>>,
        &Option<Expr<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Call(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3)),
            _ => None,
        }
    }
    pub fn as_function_pointer(&self) -> Option<(&FunctionPtrId<Ex, Fb, En, Hi>, &Vec<Targ<Hi>>)> {
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
    pub fn as_string2(&self) -> Option<&Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::String2(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_prefixed_string(&self) -> Option<(&String, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::PrefixedString(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_yield(&self) -> Option<&Afield<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Yield(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_await(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Await(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_readonly_expr(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::ReadonlyExpr(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_tuple(&self) -> Option<&Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::Tuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_list(&self) -> Option<&Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::List(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cast(&self) -> Option<(&Hint, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Cast(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_unop(&self) -> Option<(&ast_defs::Uop, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Unop(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_binop(
        &self,
    ) -> Option<(&ast_defs::Bop, &Expr<Ex, Fb, En, Hi>, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Binop(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_pipe(&self) -> Option<(&Lid, &Expr<Ex, Fb, En, Hi>, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Pipe(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_eif(
        &self,
    ) -> Option<(
        &Expr<Ex, Fb, En, Hi>,
        &Option<Expr<Ex, Fb, En, Hi>>,
        &Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Eif(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_is(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Hint)> {
        match self {
            Expr_::Is(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_as(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Hint, &bool)> {
        match self {
            Expr_::As(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_new(
        &self,
    ) -> Option<(
        &ClassId<Ex, Fb, En, Hi>,
        &Vec<Targ<Hi>>,
        &Vec<Expr<Ex, Fb, En, Hi>>,
        &Option<Expr<Ex, Fb, En, Hi>>,
        &Ex,
    )> {
        match self {
            Expr_::New(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3, &p0.4)),
            _ => None,
        }
    }
    pub fn as_record(&self) -> Option<(&Sid, &Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>)> {
        match self {
            Expr_::Record(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_efun(&self) -> Option<(&Fun_<Ex, Fb, En, Hi>, &Vec<Lid>)> {
        match self {
            Expr_::Efun(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_lfun(&self) -> Option<(&Fun_<Ex, Fb, En, Hi>, &Vec<Lid>)> {
        match self {
            Expr_::Lfun(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_xml(
        &self,
    ) -> Option<(
        &Sid,
        &Vec<XhpAttribute<Ex, Fb, En, Hi>>,
        &Vec<Expr<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Xml(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_callconv(&self) -> Option<(&ast_defs::ParamKind, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Callconv(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_import(&self) -> Option<(&ImportFlavor, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Import(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_collection(
        &self,
    ) -> Option<(
        &Sid,
        &Option<CollectionTarg<Hi>>,
        &Vec<Afield<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Collection(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_expression_tree(&self) -> Option<&ExpressionTree<Ex, Fb, En, Hi>> {
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
    pub fn as_fun_id(&self) -> Option<&Sid> {
        match self {
            Expr_::FunId(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_method_id(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Pstring)> {
        match self {
            Expr_::MethodId(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_method_caller(&self) -> Option<(&Sid, &Pstring)> {
        match self {
            Expr_::MethodCaller(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_smethod_id(&self) -> Option<(&ClassId<Ex, Fb, En, Hi>, &Pstring)> {
        match self {
            Expr_::SmethodId(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_pair(
        &self,
    ) -> Option<(
        &Option<(Targ<Hi>, Targ<Hi>)>,
        &Expr<Ex, Fb, En, Hi>,
        &Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Pair(p0) => Some((&p0.0, &p0.1, &p0.2)),
            _ => None,
        }
    }
    pub fn as_etsplice(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::ETSplice(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_enum_class_label(&self) -> Option<(&Option<Sid>, &String)> {
        match self {
            Expr_::EnumClassLabel(p0) => Some((&p0.0, &p0.1)),
            _ => None,
        }
    }
    pub fn as_hole(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Hi, &Hi, &HoleSource)> {
        match self {
            Expr_::Hole(p0) => Some((&p0.0, &p0.1, &p0.2, &p0.3)),
            _ => None,
        }
    }
    pub fn as_darray_mut(
        &mut self,
    ) -> Option<(
        &mut Option<(Targ<Hi>, Targ<Hi>)>,
        &mut Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
    )> {
        match self {
            Expr_::Darray(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_varray_mut(
        &mut self,
    ) -> Option<(&mut Option<Targ<Hi>>, &mut Vec<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::Varray(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_shape_mut(
        &mut self,
    ) -> Option<&mut Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En, Hi>)>> {
        match self {
            Expr_::Shape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_val_collection_mut(
        &mut self,
    ) -> Option<(
        &mut VcKind,
        &mut Option<Targ<Hi>>,
        &mut Vec<Expr<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::ValCollection(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_key_val_collection_mut(
        &mut self,
    ) -> Option<(
        &mut KvcKind,
        &mut Option<(Targ<Hi>, Targ<Hi>)>,
        &mut Vec<Field<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::KeyValCollection(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
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
    pub fn as_clone_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Clone(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_array_get_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Option<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::ArrayGet(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_obj_get_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Expr<Ex, Fb, En, Hi>,
        &mut OgNullFlavor,
        &mut bool,
    )> {
        match self {
            Expr_::ObjGet(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3)),
            _ => None,
        }
    }
    pub fn as_class_get_mut(
        &mut self,
    ) -> Option<(
        &mut ClassId<Ex, Fb, En, Hi>,
        &mut ClassGetExpr<Ex, Fb, En, Hi>,
        &mut bool,
    )> {
        match self {
            Expr_::ClassGet(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_class_const_mut(&mut self) -> Option<(&mut ClassId<Ex, Fb, En, Hi>, &mut Pstring)> {
        match self {
            Expr_::ClassConst(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_call_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Vec<Targ<Hi>>,
        &mut Vec<Expr<Ex, Fb, En, Hi>>,
        &mut Option<Expr<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Call(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3)),
            _ => None,
        }
    }
    pub fn as_function_pointer_mut(
        &mut self,
    ) -> Option<(&mut FunctionPtrId<Ex, Fb, En, Hi>, &mut Vec<Targ<Hi>>)> {
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
    pub fn as_string2_mut(&mut self) -> Option<&mut Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::String2(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_prefixed_string_mut(&mut self) -> Option<(&mut String, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::PrefixedString(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_yield_mut(&mut self) -> Option<&mut Afield<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Yield(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_await_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Await(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_readonly_expr_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::ReadonlyExpr(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_tuple_mut(&mut self) -> Option<&mut Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::Tuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_list_mut(&mut self) -> Option<&mut Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::List(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cast_mut(&mut self) -> Option<(&mut Hint, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Cast(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_unop_mut(&mut self) -> Option<(&mut ast_defs::Uop, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Unop(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_binop_mut(
        &mut self,
    ) -> Option<(
        &mut ast_defs::Bop,
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Binop(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_pipe_mut(
        &mut self,
    ) -> Option<(
        &mut Lid,
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Pipe(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_eif_mut(
        &mut self,
    ) -> Option<(
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Option<Expr<Ex, Fb, En, Hi>>,
        &mut Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Eif(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_is_mut(&mut self) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Hint)> {
        match self {
            Expr_::Is(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_as_mut(&mut self) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Hint, &mut bool)> {
        match self {
            Expr_::As(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_new_mut(
        &mut self,
    ) -> Option<(
        &mut ClassId<Ex, Fb, En, Hi>,
        &mut Vec<Targ<Hi>>,
        &mut Vec<Expr<Ex, Fb, En, Hi>>,
        &mut Option<Expr<Ex, Fb, En, Hi>>,
        &mut Ex,
    )> {
        match self {
            Expr_::New(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3, &mut p0.4)),
            _ => None,
        }
    }
    pub fn as_record_mut(
        &mut self,
    ) -> Option<(
        &mut Sid,
        &mut Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
    )> {
        match self {
            Expr_::Record(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_efun_mut(&mut self) -> Option<(&mut Fun_<Ex, Fb, En, Hi>, &mut Vec<Lid>)> {
        match self {
            Expr_::Efun(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_lfun_mut(&mut self) -> Option<(&mut Fun_<Ex, Fb, En, Hi>, &mut Vec<Lid>)> {
        match self {
            Expr_::Lfun(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_xml_mut(
        &mut self,
    ) -> Option<(
        &mut Sid,
        &mut Vec<XhpAttribute<Ex, Fb, En, Hi>>,
        &mut Vec<Expr<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Xml(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_callconv_mut(
        &mut self,
    ) -> Option<(&mut ast_defs::ParamKind, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Callconv(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_import_mut(&mut self) -> Option<(&mut ImportFlavor, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Import(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_collection_mut(
        &mut self,
    ) -> Option<(
        &mut Sid,
        &mut Option<CollectionTarg<Hi>>,
        &mut Vec<Afield<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Collection(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_expression_tree_mut(&mut self) -> Option<&mut ExpressionTree<Ex, Fb, En, Hi>> {
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
    pub fn as_fun_id_mut(&mut self) -> Option<&mut Sid> {
        match self {
            Expr_::FunId(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_method_id_mut(&mut self) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Pstring)> {
        match self {
            Expr_::MethodId(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_method_caller_mut(&mut self) -> Option<(&mut Sid, &mut Pstring)> {
        match self {
            Expr_::MethodCaller(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_smethod_id_mut(&mut self) -> Option<(&mut ClassId<Ex, Fb, En, Hi>, &mut Pstring)> {
        match self {
            Expr_::SmethodId(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_pair_mut(
        &mut self,
    ) -> Option<(
        &mut Option<(Targ<Hi>, Targ<Hi>)>,
        &mut Expr<Ex, Fb, En, Hi>,
        &mut Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Pair(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2)),
            _ => None,
        }
    }
    pub fn as_etsplice_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::ETSplice(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_enum_class_label_mut(&mut self) -> Option<(&mut Option<Sid>, &mut String)> {
        match self {
            Expr_::EnumClassLabel(p0) => Some((&mut p0.0, &mut p0.1)),
            _ => None,
        }
    }
    pub fn as_hole_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Hi, &mut Hi, &mut HoleSource)> {
        match self {
            Expr_::Hole(p0) => Some((&mut p0.0, &mut p0.1, &mut p0.2, &mut p0.3)),
            _ => None,
        }
    }
    pub fn as_darray_into(
        self,
    ) -> Option<(
        Option<(Targ<Hi>, Targ<Hi>)>,
        Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
    )> {
        match self {
            Expr_::Darray(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_varray_into(self) -> Option<(Option<Targ<Hi>>, Vec<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::Varray(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_shape_into(self) -> Option<Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En, Hi>)>> {
        match self {
            Expr_::Shape(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_val_collection_into(
        self,
    ) -> Option<(VcKind, Option<Targ<Hi>>, Vec<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::ValCollection(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_key_val_collection_into(
        self,
    ) -> Option<(
        KvcKind,
        Option<(Targ<Hi>, Targ<Hi>)>,
        Vec<Field<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::KeyValCollection(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
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
    pub fn as_clone_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Clone(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_array_get_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Option<Expr<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::ArrayGet(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_obj_get_into(
        self,
    ) -> Option<(
        Expr<Ex, Fb, En, Hi>,
        Expr<Ex, Fb, En, Hi>,
        OgNullFlavor,
        bool,
    )> {
        match self {
            Expr_::ObjGet(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3)),
            _ => None,
        }
    }
    pub fn as_class_get_into(
        self,
    ) -> Option<(ClassId<Ex, Fb, En, Hi>, ClassGetExpr<Ex, Fb, En, Hi>, bool)> {
        match self {
            Expr_::ClassGet(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_class_const_into(self) -> Option<(ClassId<Ex, Fb, En, Hi>, Pstring)> {
        match self {
            Expr_::ClassConst(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_call_into(
        self,
    ) -> Option<(
        Expr<Ex, Fb, En, Hi>,
        Vec<Targ<Hi>>,
        Vec<Expr<Ex, Fb, En, Hi>>,
        Option<Expr<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Call(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3)),
            _ => None,
        }
    }
    pub fn as_function_pointer_into(
        self,
    ) -> Option<(FunctionPtrId<Ex, Fb, En, Hi>, Vec<Targ<Hi>>)> {
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
    pub fn as_string2_into(self) -> Option<Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::String2(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_prefixed_string_into(self) -> Option<(String, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::PrefixedString(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_yield_into(self) -> Option<Afield<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Yield(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_await_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::Await(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_readonly_expr_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::ReadonlyExpr(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_tuple_into(self) -> Option<Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::Tuple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_list_into(self) -> Option<Vec<Expr<Ex, Fb, En, Hi>>> {
        match self {
            Expr_::List(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cast_into(self) -> Option<(Hint, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Cast(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_unop_into(self) -> Option<(ast_defs::Uop, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Unop(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_binop_into(
        self,
    ) -> Option<(ast_defs::Bop, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Binop(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_pipe_into(self) -> Option<(Lid, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Pipe(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_eif_into(
        self,
    ) -> Option<(
        Expr<Ex, Fb, En, Hi>,
        Option<Expr<Ex, Fb, En, Hi>>,
        Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Eif(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_is_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Hint)> {
        match self {
            Expr_::Is(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_as_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Hint, bool)> {
        match self {
            Expr_::As(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_new_into(
        self,
    ) -> Option<(
        ClassId<Ex, Fb, En, Hi>,
        Vec<Targ<Hi>>,
        Vec<Expr<Ex, Fb, En, Hi>>,
        Option<Expr<Ex, Fb, En, Hi>>,
        Ex,
    )> {
        match self {
            Expr_::New(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3, (*p0).4)),
            _ => None,
        }
    }
    pub fn as_record_into(
        self,
    ) -> Option<(Sid, Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>)> {
        match self {
            Expr_::Record(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_efun_into(self) -> Option<(Fun_<Ex, Fb, En, Hi>, Vec<Lid>)> {
        match self {
            Expr_::Efun(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_lfun_into(self) -> Option<(Fun_<Ex, Fb, En, Hi>, Vec<Lid>)> {
        match self {
            Expr_::Lfun(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_xml_into(
        self,
    ) -> Option<(
        Sid,
        Vec<XhpAttribute<Ex, Fb, En, Hi>>,
        Vec<Expr<Ex, Fb, En, Hi>>,
    )> {
        match self {
            Expr_::Xml(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_callconv_into(self) -> Option<(ast_defs::ParamKind, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Callconv(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_import_into(self) -> Option<(ImportFlavor, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Expr_::Import(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_collection_into(
        self,
    ) -> Option<(Sid, Option<CollectionTarg<Hi>>, Vec<Afield<Ex, Fb, En, Hi>>)> {
        match self {
            Expr_::Collection(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_expression_tree_into(self) -> Option<ExpressionTree<Ex, Fb, En, Hi>> {
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
    pub fn as_fun_id_into(self) -> Option<Sid> {
        match self {
            Expr_::FunId(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_method_id_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Pstring)> {
        match self {
            Expr_::MethodId(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_method_caller_into(self) -> Option<(Sid, Pstring)> {
        match self {
            Expr_::MethodCaller(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_smethod_id_into(self) -> Option<(ClassId<Ex, Fb, En, Hi>, Pstring)> {
        match self {
            Expr_::SmethodId(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_pair_into(
        self,
    ) -> Option<(
        Option<(Targ<Hi>, Targ<Hi>)>,
        Expr<Ex, Fb, En, Hi>,
        Expr<Ex, Fb, En, Hi>,
    )> {
        match self {
            Expr_::Pair(p0) => Some(((*p0).0, (*p0).1, (*p0).2)),
            _ => None,
        }
    }
    pub fn as_etsplice_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            Expr_::ETSplice(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_enum_class_label_into(self) -> Option<(Option<Sid>, String)> {
        match self {
            Expr_::EnumClassLabel(p0) => Some(((*p0).0, (*p0).1)),
            _ => None,
        }
    }
    pub fn as_hole_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Hi, Hi, HoleSource)> {
        match self {
            Expr_::Hole(p0) => Some(((*p0).0, (*p0).1, (*p0).2, (*p0).3)),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> ClassGetExpr<Ex, Fb, En, Hi> {
    pub fn mk_cgstring(p0: Pstring) -> Self {
        ClassGetExpr::CGstring(p0)
    }
    pub fn mk_cgexpr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
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
    pub fn as_cgexpr(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
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
    pub fn as_cgexpr_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
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
    pub fn as_cgexpr_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            ClassGetExpr::CGexpr(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> Case<Ex, Fb, En, Hi> {
    pub fn mk_default(p0: Pos, p1: Block<Ex, Fb, En, Hi>) -> Self {
        Case::Default(p0, p1)
    }
    pub fn mk_case(p0: Expr<Ex, Fb, En, Hi>, p1: Block<Ex, Fb, En, Hi>) -> Self {
        Case::Case(p0, p1)
    }
    pub fn is_default(&self) -> bool {
        match self {
            Case::Default(..) => true,
            _ => false,
        }
    }
    pub fn is_case(&self) -> bool {
        match self {
            Case::Case(..) => true,
            _ => false,
        }
    }
    pub fn as_default(&self) -> Option<(&Pos, &Block<Ex, Fb, En, Hi>)> {
        match self {
            Case::Default(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_case(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Block<Ex, Fb, En, Hi>)> {
        match self {
            Case::Case(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_default_mut(&mut self) -> Option<(&mut Pos, &mut Block<Ex, Fb, En, Hi>)> {
        match self {
            Case::Default(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_case_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Block<Ex, Fb, En, Hi>)> {
        match self {
            Case::Case(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_default_into(self) -> Option<(Pos, Block<Ex, Fb, En, Hi>)> {
        match self {
            Case::Default(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_case_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Block<Ex, Fb, En, Hi>)> {
        match self {
            Case::Case(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> Afield<Ex, Fb, En, Hi> {
    pub fn mk_afvalue(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Afield::AFvalue(p0)
    }
    pub fn mk_afkvalue(p0: Expr<Ex, Fb, En, Hi>, p1: Expr<Ex, Fb, En, Hi>) -> Self {
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
    pub fn as_afvalue(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            Afield::AFvalue(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_afkvalue(&self) -> Option<(&Expr<Ex, Fb, En, Hi>, &Expr<Ex, Fb, En, Hi>)> {
        match self {
            Afield::AFkvalue(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_afvalue_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            Afield::AFvalue(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_afkvalue_mut(
        &mut self,
    ) -> Option<(&mut Expr<Ex, Fb, En, Hi>, &mut Expr<Ex, Fb, En, Hi>)> {
        match self {
            Afield::AFkvalue(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
    pub fn as_afvalue_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            Afield::AFvalue(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_afkvalue_into(self) -> Option<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)> {
        match self {
            Afield::AFkvalue(p0, p1) => Some((p0, p1)),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> XhpAttribute<Ex, Fb, En, Hi> {
    pub fn mk_xhp_simple(p0: XhpSimple<Ex, Fb, En, Hi>) -> Self {
        XhpAttribute::XhpSimple(p0)
    }
    pub fn mk_xhp_spread(p0: Expr<Ex, Fb, En, Hi>) -> Self {
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
    pub fn as_xhp_simple(&self) -> Option<&XhpSimple<Ex, Fb, En, Hi>> {
        match self {
            XhpAttribute::XhpSimple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_spread(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            XhpAttribute::XhpSpread(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_simple_mut(&mut self) -> Option<&mut XhpSimple<Ex, Fb, En, Hi>> {
        match self {
            XhpAttribute::XhpSimple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_spread_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            XhpAttribute::XhpSpread(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_simple_into(self) -> Option<XhpSimple<Ex, Fb, En, Hi>> {
        match self {
            XhpAttribute::XhpSimple(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_xhp_spread_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
        match self {
            XhpAttribute::XhpSpread(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> FunVariadicity<Ex, Fb, En, Hi> {
    pub fn mk_fvvariadic_arg(p0: FunParam<Ex, Fb, En, Hi>) -> Self {
        FunVariadicity::FVvariadicArg(p0)
    }
    pub fn mk_fvellipsis(p0: Pos) -> Self {
        FunVariadicity::FVellipsis(p0)
    }
    pub fn mk_fvnon_variadic() -> Self {
        FunVariadicity::FVnonVariadic
    }
    pub fn is_fvvariadic_arg(&self) -> bool {
        match self {
            FunVariadicity::FVvariadicArg(..) => true,
            _ => false,
        }
    }
    pub fn is_fvellipsis(&self) -> bool {
        match self {
            FunVariadicity::FVellipsis(..) => true,
            _ => false,
        }
    }
    pub fn is_fvnon_variadic(&self) -> bool {
        match self {
            FunVariadicity::FVnonVariadic => true,
            _ => false,
        }
    }
    pub fn as_fvvariadic_arg(&self) -> Option<&FunParam<Ex, Fb, En, Hi>> {
        match self {
            FunVariadicity::FVvariadicArg(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fvellipsis(&self) -> Option<&Pos> {
        match self {
            FunVariadicity::FVellipsis(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fvvariadic_arg_mut(&mut self) -> Option<&mut FunParam<Ex, Fb, En, Hi>> {
        match self {
            FunVariadicity::FVvariadicArg(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fvellipsis_mut(&mut self) -> Option<&mut Pos> {
        match self {
            FunVariadicity::FVellipsis(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fvvariadic_arg_into(self) -> Option<FunParam<Ex, Fb, En, Hi>> {
        match self {
            FunVariadicity::FVvariadicArg(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_fvellipsis_into(self) -> Option<Pos> {
        match self {
            FunVariadicity::FVellipsis(p0) => Some(p0),
            _ => None,
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
impl<Ex, Fb, En, Hi> ClassAttr<Ex, Fb, En, Hi> {
    pub fn mk_caname(p0: Sid) -> Self {
        ClassAttr::CAName(p0)
    }
    pub fn mk_cafield(p0: CaField<Ex, Fb, En, Hi>) -> Self {
        ClassAttr::CAField(p0)
    }
    pub fn is_caname(&self) -> bool {
        match self {
            ClassAttr::CAName(..) => true,
            _ => false,
        }
    }
    pub fn is_cafield(&self) -> bool {
        match self {
            ClassAttr::CAField(..) => true,
            _ => false,
        }
    }
    pub fn as_caname(&self) -> Option<&Sid> {
        match self {
            ClassAttr::CAName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cafield(&self) -> Option<&CaField<Ex, Fb, En, Hi>> {
        match self {
            ClassAttr::CAField(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_caname_mut(&mut self) -> Option<&mut Sid> {
        match self {
            ClassAttr::CAName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cafield_mut(&mut self) -> Option<&mut CaField<Ex, Fb, En, Hi>> {
        match self {
            ClassAttr::CAField(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_caname_into(self) -> Option<Sid> {
        match self {
            ClassAttr::CAName(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cafield_into(self) -> Option<CaField<Ex, Fb, En, Hi>> {
        match self {
            ClassAttr::CAField(p0) => Some(p0),
            _ => None,
        }
    }
}
impl CaType {
    pub fn mk_cahint(p0: Hint) -> Self {
        CaType::CAHint(p0)
    }
    pub fn mk_caenum(p0: Vec<String>) -> Self {
        CaType::CAEnum(p0)
    }
    pub fn is_cahint(&self) -> bool {
        match self {
            CaType::CAHint(..) => true,
            _ => false,
        }
    }
    pub fn is_caenum(&self) -> bool {
        match self {
            CaType::CAEnum(..) => true,
            _ => false,
        }
    }
    pub fn as_cahint(&self) -> Option<&Hint> {
        match self {
            CaType::CAHint(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_caenum(&self) -> Option<&Vec<String>> {
        match self {
            CaType::CAEnum(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cahint_mut(&mut self) -> Option<&mut Hint> {
        match self {
            CaType::CAHint(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_caenum_mut(&mut self) -> Option<&mut Vec<String>> {
        match self {
            CaType::CAEnum(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_cahint_into(self) -> Option<Hint> {
        match self {
            CaType::CAHint(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_caenum_into(self) -> Option<Vec<String>> {
        match self {
            CaType::CAEnum(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> ClassConstKind<Ex, Fb, En, Hi> {
    pub fn mk_ccabstract(p0: Option<Expr<Ex, Fb, En, Hi>>) -> Self {
        ClassConstKind::CCAbstract(p0)
    }
    pub fn mk_ccconcrete(p0: Expr<Ex, Fb, En, Hi>) -> Self {
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
    pub fn as_ccabstract(&self) -> Option<&Option<Expr<Ex, Fb, En, Hi>>> {
        match self {
            ClassConstKind::CCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccconcrete(&self) -> Option<&Expr<Ex, Fb, En, Hi>> {
        match self {
            ClassConstKind::CCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccabstract_mut(&mut self) -> Option<&mut Option<Expr<Ex, Fb, En, Hi>>> {
        match self {
            ClassConstKind::CCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccconcrete_mut(&mut self) -> Option<&mut Expr<Ex, Fb, En, Hi>> {
        match self {
            ClassConstKind::CCConcrete(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccabstract_into(self) -> Option<Option<Expr<Ex, Fb, En, Hi>>> {
        match self {
            ClassConstKind::CCAbstract(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_ccconcrete_into(self) -> Option<Expr<Ex, Fb, En, Hi>> {
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
    pub fn mk_tcpartially_abstract(p0: ClassPartiallyAbstractTypeconst) -> Self {
        ClassTypeconst::TCPartiallyAbstract(p0)
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
    pub fn is_tcpartially_abstract(&self) -> bool {
        match self {
            ClassTypeconst::TCPartiallyAbstract(..) => true,
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
    pub fn as_tcpartially_abstract(&self) -> Option<&ClassPartiallyAbstractTypeconst> {
        match self {
            ClassTypeconst::TCPartiallyAbstract(p0) => Some(p0),
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
    pub fn as_tcpartially_abstract_mut(&mut self) -> Option<&mut ClassPartiallyAbstractTypeconst> {
        match self {
            ClassTypeconst::TCPartiallyAbstract(p0) => Some(p0),
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
    pub fn as_tcpartially_abstract_into(self) -> Option<ClassPartiallyAbstractTypeconst> {
        match self {
            ClassTypeconst::TCPartiallyAbstract(p0) => Some(p0),
            _ => None,
        }
    }
}
impl<Ex, Fb, En, Hi> Def<Ex, Fb, En, Hi> {
    pub fn mk_fun(p0: FunDef<Ex, Fb, En, Hi>) -> Self {
        Def::Fun(Box::new(p0))
    }
    pub fn mk_class(p0: Class_<Ex, Fb, En, Hi>) -> Self {
        Def::Class(Box::new(p0))
    }
    pub fn mk_record_def(p0: RecordDef<Ex, Fb, En, Hi>) -> Self {
        Def::RecordDef(Box::new(p0))
    }
    pub fn mk_stmt(p0: Stmt<Ex, Fb, En, Hi>) -> Self {
        Def::Stmt(Box::new(p0))
    }
    pub fn mk_typedef(p0: Typedef<Ex, Fb, En, Hi>) -> Self {
        Def::Typedef(Box::new(p0))
    }
    pub fn mk_constant(p0: Gconst<Ex, Fb, En, Hi>) -> Self {
        Def::Constant(Box::new(p0))
    }
    pub fn mk_namespace(p0: Sid, p1: Program<Ex, Fb, En, Hi>) -> Self {
        Def::Namespace(Box::new((p0, p1)))
    }
    pub fn mk_namespace_use(p0: Vec<(NsKind, Sid, Sid)>) -> Self {
        Def::NamespaceUse(p0)
    }
    pub fn mk_set_namespace_env(p0: Nsenv) -> Self {
        Def::SetNamespaceEnv(Box::new(p0))
    }
    pub fn mk_file_attributes(p0: FileAttribute<Ex, Fb, En, Hi>) -> Self {
        Def::FileAttributes(Box::new(p0))
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
    pub fn is_record_def(&self) -> bool {
        match self {
            Def::RecordDef(..) => true,
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
    pub fn as_fun(&self) -> Option<&FunDef<Ex, Fb, En, Hi>> {
        match self {
            Def::Fun(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_class(&self) -> Option<&Class_<Ex, Fb, En, Hi>> {
        match self {
            Def::Class(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_record_def(&self) -> Option<&RecordDef<Ex, Fb, En, Hi>> {
        match self {
            Def::RecordDef(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_stmt(&self) -> Option<&Stmt<Ex, Fb, En, Hi>> {
        match self {
            Def::Stmt(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_typedef(&self) -> Option<&Typedef<Ex, Fb, En, Hi>> {
        match self {
            Def::Typedef(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_constant(&self) -> Option<&Gconst<Ex, Fb, En, Hi>> {
        match self {
            Def::Constant(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_namespace(&self) -> Option<(&Sid, &Program<Ex, Fb, En, Hi>)> {
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
    pub fn as_file_attributes(&self) -> Option<&FileAttribute<Ex, Fb, En, Hi>> {
        match self {
            Def::FileAttributes(p0) => Some(&p0),
            _ => None,
        }
    }
    pub fn as_fun_mut(&mut self) -> Option<&mut FunDef<Ex, Fb, En, Hi>> {
        match self {
            Def::Fun(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_class_mut(&mut self) -> Option<&mut Class_<Ex, Fb, En, Hi>> {
        match self {
            Def::Class(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_record_def_mut(&mut self) -> Option<&mut RecordDef<Ex, Fb, En, Hi>> {
        match self {
            Def::RecordDef(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_stmt_mut(&mut self) -> Option<&mut Stmt<Ex, Fb, En, Hi>> {
        match self {
            Def::Stmt(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_typedef_mut(&mut self) -> Option<&mut Typedef<Ex, Fb, En, Hi>> {
        match self {
            Def::Typedef(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_constant_mut(&mut self) -> Option<&mut Gconst<Ex, Fb, En, Hi>> {
        match self {
            Def::Constant(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_namespace_mut(&mut self) -> Option<(&mut Sid, &mut Program<Ex, Fb, En, Hi>)> {
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
    pub fn as_file_attributes_mut(&mut self) -> Option<&mut FileAttribute<Ex, Fb, En, Hi>> {
        match self {
            Def::FileAttributes(p0) => Some(p0.as_mut()),
            _ => None,
        }
    }
    pub fn as_fun_into(self) -> Option<FunDef<Ex, Fb, En, Hi>> {
        match self {
            Def::Fun(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_class_into(self) -> Option<Class_<Ex, Fb, En, Hi>> {
        match self {
            Def::Class(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_record_def_into(self) -> Option<RecordDef<Ex, Fb, En, Hi>> {
        match self {
            Def::RecordDef(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_stmt_into(self) -> Option<Stmt<Ex, Fb, En, Hi>> {
        match self {
            Def::Stmt(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_typedef_into(self) -> Option<Typedef<Ex, Fb, En, Hi>> {
        match self {
            Def::Typedef(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_constant_into(self) -> Option<Gconst<Ex, Fb, En, Hi>> {
        match self {
            Def::Constant(p0) => Some(*p0),
            _ => None,
        }
    }
    pub fn as_namespace_into(self) -> Option<(Sid, Program<Ex, Fb, En, Hi>)> {
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
    pub fn as_file_attributes_into(self) -> Option<FileAttribute<Ex, Fb, En, Hi>> {
        match self {
            Def::FileAttributes(p0) => Some(*p0),
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
impl HoleSource {
    pub fn mk_typing() -> Self {
        HoleSource::Typing
    }
    pub fn mk_unsafe_cast() -> Self {
        HoleSource::UnsafeCast
    }
    pub fn mk_enforced_cast() -> Self {
        HoleSource::EnforcedCast
    }
    pub fn is_typing(&self) -> bool {
        match self {
            HoleSource::Typing => true,
            _ => false,
        }
    }
    pub fn is_unsafe_cast(&self) -> bool {
        match self {
            HoleSource::UnsafeCast => true,
            _ => false,
        }
    }
    pub fn is_enforced_cast(&self) -> bool {
        match self {
            HoleSource::EnforcedCast => true,
            _ => false,
        }
    }
}
impl BreakContinueLevel {
    pub fn mk_level_ok(p0: Option<isize>) -> Self {
        BreakContinueLevel::LevelOk(p0)
    }
    pub fn mk_level_non_literal() -> Self {
        BreakContinueLevel::LevelNonLiteral
    }
    pub fn mk_level_non_positive() -> Self {
        BreakContinueLevel::LevelNonPositive
    }
    pub fn is_level_ok(&self) -> bool {
        match self {
            BreakContinueLevel::LevelOk(..) => true,
            _ => false,
        }
    }
    pub fn is_level_non_literal(&self) -> bool {
        match self {
            BreakContinueLevel::LevelNonLiteral => true,
            _ => false,
        }
    }
    pub fn is_level_non_positive(&self) -> bool {
        match self {
            BreakContinueLevel::LevelNonPositive => true,
            _ => false,
        }
    }
    pub fn as_level_ok(&self) -> Option<&Option<isize>> {
        match self {
            BreakContinueLevel::LevelOk(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_level_ok_mut(&mut self) -> Option<&mut Option<isize>> {
        match self {
            BreakContinueLevel::LevelOk(p0) => Some(p0),
            _ => None,
        }
    }
    pub fn as_level_ok_into(self) -> Option<Option<isize>> {
        match self {
            BreakContinueLevel::LevelOk(p0) => Some(p0),
            _ => None,
        }
    }
}
