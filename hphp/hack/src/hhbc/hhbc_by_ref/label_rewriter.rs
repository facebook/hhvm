// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::DeclProvider;
use hash::{HashMap, HashSet};
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_hhbc_ast::{
    FcallArgs, Instruct, InstructCall, InstructControlFlow, InstructIterator, InstructMisc,
};
use hhbc_by_ref_instruction_sequence::InstrSeq;
use hhbc_by_ref_label::{Id, Label};

fn create_label_to_offset_map<'arena>(instrseq: &InstrSeq<'arena>) -> HashMap<Id, usize> {
    let mut folder =
        |(i, mut map): (usize, HashMap<Id, usize>), instr: &Instruct<'arena>| match instr {
            Instruct::ILabel(l) => {
                map.insert(*l.id(), i);
                (i, map)
            }
            _ => (i + 1, map),
        };
    instrseq.fold_left(&mut folder, (0, HashMap::default())).1
}

fn lookup_def<'h>(l: &Id, defs: &'h HashMap<Id, usize>) -> &'h usize {
    match defs.get(l) {
        Some(ix) => ix,
        None => panic!("lookup_def: label missing"),
    }
}

fn get_regular_labels<'arena>(instr: &'arena Instruct<'arena>) -> Vec<&'arena Label> {
    use Instruct::*;
    use InstructCall::*;
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
        | IContFlow(JmpNZ(l))
        | ICall(FCall(FcallArgs(_, _, _, _, Some(l), _)))
        | ICall(FCallClsMethod(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallClsMethodD(FcallArgs(_, _, _, _, Some(l), _), _, _))
        | ICall(FCallClsMethodS(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallClsMethodSD(FcallArgs(_, _, _, _, Some(l), _), _, _))
        | ICall(FCallFunc(FcallArgs(_, _, _, _, Some(l), _)))
        | ICall(FCallFuncD(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallObjMethod(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallObjMethodD(FcallArgs(_, _, _, _, Some(l), _), _, _)) => vec![l],
        IContFlow(Switch(_, _, ls)) => ls.iter().collect::<Vec<_>>(),
        IContFlow(SSwitch(pairs)) => pairs.iter().map(|x| &x.1).collect::<Vec<_>>(),
        IMisc(MemoGetEager(l1, l2, _)) => vec![l1, l2],
        _ => vec![],
    }
}

fn create_label_ref_map<'arena>(
    defs: &HashMap<Id, usize>,
    params: &[HhasParam<'arena>],
    body: &InstrSeq<'arena>,
) -> (HashSet<Id>, HashMap<Id, usize>) {
    let process_ref =
        |(mut n, (mut used, mut refs)): (usize, (HashSet<Id>, HashMap<Id, usize>)), l: &Label| {
            let id = l.id();
            let offset = lookup_def(id, defs);
            if !refs.contains_key(offset) {
                used.insert(*id);
                refs.insert(*offset, n);
                n += 1;
            }
            (n, (used, refs))
        };
    let gather_using =
        |acc: (usize, (HashSet<Id>, HashMap<Id, usize>)), instrseq: &InstrSeq<'arena>| {
            let mut folder =
                |acc: (usize, (HashSet<Id>, HashMap<Id, usize>)), instr: &Instruct<'arena>| {
                    (get_regular_labels(instr))
                        .into_iter()
                        .fold(acc, process_ref)
                };
            instrseq.fold_left(&mut folder, acc)
        };
    let init = gather_using((0, (HashSet::default(), HashMap::default())), body);
    let (_, map) = params.iter().fold(
        init,
        |acc: (usize, (HashSet<Id>, HashMap<Id, usize>)), param: &HhasParam<'arena>| match &param
            .default_value
        {
            None => acc,
            Some((l, _)) => process_ref(acc, &l),
        },
    );
    map
}

fn relabel_instr<'arena, F>(instr: &mut Instruct<'arena>, relabel: &mut F)
where
    F: FnMut(&mut Label),
{
    use Instruct::*;
    use InstructCall::*;
    use InstructControlFlow::*;
    use InstructIterator::*;
    use InstructMisc::*;
    match instr {
        IIterator(IterInit(_, l))
        | IIterator(IterNext(_, l))
        | ICall(FCall(FcallArgs(_, _, _, _, Some(l), _)))
        | ICall(FCallClsMethod(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallClsMethodD(FcallArgs(_, _, _, _, Some(l), _), _, _))
        | ICall(FCallClsMethodS(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallClsMethodSD(FcallArgs(_, _, _, _, Some(l), _), _, _))
        | ICall(FCallFunc(FcallArgs(_, _, _, _, Some(l), _)))
        | ICall(FCallFuncD(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallObjMethod(FcallArgs(_, _, _, _, Some(l), _), _))
        | ICall(FCallObjMethodD(FcallArgs(_, _, _, _, Some(l), _), _, _))
        | IContFlow(Jmp(l))
        | IContFlow(JmpNS(l))
        | IContFlow(JmpZ(l))
        | IContFlow(JmpNZ(l))
        | IMisc(MemoGet(l, _))
        | ILabel(l) => relabel(l),
        IContFlow(Switch(_, _, ll)) => ll.iter_mut().for_each(|l| relabel(l)),
        IContFlow(SSwitch(pairs)) => pairs.iter_mut().for_each(|(_, l)| relabel(l)),
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
    params: &mut Vec<HhasParam<'arena>>,
    body: &mut InstrSeq<'arena>,
) {
    let relabel_id = |id: &Id| -> Id {
        *(refs
            .get(lookup_def(&id, defs))
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
    let rewrite_param = |param: &mut HhasParam<'arena>| {
        if let Some((l, _)) = &mut param.default_value {
            *l = l.map(relabel_id);
        }
    };
    params.iter_mut().for_each(|param| rewrite_param(param));
    body.filter_map_mut(alloc, &mut rewrite_instr);
}

pub fn relabel_function<'arena>(
    alloc: &'arena bumpalo::Bump,
    params: &mut Vec<HhasParam<'arena>>,
    body: &mut InstrSeq<'arena>,
) {
    let defs = create_label_to_offset_map(body);
    let (used, refs) = create_label_ref_map(&defs, &params, body);
    rewrite_params_and_body(alloc, &defs, &used, &refs, params, body)
}

pub fn clone_with_fresh_regular_labels<'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    block: &mut InstrSeq<'arena>,
) {
    let mut folder = |mut regular: HashMap<Id, Label>, instr: &Instruct<'arena>| {
        match instr {
            Instruct::ILabel(Label::Regular(id)) => {
                regular.insert(*id, emitter.label_gen_mut().next_regular());
            }
            _ => {}
        }
        regular
    };
    let regular_labels = block.fold_left(&mut folder, HashMap::default());

    if !regular_labels.is_empty() {
        let relabel = |l: &mut Label| {
            let new_label = match l {
                Label::Regular(id) => regular_labels.get(id),
                _ => None,
            };
            if let Some(nl) = new_label {
                *l = nl.clone();
            }
        };
        block.map_mut(&mut |instr| relabel_instr(instr, &mut |l| relabel(l)));
    }
}
