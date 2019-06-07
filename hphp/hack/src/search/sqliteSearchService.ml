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
open Sqlite_utils

let select_symbols_stmt = ref None
let select_symbols_by_kind_stmt = ref None
let select_namespaces_stmt = ref None
let sqlite_file_path = ref None

(* SQL statements used by the autocomplete system *)
let sql_select_symbols_by_kind =
  "SELECT name, filename_hash FROM symbols WHERE name LIKE ? AND kind = ? LIMIT ?"
let sql_select_all_symbols =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? LIMIT ?"
let sql_check_alive =
  "SELECT name FROM symbols LIMIT 1"
let sql_select_namespaces =
  "SELECT namespace FROM namespaces"

(* Determine the correct filename to use for the db_path or build it *)
let find_or_build_sqlite_file
    (workers: MultiWorker.worker list option): string =
  match !sqlite_file_path with
  | Some path -> path
  | None ->
    (* Can we get one from the saved state fetcher? *)
    match SavedStateFetcher.find_saved_symbolindex () with
    | Ok filename -> filename
    | Error errmsg ->
      let repo_path = Relative_path.to_absolute
        (Relative_path.from_root "") in
      Hh_logger.log "Unable to fetch sqlite symbol index: %s" errmsg;
      let tempfilename = SavedStateFetcher.get_filename_for_symbol_index ".db" in
      Hh_logger.log "Sqlite saved state not specified, generating on the fly";
      Hh_logger.log "Generating [%s] from repository [%s]" tempfilename repo_path;
      let ctxt = { IndexBuilder.
        repo_folder = repo_path;
        sqlite_filename = Some tempfilename;
        text_filename = None;
        json_filename = None;
        json_chunk_size = 0;
        custom_service = None;
        custom_repo_name = None;
        include_builtins = true;
      } in
      IndexBuilder.go ctxt workers;
      tempfilename

(* Symbolindex DB may be loaded or generated *)
let symbolindex_db = ref None

(*
 * Ensure the database is available.
 * If the database is stored remotely, this will trigger it being
 * pulled local.  If no database is specified, this will generate
 * it.
 *)
let initialize
    (workers: MultiWorker.worker list option): unit =

  (* Find the database and open it *)
  let db_path = find_or_build_sqlite_file workers in
  let db = Sqlite3.db_open db_path in
  symbolindex_db := (Some db);

  (* Report that the database has been loaded *)
  Hh_logger.log "Initialized symbol index sqlite: [%s]" db_path

(*
 * Symbol search for a specific kind.
 *)
let search_symbols_by_kind
    (query_text: string)
    (max_results: int)
    (kind_filter: si_kind)
  : si_results =
  let results = ref [] in
  let stmt = prepare_or_reset_statement (Option.value_exn !symbolindex_db)
    select_symbols_by_kind_stmt sql_select_symbols_by_kind in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int (kind_to_int kind_filter))) |> check_rc;
  Sqlite3.bind stmt 3 (Sqlite3.Data.INT (Int64.of_int max_results)) |> check_rc;
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    let filehash = to_int64 (Sqlite3.column stmt 1) in
    results := {
      si_name = name;
      si_kind = kind_filter;
      si_filehash = filehash;
    } :: !results;
  done;
  !results

(*
 * Symbol search for all symbols.
 *)
let search_all_symbols
    (query_text: string)
    (max_results: int): si_results =
  let results = ref [] in
  let stmt = prepare_or_reset_statement (Option.value_exn !symbolindex_db)
    select_symbols_stmt sql_select_all_symbols in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results)) |> check_rc;
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    let kindnum = to_int (Sqlite3.column stmt 1) in
    let kind = int_to_kind kindnum in
    let filehash = to_int64 (Sqlite3.column stmt 2) in
    results := {
      si_name = name;
      si_kind = kind;
      si_filehash = filehash;
    } :: !results;
  done;
  !results

(* Main entry point *)
let sqlite_search
    (query_text: string)
    (max_results: int)
    (kind_filter: si_kind option): si_results =
  match kind_filter with
  | Some kind -> search_symbols_by_kind query_text max_results kind
  | None -> search_all_symbols query_text max_results

(* Fetch all known namespaces from the database *)
let fetch_namespaces (): string list =
  let stmt = prepare_or_reset_statement (Option.value_exn !symbolindex_db)
    select_namespaces_stmt sql_select_namespaces in
  let namespace_list = ref [] in
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    namespace_list := name :: !namespace_list;
  done;
  !namespace_list
