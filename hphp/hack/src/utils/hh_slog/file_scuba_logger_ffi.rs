// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

use ocamlrep_custom::Custom;
use unwrap_ocaml::UnwrapOcaml;

#[derive(Clone)]
pub struct FileScubaLoggerFfi(pub hh_slog::FileScubaLogger);

pub struct LoggerGuardFfi(file_scuba_config::LoggerGuard);

impl ocamlrep_custom::CamlSerialize for FileScubaLoggerFfi {
    ocamlrep_custom::caml_serialize_default_impls!();
}

impl ocamlrep_custom::CamlSerialize for LoggerGuardFfi {
    ocamlrep_custom::caml_serialize_default_impls!();
}

ocamlrep_ocamlpool::ocaml_ffi! {
    // Requires fbinit to have been initialized.
    fn file_scuba_logger_ffi_from_config_file(
        config_file: PathBuf
    ) -> (Custom<FileScubaLoggerFfi>, Custom<LoggerGuardFfi>) {
        // There's no way for OCaml to pass this token to Rust to prove that
        // folly has initialized. Our binaries should all have initialized
        // fb/folly.
        let fb = unsafe { fbinit::assume_init() };
        let (logger, guard) = file_scuba_config::from_config_file(fb, &config_file).unwrap_ocaml();
        (
            Custom::from(FileScubaLoggerFfi(logger)),
            Custom::from(LoggerGuardFfi(guard))
        )
    }

    fn file_scuba_logger_ffi_make_env_term() -> (Custom<FileScubaLoggerFfi>, Custom<LoggerGuardFfi>) {
        let (logger, guard) = file_scuba_config::env_term();
        (
            Custom::from(FileScubaLoggerFfi(logger)),
            Custom::from(LoggerGuardFfi(guard))
        )
    }
}
