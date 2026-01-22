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
  | Add_function_attribute of {
      function_pos: 'pos;
      attribute_name: string;
          (** Ideally should be a string from naming_special_names.ml *)
    }
      (** Add an attribute to the end of the attribute list for the current function. *)
[@@deriving eq, ord, show]

(** How should we indicate to the user that a quickfix is available,
additional to the primary error red-squiggle? *)
type 'pos hint_style =
  | HintStyleSilent of 'pos
      (** Make the quickfix available for the given position, but don't provide
      any visual indication. *)
  | HintStyleHint of 'pos
      (** Use the a non-error/non-warning IDE visual clue that a quickfix is
      available for the given position. Example: https://pxl.cl/4x0ZS *)
[@@deriving eq, ord, show]

val make :
  title:string ->
  edits:Pos.t edits ->
  hint_styles:Pos.t Classish_positions.pos hint_style list ->
  Pos.t t

(** Helper for the Quickfix.Eager constructor  *)
val make_eager :
  title:string ->
  new_text:string ->
  hint_styles:Pos.t Classish_positions.pos hint_style list ->
  Pos.t ->
  Pos.t t

(** Helper for [make_eager] to create a quickfix with no additional hint styles] *)
val make_eager_default_hint_style :
  title:string -> new_text:string -> Pos.t -> Pos.t t

val get_title : Pos.t t -> string

val get_edits : Pos.t t -> Pos.t edits

val get_hint_styles : 'pos t -> 'pos Classish_positions.pos hint_style list

val to_absolute : Pos.t t -> Pos.absolute t
