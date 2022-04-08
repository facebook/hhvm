// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use error::Result;
use ffi::{Maybe, Slice, Str};
use hhbc::{hhas_module::HhasModule, hhas_pos::HhasSpan, ClassName};
use oxidized::ast;

pub fn emit_module<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    ast_module: &'a ast::Module,
) -> Result<HhasModule<'arena>> {
    let attributes = emit_attribute::from_asts(emitter, &ast_module.user_attributes)?;
    let name = ClassName::from_ast_name_and_mangle(alloc, &ast_module.name.1);
    let span = HhasSpan::from_pos(&ast_module.span);
    Ok(HhasModule {
        attributes: Slice::fill_iter(alloc, attributes.into_iter()),
        name,
        span,
    })
}

pub fn emit_modules_from_program<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    ast: &'a [ast::Def],
) -> Result<Vec<HhasModule<'arena>>> {
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
        // TODO T115356820: This is temporary until parser support is added
        if let ast::Def::FileAttributes(fa) = node {
            for attr in fa.user_attributes.iter() {
                if attr.name.1 == "__Module" {
                    match attr.params[..] {
                        [ast::Expr(_, _, ast::Expr_::String(ref ctx))] => {
                            return Maybe::Just(Str::new_str(
                                e.alloc,
                                // FIXME: This is not safe--string literals are binary strings.
                                // There's no guarantee that they're valid UTF-8.
                                unsafe { std::str::from_utf8_unchecked(ctx.as_slice()) },
                            ));
                        }
                        _ => continue,
                    }
                }
            }
        }
    }
    Maybe::Nothing
}
