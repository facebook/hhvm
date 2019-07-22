(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel
open SearchUtils
open Reordered_argument_collections

(* How many locally changed files are in this env? *)
let count_local_fileinfos
    ~(sienv: si_env): int =
  (Relative_path.Map.cardinal sienv.lss_fileinfos
   + Relative_path.Map.cardinal sienv.lss_filenames)
;;

(* Determine a tombstone for a file path *)
let get_tombstone (path: Relative_path.t): int64 =
  let rel_path_str = Relative_path.suffix path in
  let path_hash = SharedMem.get_hash rel_path_str in
  path_hash
;;

(* Update files when they were discovered *)
let update_file
    ~(sienv: si_env)
    ~(path: Relative_path.t)
    ~(info: SearchUtils.info): si_env =
  let tombstone = get_tombstone path in
  match info with
  | Full fileinfo_t ->
    { sienv with
      lss_fileinfos =
        Relative_path.Map.add sienv.lss_fileinfos
          ~key:path ~data:fileinfo_t;
      lss_tombstones =
        Tombstone_set.add sienv.lss_tombstones tombstone;
    }
  | Fast fileinfo_names ->
    { sienv with
      lss_filenames =
        Relative_path.Map.add sienv.lss_filenames
          ~key:path ~data:fileinfo_names;
      lss_tombstones =
        Tombstone_set.add sienv.lss_tombstones tombstone;
    }
;;

(* Remove files from local when they are deleted *)
let remove_file
    ~(sienv: si_env)
    ~(path: Relative_path.t): si_env =
  let tombstone = get_tombstone path in
  { sienv with
    lss_fileinfos =
      Relative_path.Map.remove sienv.lss_fileinfos path;
    lss_filenames =
      Relative_path.Map.remove sienv.lss_filenames path;
    lss_tombstones =
      Tombstone_set.add sienv.lss_tombstones tombstone;
  }
;;

(* Exception we use to short-circuit out of a loop if we find enough results.
 * May be worth rewriting this in the future to use `fold_until` rather than
 * exceptions *)
exception BreakOutOfScan of si_results

(* Search local changes for symbols matching this prefix *)
let search_local_symbols
    ~(sienv: si_env)
    ~(query_text: string)
    ~(max_results: int)
    ~(kind_filter: si_kind option): si_results =

  (* case insensitive search, must include namespace, escaped for regex *)
  let query_text_regex_case_insensitive =
    Str.regexp_case_fold (Str.quote ("\\" ^ query_text))
  in

  (* In fileinfo.t, we only know that a thing is a "class," but it could
   * actually be a trait or something else.  Let's map that knowledge. *)
  let fixed_kind_filter = match kind_filter with
  | Some SI_Interface
  | Some SI_Trait -> Some SI_Class
  | other_filter -> other_filter
  in

  (* case insensitive search *)
  let check_substring_and_add_to_accumulator_and_break_if_max_reached
      ~(acc: si_results)
      ~(symbol: string)
      ~(kind: si_kind)
      ~(path: Relative_path.t): si_results =
    if Str.string_partial_match query_text_regex_case_insensitive symbol 0 then begin
      let acc_new = {
        si_name = (Utils.strip_ns symbol);
        si_kind = (match kind, kind_filter with
        | SI_Class, Some SI_Trait -> SI_Trait
        | SI_Class, Some SI_Interface -> SI_Interface
        | _ -> kind);
        si_filehash = (get_tombstone path);
        si_fullname = symbol;
      } :: acc in
      if (List.length acc_new) >= max_results then
        raise (BreakOutOfScan acc_new)
      else acc_new
    end else acc
  in

  (* Method to scan a single list *)
  let check_string_sset_using_kind_filter
      (symbols: SSet.t)
      (kind: si_kind)
      (path: Relative_path.t)
      (acc: si_results): si_results =
    if fixed_kind_filter = None || fixed_kind_filter = Some kind then begin
      SSet.fold symbols ~init:acc
        ~f:(fun symbol acc ->
          check_substring_and_add_to_accumulator_and_break_if_max_reached
          ~acc ~symbol ~kind ~path)
    end else acc
  in

  (* Method to scan a single list *)
  let check_id_tuple_list_using_kind_filter
      (ids: FileInfo.id list)
      (kind: si_kind)
      (path: Relative_path.t)
      (acc: si_results): si_results =
    if fixed_kind_filter = None || fixed_kind_filter = Some kind then begin
      List.fold ids ~init:acc
        ~f:(fun acc (_, symbol) ->
          check_substring_and_add_to_accumulator_and_break_if_max_reached
          ~acc ~symbol ~kind ~path)
    end else acc
  in

  try
    let acc = Relative_path.Map.fold sienv.lss_fileinfos
        ~init:[]
        ~f:(fun path info acc ->
            acc
            |> check_id_tuple_list_using_kind_filter info.FileInfo.classes SI_Class path
            |> check_id_tuple_list_using_kind_filter info.FileInfo.funs SI_Function path
            |> check_id_tuple_list_using_kind_filter info.FileInfo.typedefs SI_Typedef path
            |> check_id_tuple_list_using_kind_filter info.FileInfo.consts SI_GlobalConstant path
          ) in
    let acc = Relative_path.Map.fold sienv.lss_filenames
        ~init:acc
        ~f:(fun path names acc ->
            acc
            |> check_string_sset_using_kind_filter names.FileInfo.n_classes SI_Class path
            |> check_string_sset_using_kind_filter names.FileInfo.n_funs SI_Function path
            |> check_string_sset_using_kind_filter names.FileInfo.n_types SI_Typedef path
            |> check_string_sset_using_kind_filter names.FileInfo.n_consts SI_GlobalConstant path
          ) in
    acc
  with BreakOutOfScan acc -> acc
;;

(* Filter the results to extract any dead objects *)
let extract_dead_results
    ~(sienv: SearchUtils.si_env)
    ~(results: SearchUtils.si_results): si_results =
  List.filter results ~f:(fun r ->
    let is_valid_result = not (Tombstone_set.mem sienv.lss_tombstones r.si_filehash) in
    is_valid_result
  )
;;
