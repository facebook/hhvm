(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let get_project_metadata
    ~progress_callback:_
    ~saved_state_type:_
    ~repo:_
    ~ignore_hh_version:_
    ~opts:_ =
  failwith "Not implemented"

let load
    ~ssopt:_
    ~progress_callback:_
    ~watchman_opts:_
    ~ignore_hh_version:_
    ~saved_state_type:_ =
  failwith "Not implemented"

let load_internal
    ~ssopt:_
    ~progress_callback:_
    ~watchman_opts:_
    ~ignore_hh_version:_
    ~saved_state_type:_ =
  failwith "Not implemented"

let prepare_download_dir ~saved_state_type:_ = failwith "Not implemented"

let get_saved_state_target_path ~download_dir:_ ~manifold_path:_ =
  failwith "Not implemented"

let download_and_unpack_saved_state_from_manifold
    ~ssopt:_
    ~progress_callback:_
    ~manifold_path:_
    ~target_path:_
    ~saved_state_type:_ =
  failwith "Not implemented"
