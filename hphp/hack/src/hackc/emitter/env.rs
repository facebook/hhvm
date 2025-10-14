// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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

/// `'s` is an AST lifetime
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

    pub fn with_allows_array_append<F, R>(&mut self, f: F) -> R
    where
        F: FnOnce(&mut Self) -> R,
    {
        let old = self.flags.contains(Flags::ALLOWS_ARRAY_APPEND);
        self.flags.set(Flags::ALLOWS_ARRAY_APPEND, true);
        let r = f(self);
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

    pub fn do_in_loop_body<R, F>(
        &mut self,
        e: &mut Emitter,
        label_break: Label,
        label_continue: Label,
        iterator: Option<IterId>,
        b: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen
            .with_loop(label_break, label_continue, iterator);
        self.run_and_release_ids(e, |env, e| f(env, e, b))
    }

    pub fn do_in_switch_body<R, F>(
        &mut self,
        e: &mut Emitter,
        end_label: Label,
        cases: &[ast::Case],
        dfl: &Option<ast::DefaultCase>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[ast::Case], &Option<ast::DefaultCase>) -> R,
    {
        self.jump_targets_gen.with_switch(end_label);
        self.run_and_release_ids(e, |env, e| f(env, e, cases, dfl))
    }

    pub fn do_in_try_catch_body<R, F>(
        &mut self,
        e: &mut Emitter,
        finally_label: Label,
        try_block: &[ast::Stmt],
        catch_block: &[ast::Catch],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[ast::Stmt], &[ast::Catch]) -> R,
    {
        self.jump_targets_gen.with_try_catch(finally_label);
        self.run_and_release_ids(e, |env, e| f(env, e, try_block, catch_block))
    }

    pub fn do_in_try_body<R, F>(
        &mut self,
        e: &mut Emitter,
        finally_label: Label,
        block: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_try(finally_label);
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_in_finally_body<R, F>(&mut self, e: &mut Emitter, block: &[ast::Stmt], f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_finally();
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_in_using_body<R, F>(
        &mut self,
        e: &mut Emitter,
        finally_label: Label,
        block: &[ast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_using(finally_label);
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_function<R, F>(&mut self, e: &mut Emitter, defs: &[ast::Stmt], f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[ast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_function();
        self.run_and_release_ids(e, |env, e| f(env, e, defs))
    }

    fn run_and_release_ids<R, F>(&mut self, e: &mut Emitter, f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter) -> R,
    {
        let res = f(self, e);
        self.jump_targets_gen.release_ids();
        self.jump_targets_gen.revert();
        res
    }
}

#[cfg(test)]
mod tests {
    use oxidized::namespace_env::Mode;

    use super::*;

    #[test]
    fn make_env() {
        let namespace = Arc::new(NamespaceEnv::empty(vec![], Mode::ForTypecheck, false));
        let _: Env<'_> = Env::default(namespace);
    }
}
