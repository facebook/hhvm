(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Something like `worker_process_%d_out_of_%d_for_server_pid_%d` *)
val worker_name_fmt : (int -> int -> int -> string, unit, string) format

(** Tests that `Sys.argv.(1)` matches the worker
    process name format defined by `worker_name_fmt` *)
val is_worker_process : unit -> bool
