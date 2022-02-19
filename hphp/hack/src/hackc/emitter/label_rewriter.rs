// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use hash::{HashMap, HashSet};
use hhas_param::HhasParam;
use hhbc_ast::Instruct;
use instruction_sequence::InstrSeq;
use label::{Label, LabelId};
use oxidized::ast;

/// Create a mapping Label instructions to their position in the InstrSeq without
/// the labels. In other words, all instructions get numbered except labels.
fn create_label_to_offset_map<'arena>(instrseq: &InstrSeq<'arena>) -> HashMap<LabelId, u32> {
    let mut index = 0;
    instrseq
        .iter()
        .filter_map(|instr| match instr {
            Instruct::Label(label) => Some((label.id(), index)),
            _ => {
                index += 1;
                None
            }
        })
        .collect()
}

fn create_label_ref_map<'arena>(
    label_to_offset: &HashMap<LabelId, u32>,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &InstrSeq<'arena>,
) -> (HashSet<LabelId>, HashMap<u32, LabelId>) {
    let process_ref = |
        (mut next, (mut used, mut offset_to_label)): (
            u32,
            (HashSet<LabelId>, HashMap<u32, LabelId>),
        ),
        target: &Label,
    | {
        let old_id = target.id();
        let offset = label_to_offset[&old_id];
        offset_to_label.entry(offset).or_insert_with(|| {
            used.insert(old_id);
            let new_id = LabelId(next);
            next += 1;
            new_id
        });
        (next, (used, offset_to_label))
    };

    // Process the function body.
    let init = body.iter().fold(
        Default::default(),
        |acc: (u32, (HashSet<LabelId>, HashMap<u32, LabelId>)), instr: &Instruct<'arena>| {
            instr.targets().iter().fold(acc, process_ref)
        },
    );

    // Process params
    let (_, (used, offset_to_label)) =
        params
            .iter()
            .fold(init, |acc, (_param, default_value)| match &default_value {
                None => acc,
                Some((target, _)) => process_ref(acc, target),
            });
    (used, offset_to_label)
}

fn relabel_instr<'arena, F>(instr: &mut Instruct<'arena>, relabel: &mut F)
where
    F: FnMut(&mut Label),
{
    let labels = match instr {
        Instruct::Label(label) => std::slice::from_mut(label),
        _ => instr.targets_mut(),
    };
    for label in labels {
        relabel(label)
    }
}

fn rewrite_params_and_body<'arena>(
    alloc: &'arena bumpalo::Bump,
    label_to_offset: &HashMap<LabelId, u32>,
    used: &HashSet<LabelId>,
    offset_to_label: &HashMap<u32, LabelId>,
    params: &mut [(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &mut InstrSeq<'arena>,
) {
    let relabel_id = |id: &LabelId| offset_to_label[&label_to_offset[id]];
    let mut rewrite_instr = |instr: &mut Instruct<'arena>| -> bool {
        if let Instruct::Label(ref mut l) = instr {
            if used.contains(&l.id()) {
                *l = l.map(relabel_id);
                true
            } else {
                false
            }
        } else {
            relabel_instr(instr, &mut |l| *l = l.map(relabel_id));
            true
        }
    };
    let rewrite_param = |(_, default_value): &mut (HhasParam<'arena>, Option<(Label, ast::Expr)>)| {
        if let Some((l, _)) = default_value {
            *l = l.map(relabel_id);
        }
    };
    params.iter_mut().for_each(|param| rewrite_param(param));
    body.filter_map_mut(alloc, &mut rewrite_instr);
}

pub fn relabel_function<'arena>(
    alloc: &'arena bumpalo::Bump,
    params: &mut Vec<(HhasParam<'arena>, Option<(Label, ast::Expr)>)>,
    body: &mut InstrSeq<'arena>,
) {
    let label_to_offset = create_label_to_offset_map(body);
    let (used, offset_to_label) = create_label_ref_map(&label_to_offset, params, body);
    rewrite_params_and_body(
        alloc,
        &label_to_offset,
        &used,
        &offset_to_label,
        params,
        body,
    )
}

pub fn rewrite_with_fresh_regular_labels<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    block: &mut InstrSeq<'arena>,
) {
    let regular_labels = block.iter().fold(HashMap::default(), |mut acc, instr| {
        if let Instruct::Label(Label::Regular(id)) = instr {
            acc.insert(*id, emitter.label_gen_mut().next_regular());
        }
        acc
    });

    if !regular_labels.is_empty() {
        block.map_mut(&mut |instr| {
            relabel_instr(instr, &mut |label| {
                if let Label::Regular(id) = label {
                    if let Some(new_label) = regular_labels.get(id) {
                        *label = *new_label;
                    }
                }
            })
        });
    }
}
