(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  bin_root: Path.t;
  input: Relative_path.t list option;
  root: Path.t;
  executable_path: string;
  lease_id: string option;
  re_worker_job_spec: string;
  num_re_workers: int;
}

let default = failwith "not implemented"

let create_tmp_file_with_timestamp ~prefix:_ ~suffix:_ =
  failwith "not implemented"

let get_www_root () = failwith "not implemented"
