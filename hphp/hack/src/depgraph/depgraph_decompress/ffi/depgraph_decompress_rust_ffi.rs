// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use anyhow::Result;
use ocamlrep_ocamlpool::ocaml_ffi;

const DECOMPRESSED_DG_FILE_NAME: &str = "hh_mini_saved_state_64bit_dep_graph_decompressed.hhdg";

ocaml_ffi! {
    fn depgraph_decompress_ffi(compressed_dg_path: String) -> Result<String, String> {
        let compressed_dg_path = Path::new(&compressed_dg_path);
        let mut decompressed_dg_path = compressed_dg_path.to_path_buf();
        decompressed_dg_path.set_file_name(DECOMPRESSED_DG_FILE_NAME);

        if decompressed_dg_path.exists() {
            return Ok(decompressed_dg_path.display().to_string());
        }

        decompress::decompress(compressed_dg_path, &decompressed_dg_path)
            .map_err(|e| format!("{e:?}"))?;
        let decompressed_dg_path = decompressed_dg_path.as_path().to_str()
            .ok_or_else(|| "Failed to convert path to string".to_string())?
            .to_string();
        Ok(decompressed_dg_path)
    }
}
