// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use crate::Context;
use ocaml::core::mlvalues::Value;
use parser_core_types::source_text::SourceText;
use rust_to_ocaml::{SerializationContext, ToOcaml};
use smart_constructors::NoState;
use syntax_smart_constructors::StateType;

#[derive(Clone)]
pub struct OcamlContextState<'src> {
    source: SourceText<'src>,
    context: Rc<SerializationContext>,
}

impl<'src> OcamlContextState<'src> {
    pub fn initial(src: &SourceText<'src>) -> Self {
        Self {
            source: src.clone(),
            context: Rc::new(SerializationContext::new(
                src.ocaml_source_text().unwrap().as_usize(),
            )),
        }
    }
}

impl Context for OcamlContextState<'_> {
    fn serialization_context(&self) -> &SerializationContext {
        self.context.as_ref()
    }
}

impl<S: Clone> StateType<S> for OcamlContextState<'_> {
    fn next(&mut self, _inputs: &[&S]) {}
}

impl ToOcaml for OcamlContextState<'_> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        NoState.to_ocaml(context)
    }
}
