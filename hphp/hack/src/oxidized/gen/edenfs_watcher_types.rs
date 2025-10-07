// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9ea3b1e5fc65ff748788d8b2e6cf8a0c>>
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct TranslationTelemetry {
    pub commit_transition_count: isize,
    pub commit_transition_duration: isize,
    pub directory_rename_count: isize,
    pub directory_rename_duration: isize,
    pub raw_changes_count: isize,
    pub translated_files_count: isize,
    pub duration: isize,
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct AsyncTelemetry {
    pub worker_restart_count: isize,
    pub notification_count: isize,
    pub aggregated_translation_telemetry: TranslationTelemetry,
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct InstanceGetChangesSyncTelemetry {
    pub duration: isize,
    pub eden_get_changes_since_duration: isize,
    pub async_telemetry: AsyncTelemetry,
    pub worker_reset_duration: isize,
    pub translation_telemetry: Option<TranslationTelemetry>,
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct InstanceGetChangesAsyncTelemetry {
    pub duration: isize,
    pub async_telemetry: AsyncTelemetry,
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct InstanceGetAllFilesTelemetry {
    pub duration: isize,
    pub sync_duration: isize,
    pub eden_glob_files_duration: isize,
    pub post_processing_duration: isize,
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct StandaloneGetChangesSinceTelemetry {
    pub duration: isize,
    pub setup_duration: isize,
    pub eden_get_changes_since_duration: isize,
    pub translation_telemetry: Option<TranslationTelemetry>,
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
    pub report_telemetry: bool,
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
