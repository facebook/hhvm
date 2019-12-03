(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type check_kind =
  | Lazy_check
  | Full_check

val check_kind_to_string : check_kind -> string

type check_results = {
  reparse_count: int;
  total_rechecked_count: int;
}

val type_check :
  ServerEnv.genv -> ServerEnv.env -> check_kind -> ServerEnv.env * check_results

(****************************************************************************)
(* Debugging: Declared here to stop ocamlc yelling at us for unused defs *)
(****************************************************************************)

val print_defs : string -> ('a * string) list -> unit

val print_fast_pos : (('a * string) list * ('b * string) list) SMap.t -> unit

val print_fast : (SSet.t * SSet.t) SMap.t -> unit
