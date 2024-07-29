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

module Expansion : sig
  type t =
    | Enum of string
    | Type_alias of string
    | Type_constant of {
        receiver_name: string;
        type_const_name: string;
      }
end

val empty : t

(** If we are expanding the RHS of a type definition, [report_cycle] contains
      the position and id of the LHS. This way, if the RHS expands at some point
      to the LHS id, we are able to report a cycle. *)
val empty_w_cycle_report : report_cycle:(Pos.t * Expansion.t) option -> t

(** Returns:
    - [None] if there was no cycle
    - [Some None] if there was a cycle which did not involve the first
      type expansion, i.e. error reporting should be done elsewhere
    - [Some (Some pos)] if there was a cycle involving the first type
      expansion in which case an error should be reported at [pos]. *)
val add_and_check_cycles :
  t -> Pos_or_decl.t * Expansion.t -> t * Pos.t option option

(** The list of expanded type aliases or type constants as string,
      in the order they have been expanded. Useful for error reporting. *)
val to_string_list : t -> string list

val positions : t -> Pos_or_decl.t list

(** Returns true if there was an attempt to add a cycle to the expansion list. *)
val cyclic_expansion : t -> bool
