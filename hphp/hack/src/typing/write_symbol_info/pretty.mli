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
   (maps symbols within types to their defining entity).
   Used to generate predicate TypeInfo *)
val hint_to_string_and_symbols :
  Provider_context.t -> Aast.hint -> string * (Pos.t * pos) list

val get_type_from_hint : Provider_context.t -> Aast.hint -> string

val get_context_from_hint : Provider_context.t -> Aast.hint -> string
