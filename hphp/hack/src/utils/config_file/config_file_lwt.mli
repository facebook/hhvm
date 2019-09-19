(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = string SMap.t

val file_path_relative_to_repo_root : string

val parse_hhconfig : string -> (string * string SMap.t, string) Lwt_result.t
