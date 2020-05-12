// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_env_types::Env;
use oxidized_by_ref::ast_defs::Pos;

pub fn hh_show_env(pos: &Pos, env: &mut Env) {
    unsafe {
        let ocaml_env = ocamlrep_ocamlpool::to_ocaml(env);
        let ocaml_pos = ocamlrep_ocamlpool::to_ocaml(&pos);

        let ocaml_hh_show_env =
            ocaml::named_value("hh_show_env").expect("hh_show_env not registered");
        ocaml_hh_show_env
            .call_n(&[ocaml::Value::new(ocaml_pos), ocaml::Value::new(ocaml_env)])
            .unwrap();
    }
}
