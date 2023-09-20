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
[@@deriving ord, yojson_of]

type pos = Pos.t [@@deriving ord]

type entity_pos = pos * entity [@@deriving ord, yojson_of]

type logged_type =
  | Like
  | Tany
[@@deriving ord, yojson_of]

type category =
  | Expression
  | Property
  | Parameter
  | Return
[@@deriving ord, yojson_of]

type collected_reason = {
  entity_pos: entity_pos;
      (** The position of the entity for which this count holds *)
  counted_type: logged_type;  (** The type that this count is for *)
  category: category;  (** Program construct that produces this type *)
  reason_constructor: string;
      (** The constructor for the reason. We can't encode the actual
          reason, because it got lazy things in it and that's not serializable. *)
  reason_pos: pos;  (** Precomputed constructor string *)
}
[@@deriving yojson_of]

type t = collected_reason list Relative_path.Map.t [@@deriving yojson_of]

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
