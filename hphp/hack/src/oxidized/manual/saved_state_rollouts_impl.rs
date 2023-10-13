// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;

use crate::gen::saved_state_rollouts::Flag;
use crate::gen::saved_state_rollouts::FlagName;
use crate::gen::saved_state_rollouts::SavedStateRollouts;

impl Default for SavedStateRollouts {
    fn default() -> Self {
        Self::make(isize::MIN, false, None, |_| Ok(false)).expect("default had errors")
    }
}

/*
   We need to guarantee that for all flag combinations, there is an available saved
   state corresponding to that combination. There are however an exponential number
   of flag combinations.
   What follows allows to restrict the number of possible combinations per www revision
   to just two (one for the production saved state, one for the candidate saved state).

   We specify a rollout order below, and .hhconfig provides
     current_saved_state_rollout_flag_index = N
   which specifies which flag is currently being rolled out for that www revision
   using that order (current_saved_state_rollout_flag_index is an integer).
   Only that flag will get its value from JustKnob,
   while the other flags' values are determined by their order:
   flags whose order is lower than the current flag index are considered to have been already
   rolled out and therefore have there values set to true, while flags whose order is greater
   are yet to be rollout and therefore have their values set to false.
*/

impl Flag {
    /// These need to be specified manually instead of auto-numbering
    /// because we want the indices to stay consistent when we remove flags.
    pub fn rollout_order(&self) -> isize {
        match self {
            Self::DummyOne => 0,
            Self::DummyTwo => 1,
            Self::DummyThree => 2,
            Self::OptimizedMemberFanout => 4,
            Self::OptimizedParentFanout => 6,
        }
    }

    pub fn flag_name(&self) -> &'static str {
        match self {
            Self::DummyOne => "dummy_one",
            Self::DummyTwo => "dummy_two",
            Self::DummyThree => "dummy_three",
            Self::OptimizedMemberFanout => "optimized_member_fanout",
            Self::OptimizedParentFanout => "optimized_parent_fanout",
        }
    }
}

impl SavedStateRollouts {
    pub fn make(
        current_rolled_out_flag_idx: isize,
        deactivate_saved_state_rollout: bool,
        force_flag_value: Option<&str>,
        mut get_default: impl FnMut(&str) -> Result<bool, std::str::ParseBoolError>,
    ) -> Result<Self, RolloutsError> {
        use std::cmp::Ordering;
        let force_prod_or_candidate = ForcedFlags::parse(force_flag_value)?;
        let mut get_flag_value = |flag: Flag| {
            let i = flag.rollout_order();
            let flag_name = flag.flag_name();
            match current_rolled_out_flag_idx.cmp(&i) {
                Ordering::Equal => match force_prod_or_candidate.rollout_flag_value() {
                    Some(b) => Ok(b),
                    None => {
                        if deactivate_saved_state_rollout {
                            Ok(false)
                        } else {
                            get_default(flag_name)
                        }
                    }
                },
                Ordering::Less => {
                    // This flag will be rolled out next, set to false unless forced to true.
                    Ok(force_prod_or_candidate.is_forced(flag_name))
                }
                Ordering::Greater => {
                    // This flag has already been rolled out
                    Ok(true)
                }
            }
        };
        Ok(Self {
            dummy_one: get_flag_value(Flag::DummyOne)?,
            dummy_two: get_flag_value(Flag::DummyTwo)?,
            dummy_three: get_flag_value(Flag::DummyThree)?,
            optimized_member_fanout: get_flag_value(Flag::OptimizedMemberFanout)?,
            optimized_parent_fanout: get_flag_value(Flag::OptimizedParentFanout)?,
        })
    }
}

/// This enum handles the config flag 'ss_force', which can have the following values:
///   * 'prod' or 'production'
///   * 'candidate'
///   * 'prod_with_flag_on:my_flag_name' or 'production_with_flag_on:my_flag_name'
///      where 'my_flag_name' is a saved state flag from this module.
///
/// This is useful to force using (or creating) the production or candidate
/// saved state, or for using/creating an alternative candidate saved state for testing
/// (with 'prod_with_flag_on:my_flag_name')
enum ForcedFlags {
    Prod(Option<FlagName>),
    Candidate,
    None,
}

impl ForcedFlags {
    /// Parse value of config flag 'ss_force'
    fn parse(force_flag_value: Option<&str>) -> Result<Self, RolloutsError> {
        match force_flag_value {
            None => Ok(Self::None),
            Some("production" | "prod") => Ok(Self::Prod(None)),
            Some("candidate") => Ok(Self::Candidate),
            Some(forced) => match &forced.split(':').collect::<Vec<_>>()[..] {
                ["prod_with_flag_on" | "production_with_flag_on", forced_flag] => {
                    Ok(Self::Prod(Some(forced_flag.to_string())))
                }
                _ => Err(RolloutsError::ForceFlagError(forced.into())),
            },
        }
    }

    /// Return the forced value of the current rollout flag, if any.
    /// Returning None means there is no forcing.
    fn rollout_flag_value(&self) -> Option<bool> {
        match self {
            Self::Prod(_) => Some(false),
            Self::Candidate => Some(true),
            Self::None => None,
        }
    }

    /// Whether a specific flag is forced to be on.
    fn is_forced(&self, flag_name: &str) -> bool {
        match self {
            Self::Prod(Some(forced)) => forced == flag_name,
            Self::Prod(None) | Self::Candidate | Self::None => false,
        }
    }
}

#[derive(Debug, thiserror::Error)]
pub enum RolloutsError {
    #[error("Error parsing flag: {0}")]
    FlagError(#[from] std::str::ParseBoolError),

    #[error("Invalid value for flag ss_force: {0}")]
    ForceFlagError(String),
}
