(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type schedule_env = {
  bin_root: Path.t;
  eden_threshold: int;
  files: (Relative_path.t * FileInfo.names) list option;
  naming_sqlite_path: string option;
  naming_table: Naming_table.t option;
  num_remote_workers: int;
  root: Path.t;
  timeout: int;
  workers: MultiWorker.worker list option;
}

let go _ = failwith "not implemented"
