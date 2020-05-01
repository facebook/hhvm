// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use compile_rust as compile;
use ocamlrep::{bytes_from_ocamlrep, FromOcamlRep, Value};
use ocamlrep_ocamlpool::to_ocaml;
use parser_core_types::source_text::SourceText;
use stack_limit::{StackLimit, MI};

use std::io::Write;

#[no_mangle]
extern "C" fn compile_from_text_ffi(env: usize, source_text: usize) -> usize {
    ocamlrep_ocamlpool::catch_unwind(|| {
        let job_builder = move || {
            Box::new(
                move |stack_limit: &StackLimit, _nomain_stack_size: Option<usize>| {
                    let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
                    let env = unsafe { compile::Env::from_ocaml(env).unwrap() };
                    let mut w = String::new();
                    let r = compile::from_text_(&env, stack_limit, &mut w, source_text)
                        .map_err(|e| e.to_string());

                    std::io::stdout().write_all(w.as_bytes()).unwrap();
                    std::io::stdout().flush().unwrap();
                    unsafe { to_ocaml(&r) }
                },
            )
        };
        // Assume peak is 2.5x of stack.
        // This is initial estimation, need to be improved later.
        let stack_slack = |stack_size| stack_size * 6 / 10;
        let on_retry = &mut |_: usize| {};
        let job = stack_limit::retry::Job {
            nonmain_stack_min: 13 * MI,
            nonmain_stack_max: None,
            ..Default::default()
        };
        match job.with_elastic_stack(&job_builder, on_retry, stack_slack) {
            Ok(r) => r,
            Err(e) => {
                let r: &Result<compile::Profile, String> = &Err(format!("{}", e));
                unsafe { to_ocaml(r) }
            }
        }
    })
}
