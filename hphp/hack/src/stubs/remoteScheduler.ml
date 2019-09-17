(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type schedule_env = {
  bin_root: Path.t;
  eden_threshold: int option;
  files: (Relative_path.t * FileInfo.names) list option;
  naming_sqlite_path: string option;
  naming_table: Naming_table.t option;
  num_remote_workers: int;
  root: Path.t;
  timeout: int;
  version_specifier: string option;
  workers: MultiWorker.worker list option;
}

let default_env ~bin_root ~root =
  ignore (bin_root, root);
  failwith "not implemented"

let set_send_progress _ : unit = failwith "not implemented"

let set_send_percentage_progress _ : unit = failwith "not implemented"

let go _ = failwith "not implemented"
