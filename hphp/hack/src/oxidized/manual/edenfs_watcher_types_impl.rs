// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::edenfs_watcher_types::AsyncTelemetry;
use crate::edenfs_watcher_types::TranslationTelemetry;

impl TranslationTelemetry {
    pub fn add(&mut self, other: &TranslationTelemetry) {
        self.commit_transition_count += other.commit_transition_count;
        self.commit_transition_duration += other.commit_transition_duration;
        self.directory_rename_count += other.directory_rename_count;
        self.directory_rename_duration += other.directory_rename_duration;
        self.raw_changes_count += other.raw_changes_count;
        self.translated_files_count += other.translated_files_count;
        self.duration += other.duration;
    }

    pub fn zero() -> Self {
        Self {
            commit_transition_count: 0,
            commit_transition_duration: 0,
            directory_rename_count: 0,
            directory_rename_duration: 0,
            raw_changes_count: 0,
            translated_files_count: 0,
            duration: 0,
        }
    }
}

impl AsyncTelemetry {
    pub fn worker_restarted(&self) -> Self {
        let worker_restart_count = self.worker_restart_count + 1;
        Self {
            worker_restart_count,
            ..Self::zero()
        }
    }

    pub fn worker_drained(&mut self) -> Self {
        let old = self.clone();
        let worker_restart_count = self.worker_restart_count;
        *self = Self {
            worker_restart_count,
            ..Self::zero()
        };
        old
    }

    pub fn zero() -> Self {
        Self {
            notification_count: 0,
            worker_restart_count: 0,
            aggregated_translation_telemetry: TranslationTelemetry::zero(),
        }
    }
}
