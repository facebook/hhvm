(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A list of the type defs and type access we have expanded thus far. Used
      to prevent entering into a cycle when expanding these types. *)
type t

type cycle

type cycle_reporter = cycle * Typing_error.Reasons_callback.t option

module Expandable : sig
  type t =
    | Enum of string
    | Type_alias of string
    | Type_constant of {
        receiver_name: string;
        type_const_name: string;
      }
end

type expansion = {
  name: Expandable.t;
  use_pos: Pos_or_decl.t;
      (** Position of the reference to the thing to expand. *)
  def_pos: Pos_or_decl.t option;
      (** Position of the definition of the type alias or type constant. Optionally populated. *)
}

val empty : t

(** If we are expanding the RHS of a type definition, [report_cycle] contains
    the position and id of the LHS. This way, if the RHS expands at some point
    to the LHS id, we are able to report a cycle. *)
val empty_w_cycle_report : report_cycle:(Pos.t * Expandable.t) option -> t

(** [add_and_check_cycles expansions expansion] adds and [expansion] to [expansions]
  and check that that [expansion] hasn't already been done, ;
  i.e. wasn't already in [expansions]. *)
val add_and_check_cycles : t -> expansion -> (t, cycle) result

val report : cycle_reporter list -> Typing_error.t option

(** Positions of the definitions of the type aliases or type constants that were successively expanded. *)
val def_positions : t -> Pos_or_decl.t list

(** Returns true if there was an attempt to add a cycle to the expansion list. *)
val cyclic_expansion : t -> bool

(** Build a string like `report on C; A -> B::T -> C` where `A`, `B::T` and `C`
  are the successive expansions seen so far. *)
val to_log_string : t -> string
