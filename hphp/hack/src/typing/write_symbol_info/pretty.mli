(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Pretty-printers for hints and contexts. *)

type pos = {
  start: int;
  length: int;
}
[@@deriving ord]

(* Pretty-printer for hints. Also generate xrefs
   (maps symbols within types to their defining entity). *)
val hint_to_string_and_symbols :
  is_ctx:bool -> Aast.hint -> string * (Pos.t * pos) list

val hint_to_string : is_ctx:bool -> Aast.hint -> string

(* Pretty-printer for expressions. Fetch representation from the source.
   Strip enclosing double/single quotes if any.
   TODO: 1. replace with Hint_print.pp_expr. 2. why stripping? *)
val expr_to_string : Full_fidelity_source_text.t -> ('a, 'b) Aast.expr -> string

(* Convert ContainerName<TParam> to ContainerName - see D21428855 *)
val strip_tparams : string -> string
