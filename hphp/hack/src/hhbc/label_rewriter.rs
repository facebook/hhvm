// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod label_rewriter {
    use env::emitter::Emitter;
    use hhas_param_rust::HhasParam;
    use hhbc_ast_rust::{
        FcallArgs, GenDelegation, Instruct, InstructCall, InstructControlFlow, InstructIterator,
        InstructMisc,
    };
    use instruction_sequence_rust::instr_seq;
    use instruction_sequence_rust::Instr;
    use label_rust::{Id, Label};

    use std::collections::{HashMap, HashSet};

    fn create_label_to_offset_map(instrseq: &Instr) -> HashMap<Id, usize> {
        let mut folder = |(i, mut map): (usize, HashMap<Id, usize>), instr: &Instruct| match instr {
            Instruct::ILabel(l) => {
                if let Ok(id) = Label::id(l) {
                    map.insert(id, i);
                    (i, map)
                } else {
                    panic!("Label should've been rewritten by this point")
                }
            }
            _ => (i + 1, map),
        };
        instr_seq::fold_left(instrseq, &mut folder, (0, HashMap::new())).1
    }

    fn lookup_def(l: &Id, defs: &HashMap<Id, usize>) -> usize {
        match defs.get(l) {
            Some(ix) => *ix,
            None => panic!("lookup_def: label missing"),
        }
    }

    fn get_regular_labels(instr: &Instruct) -> Vec<Label> {
        use GenDelegation::*;
        use Instruct::*;
        use InstructCall::*;
        use InstructControlFlow::*;
        use InstructIterator::*;
        use InstructMisc::*;
        match instr {
            IIterator(IterInit(_, l, _))
            | IIterator(IterInitK(_, l, _, _))
            | IIterator(LIterInit(_, _, l, _))
            | IIterator(LIterInitK(_, _, l, _, _))
            | IIterator(IterNext(_, l, _))
            | IIterator(IterNextK(_, l, _, _))
            | IIterator(LIterNext(_, _, l, _))
            | IIterator(LIterNextK(_, _, l, _, _))
            | IIterator(IterBreak(l, _))
            | IGenDelegation(YieldFromDelegate(_, l))
            | IMisc(MemoGet(l, _))
            | IContFlow(Jmp(l))
            | IContFlow(JmpNS(l))
            | IContFlow(JmpZ(l))
            | IContFlow(JmpNZ(l))
            | ICall(FCall(FcallArgs(_, _, _, _, Some(l))))
            | ICall(FCallClsMethod(FcallArgs(_, _, _, _, Some(l)), _, _))
            | ICall(FCallClsMethodD(FcallArgs(_, _, _, _, Some(l)), _, _))
            | ICall(FCallClsMethodS(FcallArgs(_, _, _, _, Some(l)), _))
            | ICall(FCallClsMethodSD(FcallArgs(_, _, _, _, Some(l)), _, _))
            | ICall(FCallFunc(FcallArgs(_, _, _, _, Some(l)), _))
            | ICall(FCallFuncD(FcallArgs(_, _, _, _, Some(l)), _))
            | ICall(FCallObjMethod(FcallArgs(_, _, _, _, Some(l)), _, _))
            | ICall(FCallObjMethodD(FcallArgs(_, _, _, _, Some(l)), _, _)) => vec![l.clone()],
            IContFlow(Switch(_, _, ls)) => ls.to_vec(),
            IContFlow(SSwitch(pairs)) => pairs.iter().map(|x| x.1.clone()).collect::<Vec<_>>(),
            IMisc(MemoGetEager(l1, l2, _)) => vec![l1.clone(), l2.clone()],
            _ => vec![],
        }
    }

    fn create_label_ref_map(
        defs: &HashMap<Id, usize>,
        params: &Vec<HhasParam>,
        body: &Instr,
    ) -> (HashSet<Id>, HashMap<Id, usize>) {
        let process_ref =
            |(mut n, (mut used, mut refs)): (usize, (HashSet<Id>, HashMap<Id, usize>)),
             l: &Label| {
                if let Ok(id) = Label::id(l) {
                    let ix = lookup_def(&id, defs);
                    if !refs.contains_key(&ix) {
                        used.insert(id);
                        refs.insert(ix, n);
                        n += 1;
                    }
                    (n, (used, refs))
                } else {
                    panic!("Label should've been rewritten by this point")
                }
            };
        let gather_using = |acc: (usize, (HashSet<Id>, HashMap<Id, usize>)),
                            instrseq: &Instr,
                            get_labels: fn(&Instruct) -> Vec<Label>| {
            let mut folder = |acc: (usize, (HashSet<Id>, HashMap<Id, usize>)), instr: &Instruct| {
                (get_labels(instr)).iter().fold(acc, process_ref)
            };
            instr_seq::fold_left(instrseq, &mut folder, acc)
        };
        let init = gather_using(
            (0, (HashSet::new(), HashMap::new())),
            body,
            get_regular_labels,
        );
        let (_, map) = params.iter().fold(
            init,
            |acc: (usize, (HashSet<Id>, HashMap<Id, usize>)), param: &HhasParam| match &param
                .default_value
            {
                None => acc,
                Some((l, _)) => process_ref(acc, &l),
            },
        );
        map
    }

    fn relabel_instr<F>(instr: Instruct, relabel: &mut F) -> Instruct
    where
        F: FnMut(&Label) -> Label,
    {
        use GenDelegation::*;
        use Instruct::*;
        use InstructCall::*;
        use InstructControlFlow::*;
        use InstructIterator::*;
        use InstructMisc::*;
        match instr {
            IIterator(IterInit(id, l, v)) => IIterator(IterInit(id, relabel(&l), v)),
            IIterator(IterInitK(id, l, k, v)) => IIterator(IterInitK(id, relabel(&l), k, v)),
            IIterator(LIterInit(id, b, l, v)) => IIterator(LIterInit(id, b, relabel(&l), v)),
            IIterator(LIterInitK(id, b, l, k, v)) => {
                IIterator(LIterInitK(id, b, relabel(&l), k, v))
            }
            IIterator(IterNext(id, l, v)) => IIterator(IterNext(id, relabel(&l), v)),
            IIterator(IterNextK(id, l, k, v)) => IIterator(IterNextK(id, relabel(&l), k, v)),
            IIterator(LIterNext(id, b, l, v)) => IIterator(LIterNext(id, b, relabel(&l), v)),
            IIterator(LIterNextK(id, b, l, k, v)) => {
                IIterator(LIterNextK(id, b, relabel(&l), k, v))
            }
            IIterator(IterBreak(l, x)) => IIterator(IterBreak(relabel(&l), x)),
            IGenDelegation(YieldFromDelegate(i, l)) => {
                IGenDelegation(YieldFromDelegate(i, relabel(&l)))
            }
            ICall(FCall(FcallArgs(fl, na, nr, br, Some(l)))) => {
                ICall(FCall(FcallArgs(fl, na, nr, br, Some(relabel(&l)))))
            }
            ICall(FCallClsMethod(
                FcallArgs(fl, na, nr, br, Some(l)),
                p,
                is_log_as_dynamic_call,
            )) => ICall(FCallClsMethod(
                FcallArgs(fl, na, nr, br, Some(relabel(&l))),
                p,
                is_log_as_dynamic_call,
            )),
            ICall(FCallClsMethodD(FcallArgs(fl, na, nr, br, Some(l)), c, m)) => ICall(
                FCallClsMethodD(FcallArgs(fl, na, nr, br, Some(relabel(&l))), c, m),
            ),
            ICall(FCallClsMethodS(FcallArgs(fl, na, nr, br, Some(l)), c)) => ICall(
                FCallClsMethodS(FcallArgs(fl, na, nr, br, Some(relabel(&l))), c),
            ),
            ICall(FCallClsMethodSD(FcallArgs(fl, na, nr, br, Some(l)), c, m)) => ICall(
                FCallClsMethodSD(FcallArgs(fl, na, nr, br, Some(relabel(&l))), c, m),
            ),
            ICall(FCallFunc(FcallArgs(fl, na, nr, br, Some(l)), p)) => {
                ICall(FCallFunc(FcallArgs(fl, na, nr, br, Some(relabel(&l))), p))
            }
            ICall(FCallFuncD(FcallArgs(fl, na, nr, br, Some(l)), f)) => {
                ICall(FCallFuncD(FcallArgs(fl, na, nr, br, Some(relabel(&l))), f))
            }
            ICall(FCallObjMethod(FcallArgs(fl, na, nr, br, Some(l)), f, p)) => ICall(
                FCallObjMethod(FcallArgs(fl, na, nr, br, Some(relabel(&l))), f, p),
            ),
            ICall(FCallObjMethodD(FcallArgs(fl, na, nr, br, Some(l)), f, m)) => ICall(
                FCallObjMethodD(FcallArgs(fl, na, nr, br, Some(relabel(&l))), f, m),
            ),
            IContFlow(Jmp(l)) => IContFlow(Jmp(relabel(&l))),
            IContFlow(JmpNS(l)) => IContFlow(JmpNS(relabel(&l))),
            IContFlow(JmpZ(l)) => IContFlow(JmpZ(relabel(&l))),
            IContFlow(JmpNZ(l)) => IContFlow(JmpNZ(relabel(&l))),
            IContFlow(Switch(k, n, ll)) => {
                IContFlow(Switch(k, n, ll.into_iter().map(|l| relabel(&l)).collect()))
            }
            IContFlow(SSwitch(pairs)) => IContFlow(SSwitch(
                pairs
                    .into_iter()
                    .map(|(id, l)| (id, relabel(&l)))
                    .collect::<Vec<_>>(),
            )),
            IMisc(MemoGet(l, r)) => IMisc(MemoGet(relabel(&l), r)),
            IMisc(MemoGetEager(l1, l2, r)) => IMisc(MemoGetEager(relabel(&l1), relabel(&l2), r)),
            ILabel(l) => ILabel(relabel(&l)),
            _ => instr,
        }
    }

    fn rewrite_params_and_body(
        defs: &HashMap<Id, usize>,
        used: &HashSet<Id>,
        refs: &HashMap<Id, usize>,
        mut params: Vec<HhasParam>,
        body: &Instr,
    ) -> (Vec<HhasParam>, Instr) {
        let relabel_id = |id: Id| {
            *refs
                .get(&lookup_def(&id, defs))
                .expect("relabel_instrseq: offset not in refs")
        };
        let relabel_define_label_id = |id: Id| {
            if used.contains(&id) {
                refs.get(&lookup_def(&id, &defs)).map(|x| *x)
            } else {
                None
            }
        };
        let mut rewrite_instr = |instr: &Instruct| match instr {
            Instruct::ILabel(l) => match l.option_map(relabel_define_label_id) {
                Ok(Some(l)) => Some(Instruct::ILabel(l)),
                _ => None,
            },
            _ => Some(relabel_instr(instr.clone(), &mut |l| {
                l.map(relabel_id).unwrap()
            })),
        };
        let rewrite_param = |param: &mut HhasParam| {
            if let Some((l, _)) = &param.default_value {
                param.replace_default_value_label(l.map(relabel_id).unwrap())
            }
        };
        params.iter_mut().map(rewrite_param);
        (params, instr_seq::filter_map(body, &mut rewrite_instr))
    }

    pub fn relabel_function(params: Vec<HhasParam>, body: &Instr) -> (Vec<HhasParam>, Instr) {
        let defs = create_label_to_offset_map(body);
        let (used, refs) = create_label_ref_map(&defs, &params, body);
        rewrite_params_and_body(&defs, &used, &refs, params, body)
    }

    pub fn clone_with_fresh_regular_labels(emitter: &mut Emitter, block: Instr) -> Instr {
        let mut folder =
            |(mut regular, mut named): (HashMap<Id, Label>, HashMap<String, Label>),
             instr: &Instruct| {
                match instr {
                    Instruct::ILabel(Label::Regular(id)) => {
                        regular.insert(*id, emitter.label_gen_mut().next_regular());
                    }
                    Instruct::ILabel(Label::Named(name)) => {
                        named.insert(name.to_string(), emitter.label_gen_mut().next_regular());
                    }
                    _ => (),
                }
                (regular, named)
            };
        let (regular_labels, named_labels) =
            instr_seq::fold_left(&block, &mut folder, (HashMap::new(), HashMap::new()));

        if regular_labels.is_empty() && named_labels.is_empty() {
            return block;
        }

        let mut relabel = |l: &Label| {
            let lopt = match l {
                Label::Regular(id) => regular_labels.get(id),
                Label::Named(name) => named_labels.get(name),
                _ => None,
            };
            lopt.unwrap_or(l).clone()
        };
        instr_seq::map(&block, &mut |instr| relabel_instr(instr, &mut relabel))
    }
}
