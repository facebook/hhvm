(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type severity =
  | Lint_error
  | Lint_warning
  | Lint_advice

type 'pos t = {
  code: int;
  severity: severity;
  pos: 'pos; [@opaque]
  message: string;
  bypass_changed_lines: bool;
  autofix: string * string;
}
[@@deriving show]

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

val add_lint : Pos.t t -> unit

val to_absolute : Pos.t t -> Pos.absolute t

val to_string : Pos.absolute t -> string

val to_contextual_string : Pos.absolute t -> string

val to_highlighted_string : Pos.absolute t -> string

val to_json : Pos.absolute t -> Hh_json.json

val do_ : (unit -> 'a) -> Pos.t t list * 'a
