(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let get_project_metadata ~repo:_ ~ignore_hh_version:_ ~opts:_ =
  Lwt.return (Error ("Not implemented", Telemetry.create ()))

let load ~ssopt:_ ~progress_callback:_ ~watchman_opts:_ ~ignore_hh_version:_ =
  Lwt.return (Error "Not implemented")

let load_internal
    ~ssopt:_
    ~progress_callback:_
    ~watchman_opts:_
    ~ignore_hh_version:_
    ~saved_state_type:_ =
  failwith "Not implemented"

let prepare_download_dir () = failwith "Not implemented"

let get_saved_state_target_path ~download_dir:_ ~manifold_path:_ =
  failwith "Not implemented"

let download_and_unpack_saved_state_from_manifold
    ~ssopt:_ ~progress_callback:_ ~manifold_path:_ ~target_path:_ =
  failwith "Not implemented"

module FromDisk = struct
  type load_result = {
    naming_table_path: Path.t;
    warning_saved_state_path: Path.t;
    files_changed: Saved_state_loader.changed_files;
  }

  let load ~project_metadata:_ ~threshold:_ ~root:_ = Error "Not implemented"
end
