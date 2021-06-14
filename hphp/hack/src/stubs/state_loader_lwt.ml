(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let load ~watchman_opts:_ ~ignore_hh_version:_ ~saved_state_type:_ =
  failwith "Not implemented"

let load_internal ~watchman_opts:_ ~ignore_hh_version:_ ~saved_state_type:_ =
  failwith "Not implemented"

let prepare_download_dir ~saved_state_type:_ = failwith "Not implemented"

let get_saved_state_target_path ~download_dir:_ ~manifold_path:_ =
  failwith "Not implemented"

let download_and_unpack_saved_state_from_manifold
    ~manifold_path:_ ~target_path:_ ~saved_state_type:_ =
  failwith "Not implemented"
