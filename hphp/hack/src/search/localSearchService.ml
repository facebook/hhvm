(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open SearchUtils

(* How many locally changed files are in this env? *)
let count_local_fileinfos ~(sienv : si_env) : int =
  Relative_path.Map.cardinal sienv.lss_fullitems

(* Determine a tombstone for a file path *)
let get_tombstone (path : Relative_path.t) : int64 =
  let rel_path_str = Relative_path.suffix path in
  let path_hash = SharedMem.get_hash rel_path_str in
  path_hash

(* Update files when they were discovered *)
let update_file
    ~(sienv : si_env) ~(path : Relative_path.t) ~(info : SearchUtils.info) :
    si_env =
  let _ = info in
  let tombstone = get_tombstone path in
  let contents = IndexBuilder.parse_one_file ~path in
  {
    sienv with
    lss_fullitems =
      Relative_path.Map.add sienv.lss_fullitems ~key:path ~data:contents;
    lss_tombstones = Tombstone_set.add sienv.lss_tombstones tombstone;
  }

(* Remove files from local when they are deleted *)
let remove_file ~(sienv : si_env) ~(path : Relative_path.t) : si_env =
  let tombstone = get_tombstone path in
  {
    sienv with
    lss_fullitems = Relative_path.Map.remove sienv.lss_fullitems path;
    lss_tombstones = Tombstone_set.add sienv.lss_tombstones tombstone;
  }

(* Exception we use to short-circuit out of a loop if we find enough results.
 * May be worth rewriting this in the future to use `fold_until` rather than
 * exceptions *)
exception BreakOutOfScan of si_results

(* Search local changes for symbols matching this prefix *)
let search_local_symbols
    ~(sienv : si_env)
    ~(query_text : string)
    ~(max_results : int)
    ~(context : autocomplete_type option)
    ~(kind_filter : si_kind option) : si_results =
  (* case insensitive search, must include namespace, escaped for regex *)
  let query_text_regex_case_insensitive =
    Str.regexp_case_fold (Str.quote query_text)
  in
  (* case insensitive search, break out if max results reached *)
  let check_symbol_and_add_to_accumulator_and_break_if_max_reached
      ~(acc : si_results)
      ~(symbol : si_fullitem)
      ~(context : autocomplete_type option)
      ~(kind_filter : si_kind option)
      ~(path : Relative_path.t) : si_results =
    let is_valid_match =
      match (context, kind_filter) with
      | (Some Actype, _) -> SearchUtils.valid_for_actype symbol
      | (Some Acnew, _) -> SearchUtils.valid_for_acnew symbol
      | (Some Acid, _) -> SearchUtils.valid_for_acid symbol
      | (Some Actrait_only, _) -> symbol.sif_kind = SI_Trait
      | (_, Some kind_match) -> symbol.sif_kind = kind_match
      | _ -> true
    in
    if
      is_valid_match
      && Str.string_partial_match
           query_text_regex_case_insensitive
           symbol.sif_name
           0
    then
      let fullname = Utils.strip_ns symbol.sif_name in
      let acc_new =
        {
          si_name = fullname;
          si_kind = symbol.sif_kind;
          si_filehash = get_tombstone path;
          si_fullname = fullname;
        }
        :: acc
      in
      if List.length acc_new >= max_results then
        raise (BreakOutOfScan acc_new)
      else
        acc_new
    else
      acc
  in
  try
    let acc =
      Relative_path.Map.fold
        sienv.lss_fullitems
        ~init:[]
        ~f:(fun path fullitems acc ->
          let matches =
            List.fold fullitems ~init:[] ~f:(fun acc symbol ->
                check_symbol_and_add_to_accumulator_and_break_if_max_reached
                  ~acc
                  ~symbol
                  ~context
                  ~kind_filter
                  ~path)
          in
          List.append acc matches)
    in
    acc
  with BreakOutOfScan acc -> acc

(* Filter the results to extract any dead objects *)
let extract_dead_results
    ~(sienv : SearchUtils.si_env) ~(results : SearchUtils.si_results) :
    si_results =
  List.filter results ~f:(fun r ->
      let is_valid_result =
        not (Tombstone_set.mem sienv.lss_tombstones r.si_filehash)
      in
      is_valid_result)
