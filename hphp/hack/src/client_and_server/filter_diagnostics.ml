(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Code = Error_codes.Warning
module Hashtbl = Stdlib.Hashtbl

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
    filter_out_generated_files: bool;
  }

  let empty =
    {
      codes = Warning_set.empty;
      ignore_files = [];
      generated_files = [];
      filter_out_generated_files = true;
    }

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
    | Generated_files_on ->
      { filter with generated_files = []; filter_out_generated_files = false }

  let make ~default_all ~generated_files switches =
    let switches =
      if default_all then
        WAll :: switches
      else
        WNone :: switches
    in
    (* TODO: consider reordering switches to put all the -Wall and -Wnone first *)
    List.fold switches ~init:{ empty with generated_files } ~f:apply_switch

  let pass_code codes code = Warning_set.mem code codes

  (* suppress warning about Not_found below being deprecated. *)
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

  module IsNonGenerated = struct
    let is_generated_cache : (path, bool) Hashtbl.t = Hashtbl.create 1000

    let at_generated =
      Re.str
        ((* Writing like this to prevent CI from thinking this that file is generated...*)
         "@"
        ^ "generated")
      |> Re.compile

    let max_lines = 50

    let is_generated path =
      try
        Stdlib.In_channel.with_open_text path @@ fun ic ->
        let rec go i =
          if i >= max_lines then
            false
          else
            match Stdlib.In_channel.input_line ic with
            | None -> false
            | Some line -> Re.execp at_generated line || go (i + 1)
        in
        go 0
      with
      | Sys_error _ ->
        (* File not found. This could be the case for some tests. *) false

    let pass ~skip path =
      if skip then
        true
      else
        match Hashtbl.find_opt is_generated_cache path with
        | Some is_generated -> not is_generated
        | None ->
          let is_generated = is_generated path in
          Hashtbl.add is_generated_cache path is_generated;
          not is_generated
  end

  let pass
      { codes; ignore_files; generated_files; filter_out_generated_files }
      (code, file_path) =
    match Error_codes.Warning.of_enum code with
    | None -> (* This is not a warning, so don't filter out *) true
    | Some code ->
      pass_code codes code
      && pass_path (ignore_files @ generated_files) file_path
      && IsNonGenerated.pass ~skip:(not filter_out_generated_files) file_path
end

let filter filter (error_list : Diagnostics.finalized_diagnostic list) =
  List.filter error_list ~f:(fun error ->
      Filter.pass
        filter
        ( User_diagnostic.get_code error,
          User_diagnostic.get_pos error |> Pos.filename ))

let filter_with_hash
    filter (error_list : (Diagnostics.finalized_diagnostic * int) list) =
  List.filter error_list ~f:(fun (error, _hash) ->
      Filter.pass
        filter
        ( User_diagnostic.get_code error,
          User_diagnostic.get_pos error |> Pos.filename ))

let filter_rel filter (errors : Diagnostics.t) =
  Diagnostics.filter errors ~f:(fun path error ->
      Filter.pass
        filter
        (User_diagnostic.get_code error, Relative_path.suffix path))
