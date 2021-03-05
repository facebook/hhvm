// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub type SMap<T> = std::collections::BTreeMap<String, T>;
pub type SSet = std::collections::BTreeSet<String>;

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

pub fn get_unique_id_for_main() -> String {
    String::from("|")
}

pub fn get_unique_id_for_method(cls_name: &str, md_name: &str) -> String {
    format!("{}|{}", cls_name, md_name)
}

pub fn get_unique_id_for_function(fun_name: &str) -> String {
    format!("|{}", fun_name)
}
