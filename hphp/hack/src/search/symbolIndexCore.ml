(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open SearchUtils
open SearchTypes

(* Log information about calls to the symbol index service *)
let log_symbol_index_search
    ~(sienv : si_env)
    ~(query_text : string)
    ~(max_results : int)
    ~(results : int)
    ~(kind_filter : si_kind option)
    ~(start_time : float)
    ~(caller : string) : unit =
  (* In quiet mode we don't log anything to either scuba or console *)
  if sienv.sie_quiet_mode then
    ()
  else
    (* Calculate duration *)
    let end_time = Unix.gettimeofday () in
    let duration = end_time -. start_time in
    (* Clean up strings *)
    let kind_filter_str =
      match kind_filter with
      | None -> "None"
      | Some kind -> show_si_kind kind
    in
    let search_provider = descriptive_name_of_provider sienv.sie_provider in
    (* Send information to remote logging system *)
    if sienv.sie_log_timings then
      Hh_logger.log
        "[symbolindex] Search [%s] for [%s] found %d results in [%0.3f]"
        search_provider
        query_text
        results
        duration;
    HackEventLogger.search_symbol_index
      ~query_text
      ~max_results
      ~results
      ~kind_filter:kind_filter_str
      ~duration
      ~caller
      ~search_provider

(*
 * This method is called when the typechecker has finished re-checking a file,
 * or when the saved-state is fully loaded.  Any system that needs to cache
 * this information should capture it here.
 *)
let update_files
    ~(ctx : Provider_context.t)
    ~(sienv : si_env)
    ~(paths : (Relative_path.t * FileInfo.t * file_source) list) : si_env =
  match sienv.sie_provider with
  | NoIndex
  | MockIndex _ ->
    sienv
  | CustomIndex
  | LocalIndex ->
    List.fold paths ~init:sienv ~f:(fun sienv (path, info, detector) ->
        match detector with
        | SearchUtils.TypeChecker ->
          LocalSearchService.update_file ~ctx ~sienv ~path ~info
        | _ -> sienv)

type paths_with_addenda =
  (Relative_path.t * SearchTypes.si_addendum list * SearchUtils.file_source)
  list

let update_from_addenda
    ~(sienv : si_env) ~(paths_with_addenda : paths_with_addenda) : si_env =
  match sienv.sie_provider with
  | NoIndex
  | MockIndex _ ->
    sienv
  | CustomIndex
  | LocalIndex ->
    List.fold
      paths_with_addenda
      ~init:sienv
      ~f:(fun sienv (path, addenda, detector) ->
        match detector with
        | SearchUtils.TypeChecker ->
          LocalSearchService.update_file_from_addenda ~sienv ~path ~addenda
        | _ -> sienv)

(*
 * This method is called when the typechecker is about to re-check a file.
 * Any local caches should be cleared of values for this file.
 *)
let remove_files ~(sienv : SearchUtils.si_env) ~(paths : Relative_path.Set.t) :
    si_env =
  match sienv.sie_provider with
  | NoIndex
  | MockIndex _ ->
    sienv
  | CustomIndex
  | LocalIndex ->
    Relative_path.Set.fold paths ~init:sienv ~f:(fun path sienv ->
        LocalSearchService.remove_file ~sienv ~path)

(* Fetch best available position information for a symbol *)
let get_position_for_symbol
    (ctx : Provider_context.t) (symbol : string) (kind : si_kind) :
    (Relative_path.t * int * int) option =
  (* Symbols can only be found if they are properly namespaced.
   * Even XHP classes must start with a backslash. *)
  let name_with_ns = Utils.add_ns symbol in

  let helper get_pos resolve_pos =
    (* get_pos only gives us filename+nametype *)
    let fileinfo_pos = get_pos ctx name_with_ns in
    (* we have to resolve that into a proper position *)
    Option.map fileinfo_pos ~f:(fun fileinfo_pos ->
        let (pos, _) = resolve_pos ctx (fileinfo_pos, name_with_ns) in
        let relpath = FileInfo.get_pos_filename fileinfo_pos in
        let (line, col, _) = Pos.info_pos pos in
        (relpath, line, col))
  in
  match kind with
  | SI_XHP
  | SI_Interface
  | SI_Trait
  | SI_Enum
  | SI_Typedef
  | SI_Class
  | SI_Constructor ->
    helper Naming_provider.get_type_pos Naming_global.GEnv.get_type_full_pos
  | SI_Function ->
    helper Naming_provider.get_fun_pos Naming_global.GEnv.get_fun_full_pos
  | SI_GlobalConstant ->
    helper Naming_provider.get_const_pos Naming_global.GEnv.get_const_full_pos
  (* Items below this are not global symbols and cannot be 'position'ed *)
  | SI_Unknown
  | SI_Namespace
  | SI_ClassMethod
  | SI_Literal
  | SI_ClassConstant
  | SI_Property
  | SI_LocalVariable
  | SI_Keyword
  | SI_Mixed ->
    None

let absolute_none = Pos.none |> Pos.to_absolute

(* Shortcut to use the above method to get an absolute pos *)
let get_pos_for_item_opt (ctx : Provider_context.t) (item : si_item) :
    Pos.absolute option =
  let result = get_position_for_symbol ctx item.si_fullname item.si_kind in
  match result with
  | None -> None
  | Some (relpath, line, col) ->
    let symbol_len = String.length item.si_fullname in
    let pos =
      Pos.make_from_lnum_bol_offset
        ~pos_file:relpath
        ~pos_start:(line, 1, col)
        ~pos_end:(line, 1, col + symbol_len)
    in
    Some (Pos.to_absolute pos)

(* Shortcut to use the above method to get an absolute pos *)
let get_pos_for_item (ctx : Provider_context.t) (item : si_item) : Pos.absolute
    =
  let result = get_pos_for_item_opt ctx item in
  match result with
  | None -> absolute_none
  | Some pos -> pos
