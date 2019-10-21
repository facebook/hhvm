(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type type_check_input = Relative_path.t list

type work_env = {
  bin_root: Path.t;
  check_id: string;
  key: string;
  root: Path.t;
  timeout: int;
  type_check: (type_check_input -> Errors.t) option;
}

let go _ = failwith "not implemented"
