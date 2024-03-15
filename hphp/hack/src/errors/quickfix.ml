(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type 'pos edits =
  | Eager of (string * 'pos) list
  | Classish_end of {
      classish_end_new_text: string;
      classish_end_name: string;
    }
      (** A quickfix might want to add things to an empty class declaration,
      which requires FFP to compute the { position. *)
[@@deriving eq, ord, show]

type 'pos t = {
  title: string;
  edits: 'pos edits;
}
[@@deriving eq, ord, show]

let make ~title ~edits = { title; edits }

let make_eager ~title ~new_text pos = { title; edits = Eager [(new_text, pos)] }

let map_positions : 'a t -> f:('a -> 'b) -> 'b t =
 fun { title; edits } ~f ->
  let transform_edits = function
    | Eager edits ->
      let edits = List.map edits ~f:(fun (s, p) -> (s, f p)) in
      Eager edits
    | Classish_end fields -> Classish_end fields
  in
  let edits = transform_edits edits in
  { title; edits }

let to_absolute : Pos.t t -> Pos.absolute t = map_positions ~f:Pos.to_absolute

let get_title (quickfix : 'a t) : string = quickfix.title

let get_edits (quickfix : Pos.t t) : Pos.t edits = quickfix.edits
