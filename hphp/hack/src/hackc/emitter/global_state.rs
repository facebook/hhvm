// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use hhbc::Coeffects;
use oxidized::ast_defs::Abstraction;
use oxidized::ast_defs::ClassishKind;
use oxidized::namespace_env::Env as NamespaceEnv;
use unique_id_builder::get_unique_id_for_method;
use unique_id_builder::SMap;
use unique_id_builder::SSet;

#[derive(Debug, Clone)]
pub struct ClosureEnclosingClassInfo {
    pub kind: ClassishKind,
    pub name: String,
    pub parent_class_name: Option<String>,
}

impl Default for ClosureEnclosingClassInfo {
    fn default() -> Self {
        Self {
            kind: ClassishKind::Cclass(Abstraction::Concrete),
            name: "".to_string(),
            parent_class_name: None,
        }
    }
}

#[derive(Default, Debug)]
pub struct GlobalState {
    pub explicit_use_set: SSet,
    pub closure_namespaces: SMap<Arc<NamespaceEnv>>,
    pub closure_enclosing_classes: SMap<ClosureEnclosingClassInfo>,
    pub lambda_coeffects_of_scope: SMap<Coeffects>,
}

impl GlobalState {
    pub fn init() -> Self {
        GlobalState::default()
    }

    pub fn get_lambda_coeffects_of_scope(
        &self,
        class_name: &str,
        meth_name: &str,
    ) -> Option<&Coeffects> {
        let key = get_unique_id_for_method(class_name, meth_name);
        self.lambda_coeffects_of_scope.get(&key)
    }

    pub fn get_closure_enclosing_class(
        &self,
        class_name: &str,
    ) -> Option<&ClosureEnclosingClassInfo> {
        self.closure_enclosing_classes.get(class_name)
    }
}
