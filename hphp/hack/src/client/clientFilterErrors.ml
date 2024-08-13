(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Code = Error_codes.Warning

type switch =
  | WAll
  | WNone
  | Code_on of Code.t
  | Code_off of Code.t
  | Ignored_files of Str.regexp
  | Generated_files_on

module Warning_set : sig
  type t

  val empty : t

  val all : t

  val add : Code.t -> t -> t

  val remove : Code.t -> t -> t

  val mem : Code.t -> t -> bool
end = struct
  module Set = Error_codes.Warning_set

  type t =
    | All_except of Set.t
    | None_except of Set.t

  let empty = None_except Set.empty

  let all = All_except Set.empty

  let add code (t : t) : t =
    match t with
    | All_except set -> All_except (Set.remove code set)
    | None_except set -> None_except (Set.add code set)

  let remove code t =
    match t with
    | All_except set -> All_except (Set.add code set)
    | None_except set -> None_except (Set.remove code set)

  let mem code t =
    match t with
    | All_except set -> not (Set.mem code set)
    | None_except set -> Set.mem code set
end

type path = string

type code = int

module Filter : sig
  type t

  val make :
    default_all:bool -> generated_files:Str.regexp list -> switch list -> t

  val pass : t -> code * path -> bool
end = struct
  type t = {
    codes: Warning_set.t;
    ignore_files: Str.regexp list;
    generated_files: Str.regexp list;
  }

  let empty =
    { codes = Warning_set.empty; ignore_files = []; generated_files = [] }

  let add_code code filter =
    { filter with codes = Warning_set.add code filter.codes }

  let remove_code code filter =
    { filter with codes = Warning_set.remove code filter.codes }

  let with_all_codes filter = { filter with codes = Warning_set.all }

  let with_no_codes filter = { filter with codes = Warning_set.empty }

  let add_ignored_files re filter =
    { filter with ignore_files = re :: filter.ignore_files }

  let apply_switch filter switch =
    match switch with
    | WAll -> with_all_codes filter
    | WNone -> with_no_codes filter
    | Code_on code -> add_code code filter
    | Code_off code -> remove_code code filter
    | Ignored_files re -> add_ignored_files re filter
    | Generated_files_on -> { filter with generated_files = [] }

  let make ~default_all ~generated_files switches =
    let switches =
      if default_all then
        WAll :: switches
      else
        WNone :: switches
    in
    (* TODO: consider reordering switches to put all the -Wall and -Wnone first *)
    List.fold switches ~init:{ empty with generated_files } ~f:apply_switch

  let pass_code codes code =
    match Code.of_enum code with
    | None -> (* This is not a warning, so don't filter out *) true
    | Some code -> Warning_set.mem code codes

  (* Silence warning about Not_found below being deprecated. *)
  [@@@ocaml.warning "-3"]

  let match_regexp (path : path) (regexp : Str.regexp) =
    try
      let _i = Str.search_forward regexp path 0 in
      true
    with
    | Not_found
    | Not_found_s _ ->
      false

  [@@@ocaml.warning "+3"]

  let pass_path ignore_files file_path =
    not (List.exists ignore_files ~f:(match_regexp file_path))

  let pass { codes; ignore_files; generated_files } (code, file_path) =
    pass_code codes code && pass_path (ignore_files @ generated_files) file_path
end

let filter filter (error_list : Errors.finalized_error list) =
  List.filter error_list ~f:(fun error ->
      Filter.pass
        filter
        (User_error.get_code error, User_error.get_pos error |> Pos.filename))
