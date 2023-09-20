(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type entity =
  | Class of string
  | Function of string
[@@deriving ord]

type entity_pos = Pos.t * entity [@@deriving ord]

type logged_type =
  | Like
  | NonLike
  | Mixed
  | SupportdynOfMixed
  | Dynamic
  | Tany
[@@deriving ord]

type category =
  | Expression
  | Obj_get_receiver
  | Class_get_receiver
  | Class_const_receiver
  | Property
  | Parameter
  | Return
[@@deriving ord]

type count = {
  entity_pos: entity_pos;
      (** The position of the entity for which this count holds *)
  counted_type: logged_type;  (** The type that this count is for *)
  category: category;  (** Program construct that produces this type *)
  value: int;  (** The actual count *)
}
[@@deriving yojson_of]

(** Summary for one file, only counting like types. **)
type summary = {
  num_like_types: int;
  num_non_like_types: int;
  num_mixed: int;
  num_supportdyn_of_mixed: int;
  num_dynamic: int;
  num_tany: int;
}
[@@deriving yojson_of]

type t = summary Relative_path.Map.t [@@deriving yojson_of]

val is_enabled : TypecheckerOptions.t -> bool

val map :
  Provider_context.t -> Relative_path.t -> Tast.by_names -> Errors.t -> t

val reduce : t -> t -> t

val finalize :
  progress:(string -> unit) ->
  init_id:string ->
  recheck_id:string option ->
  t ->
  unit
