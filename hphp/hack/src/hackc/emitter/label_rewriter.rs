// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use ffi::{Maybe::Just, Pair};
use hash::{HashMap, HashSet};
use hhas_param::HhasParam;
use hhbc_ast::{FcallArgs, Instruct, InstructControlFlow, InstructIterator, InstructMisc};
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

fn lookup_def<'h>(l: &Id, defs: &'h HashMap<Id, usize>) -> &'h usize {
    match defs.get(l) {
        Some(ix) => ix,
        None => panic!("lookup_def: label missing"),
    }
}

fn get_regular_labels<'arena>(instr: &Instruct<'arena>) -> Vec<Label> {
    use Instruct::*;
    use InstructControlFlow::*;
    use InstructIterator::*;
    use InstructMisc::*;
    match instr {
        IIterator(IterInit(_, l))
        | IIterator(IterNext(_, l))
        | IMisc(MemoGet(l, _))
        | IContFlow(Jmp(l))
        | IContFlow(JmpNS(l))
        | IContFlow(JmpZ(l))
        | IContFlow(JmpNZ(l)) => vec![*l],
        ICall(call) => match call.fcall_args() {
            Some(FcallArgs {
                async_eager_label: Just(l),
                ..
            }) => vec![*l],
            Some(_) | None => vec![],
        },
        IContFlow(Switch { labels, .. }) => labels.iter().copied().collect(),
        IContFlow(SSwitch { labels }) => labels.iter().map(|Pair(_, label)| *label).collect(),
        IMisc(MemoGetEager(l1, l2, _)) => vec![*l1, *l2],
        _ => vec![],
    }
}

fn create_label_ref_map<'arena>(
    defs: &HashMap<Id, usize>,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &InstrSeq<'arena>,
) -> (HashSet<Id>, HashMap<Id, usize>) {
    let process_ref =
        |(mut n, (mut used, mut refs)): (usize, (HashSet<Id>, HashMap<Id, usize>)), l: Label| {
            let id = l.id();
            let offset = lookup_def(id, defs);
            if !refs.contains_key(offset) {
                used.insert(*id);
                refs.insert(*offset, n);
                n += 1;
            }
            (n, (used, refs))
        };

    // Process body
    let init = body.iter().fold(
        (0, (HashSet::default(), HashMap::default())),
        |acc, instr| get_regular_labels(instr).into_iter().fold(acc, process_ref),
    );

    // Process params
    let (_, (used, refs)) =
        params
            .iter()
            .fold(init, |acc, (_, default_value)| match &default_value {
                None => acc,
                Some((label, _)) => process_ref(acc, *label),
            });
    (used, refs)
}

fn relabel_instr<'arena, F>(instr: &mut Instruct<'arena>, relabel: &mut F)
where
    F: FnMut(&mut Label),
{
    use Instruct::*;
    use InstructControlFlow::*;
    use InstructIterator::*;
    use InstructMisc::*;
    match instr {
        IIterator(IterInit(_, l))
        | IIterator(IterNext(_, l))
        | IContFlow(Jmp(l))
        | IContFlow(JmpNS(l))
        | IContFlow(JmpZ(l))
        | IContFlow(JmpNZ(l))
        | IMisc(MemoGet(l, _))
        | ILabel(l) => relabel(l),
        ICall(call) => match call.fcall_args_mut() {
            Some(FcallArgs {
                async_eager_label: Just(l),
                ..
            }) => relabel(l),
            Some(_) | None => {}
        },
        IContFlow(Switch { labels, .. }) => labels.iter_mut().for_each(relabel),
        IContFlow(SSwitch { labels }) => labels.iter_mut().for_each(|Pair(_, l)| relabel(l)),
        IMisc(MemoGetEager(l1, l2, _)) => {
            relabel(l1);
            relabel(l2);
        }
        _ => {}
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
    let relabel_id = |id: &Id| -> Id {
        *(refs
            .get(lookup_def(id, defs))
            .expect("relabel_instrseq: offset not in refs"))
    };
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
