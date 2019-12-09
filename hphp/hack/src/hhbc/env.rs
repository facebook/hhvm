// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod emitter; // emitter is public API for mutating state
pub mod iterator;

use iterator::Iter;
use label_rust::Label;
use oxidized::ast as tast;
use rx_rust as rx;

mod jump_targets;

extern crate bitflags;
use bitflags::bitflags;

bitflags! {
    pub struct Flags: u8 {
        const NEEDS_LOCAL_THIS =    0b0000_0001;
        const IN_TRY =              0b0000_0010;
        const ALLOWS_ARRAY_APPEND = 0b0000_0100;
        const IN_RX_BODY =          0b0000_1000;
    }
}

pub struct Env {
    pub flags: Flags,
    pub jump_targets_gen: jump_targets::Gen,
    // TODO(hrust)
    // - pipe_var after porting Local
    // - namespace after porting Namespace_env
}

impl Env {
    pub fn do_in_loop_body<R, F>(
        &mut self,
        label_break: Label,
        label_continue: Label,
        iterator: Option<Iter>,
        s: &tast::Stmt,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Self, &tast::Stmt) -> R,
    {
        self.jump_targets_gen
            .with_loop(label_break, label_continue, iterator, s);
        self.run_and_release_ids(s, f)
    }

    pub fn do_in_switch_body<R, F>(&mut self, end_label: Label, cases: &Vec<tast::Case>, f: F) -> R
    where
        F: FnOnce(&mut Self, &Vec<tast::Case>) -> R,
    {
        self.jump_targets_gen.with_switch(end_label, cases);
        self.run_and_release_ids(cases, f)
    }

    pub fn do_in_try_body<R, F>(&mut self, finally_label: Label, stmt: &tast::Stmt, f: F) -> R
    where
        F: FnOnce(&mut Self, &tast::Stmt) -> R,
    {
        self.jump_targets_gen.with_try(finally_label, stmt);
        self.run_and_release_ids(stmt, f)
    }

    pub fn do_in_finally_body<R, F>(&mut self, stmt: &tast::Stmt, f: F) -> R
    where
        F: FnOnce(&mut Self, &tast::Stmt) -> R,
    {
        self.jump_targets_gen.with_finally(stmt);
        self.run_and_release_ids(stmt, f)
    }

    pub fn do_in_using_body<R, F>(&mut self, finally_label: Label, stmt: &tast::Stmt, f: F) -> R
    where
        F: FnOnce(&mut Self, &tast::Stmt) -> R,
    {
        self.jump_targets_gen.with_using(finally_label, stmt);
        self.run_and_release_ids(stmt, f)
    }

    pub fn do_function<R, F>(&mut self, defs: &tast::Program, f: F) -> R
    where
        F: FnOnce(&mut Self, &tast::Program) -> R,
    {
        self.jump_targets_gen.with_function(defs);
        self.run_and_release_ids(defs, f)
    }

    fn run_and_release_ids<X, R, F>(&mut self, x: X, f: F) -> R
    where
        F: FnOnce(&mut Self, X) -> R,
    {
        let res = f(self, x);
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

#[derive(Default, Debug)]
pub struct GlobalState {
    pub explicit_use_set: SSet,
    // closure_namespaces: SMap<NamespaceEnv>, // TODO(hrust) use oxidized
    // closure_enclosing_classes // TODO(hrust) need Tast
    pub function_to_labels_map: SMap<SMap<bool>>,
    pub lambda_rx_of_scope: SMap<rx::Level>,
}

impl GlobalState {
    pub fn get_lambda_rx_of_scope(&self, class_name: &str, meth_name: &str) -> rx::Level {
        let key = UniqueIdBuilder::new().method(class_name, meth_name);
        *self
            .lambda_rx_of_scope
            .get(&key)
            .unwrap_or(&rx::Level::NonRx)
    }
}
