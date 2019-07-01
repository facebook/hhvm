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

(* SQL statements used by the autocomplete system *)
let sql_select_symbols_by_kind =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? AND kind = ? LIMIT ?"
let sql_select_acid =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? AND valid_for_acid = 1 LIMIT ?"
let sql_select_acnew =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? AND valid_for_acnew = 1 LIMIT ?"
let sql_select_actype =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? AND valid_for_actype = 1 LIMIT ?"
let sql_select_all_symbols =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? LIMIT ?"
let sql_check_alive =
  "SELECT name FROM symbols LIMIT 1"
let sql_select_namespaces =
  "SELECT namespace FROM namespaces"

(* Determine the correct filename to use for the db_path or build it *)
let find_or_build_sqlite_file
    (workers: MultiWorker.worker list option)
    (savedstate_file_opt: string option): string =
  match savedstate_file_opt with
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
      let ctxt = { IndexBuilderTypes.
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

(*
 * Ensure the database is available.
 * If the database is stored remotely, this will trigger it being
 * pulled local.  If no database is specified, this will generate
 * it.
 *)
let initialize
    ~(sienv: si_env)
    ~(workers: MultiWorker.worker list option)
    ~(savedstate_file_opt: string option): si_env =

  (* Find the database and open it *)
  let db_path = find_or_build_sqlite_file workers savedstate_file_opt in
  let db = Sqlite3.db_open db_path in

  (* Report that the database has been loaded *)
  Hh_logger.log "Initialized symbol index sqlite: [%s]" db_path;

  (* Here's the updated environment *)
  { sienv with
    sql_symbolindex_db = ref (Some db);
  }

(* Single function for reading results from an executed statement *)
let read_si_results (stmt: Sqlite3.stmt): si_results =
  let results = ref [] in
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

(* Find all symbols matching a specific kind *)
let search_symbols_by_kind
    ~(sienv: si_env)
    ~(query_text: string)
    ~(max_results: int)
    ~(kind_filter: si_kind)
  : si_results =
  let stmt = prepare_or_reset_statement sienv.sql_symbolindex_db
    sienv.sql_select_symbols_by_kind_stmt sql_select_symbols_by_kind in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int (kind_to_int kind_filter))) |> check_rc;
  Sqlite3.bind stmt 3 (Sqlite3.Data.INT (Int64.of_int max_results)) |> check_rc;
  read_si_results stmt

(* Symbol search for all symbols. *)
let search_all_symbols
    ~(sienv: si_env)
    ~(query_text: string)
    ~(max_results: int): si_results =
  let stmt = prepare_or_reset_statement sienv.sql_symbolindex_db
    sienv.sql_select_symbols_stmt sql_select_all_symbols in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results)) |> check_rc;
  read_si_results stmt

(* Symbol search for symbols valid in ACID context *)
let search_acid
    ~(sienv: si_env)
    ~(query_text: string)
    ~(max_results: int): si_results =
  let stmt = prepare_or_reset_statement sienv.sql_symbolindex_db
    sienv.sql_select_acid_stmt sql_select_acid in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results)) |> check_rc;
  read_si_results stmt

(* Symbol search for symbols valid in ACNEW context *)
let search_acnew
    ~(sienv: si_env)
    ~(query_text: string)
    ~(max_results: int): si_results =
  let stmt = prepare_or_reset_statement sienv.sql_symbolindex_db
    sienv.sql_select_acnew_stmt sql_select_acnew in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results)) |> check_rc;
  read_si_results stmt

(* Symbol search for symbols valid in ACTYPE context *)
let search_actype
    ~(sienv: si_env)
    ~(query_text: string)
    ~(max_results: int): si_results =
  let stmt = prepare_or_reset_statement sienv.sql_symbolindex_db
    sienv.sql_select_actype_stmt sql_select_actype in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results)) |> check_rc;
  read_si_results stmt

(* Main entry point *)
let sqlite_search
    ~(sienv: si_env)
    ~(query_text: string)
    ~(max_results: int)
    ~(context: autocomplete_type option): si_results =
  match context with
  | Some Acid -> search_acid ~sienv ~query_text ~max_results
  | Some Acnew -> search_acnew ~sienv ~query_text ~max_results
  | Some Actype -> search_actype ~sienv ~query_text ~max_results
  | Some Actrait_only -> search_symbols_by_kind
    ~sienv ~query_text ~max_results ~kind_filter:SI_Trait
  | _ -> search_all_symbols ~sienv ~query_text ~max_results

(* Fetch all known namespaces from the database *)
let fetch_namespaces
    ~(sienv: si_env): string list =
  let stmt = prepare_or_reset_statement sienv.sql_symbolindex_db
    sienv.sql_select_namespaces_stmt sql_select_namespaces in
  let namespace_list = ref [] in
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    namespace_list := name :: !namespace_list;
  done;
  !namespace_list
