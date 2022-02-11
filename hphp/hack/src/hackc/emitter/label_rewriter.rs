// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use hash::{HashMap, HashSet};
use hhas_param::HhasParam;
use hhbc_ast::Instruct;
use instruction_sequence::InstrSeq;
use label::{Id, Label};
use oxidized::ast;

fn create_label_to_offset_map<'arena>(instrseq: &InstrSeq<'arena>) -> HashMap<Id, usize> {
    let (_, map) = instrseq.iter().fold(
        (0, HashMap::default()),
        |(i, mut map): (usize, HashMap<Id, usize>), instr: &Instruct<'arena>| match instr {
            Instruct::ILabel(l) => {
                map.insert(*l.id(), i);
                (i, map)
            }
            _ => (i + 1, map),
        },
    );
    map
}

fn create_label_ref_map<'arena>(
    defs: &HashMap<Id, usize>,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &InstrSeq<'arena>,
) -> (HashSet<Id>, HashMap<Id, usize>) {
    let process_ref = |
        (mut next, (mut used, mut refs)): (usize, (HashSet<Id>, HashMap<Id, usize>)),
        target: &Label,
    | {
        let id = target.id();
        let offset = defs[id];
        refs.entry(offset).or_insert_with(|| {
            used.insert(*id);
            next += 1;
            next - 1
        });
        (next, (used, refs))
    };

    // Process the function body.
    let init = body.iter().fold(
        Default::default(),
        |acc: (usize, (HashSet<Id>, HashMap<Id, usize>)), instr: &Instruct<'arena>| {
            instr.targets().iter().fold(acc, process_ref)
        },
    );

    // Process params
    let (_, (used, refs)) =
        params
            .iter()
            .fold(init, |acc, (_param, default_value)| match &default_value {
                None => acc,
                Some((target, _)) => process_ref(acc, target),
            });
    (used, refs)
}

fn relabel_instr<'arena, F>(instr: &mut Instruct<'arena>, relabel: &mut F)
where
    F: FnMut(&mut Label),
{
    let labels = match instr {
        Instruct::ILabel(label) => std::slice::from_mut(label),
        _ => instr.targets_mut(),
    };
    for label in labels {
        relabel(label)
    }
}

fn rewrite_params_and_body<'arena>(
    alloc: &'arena bumpalo::Bump,
    defs: &HashMap<Id, usize>,
    used: &HashSet<Id>,
    refs: &HashMap<Id, usize>,
    params: &mut Vec<(HhasParam<'arena>, Option<(Label, ast::Expr)>)>,
    body: &mut InstrSeq<'arena>,
) {
    let relabel_id = |id: &Id| -> Id { refs[&defs[id]] };
    let mut rewrite_instr = |instr: &mut Instruct<'arena>| -> bool {
        if let Instruct::ILabel(ref mut l) = instr {
            if used.contains(l.id()) {
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
    let defs = create_label_to_offset_map(body);
    let (used, refs) = create_label_ref_map(&defs, params.as_slice(), body);
    rewrite_params_and_body(alloc, &defs, &used, &refs, params, body)
}

pub fn rewrite_with_fresh_regular_labels<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    block: &mut InstrSeq<'arena>,
) {
    let regular_labels = block.iter().fold(HashMap::default(), |mut acc, instr| {
        if let Instruct::ILabel(Label::Regular(id)) = instr {
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
