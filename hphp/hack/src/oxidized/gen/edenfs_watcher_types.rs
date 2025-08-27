// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<4f9e1d96ae9dc097a9be882231e35789>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Settings {
    pub root: std::path::PathBuf,
    pub watch_spec: files_to_ignore::WatchSpec,
    pub debug_logging: bool,
    /// Timeout, in seconds
    pub timeout_secs: isize,
    /// Value of throttle_time_ms parameter passed to Eden's stream_changes_since API.
    /// This means that this is the minimum period (in milliseconds) between each time
    /// Eden will send us a change notification.
    pub throttle_time_ms: isize,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum Changes {
    /// List is not guaranteed to be deduplicated
    FileChanges(Vec<String>),
    CommitTransition {
        from_commit: String,
        to_commit: String,
        /// List is not guaranteed to be deduplicated
        file_changes: Vec<String>,
    },
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum EdenfsWatcherError {
    EdenfsWatcherError(String),
    /// There may have been some changes that Eden has lost track of.
    /// (e.g., due to Eden restarting).
    /// The string is just a description of what happened.
    LostChanges(String),
    /// The root directory is not inside an EdenFS mount.
    NonEdenWWW,
}
