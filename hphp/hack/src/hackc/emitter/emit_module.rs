// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use error::Result;
use ffi::Maybe;
use hhbc::Module;
use hhbc::ModuleName;
use hhbc::Span;
use oxidized::ast;

use crate::emit_attribute;

pub fn emit_module<'a>(emitter: &mut Emitter, ast_module: &'a ast::ModuleDef) -> Result<Module> {
    let attributes = emit_attribute::from_asts(emitter, &ast_module.user_attributes)?;
    let name = ModuleName::intern(&ast_module.name.1);
    let span = Span::from_pos(&ast_module.span);
    let doc_comment = ast_module.doc_comment.clone();

    Ok(Module {
        attributes: attributes.into(),
        name,
        span,
        doc_comment: doc_comment
            .map(|(_, comment)| comment.into_bytes().into())
            .into(),
    })
}

pub fn emit_modules_from_program<'a>(
    emitter: &mut Emitter,
    ast: &'a [ast::Def],
) -> Result<Vec<Module>> {
    ast.iter()
        .filter_map(|def| {
            if let ast::Def::Module(md) = def {
                Some(emit_module(emitter, md))
            } else {
                None
            }
        })
        .collect()
}

pub fn emit_module_use_from_program(prog: &[ast::Def]) -> Maybe<ModuleName> {
    for node in prog.iter() {
        if let ast::Def::SetModule(s) = node {
            return Maybe::Just(ModuleName::intern(&s.1));
        }
    }
    Maybe::Nothing
}
