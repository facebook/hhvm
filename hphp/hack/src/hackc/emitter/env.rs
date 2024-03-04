// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod adata_state;
mod class_expr;
pub mod emitter; // emitter is public API for mutating state
mod iterator;
pub mod jump_targets;
mod label;
mod local;

use std::sync::Arc;

use ast_scope::Scope;
use ast_scope::ScopeItem;
use bitflags::bitflags;
pub use class_expr::ClassExpr;
use emitter::Emitter;
use hhbc::IterId;
use hhbc::Label;
use hhbc::Local;
use hhbc::StringId;
pub use iterator::*;
pub use label::*;
pub use local::*;
use oxidized::ast;
use oxidized::namespace_env::Env as NamespaceEnv;

bitflags! {
    #[derive(Default, PartialEq, Eq, PartialOrd, Ord, Hash, Debug, Clone, Copy)]
    pub struct Flags: u8 {
        const NEEDS_LOCAL_THIS =    0b0000_0001;
        const IN_TRY =              0b0000_0010;
        const ALLOWS_ARRAY_APPEND = 0b0000_0100;
    }
}

/// `'a` is an AST lifetime, `'arena` the lifetime of the `InstrSeq`
/// arena.
#[derive(Clone, Debug)]
pub struct Env<'s> {
    pub flags: Flags,
    pub jump_targets_gen: jump_targets::Gen,
    pub scope: Scope<'s>,
    pub namespace: Arc<NamespaceEnv>,
    pub call_context: Option<StringId>,
    pub pipe_var: Option<Local>,
}

impl<'s> Env<'s> {
    pub fn default(namespace: Arc<NamespaceEnv>) -> Self {
        Env {
            namespace,
            flags: Flags::default(),
            jump_targets_gen: jump_targets::Gen::default(),
            scope: Scope::default(),
            call_context: None,
            pipe_var: None,
        }
    }

    pub fn with_allows_array_append<'a, F, R>(&mut self, alloc: &'a bumpalo::Bump, f: F) -> R
    where
        F: FnOnce(&'a bumpalo::Bump, &mut Self) -> R,
    {
        let old = self.flags.contains(Flags::ALLOWS_ARRAY_APPEND);
        self.flags.set(Flags::ALLOWS_ARRAY_APPEND, true);
        let r = f(alloc, self);
        self.flags.set(Flags::ALLOWS_ARRAY_APPEND, old);
        r
    }

    pub fn with_need_local_this(&mut self, need_local_this: bool) {
        if need_local_this {
            self.flags |= Flags::NEEDS_LOCAL_THIS;
        }
    }

    pub fn with_pipe_var(&mut self, local: Local) {
        self.pipe_var = Some(local);
    }

    pub fn with_scope(mut self, scope: Scope<'s>) -> Env<'s> {
        self.scope = scope;
        self
    }

    pub fn make_class_env(class: &'s ast::Class_) -> Env<'s> {
        let scope = Scope::with_item(ScopeItem::Class(ast_scope::Class::new_ref(class)));
        Env::default(Arc::clone(&class.namespace)).with_scope(scope)
    }

    pub fn do_in_loop_body<'a, 'decl, R, F>(
        &mut self,
        e: &mut Emitter<'a, 'decl>,
        label_break: Label,
        label_continue: Label,
        iterator: Option<IterId>,
        b: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen
            .with_loop(label_break, label_continue, iterator);
        self.run_and_release_ids(e, |env, e| f(env, e, b))
    }

    pub fn do_in_switch_body<'a, 'decl, R, F>(
        &mut self,
        e: &mut Emitter<'a, 'decl>,
        end_label: Label,
        cases: &[ast::Case],
        dfl: &Option<ast::DefaultCase>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>, &[ast::Case], &Option<ast::DefaultCase>) -> R,
    {
        self.jump_targets_gen.with_switch(end_label);
        self.run_and_release_ids(e, |env, e| f(env, e, cases, dfl))
    }

    pub fn do_in_try_catch_body<'a, 'decl, R, F>(
        &mut self,
        e: &mut Emitter<'a, 'decl>,
        finally_label: Label,
        try_block: &[ast::Stmt],
        catch_block: &[ast::Catch],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>, &[ast::Stmt], &[ast::Catch]) -> R,
    {
        self.jump_targets_gen.with_try_catch(finally_label);
        self.run_and_release_ids(e, |env, e| f(env, e, try_block, catch_block))
    }

    pub fn do_in_try_body<'a, 'decl, R, F>(
        &mut self,
        e: &mut Emitter<'a, 'decl>,
        finally_label: Label,
        block: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_try(finally_label);
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_in_finally_body<'a, 'decl, R, F>(
        &mut self,
        e: &mut Emitter<'a, 'decl>,
        block: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_finally();
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_in_using_body<'a, 'decl, R, F>(
        &mut self,
        e: &mut Emitter<'a, 'decl>,
        finally_label: Label,
        block: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_using(finally_label);
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_function<'a, 'decl, R, F>(
        &mut self,
        e: &mut Emitter<'a, 'decl>,
        defs: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_function();
        self.run_and_release_ids(e, |env, e| f(env, e, defs))
    }

    fn run_and_release_ids<'a, 'decl, R, F>(&mut self, e: &mut Emitter<'a, 'decl>, f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'a, 'decl>) -> R,
    {
        let res = f(self, e);
        self.jump_targets_gen.release_ids();
        self.jump_targets_gen.revert();
        res
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn make_env() {
        let namespace = Arc::new(NamespaceEnv::empty(vec![], false, false));
        let _: Env<'_> = Env::default(namespace);
    }
}
