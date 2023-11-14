// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::sync::Arc;
use std::sync::Mutex;
use std::time::Instant;

use ocamlrep_custom::Custom;
use oxidized::file_info::SiAddendum;
use relative_path::RelativePath;
use unwrap_ocaml::UnwrapOcaml;

/// A future representing a naming table build. Like Rust's standard future, if
/// this is polled and is completed, then it shouldn't be polled again.
#[derive(Clone, Default)]
struct BuildProgress(Arc<Mutex<Option<BuildResultWithTelemetry>>>);

struct BuildResultWithTelemetry {
    pub result: anyhow::Result<BuildResult>,
    pub time_elapsed_secs: f64,
}

struct BuildResult {
    pub exit_status: naming_table_builder::ExitStatus,
    pub si_addenda: Vec<(RelativePath, Vec<SiAddendum>)>,
}

impl BuildProgress {
    pub fn poll_and_take(&self) -> Option<BuildResultWithTelemetry> {
        let mut guard = self.0.lock().unwrap();
        guard.take()
    }

    pub fn set(&self, result: BuildResultWithTelemetry) {
        let mut guard = self.0.lock().unwrap();
        assert!(guard.is_none());
        *guard = Some(result);
    }
}
impl ocamlrep_custom::CamlSerialize for BuildProgress {
    ocamlrep_custom::caml_serialize_default_impls!();
}

fn spawn_build(www: PathBuf, hhi_path: PathBuf, output: PathBuf) -> BuildProgress {
    let progress = BuildProgress::default();
    let progress_in_builder_thread = progress.clone();
    let _handle = std::thread::spawn(move || {
        let build_benchmark_start = Instant::now();

        // TODO: an atomic rename would probably be more robust here.
        if output.exists() {
            if let Err(err) = std::fs::remove_file(&output) {
                progress_in_builder_thread.set(BuildResultWithTelemetry {
                    result: Err(anyhow::anyhow!(err)),
                    time_elapsed_secs: build_benchmark_start.elapsed().as_secs_f64(),
                });
                return;
            }
        }

        let result = naming_table_builder::build_naming_table_ide(&www, &hhi_path, &output).map(
            |(exit_status, si_addenda)| BuildResult {
                exit_status,
                si_addenda,
            },
        );
        let time_elapsed_secs = build_benchmark_start.elapsed().as_secs_f64();

        progress_in_builder_thread.set(BuildResultWithTelemetry {
            result,
            time_elapsed_secs,
        });
    });

    progress
    // thread is detached
}

ocamlrep_ocamlpool::ocaml_ffi! {
    fn naming_table_builder_ffi_build(www: PathBuf, custom_hhi_path: PathBuf, output: PathBuf) -> Custom<BuildProgress> {
        let progress = spawn_build(www, custom_hhi_path, output);
        Custom::from(progress)
    }

    // Returns (exit_status, si_addenda, time_taken_ms)
    fn naming_table_builder_ffi_poll(progress: Custom<BuildProgress>) -> Option<(i32, Vec<(RelativePath, Vec<SiAddendum>)>, f64)> {
        progress.poll_and_take().map(
            |BuildResultWithTelemetry {
                result,
                time_elapsed_secs,
            }| {
                let result = result.unwrap_ocaml();
                (result.exit_status.as_code(), result.si_addenda, time_elapsed_secs)
            }
        )
    }
}
