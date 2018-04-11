(**
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

val type_check: ServerEnv.genv -> ServerEnv.env -> check_kind ->
  ServerEnv.env * int * int

(* just add also some debugging information on stdout *)
val check: ServerEnv.genv -> ServerEnv.env -> check_kind ->
  ServerEnv.env * int * int

(****************************************************************************)
(* Debugging: Declared here to stop ocamlc yelling at us for unused defs *)
(****************************************************************************)

val print_defs: string -> ('a * string) list -> unit
val print_fast_pos: (('a * string) list * ('b * string) list) SMap.t -> unit
val print_fast: (SSet.t * SSet.t) SMap.t -> unit
