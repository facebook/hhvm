// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_hhas_coeffects::HhasCoeffects;
use hhbc_by_ref_unique_id_builder::{SMap, SSet, UniqueIdBuilder};
use lazy_static::lazy_static;
use ocamlrep::rc::RcOc;
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use oxidized::{ast_defs::ClassKind, namespace_env::Env as NamespaceEnv};

#[derive(Debug, FromOcamlRep, ToOcamlRep, Clone)]
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

#[derive(Default, FromOcamlRep, ToOcamlRep, Debug)]
pub struct GlobalState {
    pub explicit_use_set: SSet,
    pub closure_namespaces: SMap<RcOc<NamespaceEnv>>,
    pub closure_enclosing_classes: SMap<ClosureEnclosingClassInfo>,
    pub functions_with_finally: SSet,
    pub lambda_coeffects_of_scope: SMap<HhasCoeffects>,
}

impl GlobalState {
    pub fn init() -> Self {
        GlobalState::default()
    }

    pub fn get_lambda_coeffects_of_scope(
        &self,
        class_name: &str,
        meth_name: &str,
    ) -> &HhasCoeffects {
        lazy_static! {
            static ref DEFAULT_HHAS_COEFFECTS: HhasCoeffects = HhasCoeffects::default();
        }
        let key = UniqueIdBuilder::new().method(class_name, meth_name);
        self.lambda_coeffects_of_scope
            .get(&key)
            .unwrap_or(&DEFAULT_HHAS_COEFFECTS)
    }

    pub fn get_closure_enclosing_class(
        &self,
        class_name: &str,
    ) -> Option<&ClosureEnclosingClassInfo> {
        self.closure_enclosing_classes.get(class_name)
    }
}
