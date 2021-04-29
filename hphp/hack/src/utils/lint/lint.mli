(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type severity =
  | Lint_error
  | Lint_warning
  | Lint_advice

type 'pos t [@@deriving show]

val get_code : 'pos t -> int

val get_pos : 'pos t -> 'pos

val add :
  ?bypass_changed_lines:bool ->
  ?autofix:string * string ->
  int ->
  severity ->
  Pos.t ->
  string ->
  unit

val to_absolute : Pos.t t -> Pos.absolute t

val to_string : Pos.absolute t -> string

val to_contextual_string : Pos.absolute t -> string

val to_highlighted_string : Pos.absolute t -> string

val to_json : Pos.absolute t -> Hh_json.json

val internal_error : Pos.t -> string -> unit

val lowercase_constant : Pos.t -> string -> unit

val mk_lowercase_constant : Pos.t -> string -> Pos.t t

val use_collection_literal : Pos.t -> string -> unit

val static_string : ?no_consts:bool -> Pos.t -> unit

val shape_idx_access_required_field : Pos.t -> string -> unit

val opt_closed_shape_idx_missing_field : string option -> Pos.t -> unit

val do_ : (unit -> 'a) -> Pos.t t list * 'a

val add_lint : Pos.t t -> unit
