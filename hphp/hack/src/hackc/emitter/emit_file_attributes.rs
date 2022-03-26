// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use emit_attribute::from_asts;
use env::emitter::Emitter;
use ffi::{Maybe, Str};
use hhas_attribute::HhasAttribute;
use instruction_sequence::Result;
use itertools::Itertools;
use oxidized::ast;

fn emit_file_attributes<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    fa: &ast::FileAttribute,
) -> Result<Vec<HhasAttribute<'arena>>> {
    from_asts(e, &fa.user_attributes[..])
}

pub fn emit_file_attributes_from_program<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    prog: &[ast::Def],
) -> Result<Vec<HhasAttribute<'arena>>> {
    prog.iter()
        .filter_map(|node| {
            if let ast::Def::FileAttributes(fa) = node {
                Some(emit_file_attributes(e, fa))
            } else {
                None
            }
        })
        .fold_ok(vec![], |mut acc, attrs| {
            acc.extend(attrs);
            acc
        })
}

pub fn emit_module_from_program<'arena, 'decl>(
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
