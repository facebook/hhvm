// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod emitter; // emitter is public API for mutating state
pub mod iterator;
pub mod jump_targets;
pub mod local;

use ast_scope_rust::{self as ast_scope, Scope, ScopeItem};
use emitter::Emitter;
use hhbc_by_ref_ast_body::AstBody;
use hhbc_by_ref_label::Label;
use ocamlrep::rc::RcOc;
use oxidized::{ast as tast, namespace_env::Env as NamespaceEnv};

use bitflags::bitflags;

bitflags! {
    #[derive(Default)]
    pub struct Flags: u8 {
        const NEEDS_LOCAL_THIS =    0b0000_0001;
        const IN_TRY =              0b0000_0010;
        const ALLOWS_ARRAY_APPEND = 0b0000_0100;
        const IN_RX_BODY =          0b0000_1000;
    }
}

#[derive(Clone, Debug)]
pub struct Env<'a, 'arena> {
    pub arena: &'arena bumpalo::Bump,
    pub flags: Flags,
    pub jump_targets_gen: jump_targets::Gen<'arena>,
    pub scope: Scope<'a>,
    pub namespace: RcOc<NamespaceEnv>,
    pub call_context: Option<String>,
    pub pipe_var: Option<local::Type<'arena>>,
}

impl<'a, 'arena> Env<'a, 'arena> {
    pub fn default(arena: &'arena bumpalo::Bump, namespace: RcOc<NamespaceEnv>) -> Self {
        Env {
            arena,
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

    pub fn with_rx_body(&mut self, in_rx_body: bool) {
        if in_rx_body {
            self.flags |= Flags::IN_RX_BODY;
        }
    }

    pub fn with_pipe_var(&mut self, local: local::Type<'arena>) {
        self.pipe_var = Some(local);
    }

    pub fn with_scope(mut self, scope: Scope<'a>) -> Env<'a, 'arena> {
        self.scope = scope;
        self
    }

    pub fn make_class_env(
        arena: &'arena bumpalo::Bump,
        class: &'a tast::Class_,
    ) -> Env<'a, 'arena> {
        Env::default(arena, RcOc::clone(&class.namespace)).with_scope(Scope {
            items: vec![ScopeItem::Class(ast_scope::Class::new_ref(class))],
        })
    }

    pub fn do_in_loop_body<R, F>(
        &mut self,
        e: &mut Emitter<'arena>,
        label_break: Label<'arena>,
        label_continue: Label<'arena>,
        iterator: Option<iterator::Id>,
        b: &[tast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>, &[tast::Stmt]) -> R,
    {
        self.jump_targets_gen
            .with_loop(label_break, label_continue, iterator, b);
        self.run_and_release_ids(e, |env, e| f(env, e, b))
    }

    pub fn do_in_switch_body<R, F>(
        &mut self,
        e: &mut Emitter<'arena>,
        end_label: Label<'arena>,
        cases: &[tast::Case],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>, &[tast::Case]) -> R,
    {
        self.jump_targets_gen.with_switch(end_label, cases);
        self.run_and_release_ids(e, |env, e| f(env, e, cases))
    }

    pub fn do_in_try_catch_body<R, F>(
        &mut self,
        e: &mut Emitter<'arena>,
        finally_label: Label<'arena>,
        try_block: &[tast::Stmt],
        catch_block: &[tast::Catch],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>, &[tast::Stmt], &[tast::Catch]) -> R,
    {
        self.jump_targets_gen
            .with_try_catch(finally_label, try_block, catch_block);
        self.run_and_release_ids(e, |env, e| f(env, e, try_block, catch_block))
    }

    pub fn do_in_try_body<R, F>(
        &mut self,
        e: &mut Emitter<'arena>,
        finally_label: Label<'arena>,
        block: &[tast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>, &[tast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_try(finally_label, block);
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_in_finally_body<R, F>(
        &mut self,
        e: &mut Emitter<'arena>,
        block: &[tast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>, &[tast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_finally(block);
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_in_using_body<R, F>(
        &mut self,
        e: &mut Emitter<'arena>,
        finally_label: Label<'arena>,
        block: &[tast::Stmt],
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>, &[tast::Stmt]) -> R,
    {
        self.jump_targets_gen.with_using(finally_label, block);
        self.run_and_release_ids(e, |env, e| f(env, e, block))
    }

    pub fn do_function<R, F>(&mut self, e: &mut Emitter<'arena>, defs: &AstBody, f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>, &AstBody) -> R,
    {
        self.jump_targets_gen.with_function(defs);
        self.run_and_release_ids(e, |env, e| f(env, e, defs))
    }

    fn run_and_release_ids<R, F>(&mut self, e: &mut Emitter<'arena>, f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter<'arena>) -> R,
    {
        let res = f(self, e);
        self.jump_targets_gen.release_ids();
        self.jump_targets_gen.revert();
        res
    }
}

/// Builder for creating unique ids; e.g.
/// the OCaml function
///    Emit_env.get_unique_id_for_FOO
/// can be called in Rust via:
/// ```
///    UniqueIdBuilder::new().FOO("some_fun")
/// ```
pub struct UniqueIdBuilder {
    id: String,
}
impl UniqueIdBuilder {
    pub fn new() -> UniqueIdBuilder {
        UniqueIdBuilder { id: "|".to_owned() }
    }
    pub fn main(self) -> String {
        self.id
    }
    pub fn function(mut self, fun_name: &str) -> String {
        self.id.push_str(fun_name);
        self.id
    }
    pub fn method(self, class_name: &str, meth_name: &str) -> String {
        let mut ret = class_name.to_owned();
        ret.push_str(&self.id);
        ret.push_str(meth_name);
        ret
    }
}

pub type SMap<T> = std::collections::BTreeMap<String, T>;
pub type SSet = std::collections::BTreeSet<String>;

pub fn get_unique_id_for_main() -> &'static str {
    "|"
}

pub fn get_unique_id_for_method(cls_name: &str, md_name: &str) -> String {
    format!("{}|{}", cls_name, md_name)
}

pub fn get_unique_id_for_function(fun_name: &str) -> String {
    format!("|{}", fun_name)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn make_env() {
        let a = bumpalo::Bump::new();
        let alloc: &bumpalo::Bump = &a;
        let namespace = RcOc::new(NamespaceEnv::empty(vec![], false, false));
        let _: Env<'_, '_> = Env::default(&alloc, namespace);
    }
}
