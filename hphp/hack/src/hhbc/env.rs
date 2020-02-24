// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod emitter; // emitter is public API for mutating state
pub mod iterator;
pub mod jump_targets;
pub mod local;

use ast_scope_rust::{Scope, ScopeItem};
use emitter::Emitter;
use iterator::Iter;
use label_rust::Label;
use oxidized::{ast as tast, ast_defs::Id, namespace_env::Env as NamespaceEnv};

extern crate bitflags;
use bitflags::bitflags;

use std::borrow::Cow;

bitflags! {
    #[derive(Default)]
    pub struct Flags: u8 {
        const NEEDS_LOCAL_THIS =    0b0000_0001;
        const IN_TRY =              0b0000_0010;
        const ALLOWS_ARRAY_APPEND = 0b0000_0100;
        const IN_RX_BODY =          0b0000_1000;
    }
}

#[derive(Clone, Default, Debug)]
pub struct Env<'a> {
    pub flags: Flags,
    pub jump_targets_gen: jump_targets::Gen,
    pub scope: Scope<'a>,
    pub namespace: NamespaceEnv,

    allows_array_append: bool,
    // TODO(hrust)
    // - pipe_var after porting Local
}

impl<'a> Env<'a> {
    pub fn with_allows_array_append<F, R>(&mut self, f: F) -> R
    where
        F: FnOnce(&mut Self) -> R,
    {
        let old = self.allows_array_append;
        self.allows_array_append = true;
        let r = f(self);
        self.allows_array_append = old;
        r
    }

    pub fn with_need_local_this(&mut self, need_local_this: bool) {
        if need_local_this {
            self.flags = self.flags | Flags::NEEDS_LOCAL_THIS;
        }
    }

    pub fn with_rx_body(&mut self, in_rx_body: bool) {
        if in_rx_body {
            self.flags = self.flags | Flags::IN_RX_BODY;
        }
    }

    fn with_scope(mut self, scope: Scope<'a>) -> Env {
        self.scope = scope;
        self
    }

    pub fn make_class_env(class: &'a tast::Class_) -> Env {
        Env::default().with_scope(Scope {
            items: vec![ScopeItem::Class(Cow::Borrowed(class))],
        })
        //TODO(hrust): set namespace
    }

    pub fn do_in_loop_body<R, F>(
        &mut self,
        e: &mut Emitter,
        label_break: Label,
        label_continue: Label,
        iterator: Option<Iter>,
        s: &tast::Stmt,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &tast::Stmt) -> R,
    {
        self.jump_targets_gen
            .with_loop(label_break, label_continue, iterator, s);
        self.run_and_release_ids(e, s, f)
    }

    pub fn do_in_switch_body<R, F>(
        &mut self,
        e: &mut Emitter,
        end_label: Label,
        cases: &Vec<tast::Case>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &Vec<tast::Case>) -> R,
    {
        self.jump_targets_gen.with_switch(end_label, cases);
        self.run_and_release_ids(e, cases, f)
    }

    pub fn do_in_try_body<R, F>(
        &mut self,
        e: &mut Emitter,
        finally_label: Label,
        stmt: &tast::Stmt,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &tast::Stmt) -> R,
    {
        self.jump_targets_gen.with_try(finally_label, stmt);
        self.run_and_release_ids(e, stmt, f)
    }

    pub fn do_in_finally_body<R, F>(&mut self, e: &mut Emitter, stmt: &tast::Stmt, f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &tast::Stmt) -> R,
    {
        self.jump_targets_gen.with_finally(stmt);
        self.run_and_release_ids(e, stmt, f)
    }

    pub fn do_in_using_body<R, F>(
        &mut self,
        e: &mut Emitter,
        finally_label: Label,
        stmt: &tast::Stmt,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &tast::Stmt) -> R,
    {
        self.jump_targets_gen.with_using(finally_label, stmt);
        self.run_and_release_ids(e, stmt, f)
    }

    pub fn do_function<R, F>(&mut self, e: &mut Emitter, defs: &tast::Program, f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, &[tast::Def]) -> R,
    {
        self.jump_targets_gen.with_function(defs);
        self.run_and_release_ids(e, defs.as_slice(), f)
    }

    fn run_and_release_ids<X, R, F>(&mut self, e: &mut Emitter, x: X, f: F) -> R
    where
        F: FnOnce(&mut Self, &mut Emitter, X) -> R,
    {
        let res = f(self, e, x);
        self.jump_targets_gen.release_ids();
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

pub fn get_unique_id_for_main() -> String {
    String::from("|")
}

pub fn get_unique_id_for_method(cls: &tast::Class_, md: &tast::Method_) -> String {
    let Id(_, cls_name) = &cls.name;
    let Id(_, md_name) = &md.name;
    format!("{}|{}", cls_name, md_name)
}

pub fn get_unique_id_for_function(fun: &tast::Fun_) -> String {
    let Id(_, fn_name) = &fun.name;
    format!("|{}", fn_name)
}
