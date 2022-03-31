// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_scope::{Scope, ScopeItem};

pub type SMap<T> = std::collections::BTreeMap<String, T>;
pub type SSet = std::collections::BTreeSet<String>;

pub fn get_unique_id_for_main() -> String {
    String::from("|")
}

pub fn get_unique_id_for_method(cls_name: &str, md_name: &str) -> String {
    format!("{}|{}", cls_name, md_name)
}

pub fn get_unique_id_for_function(fun_name: &str) -> String {
    format!("|{}", fun_name)
}

pub fn get_unique_id_for_scope(scope: &Scope<'_, '_>) -> String {
    match scope.items.as_slice() {
        [] => get_unique_id_for_main(),
        [.., ScopeItem::Class(cls), ScopeItem::Method(md)]
        | [
            ..,
            ScopeItem::Class(cls),
            ScopeItem::Method(md),
            ScopeItem::Lambda(_),
        ] => get_unique_id_for_method(cls.get_name_str(), md.get_name_str()),
        [.., ScopeItem::Function(fun)] => get_unique_id_for_function(fun.get_name_str()),
        _ => panic!("unexpected scope shape"),
    }
}
