// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::iterator::Iter;
use label_rust::Label;
use oxidized::aast::*;
use std::collections::{HashMap, HashSet};

type Id = usize;
type LabelSet = HashSet<String>;

pub struct LoopLabels {
    label_break: Label,
    label_continue: Label,
    iterator: Option<Iter>,
}

pub enum Region {
    Loop(LoopLabels, LabelSet),
    Switch(Label, LabelSet),
    TryFinally(Label, LabelSet),
    Finally(LabelSet),
    Function(LabelSet),
    Using(Label, LabelSet),
}

#[derive(PartialEq, Eq, Hash)]
pub enum IdKey {
    IdReturn,
    IdLabel(Label),
}

pub struct Gen {
    label_id_map: HashMap<IdKey, Id>,
    labels_in_function: HashMap<Label, bool>,
    function_has_goto: bool,
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

    pub fn get_labels_in_function(&self) -> &HashMap<Label, bool> {
        &self.labels_in_function
    }

    pub fn get_function_has_goto(&self) -> bool {
        self.function_has_goto
    }

    pub fn set_labels_in_function(&mut self, labels_in_function: HashMap<Label, bool>) {
        self.labels_in_function = labels_in_function;
    }

    pub fn set_function_has_goto(&mut self, function_has_goto: bool) {
        self.function_has_goto = function_has_goto;
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

    pub fn with_loop<Ex, Fb, En, Hi, R, F>(
        &mut self,
        label_break: Label,
        label_continue: Label,
        iterator: Option<Iter>,
        t: &mut Vec<Region>,
        s: &Stmt<Ex, Fb, En, Hi>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Vec<Region>, &Stmt<Ex, Fb, En, Hi>) -> R,
    {
        let labels = self.collect_valid_target_labels_for_stmt(s);
        t.push(Region::Loop(
            LoopLabels {
                label_break,
                label_continue,
                iterator,
            },
            labels,
        ));
        self.run_and_release_ids(t, s, f)
    }

    pub fn with_switch<Ex, Fb, En, Hi, R, F>(
        &mut self,
        end_label: Label,
        t: &mut Vec<Region>,
        cases: &Vec<Case<Ex, Fb, En, Hi>>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Vec<Region>, ()) -> R,
    {
        let labels = self.collect_valid_target_labels_for_switch_cases(cases);
        // CONSIDER: now HHVM eagerly reserves state id for the switch end label
        // which does not seem to be necessary - do it for now for HHVM compatibility
        let _ = self.get_id_for_label(end_label.clone());
        t.push(Region::Switch(end_label, labels));
        self.run_and_release_ids(t, (), f)
    }

    pub fn with_try<Ex, Fb, En, Hi, R, F>(
        &mut self,
        finally_label: Label,
        t: &mut Vec<Region>,
        s: &Stmt<Ex, Fb, En, Hi>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Vec<Region>, &Stmt<Ex, Fb, En, Hi>) -> R,
    {
        let labels = self.collect_valid_target_labels_for_stmt(s);
        t.push(Region::TryFinally(finally_label, labels));
        self.run_and_release_ids(t, s, f)
    }

    pub fn with_finally<Ex, Fb, En, Hi, R, F>(
        &mut self,
        t: &mut Vec<Region>,
        s: &Stmt<Ex, Fb, En, Hi>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Vec<Region>, &Stmt<Ex, Fb, En, Hi>) -> R,
    {
        let labels = self.collect_valid_target_labels_for_stmt(s);
        t.push(Region::Finally(labels));
        self.run_and_release_ids(t, s, f)
    }

    pub fn with_function<Ex, Fb, En, Hi, R, F>(
        &mut self,
        t: &mut Vec<Region>,
        s: &Program<Ex, Fb, En, Hi>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Vec<Region>, &Program<Ex, Fb, En, Hi>) -> R,
    {
        let labels = self.collect_valid_target_labels_for_defs(s);
        t.push(Region::Function(labels));
        self.run_and_release_ids(t, s, f)
    }

    pub fn with_using<Ex, Fb, En, Hi, R, F>(
        &mut self,
        finally_label: Label,
        t: &mut Vec<Region>,
        s: &Stmt<Ex, Fb, En, Hi>,
        f: F,
    ) -> R
    where
        F: FnOnce(&mut Vec<Region>, &Stmt<Ex, Fb, En, Hi>) -> R,
    {
        let labels = self.collect_valid_target_labels_for_stmt(s);
        t.push(Region::Using(finally_label, labels));
        self.run_and_release_ids(t, s, f)
    }

    pub fn get_closest_enclosing_finally_label(t: Vec<Region>) -> Option<(Label, Vec<Iter>)> {
        let mut iters = vec![];
        for r in t.into_iter().rev() {
            match r {
                Region::Using(l, _) | Region::TryFinally(l, _) => {
                    return Some((l, iters));
                }
                Region::Loop(LoopLabels { iterator, .. }, _) => {
                    add_iterator(iterator, &mut iters);
                }
                _ => (),
            }
        }
        None
    }

    pub fn collect_iterators(t: Vec<Region>) -> Vec<Iter> {
        let mut iters = vec![];
        for r in t.into_iter().rev() {
            if let Region::Loop(LoopLabels { iterator, .. }, _) = r {
                add_iterator(iterator, &mut iters);
            }
        }
        iters
    }
}

pub struct ResolvedTryFinally {
    target_label: Label,
    finally_label: Label,
    adjusted_level: usize,
    iterators_to_release: Vec<Iter>,
}

pub enum ResolvedJumpTarget {
    NotFound,
    ResolvedTryFinally(ResolvedTryFinally),
    ResolvedRegular(Label, Vec<Iter>),
}

impl ResolvedJumpTarget {
    /// Tries to find a target label given a level and a jump kind (break or continue)
    pub fn get_target_for_level(is_break: bool, mut level: usize, t: Vec<Region>) -> Self {
        let mut iters = vec![];
        let mut acc = vec![];
        let mut label = None;
        let mut skip_try_finally = None;
        /// skip_try_finally is Some if we've already determined that we need to jump out of
        ///  finally and now we are looking for the actual target label (break label of the
        ///  while loop in the example below: )
        ///
        ///  while (1) {
        ///     try {
        ///        break;
        ///     }
        ///     finally {
        ///        ...
        ///     }
        ///  }
        for r in t.into_iter().rev() {
            match r {
                Region::Using(finally_label, _) | Region::TryFinally(finally_label, _) => {
                    // we need to jump out of try body in try/finally - in order to do this
                    // we should go through the finally block first
                    skip_try_finally = Some(finally_label);
                }
                Region::Switch(end_label, _) => {
                    if level == 1 {
                        label = Some(end_label);
                        iters.extend_from_slice(&std::mem::replace(&mut acc, vec![]));
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
                            add_iterator(iterator, &mut acc);
                            label = Some(label_break);
                            iters.extend_from_slice(&std::mem::replace(&mut acc, vec![]));
                        } else {
                            label = Some(label_continue);
                            iters.extend_from_slice(&std::mem::replace(&mut acc, vec![]));
                        };
                    } else {
                        add_iterator(iterator, &mut acc);
                        level -= 1;
                    }
                }
                _ => (),
            }
        }
        if let Some(finally_label) = skip_try_finally {
            if let Some(target_label) = label {
                return Self::ResolvedTryFinally(ResolvedTryFinally {
                    target_label,
                    finally_label,
                    adjusted_level: level,
                    iterators_to_release: iters,
                });
            }
        };
        if label.is_none() {
            Self::NotFound
        } else {
            Self::ResolvedRegular(label.unwrap(), iters)
        }
    }
}

pub struct ResolvedGotoFinally {
    rgf_finally_start_label: Label,
    rgf_iterators_to_release: Vec<Iter>,
}

pub enum ResolvedGotoTarget {
    Label(Vec<Iter>),
    Finally(ResolvedGotoFinally),
    GotoFromFinally,
    GotoInvalidLabel,
}

impl ResolvedGotoTarget {
    pub fn find_goto_target(t: Vec<Region>, label: String) -> Self {
        assert_eq!(t.is_empty(), false);

        let mut iters = vec![];
        for r in t.into_iter().rev() {
            match r {
                Region::Loop(LoopLabels { iterator, .. }, labels) => {
                    if labels.contains(&label) {
                        return Self::Label(iters);
                    } else {
                        add_iterator(iterator, &mut iters);
                    }
                }
                Region::Switch(_, labels) => {
                    if labels.contains(&label) {
                        return Self::Label(iters);
                    }
                }
                Region::Using(finally_start, labels)
                | Region::TryFinally(finally_start, labels) => {
                    if labels.contains(&label) {
                        return Self::Label(iters);
                    } else {
                        return Self::Finally(ResolvedGotoFinally {
                            rgf_finally_start_label: finally_start.clone(),
                            rgf_iterators_to_release: iters,
                        });
                    }
                }
                Region::Finally(labels) => {
                    if labels.contains(&label) {
                        return Self::Label(iters);
                    } else {
                        return Self::GotoFromFinally;
                    }
                }
                Region::Function(labels) => {
                    if labels.contains(&label) {
                        return Self::Label(iters);
                    } else {
                        return Self::GotoInvalidLabel;
                    }
                }
            }
        }
        panic!("impossible")
    }
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

    fn collect_valid_target_labels_for_stmt<Ex, Fb, En, Hi>(
        &self,
        stmt: &Stmt<Ex, Fb, En, Hi>,
    ) -> LabelSet {
        self.collect_valid_target_labels(stmt, Self::collect_valid_target_labels_for_stmt_aux)
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

    fn release_id(&mut self, l: Label) {
        self.label_id_map.remove(&IdKey::IdLabel(l));
    }

    /// runs a given function and then released label ids that were possibly assigned
    /// to labels at the head of the list
    fn run_and_release_ids<S, R, F>(&mut self, t: &mut Vec<Region>, s: S, f: F) -> R
    where
        F: FnOnce(&mut Vec<Region>, S) -> R,
    {
        use Region::*;
        let res = f(t, s);
        let _labels = match t
            .last()
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
                self.release_id(label_break.clone());
                self.release_id(label_continue.clone());
                labels
            }
            Switch(l, labels) | TryFinally(l, labels) | Using(l, labels) => {
                self.release_id(l.clone());
                labels
            }
            Finally(labels) | Function(labels) => labels,
        };
        // CONSIDER: now HHVM does not release state ids for named labels
        // even after leaving the scope where labels are accessible
        // Do the same for now for compatibility reasons
        // labels
        //     .iter()
        //     .for_each(|l| self.release_id(Label::Named(l.to_string())));
        res
    }
}

fn add_iterator(it_opt: Option<Iter>, iters: &mut Vec<Iter>) {
    if let Some(it) = it_opt {
        iters.push(it);
    }
}
