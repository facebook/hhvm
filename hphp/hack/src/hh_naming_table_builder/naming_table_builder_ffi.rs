// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::sync::Arc;
use std::sync::Mutex;

use ocamlrep_custom::Custom;
use unwrap_ocaml::UnwrapOcaml;

/// Represents the progress of a naming table build.
#[derive(Clone, Default)]
struct BuildProgress(Arc<Mutex<Option<anyhow::Result<naming_table_builder::ExitStatus>>>>);

impl BuildProgress {
    /// Returns `Some` if the build is complete.
    pub fn poll_result(&self) -> Option<Result<naming_table_builder::ExitStatus, String>> {
        let guard = self.0.lock().unwrap();
        guard.as_ref().map(|result| match result {
            Ok(exit_status) => Ok(*exit_status),
            Err(err) => Err(format!("{err:?}")),
        })
    }

    pub fn set_result(&self, result: anyhow::Result<naming_table_builder::ExitStatus>) {
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
        let result = naming_table_builder::build_naming_table(args);
        progress_in_builder_thread.set_result(result);
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

    fn naming_table_builder_ffi_poll(progress: Custom<BuildProgress>) -> Option<i32> {
        progress.poll_result().map(|result| result.unwrap_ocaml().as_code())
    }
}
