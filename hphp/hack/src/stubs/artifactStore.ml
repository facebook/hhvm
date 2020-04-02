(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type config = { temp_dir: Path.t }

type artifact =
  (* Just a plain value *)
  | Inline_artifact of string
  (* An artifact that is a directory *)
  | Path_artifact of Path.t
[@@deriving show]

let default_config ~temp_dir = { temp_dir }
