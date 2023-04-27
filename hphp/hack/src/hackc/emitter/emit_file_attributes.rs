// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use error::Result;
use hhbc::Attribute;
use itertools::Itertools;
use oxidized::ast;

use crate::emit_attribute::from_asts;

fn emit_file_attributes<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    fa: &ast::FileAttribute,
) -> Result<Vec<Attribute<'arena>>> {
    from_asts(e, &fa.user_attributes[..])
}

pub fn emit_file_attributes_from_program<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    prog: &[ast::Def],
) -> Result<Vec<Attribute<'arena>>> {
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
