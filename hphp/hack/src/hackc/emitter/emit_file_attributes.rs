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

fn emit_file_attributes(
    e: &mut Emitter<'_, '_>,
    fa: &ast::FileAttribute,
) -> Result<Vec<Attribute>> {
    from_asts(e, &fa.user_attributes[..])
}

pub fn emit_file_attributes_from_program(
    e: &mut Emitter<'_, '_>,
    prog: &[ast::Def],
) -> Result<Vec<Attribute>> {
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
