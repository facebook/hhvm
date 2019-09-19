// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ocaml_syntax::Context;
use coroutine_smart_constructors::CoroutineStateType;
use parser_rust::{parser_env::ParserEnv, source_text::SourceText};
use rust_to_ocaml::SerializationContext;
use syntax_smart_constructors::StateType;

use std::marker::PhantomData;
use std::rc::Rc;

#[derive(Clone)]
pub struct OcamlCoroutineState<'src, S> {
    seen_ppl: bool,
    source: SourceText<'src>,
    is_codegen: bool,
    context: Rc<SerializationContext>,
    phantom_s: PhantomData<S>,
}

impl<'src, S> CoroutineStateType for OcamlCoroutineState<'src, S> {
    fn set_seen_ppl(&mut self, v: bool) {
        self.seen_ppl = v;
    }

    fn seen_ppl(&self) -> bool {
        self.seen_ppl
    }
    fn source(&self) -> &SourceText {
        &self.source
    }
    fn is_codegen(&self) -> bool {
        self.is_codegen
    }
}

impl<'src, S> Context for OcamlCoroutineState<'src, S> {
    fn serialization_context(&self) -> &SerializationContext {
        self.context.as_ref()
    }
}

impl<'src, S: Clone> StateType<'src, S> for OcamlCoroutineState<'src, S> {
    fn initial(env: &ParserEnv, src: &SourceText<'src>) -> Self {
        Self {
            seen_ppl: false,
            source: src.clone(),
            is_codegen: env.codegen,
            context: Rc::new(SerializationContext::new(src.ocaml_source_text())),
            phantom_s: PhantomData,
        }
    }

    fn next(&mut self, _inputs: &[&S]) {}
}
