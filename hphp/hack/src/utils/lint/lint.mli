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

type 'a t [@@deriving show]

val get_code : 'a t -> int

val get_pos : 'a t -> 'a Pos.pos

val add :
  ?bypass_changed_lines:bool ->
  ?autofix:string * string ->
  int ->
  severity ->
  Pos.t ->
  string ->
  unit

val to_absolute : Relative_path.t t -> string t

val to_string : string t -> string

val to_contextual_string : string t -> string

val to_json : string t -> Hh_json.json

val internal_error : Pos.t -> string -> unit

val lowercase_constant : Pos.t -> string -> unit

val mk_lowercase_constant : Pos.t -> string -> Relative_path.t t

val use_collection_literal : Pos.t -> string -> unit

val static_string : ?no_consts:bool -> Pos.t -> unit

val shape_idx_access_required_field : Pos.t -> string -> unit

val do_ : (unit -> 'a) -> Relative_path.t t list * 'a

val add_lint : Relative_path.t t -> unit
