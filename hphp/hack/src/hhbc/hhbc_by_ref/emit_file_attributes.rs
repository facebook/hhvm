// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use decl_provider::DeclProvider;
use hhbc_by_ref_emit_attribute::from_asts;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_instruction_sequence::Result;
use oxidized::ast as tast;

extern crate itertools;
use itertools::Itertools;

fn emit_file_attributes<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    fa: &tast::FileAttribute,
) -> Result<Vec<HhasAttribute<'arena>>> {
    from_asts(alloc, e, &fa.user_attributes[..])
}

pub fn emit_file_attributes_from_program<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    prog: &[tast::Def],
) -> Result<Vec<HhasAttribute<'arena>>> {
    prog.iter()
        .filter_map(|node| {
            if let tast::Def::FileAttributes(fa) = node {
                Some(emit_file_attributes(alloc, e, fa))
            } else {
                None
            }
        })
        .fold_ok(vec![], |mut acc, attrs| {
            acc.extend(attrs);
            acc
        })
}
