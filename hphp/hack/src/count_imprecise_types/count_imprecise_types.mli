(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type result

(** Counts the number of mixed, dynamic, and types occurring in functions and
    methods. *)
val count : Provider_context.t -> Tast.program -> result S_map.t

val json_of_results : result S_map.t -> Yojson.Safe.t
