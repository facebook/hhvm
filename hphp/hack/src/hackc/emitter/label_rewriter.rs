// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use hash::{HashMap, HashSet};
use hhas_param::HhasParam;
use hhbc_ast::{Instruct, Pseudo};
use instruction_sequence::InstrSeq;
use label::Label;
use oxidized::ast;

/// Create a mapping Label instructions to their position in the InstrSeq without
/// the labels. In other words, all instructions get numbered except labels.
fn create_label_to_offset_map<'arena>(instrseq: &InstrSeq<'arena>) -> HashMap<Label, u32> {
    let mut index = 0;
    instrseq
        .iter()
        .filter_map(|instr| match instr {
            Instruct::Pseudo(Pseudo::Label(label)) => Some((*label, index)),
            _ => {
                index += 1;
                None
            }
        })
        .collect()
}

fn create_label_ref_map<'arena>(
    label_to_offset: &HashMap<Label, u32>,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &InstrSeq<'arena>,
) -> (HashSet<Label>, HashMap<u32, Label>) {
    let mut label_gen = label::Gen::default();
    let mut used = HashSet::default();
    let mut offset_to_label = HashMap::default();

    let mut process_ref = |target: &Label| {
        let offset = label_to_offset[target];
        offset_to_label.entry(offset).or_insert_with(|| {
            used.insert(*target);
            label_gen.next_regular()
        });
    };

    // Process the function body.
    for instr in body.iter() {
        for target in instr.targets().iter() {
            process_ref(target);
        }
    }

    // Process params
    for (_param, dv) in params {
        if let Some((target, _)) = dv {
            process_ref(target);
        }
    }
    (used, offset_to_label)
}

fn rewrite_params_and_body<'arena>(
    label_to_offset: &HashMap<Label, u32>,
    used: &HashSet<Label>,
    offset_to_label: &HashMap<u32, Label>,
    params: &mut [(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &mut InstrSeq<'arena>,
) {
    let relabel = |id: Label| offset_to_label[&label_to_offset[&id]];
    for (_, dv) in params.iter_mut() {
        if let Some((l, _)) = dv {
            *l = relabel(*l);
        }
    }
    body.retain_mut(|instr| {
        if let Instruct::Pseudo(Pseudo::Label(l)) = instr {
            if used.contains(l) {
                *l = relabel(*l);
                true
            } else {
                false
            }
        } else {
            for target in instr.targets_mut() {
                *target = relabel(*target);
            }
            true
        }
    });
}

pub fn relabel_function<'arena>(
    params: &mut [(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &mut InstrSeq<'arena>,
) {
    let label_to_offset = create_label_to_offset_map(body);
    let (used, offset_to_label) = create_label_ref_map(&label_to_offset, params, body);
    rewrite_params_and_body(&label_to_offset, &used, &offset_to_label, params, body)
}

pub fn rewrite_with_fresh_regular_labels<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    block: &mut InstrSeq<'arena>,
) {
    let mut old_to_new = HashMap::default();
    for instr in block.iter() {
        if let Instruct::Pseudo(Pseudo::Label(label)) = instr {
            old_to_new.insert(*label, emitter.label_gen_mut().next_regular());
        }
    }

    if !old_to_new.is_empty() {
        for instr in block.iter_mut() {
            let labels = match instr {
                Instruct::Pseudo(Pseudo::Label(label)) => std::slice::from_mut(label),
                _ => instr.targets_mut(),
            };
            for label in labels {
                if let Some(new_label) = old_to_new.get(label) {
                    *label = *new_label;
                }
            }
        }
    }
}
