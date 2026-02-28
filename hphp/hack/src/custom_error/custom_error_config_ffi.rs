// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use oxidized::custom_error_config::CustomErrorConfig;

ocamlrep_ocamlpool::ocaml_ffi! {

fn initialize_custom_error_config(path: String) -> Result<CustomErrorConfig, String> {
    CustomErrorConfig::from_path(Path::new(&path)).map_err(|e| format!("{}", e))
}

}
