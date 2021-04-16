// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_label::Label;

type Id = usize;

#[derive(Clone, Debug)]
pub struct LoopLabels {
    label_break: Label,
    label_continue: Label,
    iterator: Option<hhbc_by_ref_iterator::Id>,
}

#[derive(Clone, Debug)]
pub enum Region {
    Loop(LoopLabels),
    Switch(Label),
    TryFinally(Label),
    Finally,
    Function,
    Using(Label),
}

#[derive(Debug)]
pub struct ResolvedTryFinally {
    pub target_label: Label,
    pub finally_label: Label,
    pub adjusted_level: usize,
    pub iterators_to_release: Vec<hhbc_by_ref_iterator::Id>,
}

#[derive(Debug)]
pub enum ResolvedJumpTarget {
    NotFound,
    ResolvedTryFinally(ResolvedTryFinally),
    ResolvedRegular(Label, Vec<hhbc_by_ref_iterator::Id>),
}

#[derive(Clone, Debug, Default)]
pub struct JumpTargets(Vec<Region>);

impl JumpTargets {
    pub fn as_slice(&self) -> &[Region] {
        self.0.as_slice()
    }

    pub fn get_closest_enclosing_finally_label(
        &self,
    ) -> Option<(Label, Vec<hhbc_by_ref_iterator::Id>)> {
        let mut iters = vec![];
        for r in self.0.iter().rev() {
            match r {
                Region::Using(l) | Region::TryFinally(l) => {
                    return Some((*l, iters));
                }
                Region::Loop(LoopLabels { iterator, .. }) => {
                    add_iterator(*iterator, &mut iters);
                }
                _ => {}
            }
        }
        None
    }

    // NOTE(hrust) this corresponds to collect_iterators in OCaml but doesn't allocate/clone
    pub fn iterators(&self) -> impl Iterator<Item = &hhbc_by_ref_iterator::Id> {
        self.0.iter().rev().filter_map(|r| {
            if let Region::Loop(LoopLabels { iterator, .. }) = r {
                iterator.as_ref()
            } else {
                None
            }
        })
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
                Region::Function | Region::Finally => return ResolvedJumpTarget::NotFound,
                Region::Using(finally_label) | Region::TryFinally(finally_label) => {
                    // we need to jump out of try body in try/finally - in order to do this
                    // we should go through the finally block first
                    if skip_try_finally.is_none() {
                        skip_try_finally = Some((finally_label, level, iters.clone()));
                    }
                }
                Region::Switch(end_label) => {
                    if level == 1 {
                        label = Some(end_label);
                        iters.extend_from_slice(&std::mem::replace(&mut acc, vec![]));
                        break;
                    } else {
                        level -= 1;
                    }
                }
                Region::Loop(LoopLabels {
                    label_break,
                    label_continue,
                    iterator,
                }) => {
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
    label_id_map: std::collections::BTreeMap<IdKey, Id>,
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

    pub fn with_loop(
        &mut self,
        label_break: Label,
        label_continue: Label,
        iterator: Option<hhbc_by_ref_iterator::Id>,
    ) {
        self.jump_targets.0.push(Region::Loop(LoopLabels {
            label_break,
            label_continue,
            iterator,
        }))
    }

    pub fn with_switch(&mut self, end_label: Label) {
        //let labels = self.collect_valid_target_labels_for_switch_cases(cases);
        // CONSIDER: now HHVM eagerly reserves state id for the switch end label
        // which does not seem to be necessary - do it for now for HHVM compatibility
        let _ = self.get_id_for_label(end_label);
        self.jump_targets.0.push(Region::Switch(end_label));
    }

    pub fn with_try_catch(&mut self, finally_label: Label) {
        self.jump_targets.0.push(Region::TryFinally(finally_label));
    }

    pub fn with_try(&mut self, finally_label: Label) {
        self.jump_targets.0.push(Region::TryFinally(finally_label));
    }

    pub fn with_finally(&mut self) {
        self.jump_targets.0.push(Region::Finally);
    }

    pub fn with_function(&mut self) {
        self.jump_targets.0.push(Region::Function);
    }

    pub fn with_using(&mut self, finally_label: Label) {
        self.jump_targets.0.push(Region::Using(finally_label));
    }

    pub fn revert(&mut self) {
        self.jump_targets.0.pop();
    }

    pub fn release_ids(&mut self) {
        use Region::*;
        match self
            .jump_targets
            .0
            .last_mut()
            .expect("empty region after executing run_and_release")
        {
            Loop(LoopLabels {
                label_break,
                label_continue,
                ..
            }) => {
                self.label_id_map.remove(&IdKey::IdLabel(*label_break));
                self.label_id_map.remove(&IdKey::IdLabel(*label_continue));
            }
            Switch(l) | TryFinally(l) | Using(l) => {
                self.label_id_map.remove(&IdKey::IdLabel(*l));
            }
            Finally | Function => {}
        };
        // CONSIDER: now HHVM does not release state ids for named labels
        // even after leaving the scope where labels are accessible
        // Do the same for now for compatibility reasons
        // labels
        //     .iter()
        //     .for_each(|l| self.label_id_map.remove(&IdKey::IdLabel(Label::Named(l.to_string()))));
    }
}

fn add_iterator(
    it_opt: Option<hhbc_by_ref_iterator::Id>,
    iters: &mut Vec<hhbc_by_ref_iterator::Id>,
) {
    if let Some(it) = it_opt {
        iters.push(it);
    }
}
