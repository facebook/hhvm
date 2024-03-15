(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type 'pos t [@@deriving eq, ord, show]

type 'pos edits =
  | Eager of (string * 'pos) list
      (** Make a quickfix when all the information about
  * edits is already available and does not need to be calculated.
  *)
  | Classish_end of {
      classish_end_new_text: string;
      classish_end_name: string;
    }
      (** A quickfix might want to add things to an empty class declaration,
      which requires FFP to compute the { position. *)
[@@deriving eq, ord, show]

val make : title:string -> edits:Pos.t edits -> Pos.t t

(** Helper for the Quickfix.Eager constructor  *)
val make_eager : title:string -> new_text:string -> Pos.t -> Pos.t t

val get_title : Pos.t t -> string

val get_edits : Pos.t t -> Pos.t edits

val to_absolute : Pos.t t -> Pos.absolute t
