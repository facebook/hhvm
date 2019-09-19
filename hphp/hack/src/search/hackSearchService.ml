(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Utils
open Reordered_argument_collections
open SearchUtils

let fuzzy = ref false

module SS = SearchService.Make (struct
  type t = si_kind

  let fuzzy_types =
    [
      SI_Class;
      SI_Interface;
      SI_Trait;
      SI_Enum;
      SI_Function;
      SI_GlobalConstant;
      SI_Typedef;
    ]

  let compare_result_type a b = kind_to_int a - kind_to_int b
end)

module WorkerApi = struct
  (* cleans off namespace and colon at the start of xhp name because the
   * user will want to search for xhp classes without typing a : at
   * the start of every search *)
  let clean_key key =
    if String.length key > 0 then
      let key = String.lowercase (Utils.strip_ns key) in
      if String.length key > 0 && key.[0] = ':' then
        String.sub key 1 (String.length key - 1)
      else
        key
    else
      key

  let add_trie_term id type_ acc =
    let (pos, name) = id in
    SS.WorkerApi.process_trie_term (strip_ns name) name pos type_ acc

  let add_fuzzy_term id type_ acc =
    let name = snd id in
    let key = strip_ns name in
    SS.WorkerApi.process_fuzzy_term key name (fst id) type_ acc

  let add_autocomplete_term id type_ acc =
    let (pos, name) = id in
    let key = strip_ns name in
    SS.WorkerApi.process_autocomplete_term key name pos type_ acc

  let update_defs id type_ fuzzy_defs trie_defs =
    if !fuzzy then
      (add_fuzzy_term id type_ fuzzy_defs, trie_defs)
    else
      (fuzzy_defs, add_trie_term id type_ trie_defs)

  (* We don't have full info here, so we give File positions and None for *)
  (* class kind. We then lazily populate class methods for search later *)
  let update_from_fast fn (names : FileInfo.names) =
    let { FileInfo.n_funs; n_types; n_consts; n_classes } = names in
    (* Fold all the functions *)
    let (fuzzy, trie, auto) =
      SSet.fold
        n_funs
        ~init:(SS.Fuzzy.TMap.empty, [], [])
        ~f:(fun name (f, t, a) ->
          let pos = FileInfo.File (FileInfo.Fun, fn) in
          let (f, t) = update_defs (pos, name) SI_Function f t in
          (f, t, add_autocomplete_term (pos, name) SI_Function a))
    in
    let (fuzzy, trie, auto) =
      SSet.fold n_classes ~init:(fuzzy, trie, auto) ~f:(fun name (f, t, a) ->
          let pos = FileInfo.File (FileInfo.Class, fn) in
          let (f, t) = update_defs (pos, name) SI_Class f t in
          (f, t, add_autocomplete_term (pos, name) SI_Class a))
    in
    let (fuzzy, trie, auto) =
      SSet.fold n_types ~init:(fuzzy, trie, auto) ~f:(fun name (f, t, _a) ->
          let pos = FileInfo.File (FileInfo.Typedef, fn) in
          let (f, t) = update_defs (pos, name) SI_Typedef f t in
          (f, t, _a))
    in
    let (fuzzy, trie, auto) =
      SSet.fold n_consts ~init:(fuzzy, trie, auto) ~f:(fun name (f, t, _a) ->
          let pos = FileInfo.File (FileInfo.Const, fn) in
          let (f, t) = update_defs (pos, name) SI_GlobalConstant f t in
          (f, t, _a))
    in
    SS.WorkerApi.update fn trie fuzzy auto

  (* Update from the full fileInfo object: get all data except class kind *)
  let update_from_fileinfo fn (info : FileInfo.t) =
    let { FileInfo.funs; typedefs; consts; classes; _ } = info in
    let (fuzzy, trie, auto) =
      List.fold
        funs
        ~init:(SS.Fuzzy.TMap.empty, [], [])
        ~f:(fun (f, t, a) id ->
          let (f, t) = update_defs id SI_Function f t in
          (f, t, add_autocomplete_term id SI_Function a))
    in
    let (fuzzy, trie, auto) =
      List.fold classes ~init:(fuzzy, trie, auto) ~f:(fun (f, t, a) id ->
          let (f, t) = update_defs id SI_Class f t in
          (f, t, add_autocomplete_term id SI_Class a))
    in
    let (fuzzy, trie, auto) =
      List.fold typedefs ~init:(fuzzy, trie, auto) ~f:(fun (f, t, _a) id ->
          let (f, t) = update_defs id SI_Typedef f t in
          (f, t, _a))
    in
    let (fuzzy, trie, auto) =
      List.fold consts ~init:(fuzzy, trie, auto) ~f:(fun (f, t, _a) id ->
          let (f, t) = update_defs id SI_GlobalConstant f t in
          (f, t, _a))
    in
    SS.WorkerApi.update fn trie fuzzy auto
end

module MasterApi = struct
  let query workers input type_ ~fuzzy =
    let results =
      SS.MasterApi.query workers input (string_to_kind type_) ~fuzzy
    in
    List.filter_map results (fun search_term ->
        let { SearchUtils.name; pos; result_type } = search_term in
        match (pos, result_type) with
        (* Need both class kind and position *)
        | (FileInfo.File (FileInfo.Class, fn), SI_Class) ->
          (match Ast_provider.find_class_in_file fn name with
          | Some c ->
            let (pos, result_type) = (fst c.Aast.c_name, SI_Class) in
            Some { SearchUtils.name; pos; result_type }
          | None -> None)
        | (FileInfo.File (FileInfo.Fun, fn), SI_Function) ->
          (match Ast_provider.find_fun_in_file fn name with
          | Some c ->
            let pos = fst c.Aast.f_name in
            Some { SearchUtils.name; pos; result_type }
          | None -> None)
        | (FileInfo.File (FileInfo.Typedef, fn), SI_Typedef) ->
          (match Ast_provider.find_typedef_in_file fn name with
          | Some c ->
            let pos = fst c.Aast.t_name in
            Some { SearchUtils.name; pos; result_type }
          | None -> None)
        | (FileInfo.File (FileInfo.Const, fn), SI_GlobalConstant) ->
          (match Ast_provider.find_gconst_in_file fn name with
          | Some c ->
            let pos = fst c.Aast.cst_name in
            Some { SearchUtils.name; pos; result_type }
          | None -> None)
        | (FileInfo.Full p, SI_Class) ->
          let fn = Pos.filename p in
          (match Ast_provider.find_class_in_file fn name with
          | Some _ ->
            let result_type = SI_Class in
            Some { SearchUtils.name; pos = p; result_type }
          | None -> None)
        | (FileInfo.Full p, _) ->
          Some { SearchUtils.name; pos = p; result_type }
        | _ -> Utils.assert_false_log_backtrace (Some "Incorrect position"))

  let query_autocomplete
      (input : string)
      ~(limit : int option)
      ~(filter_map : string -> string -> legacy_symbol -> 'a option) :
      'a list Utils.With_complete_flag.t =
    SS.AutocompleteTrie.MasterApi.query input ~limit ~filter_map

  let clear_shared_memory = SS.MasterApi.clear_shared_memory

  let update_search_index ~fuzzy files =
    SS.MasterApi.update_search_index ~fuzzy files
end

(* Differentiate between two types of data sets provided via either
 * the saved state or the typechecker *)
let index_symbols_in_file (fn, info) =
  match info with
  | Full infos -> WorkerApi.update_from_fileinfo fn infos
  | Fast names -> WorkerApi.update_from_fast fn names

(* Method for importing data from the typechecker or saved-state *)
let update_from_typechecker
    (worker_list_opt : MultiWorker.worker list option)
    (files : (Relative_path.t * info * SearchUtils.file_source) list) : unit =
  let mapped_files = List.map files ~f:(fun (a, b, _) -> (a, b)) in
  if List.length mapped_files < 100 then
    List.iter mapped_files index_symbols_in_file
  else
    MultiWorker.call
      worker_list_opt
      ~job:(fun () mapped_files ->
        List.iter mapped_files index_symbols_in_file)
      ~neutral:()
      ~merge:(fun _ _ -> ())
      ~next:(MultiWorker.next worker_list_opt mapped_files);
  MasterApi.update_search_index ~fuzzy:!fuzzy (List.map mapped_files fst)

(* Method for retrieving data that matches the symbolindex search *)
let index_search
    (prefix : string) (max_results : int) (kind_opt : si_kind option) :
    si_results =
  (* What kind of kind-matching are we doing? *)
  let kind_match (res_kind : si_kind) : bool =
    if kind_opt = None then
      true
    else
      match kind_opt with
      | None -> false
      | Some kind -> kind = res_kind
  in
  (* Raw query *)
  let results =
    MasterApi.query_autocomplete
      prefix
      ~limit:(Some max_results)
      ~filter_map:(fun _ _ result ->
        let name = result.SearchUtils.name in
        let kind = result.SearchUtils.result_type in
        if kind_match kind then
          Some
            {
              si_kind = kind;
              si_name = name;
              si_filehash = 0L;
              si_fullname = name;
            }
        else
          None)
  in
  results.With_complete_flag.value
