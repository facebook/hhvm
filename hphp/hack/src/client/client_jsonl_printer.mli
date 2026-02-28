(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Prints JSON Lines (one JSON object per line) to stdout for [hh_client --jsonl].
    Used by both the streaming ([go_streaming]) and non-streaming ([go]) paths
    in {!ClientCheckStatus}. Each line has a ["kind"] field: ["diagnostic"],
    ["summary"], ["restarted"], or ["stopped"]. *)

val print_diagnostic :
  error_format:Diagnostics.format -> Diagnostics.finalized_diagnostic -> unit

val print_summary : passed:bool -> error_count:int -> warning_count:int -> unit

val print_restarted : message:string -> unit

val print_stopped : message:string -> unit
