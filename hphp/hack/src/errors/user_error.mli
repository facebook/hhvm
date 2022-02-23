(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type ('prim_pos, 'pos) t = {
  code: int;
  claim: 'prim_pos Message.t;
  reasons: 'pos Message.t list;
  quickfixes: Quickfix.t list;
  is_fixmed: bool;
}
[@@deriving eq, ord, show]

type severity =
  | Warning
  | Error

val make :
  int ->
  ?is_fixmed:bool ->
  ?quickfixes:Quickfix.t list ->
  'a Message.t ->
  'b Message.t list ->
  ('a, 'b) t

val get_code : ('a, 'b) t -> int

val get_pos : ('a, 'b) t -> 'a

val quickfixes : ('a, 'b) t -> Quickfix.t list

val get_code_severity : int -> severity

val get_severity : ('a, 'b) t -> severity

val to_list : ('a, 'a) t -> 'a Message.t list

val to_list_ : (Pos.t, Pos_or_decl.t) t -> Pos_or_decl.t Message.t list

val get_messages : ('a, 'a) t -> 'a Message.t list

val to_absolute : (Pos.t, Pos_or_decl.t) t -> (Pos.absolute, Pos.absolute) t

val make_absolute : int -> 'a Message.t list -> ('a, 'a) t

val to_absolute_for_test :
  (Pos.t, Pos_or_decl.t) t -> (Pos.absolute, Pos.absolute) t

val error_kind : int -> string

val error_code_to_string : int -> string

val to_string : bool -> (Pos.absolute, Pos.absolute) t -> string

val to_json : (Pos.absolute, Pos.absolute) t -> Hh_json.json
