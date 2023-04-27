// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::IterId;
use hhbc::Label;

#[derive(Clone, Debug, Default, Copy, Eq, PartialEq, Hash, Ord, PartialOrd)]
pub struct StateId(pub u32);

#[derive(Clone, Debug)]
pub struct LoopLabels {
    label_break: Label,
    label_continue: Label,
    iterator: Option<IterId>,
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
    pub iterators_to_release: Vec<IterId>,
}

#[derive(Debug)]
pub enum ResolvedJumpTarget {
    NotFound,
    ResolvedTryFinally(ResolvedTryFinally),
    ResolvedRegular(Label, Vec<IterId>),
}

#[derive(Clone, Debug, Default)]
pub struct JumpTargets {
    regions: Vec<Region>,
}

impl JumpTargets {
    pub fn as_slice(&self) -> &[Region] {
        self.regions.as_slice()
    }

    pub fn get_closest_enclosing_finally_label(&self) -> Option<(Label, Vec<IterId>)> {
        let mut iters = vec![];
        for r in self.regions.iter().rev() {
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

    /// Return the IterIds of the enclosing iterator loops without allocating/cloning.
    pub fn iterators(&self) -> impl Iterator<Item = IterId> + '_ {
        self.regions.iter().rev().filter_map(|r| match r {
            Region::Loop(LoopLabels { iterator, .. }) => *iterator,
            _ => None,
        })
    }

    /// Tries to find a target label for the given jump kind (break or continue)
    pub fn get_target(&self, is_break: bool) -> ResolvedJumpTarget {
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
        for r in self.regions.iter().rev() {
            match *r {
                Region::Function | Region::Finally => return ResolvedJumpTarget::NotFound,
                Region::Using(finally_label) | Region::TryFinally(finally_label) => {
                    // we need to jump out of try body in try/finally - in order to do this
                    // we should go through the finally block first
                    if skip_try_finally.is_none() {
                        skip_try_finally = Some((finally_label, iters.clone()));
                    }
                }
                Region::Switch(end_label) => {
                    label = Some(end_label);
                    iters.append(&mut acc);
                    break;
                }
                Region::Loop(LoopLabels {
                    label_break,
                    label_continue,
                    iterator,
                }) => {
                    label = Some(if is_break {
                        add_iterator(iterator, &mut acc);
                        label_break
                    } else {
                        label_continue
                    });
                    iters.append(&mut acc);
                    break;
                }
            }
        }
        match (skip_try_finally, label) {
            (Some((finally_label, iterators_to_release)), Some(target_label)) => {
                ResolvedJumpTarget::ResolvedTryFinally(ResolvedTryFinally {
                    target_label,
                    finally_label,
                    iterators_to_release,
                })
            }
            (None, Some(l)) => ResolvedJumpTarget::ResolvedRegular(l, iters),
            (_, None) => ResolvedJumpTarget::NotFound,
        }
    }
}

#[derive(Clone, PartialEq, Eq, std::cmp::Ord, std::cmp::PartialOrd, Debug)]
pub enum IdKey {
    Return,
    Label(Label),
}

#[derive(Clone, Debug, Default)]
pub struct Gen {
    label_id_map: std::collections::BTreeMap<IdKey, StateId>,
    jump_targets: JumpTargets,
}

impl Gen {
    fn new_id(&mut self, key: IdKey) -> StateId {
        match self.label_id_map.get(&key) {
            Some(id) => *id,
            None => {
                let mut next_id = StateId(0);
                while self.label_id_map.values().any(|&id| id == next_id) {
                    next_id.0 += 1;
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

    pub fn get_id_for_return(&mut self) -> StateId {
        self.new_id(IdKey::Return)
    }

    pub fn get_id_for_label(&mut self, l: Label) -> StateId {
        self.new_id(IdKey::Label(l))
    }

    pub fn with_loop(
        &mut self,
        label_break: Label,
        label_continue: Label,
        iterator: Option<IterId>,
    ) {
        self.jump_targets.regions.push(Region::Loop(LoopLabels {
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
        self.jump_targets.regions.push(Region::Switch(end_label));
    }

    pub fn with_try_catch(&mut self, finally_label: Label) {
        self.jump_targets
            .regions
            .push(Region::TryFinally(finally_label));
    }

    pub fn with_try(&mut self, finally_label: Label) {
        self.jump_targets
            .regions
            .push(Region::TryFinally(finally_label));
    }

    pub fn with_finally(&mut self) {
        self.jump_targets.regions.push(Region::Finally);
    }

    pub fn with_function(&mut self) {
        self.jump_targets.regions.push(Region::Function);
    }

    pub fn with_using(&mut self, finally_label: Label) {
        self.jump_targets.regions.push(Region::Using(finally_label));
    }

    pub fn revert(&mut self) {
        self.jump_targets.regions.pop();
    }

    pub fn release_ids(&mut self) {
        match self
            .jump_targets
            .regions
            .last_mut()
            .expect("empty region after executing run_and_release")
        {
            Region::Loop(LoopLabels {
                label_break,
                label_continue,
                ..
            }) => {
                self.label_id_map.remove(&IdKey::Label(*label_break));
                self.label_id_map.remove(&IdKey::Label(*label_continue));
            }
            Region::Switch(l) | Region::TryFinally(l) | Region::Using(l) => {
                self.label_id_map.remove(&IdKey::Label(*l));
            }
            Region::Finally | Region::Function => {}
        };
        // CONSIDER: now HHVM does not release state ids for named labels
        // even after leaving the scope where labels are accessible
        // Do the same for now for compatibility reasons
        // labels
        //     .iter()
        //     .for_each(|l| self.label_id_map.remove(&IdKey::Label(Label::Named(l.to_string()))));
    }
}

fn add_iterator(it_opt: Option<IterId>, iters: &mut Vec<IterId>) {
    if let Some(it) = it_opt {
        iters.push(it);
    }
}
