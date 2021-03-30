(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Pos.t [@@deriving eq, ord, show]

module Map = Pos.Map

let none : t = Pos.none

let btw = Pos.btw

let get_raw_pos : t -> Pos.t option = (fun p -> Some p)

let of_raw_pos : Pos.t -> t = (fun p -> p)

let make_decl_pos : Pos.t -> Decl_reference.t -> t =
 (fun p _decl -> (* TODO *) of_raw_pos p)

let make_decl_pos_of_option : Pos.t -> Decl_reference.t option -> t =
 (fun p _decl -> (* TODO *) of_raw_pos p)

let is_hhi : t -> bool =
 fun p ->
  match get_raw_pos p with
  | None -> (* TODO T81321312 *) false
  | Some p -> Pos.is_hhi p

let set_from_reason : t -> t = Pos.set_from_reason

let unsafe_to_raw_pos : t -> Pos.t = (fun p -> p)

let line_start_end_columns : t -> int * int * int = Pos.info_pos

let json : t -> Hh_json.json = (fun p -> p |> Pos.to_absolute |> Pos.json)

let show_as_absolute_file_line_characters : t -> string =
 (fun p -> p |> Pos.to_absolute |> Pos.string)

let resolve : Relative_path.t -> t -> Pos.t = (fun _filename p -> p)

let assert_is_in_current_decl :
    t ->
    current_decl:Decl_reference.t option ->
    current_file:Relative_path.t ->
    Pos.t option =
 fun p ~current_decl:_ ~current_file ->
  (* TODO use current_decl *)
  if Relative_path.equal (Pos.filename p) current_file then
    Some p
  else
    None
