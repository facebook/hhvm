// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use aast_parser::{rust_aast_parser_types::Env, AastParser};
use env::emitter::Emitter;
use global_state::{GlobalState, LazyState};
use ocamlrep_ocamlpool::ocaml_ffi;
use options::{CompilerFlags, Options};
use oxidized::namespace_env;
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};

ocaml_ffi! {
    fn rust_closure_convert_from_text(source_text: SourceText) -> (oxidized::ast::Program, GlobalState) {
        let mut env = Env::default();
        env.keep_errors = true;
        env.show_all_errors = true;
        env.fail_open = true;
        let indexed_source_text = IndexedSourceText::new(source_text);
        let mut res = AastParser::from_text(&env, &indexed_source_text, None).unwrap().aast.unwrap();

        let empty_namespace = ocamlrep::rc::RcOc::new(namespace_env::Env::empty(vec![], false, false));
        elaborate_namespaces_visitor::elaborate_program(empty_namespace, &mut res);

        let mut options = Options::default();
        options.hack_compiler_flags.set(CompilerFlags::CONSTANT_FOLDING, true);
        let mut emitter = Emitter::new(options);
        closure_convert_rust::convert_toplevel_prog(&mut emitter, &mut res);
        (res, emitter.into_emit_state())
    }
}
