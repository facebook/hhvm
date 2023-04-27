// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::sync::Arc;
use std::sync::Mutex;
use std::time::Instant;

use ocamlrep_custom::Custom;
use unwrap_ocaml::UnwrapOcaml;

/// Represents the progress of a naming table build.
#[derive(Clone, Default)]
struct BuildProgress(Arc<Mutex<Option<BuildResult>>>);

#[derive(Clone)]
struct BuildResult {
    pub exit_status: Result<naming_table_builder::ExitStatus, String>,
    pub time_elapsed_secs: f64,
}

impl BuildProgress {
    /// Returns `Some` if the build is complete.
    pub fn poll_result(&self) -> Option<BuildResult> {
        let guard = self.0.lock().unwrap();
        guard.clone()
    }

    pub fn set_result(&self, result: BuildResult) {
        let mut guard = self.0.lock().unwrap();
        assert!(guard.is_none());
        *guard = Some(result);
    }
}
impl ocamlrep_custom::CamlSerialize for BuildProgress {
    ocamlrep_custom::caml_serialize_default_impls!();
}

fn spawn_build(args: naming_table_builder::Args) -> BuildProgress {
    let progress = BuildProgress::default();
    let progress_in_builder_thread = progress.clone();
    let _handle = std::thread::spawn(move || {
        let build_benchmark_start = Instant::now();
        let build_result = naming_table_builder::build_naming_table(args);
        let time_elapsed_secs = build_benchmark_start.elapsed().as_secs_f64();

        progress_in_builder_thread.set_result(BuildResult {
            exit_status: build_result.map_err(|err| format!("{err:?}")),
            time_elapsed_secs,
        });
    });

    progress
    // thread is detached
}

ocamlrep_ocamlpool::ocaml_ffi! {
    fn naming_table_builder_ffi_build(www: PathBuf, custom_hhi_path: PathBuf, output: PathBuf) -> Custom<BuildProgress> {
        let progress = spawn_build(naming_table_builder::Args {
            www,
            output,
            overwrite: true,
            custom_hhi_path: Some(custom_hhi_path),
            allow_collisions: false,
            unsorted: true,
        });
        Custom::from(progress)
    }

    // Return (exit_status, time_taken_ms)
    fn naming_table_builder_ffi_poll(progress: Custom<BuildProgress>) -> Option<(i32, f64)> {
        progress.poll_result().map(
            |BuildResult {
                exit_status,
                time_elapsed_secs,
            }| (exit_status.unwrap_ocaml().as_code(), time_elapsed_secs),
        )
    }
}
