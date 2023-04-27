(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type 'pos qf_pos =
  (* Normal position. *)
  | Qpos of 'pos
  (* A quickfix might want to add things to an empty class declaration,
     which requires FFP to compute the { position. *)
  | Qclassish_start of string
[@@deriving eq, ord, show]

type 'pos edit = string * 'pos qf_pos [@@deriving eq, ord, show]

type 'pos t = {
  title: string;
  edits: 'pos edit list;
}
[@@deriving eq, ord, show]

let to_absolute { title; edits } =
  let edits =
    List.map edits ~f:(fun (s, qfp) ->
        ( s,
          match qfp with
          | Qpos pos -> Qpos (pos |> Pos.to_absolute)
          | Qclassish_start s -> Qclassish_start s ))
  in
  { title; edits }

let make ~title ~new_text pos = { title; edits = [(new_text, Qpos pos)] }

let make_with_edits ~title ~edits =
  {
    title;
    edits = List.map edits ~f:(fun (new_text, pos) -> (new_text, Qpos pos));
  }

let make_classish ~title ~new_text ~classish_name =
  { title; edits = [(new_text, Qclassish_start classish_name)] }

let of_qf_pos ~(classish_starts : Pos.t SMap.t) (p : Pos.t qf_pos) : Pos.t =
  match p with
  | Qpos pos -> pos
  | Qclassish_start name ->
    (match SMap.find_opt name classish_starts with
    | Some pos -> pos
    | None -> Pos.none)

let get_title (quickfix : 'a t) : string = quickfix.title

let get_edits ~(classish_starts : Pos.t SMap.t) (quickfix : Pos.t t) :
    (string * Pos.t) list =
  List.map quickfix.edits ~f:(fun (new_text, qfp) ->
      (new_text, of_qf_pos ~classish_starts qfp))

(* Sort [quickfixes] with their edit positions in descending
   order. This allows us to iteratively apply the quickfixes without
   messing up positions earlier in the file.*)
let sort_for_application
    (classish_starts : Pos.t SMap.t) (quickfixes : Pos.t t list) : Pos.t t list
    =
  let first_qf_offset (quickfix : Pos.t t) : int =
    let pos =
      match List.hd quickfix.edits with
      | Some (_, qfp) -> of_qf_pos ~classish_starts qfp
      | _ -> Pos.none
    in
    snd (Pos.info_raw pos)
  in
  let compare x y = Int.compare (first_qf_offset x) (first_qf_offset y) in
  List.rev (List.sort ~compare quickfixes)

let sort_edits_for_application
    (classish_starts : Pos.t SMap.t) (edits : Pos.t edit list) : Pos.t edit list
    =
  let offset (_, qfp) =
    let pos = of_qf_pos ~classish_starts qfp in
    snd (Pos.info_raw pos)
  in
  let compare x y = Int.compare (offset x) (offset y) in
  List.rev (List.sort ~compare edits)

(* Apply [edit] to [src], replacing the text at the position specified. *)
let apply_edit
    (classish_starts : Pos.t SMap.t) (src : string) (edit : Pos.t edit) : string
    =
  let (new_text, p) = edit in
  let pos = of_qf_pos ~classish_starts p in
  if Pos.equal pos Pos.none then
    src
  else
    let (start_offset, end_offset) = Pos.info_raw pos in
    let src_before = String.subo src ~len:start_offset in
    let src_after = String.subo src ~pos:end_offset in
    src_before ^ new_text ^ src_after

let apply_quickfix
    (classish_starts : Pos.t SMap.t) (src : string) (quickfix : Pos.t t) :
    string =
  List.fold
    (sort_edits_for_application classish_starts quickfix.edits)
    ~init:src
    ~f:(apply_edit classish_starts)

(** Apply all [quickfixes] by replacing/inserting the new text in [src].
    Normally this is done by the user's editor (the LSP client), but
    this is useful for testing quickfixes. **)
let apply_all
    (src : string) (classish_starts : Pos.t SMap.t) (quickfixes : Pos.t t list)
    : string =
  List.fold
    (sort_for_application classish_starts quickfixes)
    ~init:src
    ~f:(apply_quickfix classish_starts)
