(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type pos =
  (* Normal position. *)
  | Qpos of Pos.t
  (* A quickfix might want to add things to an empty class declaration,
     which requires FFP to compute the { position. *)
  | Qclassish_start of string
[@@deriving eq, ord, show]

type t = {
  title: string;
  new_text: string;
  pos: pos;
}
[@@deriving eq, ord, show]

let make ~title ~new_text pos = { title; new_text; pos = Qpos pos }

let make_classish ~title ~new_text ~classish_name =
  { title; new_text; pos = Qclassish_start classish_name }

let get_pos ~(classish_starts : Pos.t SMap.t) (quickfix : t) : Pos.t =
  match quickfix.pos with
  | Qpos pos -> pos
  | Qclassish_start name ->
    (match SMap.find_opt name classish_starts with
    | Some pos -> pos
    | None -> Pos.none)

let get_title (quickfix : t) : string = quickfix.title

let get_new_text (quickfix : t) : string = quickfix.new_text

let sort_by_pos (quickfixes : t list) : t list =
  let pos_start_offset p = snd (Pos.info_raw p) in
  let qf_start_offset _qf = pos_start_offset Pos.none in
  let cmp_qf x y = Int.compare (qf_start_offset x) (qf_start_offset y) in
  List.sort ~compare:cmp_qf quickfixes

let replace_at (src : string) (pos : Pos.t) (new_text : string) : string =
  let (start_offset, end_offset) = Pos.info_raw pos in
  let src_before = String.subo src ~len:start_offset in
  let src_after = String.subo src ~pos:end_offset in
  src_before ^ new_text ^ src_after

(** Apply all [quickfixes] by replacing/inserting the new text in [src].
    Normally this is done by the user's editor (the LSP client), but
    this is useful for testing quickfixes. **)
let apply_all
    (src : string) (classish_starts : Pos.t SMap.t) (quickfixes : t list) :
    string =
  (* Start with the quickfix that occurs last in the file, so we
     don't affect the position of earlier code.*)
  let quickfixes = List.rev (sort_by_pos quickfixes) in

  List.fold quickfixes ~init:src ~f:(fun src qf ->
      replace_at src (get_pos ~classish_starts qf) qf.new_text)
