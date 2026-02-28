(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type 'pos hint_style =
  | HintStyleSilent of 'pos
  | HintStyleHint of 'pos
[@@deriving eq, ord, show]

type 'pos edits =
  | Eager of (string * 'pos) list
  | Classish_end of {
      classish_end_new_text: string;
      classish_end_name: string;
    }
      (** A quickfix might want to add things to an empty class declaration,
      which requires FFP to compute the { position. *)
  | Add_function_attribute of {
      function_pos: 'pos;
      attribute_name: string;
          (** Ideally should be a string from naming_special_names.ml .
          * Restriction: only resolves to a quickfix when the function_pos is in the same file as the error
          *)
    }
[@@deriving eq, ord, show]

type 'pos t = {
  title: string;
  edits: 'pos edits;
  hint_styles: 'pos Classish_positions_types.pos hint_style list;
}
[@@deriving eq, ord, show]

let make ~title ~edits ~hint_styles = { title; edits; hint_styles }

let make_eager ~title ~new_text ~hint_styles pos =
  { title; edits = Eager [(new_text, pos)]; hint_styles }

let make_eager_default_hint_style ~title ~new_text pos =
  make_eager ~title ~new_text ~hint_styles:[] pos

let map_positions : 'a t -> f:('a -> 'b) -> 'b t =
 fun { title; edits; hint_styles } ~f ->
  let transform_edits = function
    | Eager edits ->
      let edits = List.map edits ~f:(fun (s, p) -> (s, f p)) in
      Eager edits
    | Classish_end fields -> Classish_end fields
    | Add_function_attribute { function_pos; attribute_name } ->
      Add_function_attribute { function_pos = f function_pos; attribute_name }
  in
  let edits = transform_edits edits in

  let transform_hint_style = function
    | HintStyleSilent p -> HintStyleSilent (Classish_positions.map_pos ~f p)
    | HintStyleHint p -> HintStyleHint (Classish_positions.map_pos ~f p)
  in
  let hint_styles = List.map hint_styles ~f:transform_hint_style in
  { title; edits; hint_styles }

let to_absolute : Pos.t t -> Pos.absolute t = map_positions ~f:Pos.to_absolute

let get_title (quickfix : 'a t) : string = quickfix.title

let get_edits (quickfix : Pos.t t) : Pos.t edits = quickfix.edits

let get_hint_styles (quickfix : 'p t) :
    'p Classish_positions.pos hint_style list =
  quickfix.hint_styles
