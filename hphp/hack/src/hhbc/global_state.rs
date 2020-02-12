// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::{emitter::Emitter, SMap, SSet, UniqueIdBuilder};
use ocamlrep_derive::OcamlRep;
use oxidized::{ast_defs::ClassKind, namespace_env::Env as NamespaceEnv};
use rx_rust as rx;

#[derive(Debug, OcamlRep, Clone)]
pub struct ClosureEnclosingClassInfo {
    pub kind: ClassKind,
    pub name: String,
    pub parent_class_name: Option<String>,
}

impl Default for ClosureEnclosingClassInfo {
    fn default() -> Self {
        Self {
            kind: ClassKind::Cnormal,
            name: "".to_string(),
            parent_class_name: None,
        }
    }
}

#[derive(Default, OcamlRep, Debug)]
pub struct GlobalState {
    pub explicit_use_set: SSet,
    pub closure_namespaces: SMap<NamespaceEnv>,
    pub closure_enclosing_classes: SMap<ClosureEnclosingClassInfo>,
    pub functions_with_finally: SSet,
    pub function_to_labels_map: SMap<SMap<bool>>,
    pub lambda_rx_of_scope: SMap<rx::Level>,
}

impl GlobalState {
    fn init() -> Box<dyn std::any::Any> {
        Box::new(GlobalState::default())
    }

    pub fn get_lambda_rx_of_scope(&self, class_name: &str, meth_name: &str) -> rx::Level {
        let key = UniqueIdBuilder::new().method(class_name, meth_name);
        *self
            .lambda_rx_of_scope
            .get(&key)
            .unwrap_or(&rx::Level::NonRx)
    }

    pub fn get_closure_enclosing_class(
        &self,
        class_name: &str,
    ) -> Option<&ClosureEnclosingClassInfo> {
        self.closure_enclosing_classes.get(class_name)
    }
}

env::lazy_emit_state!(global_state, GlobalState, GlobalState::init);

pub fn set_state(e: &mut Emitter, global_state: GlobalState) {
    *e.emit_state_mut() = global_state;
}
