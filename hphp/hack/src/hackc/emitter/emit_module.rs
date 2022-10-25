// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use error::Result;
use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use hhbc::ClassName;
use hhbc::Module;
use hhbc::Span;
use oxidized::ast;

use crate::emit_attribute;

pub fn emit_module<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    ast_module: &'a ast::Module,
) -> Result<Module<'arena>> {
    let attributes = emit_attribute::from_asts(emitter, &ast_module.user_attributes)?;
    let name = ClassName::from_ast_name_and_mangle(alloc, &ast_module.name.1);
    let span = Span::from_pos(&ast_module.span);
    let doc_comment = ast_module.doc_comment.clone();
    Ok(Module {
        attributes: Slice::fill_iter(alloc, attributes.into_iter()),
        name,
        span,
        doc_comment: Maybe::from(doc_comment.map(|c| Str::new_str(alloc, &c.1))),
    })
}

pub fn emit_modules_from_program<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    ast: &'a [ast::Def],
) -> Result<Vec<Module<'arena>>> {
    ast.iter()
        .filter_map(|def| {
            if let ast::Def::Module(md) = def {
                Some(emit_module(alloc, emitter, md))
            } else {
                None
            }
        })
        .collect()
}

pub fn emit_module_use_from_program<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    prog: &[ast::Def],
) -> Maybe<Str<'arena>> {
    for node in prog.iter() {
        if let ast::Def::SetModule(s) = node {
            return Maybe::Just(Str::new_str(e.alloc, &*s.1));
        }
    }
    Maybe::Nothing
}
