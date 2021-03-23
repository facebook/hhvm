// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use emit_attribute_rust::from_asts;
use env::emitter::Emitter;
use hhas_attribute_rust::HhasAttribute;
use instruction_sequence::Result;
use oxidized::ast as tast;

extern crate itertools;
use itertools::Itertools;

fn emit_file_attributes(e: &mut Emitter, fa: &tast::FileAttribute) -> Result<Vec<HhasAttribute>> {
    from_asts(e, &fa.namespace, &fa.user_attributes[..])
}

pub fn emit_file_attributes_from_program(
    e: &mut Emitter,
    prog: &tast::Program,
) -> Result<Vec<HhasAttribute>> {
    prog.iter()
        .filter_map(|node| {
            if let tast::Def::FileAttributes(fa) = node {
                Some(emit_file_attributes(e, fa))
            } else {
                None
            }
        })
        .fold_results(vec![], |mut acc, attrs| {
            acc.extend(attrs);
            acc
        })
}
