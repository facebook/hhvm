// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::iterator;
use label_rust::Label;
use oxidized::aast::*;
use std::collections::{BTreeMap, HashSet};

type Id = usize;
type LabelSet = HashSet<String>;

#[derive(Clone, Debug)]
pub struct LoopLabels {
    label_break: Label,
    label_continue: Label,
    iterator: Option<iterator::Id>,
}

#[derive(Clone, Debug)]
pub enum Region {
    Loop(LoopLabels, LabelSet),
    Switch(Label, LabelSet),
    TryFinally(Label, LabelSet),
    Finally(LabelSet),
    Function(LabelSet),
    Using(Label, LabelSet),
}

#[derive(Clone, Debug, Default)]
pub struct JumpTargets(Vec<Region>);
impl JumpTargets {
    pub fn as_slice(&self) -> &[Region] {
        self.0.as_slice()
    }

    pub fn get_closest_enclosing_finally_label(&self) -> Option<(Label, Vec<iterator::Id>)> {
        let mut iters = vec![];
        for r in self.0.iter().rev() {
            match r {
                Region::Using(l, _) | Region::TryFinally(l, _) => {
                    return Some((l.clone(), iters));
                }
                Region::Loop(LoopLabels { iterator, .. }, _) => {
                    add_iterator(iterator.clone(), &mut iters);
                }
                _ => (),
            }
        }
        None
    }

    // NOTE(hrust) this corresponds to collect_iterators in OCaml but doesn't allocate/clone
    pub fn iterators(&self) -> impl Iterator<Item = &iterator::Id> {
        self.0.iter().rev().filter_map(|r| {
            if let Region::Loop(LoopLabels { iterator, .. }, _) = r {
                iterator.as_ref()
            } else {
                None
            }
        })
    }

    pub fn find_goto_target(&self, label: &str) -> ResolvedGotoTarget {
        assert_eq!(self.0.is_empty(), false);

        let mut iters = vec![];
        for r in self.0.iter().rev() {
            match r {
                Region::Loop(LoopLabels { iterator, .. }, labels) => {
                    if labels.contains(label) {
                        return ResolvedGotoTarget::Label(iters);
                    } else {
                        add_iterator(iterator.clone(), &mut iters);
                    }
                }
                Region::Switch(_, labels) => {
                    if labels.contains(label) {
                        return ResolvedGotoTarget::Label(iters);
                    }
                }
                Region::Using(finally_start, labels)
                | Region::TryFinally(finally_start, labels) => {
                    if labels.contains(label) {
                        return ResolvedGotoTarget::Label(iters);
                    } else {
                        return ResolvedGotoTarget::Finally(ResolvedGotoFinally {
                            rgf_finally_start_label: finally_start.clone(),
                            rgf_iterators_to_release: iters,
                        });
                    }
                }
                Region::Finally(labels) => {
                    if labels.contains(label) {
                        return ResolvedGotoTarget::Label(iters);
                    } else {
                        return ResolvedGotoTarget::GotoFromFinally;
                    }
                }
                Region::Function(labels) => {
                    if labels.contains(label) {
                        return ResolvedGotoTarget::Label(iters);
                    } else {
                        return ResolvedGotoTarget::GotoInvalidLabel;
                    }
                }
            }
        }
        panic!("impossible")
    }

    /// Tries to find a target label given a level and a jump kind (break or continue)
    pub fn get_target_for_level(&self, is_break: bool, mut level: usize) -> ResolvedJumpTarget {
        let mut iters = vec![];
        let mut acc = vec![];
        let mut label = None;
        let mut skip_try_finally = None;
        // skip_try_finally is Some if we've already determined that we need to jump out of
        //  finally and now we are looking for the actual target label (break label of the
        //  while loop in the example below: )
        //
        //  while (1) {
        //     try {
        //        break;
        //     }
        //     finally {
        //        ...
        //     }
        //  }
        for r in self.0.iter().rev() {
            match r {
                Region::Function(_) | Region::Finally(_) => return ResolvedJumpTarget::NotFound,
                Region::Using(finally_label, _) | Region::TryFinally(finally_label, _) => {
                    // we need to jump out of try body in try/finally - in order to do this
                    // we should go through the finally block first
                    if skip_try_finally.is_none() {
                        skip_try_finally = Some((finally_label, level, iters.clone()));
                    }
                }
                Region::Switch(end_label, _) => {
                    if level == 1 {
                        label = Some(end_label);
                        iters.extend_from_slice(&std::mem::replace(&mut acc, vec![]));
                        break;
                    } else {
                        level -= 1;
                    }
                }
                Region::Loop(
                    LoopLabels {
                        label_break,
                        label_continue,
                        iterator,
                    },
                    _,
                ) => {
                    if level == 1 {
                        if is_break {
                            add_iterator(iterator.clone(), &mut acc);
                            label = Some(label_break);
                            iters.extend_from_slice(&std::mem::replace(&mut acc, vec![]));
                        } else {
                            label = Some(label_continue);
                            iters.extend_from_slice(&std::mem::replace(&mut acc, vec![]));
                        };
                        break;
                    } else {
                        add_iterator(iterator.clone(), &mut acc);
                        level -= 1;
                    }
                }
            }
        }
        if let Some((finally_label, level, iters)) = skip_try_finally {
            if let Some(target_label) = label {
                return ResolvedJumpTarget::ResolvedTryFinally(ResolvedTryFinally {
                    target_label: target_label.clone(),
                    finally_label: finally_label.clone(),
                    adjusted_level: level,
                    iterators_to_release: iters,
                });
            }
        };
        label.map_or(ResolvedJumpTarget::NotFound, |l| {
            ResolvedJumpTarget::ResolvedRegular(l.clone(), iters)
        })
    }
}

#[derive(Clone, PartialEq, Eq, std::cmp::Ord, std::cmp::PartialOrd, Debug)]
pub enum IdKey {
    IdReturn,
    IdLabel(Label),
}

#[derive(Clone, Debug, Default)]
pub struct Gen {
    label_id_map: BTreeMap<IdKey, Id>,
    labels_in_function: BTreeMap<String, bool>,
    function_has_goto: bool,
    jump_targets: JumpTargets,
}

impl Gen {
    pub fn new_id(&mut self, key: IdKey) -> Id {
        match self.label_id_map.get(&key) {
            Some(id) => *id,
            None => {
                let mut next_id = 0;
                while self.label_id_map.values().any(|&id| id == next_id) {
                    next_id += 1;
                }
                self.label_id_map.insert(key, next_id);
                next_id
            }
        }
    }

    pub fn get_labels_in_function(&self) -> &BTreeMap<String, bool> {
        &self.labels_in_function
    }

    pub fn get_function_has_goto(&self) -> bool {
        self.function_has_goto
    }

    pub fn set_labels_in_function(&mut self, labels_in_function: BTreeMap<String, bool>) {
        self.labels_in_function = labels_in_function;
    }

    pub fn set_function_has_goto(&mut self, function_has_goto: bool) {
        self.function_has_goto = function_has_goto;
    }

    pub fn jump_targets(&self) -> &JumpTargets {
        &self.jump_targets
    }

    pub fn reset(&mut self) {
        self.label_id_map.clear();
    }

    pub fn get_id_for_return(&mut self) -> Id {
        self.new_id(IdKey::IdReturn)
    }

    pub fn get_id_for_label(&mut self, l: Label) -> Id {
        self.new_id(IdKey::IdLabel(l))
    }

    pub fn with_loop<Ex, Fb, En, Hi>(
        &mut self,
        label_break: Label,
        label_continue: Label,
        iterator: Option<iterator::Id>,
        block: &Block<Ex, Fb, En, Hi>,
    ) {
        let labels = self.collect_valid_target_labels_for_block(block);
        self.jump_targets.0.push(Region::Loop(
            LoopLabels {
                label_break,
                label_continue,
                iterator,
            },
            labels,
        ))
    }

    pub fn with_switch<Ex, Fb, En, Hi>(
        &mut self,
        end_label: Label,
        cases: &Vec<Case<Ex, Fb, En, Hi>>,
    ) {
        let labels = self.collect_valid_target_labels_for_switch_cases(cases);
        // CONSIDER: now HHVM eagerly reserves state id for the switch end label
        // which does not seem to be necessary - do it for now for HHVM compatibility
        let _ = self.get_id_for_label(end_label.clone());
        self.jump_targets.0.push(Region::Switch(end_label, labels));
    }

    pub fn with_try<Ex, Fb, En, Hi>(
        &mut self,
        finally_label: Label,
        block: &Block<Ex, Fb, En, Hi>,
    ) {
        let labels = self.collect_valid_target_labels_for_block(block);
        self.jump_targets
            .0
            .push(Region::TryFinally(finally_label, labels));
    }

    pub fn with_finally<Ex, Fb, En, Hi>(&mut self, block: &Block<Ex, Fb, En, Hi>) {
        let labels = self.collect_valid_target_labels_for_block(block);
        self.jump_targets.0.push(Region::Finally(labels));
    }

    pub fn with_function<Ex, Fb, En, Hi>(&mut self, defs: &Program<Ex, Fb, En, Hi>) {
        let labels = self.collect_valid_target_labels_for_defs(defs);
        self.jump_targets.0.push(Region::Function(labels));
    }

    pub fn with_using<Ex, Fb, En, Hi>(
        &mut self,
        finally_label: Label,
        block: &Block<Ex, Fb, En, Hi>,
    ) {
        let labels = self.collect_valid_target_labels_for_block(block);
        self.jump_targets
            .0
            .push(Region::Using(finally_label, labels));
    }

    pub fn revert(&mut self) {
        self.jump_targets.0.pop();
    }
}

pub struct ResolvedTryFinally {
    pub target_label: Label,
    pub finally_label: Label,
    pub adjusted_level: usize,
    pub iterators_to_release: Vec<iterator::Id>,
}

pub enum ResolvedJumpTarget {
    NotFound,
    ResolvedTryFinally(ResolvedTryFinally),
    ResolvedRegular(Label, Vec<iterator::Id>),
}

pub struct ResolvedGotoFinally {
    pub rgf_finally_start_label: Label,
    pub rgf_iterators_to_release: Vec<iterator::Id>,
}

pub enum ResolvedGotoTarget {
    Label(Vec<iterator::Id>),
    Finally(ResolvedGotoFinally),
    GotoFromFinally,
    GotoInvalidLabel,
}

impl Gen {
    fn collect_valid_target_labels_for_stmt_aux<Ex, Fb, En, Hi>(
        acc: &mut LabelSet,
        s: &Stmt<Ex, Fb, En, Hi>,
    ) {
        use Stmt_::*;
        match &s.1 {
            Block(block) => Self::collect_valid_target_labels_for_block_aux(acc, block),
            Try(x) => {
                let (try_block, catch_blocks, _) = &**x;
                Self::collect_valid_target_labels_for_block_aux(acc, &try_block);
                for Catch(_, _, block) in catch_blocks.iter() {
                    Self::collect_valid_target_labels_for_block_aux(acc, block);
                }
            }
            GotoLabel(x) => {
                let (_, s) = &**x;
                acc.insert(s.to_string());
            }
            If(x) => {
                let (_, then_block, else_block) = &**x;
                Self::collect_valid_target_labels_for_block_aux(acc, then_block);
                Self::collect_valid_target_labels_for_block_aux(acc, else_block);
            }
            While(x) => {
                let (_, block) = &**x;
                Self::collect_valid_target_labels_for_block_aux(acc, block)
            }
            Using(x) => {
                let UsingStmt { block, .. } = &**x;
                Self::collect_valid_target_labels_for_block_aux(acc, block)
            }
            Switch(x) => {
                let (_, cases) = &**x;
                Self::collect_valid_target_labels_for_switch_cases_aux(acc, cases)
            }
            _ => (),
        }
    }

    fn collect_valid_target_labels_for_block_aux<Ex, Fb, En, Hi>(
        acc: &mut LabelSet,
        block: &Block<Ex, Fb, En, Hi>,
    ) {
        block
            .iter()
            .for_each(|stmt| Self::collect_valid_target_labels_for_stmt_aux(acc, stmt));
    }

    fn collect_valid_target_labels_for_switch_cases_aux<Ex, Fb, En, Hi>(
        acc: &mut LabelSet,
        cases: &Vec<Case<Ex, Fb, En, Hi>>,
    ) {
        cases.iter().for_each(|case| match case {
            Case::Default(_, block) | Case::Case(_, block) => {
                Self::collect_valid_target_labels_for_block_aux(acc, block)
            }
        })
    }

    fn collect_valid_target_labels_for_def_aux<Ex, Fb, En, Hi>(
        acc: &mut LabelSet,
        def: &Def<Ex, Fb, En, Hi>,
    ) {
        match def {
            Def::Stmt(s) => Self::collect_valid_target_labels_for_stmt_aux(acc, &**s),
            Def::Namespace(x) => {
                let (_, defs) = &**x;
                Self::collect_valid_target_labels_for_defs_aux(acc, defs)
            }
            _ => (),
        }
    }

    fn collect_valid_target_labels_for_defs_aux<Ex, Fb, En, Hi>(
        acc: &mut LabelSet,
        defs: &Program<Ex, Fb, En, Hi>,
    ) {
        defs.iter()
            .for_each(|def| Self::collect_valid_target_labels_for_def_aux(acc, def));
    }

    fn collect_valid_target_labels<X, F>(&self, x: &X, f: F) -> LabelSet
    where
        F: FnOnce(&mut LabelSet, &X),
    {
        let mut labels = HashSet::new();
        if self.function_has_goto {
            f(&mut labels, x);
        };
        labels
    }

    fn collect_valid_target_labels_for_block<Ex, Fb, En, Hi>(
        &self,
        block: &Block<Ex, Fb, En, Hi>,
    ) -> LabelSet {
        self.collect_valid_target_labels(block, Self::collect_valid_target_labels_for_block_aux)
    }

    fn collect_valid_target_labels_for_defs<Ex, Fb, En, Hi>(
        &self,
        defs: &Program<Ex, Fb, En, Hi>,
    ) -> LabelSet {
        self.collect_valid_target_labels(defs, Self::collect_valid_target_labels_for_defs_aux)
    }

    fn collect_valid_target_labels_for_switch_cases<Ex, Fb, En, Hi>(
        &self,
        cases: &Vec<Case<Ex, Fb, En, Hi>>,
    ) -> LabelSet {
        self.collect_valid_target_labels(
            cases,
            Self::collect_valid_target_labels_for_switch_cases_aux,
        )
    }

    pub fn release_ids(&mut self) {
        use Region::*;
        let _labels = match self
            .jump_targets
            .0
            .last_mut()
            .expect("empty region after executing run_and_release")
        {
            Loop(
                LoopLabels {
                    label_break,
                    label_continue,
                    ..
                },
                labels,
            ) => {
                self.label_id_map
                    .remove(&IdKey::IdLabel(label_break.clone()));
                self.label_id_map
                    .remove(&IdKey::IdLabel(label_continue.clone()));
                labels
            }
            Switch(l, labels) | TryFinally(l, labels) | Using(l, labels) => {
                self.label_id_map.remove(&IdKey::IdLabel(l.clone()));
                labels
            }
            Finally(labels) | Function(labels) => labels,
        };
        // CONSIDER: now HHVM does not release state ids for named labels
        // even after leaving the scope where labels are accessible
        // Do the same for now for compatibility reasons
        // labels
        //     .iter()
        //     .for_each(|l| self.label_id_map.remove(&IdKey::IdLabel(Label::Named(l.to_string()))));
    }
}

fn add_iterator(it_opt: Option<iterator::Id>, iters: &mut Vec<iterator::Id>) {
    if let Some(it) = it_opt {
        iters.push(it);
    }
}
