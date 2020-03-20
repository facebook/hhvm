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
open Sqlite_utils

(* SQL statements used by the autocomplete system *)
let sql_select_symbols_by_kind =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? ESCAPE '\\' AND kind = ? LIMIT ?"

let sql_select_acid =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? ESCAPE '\\' AND valid_for_acid = 1 LIMIT ?"

let sql_select_acnew =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? ESCAPE '\\' AND valid_for_acnew = 1 LIMIT ?"

let sql_select_actype =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? ESCAPE '\\' AND valid_for_actype = 1 LIMIT ?"

let sql_select_all_symbols =
  "SELECT name, kind, filename_hash FROM symbols WHERE name LIKE ? ESCAPE '\\' LIMIT ?"

(* This syntax means that we add back in the root namespace prefix.
 * It allows us to use LIKE "%\Foo%" to find both "\FooClass" and "\HH\Lib\FooTrait".
 * This feature should be removed if we decide to add back in the original backslash when generating
 * symbol indexes.
 *
 * In SQLite, `||` is the string concatentation operator; it is not the logical OR operator you
 * might be expecting.
 *)
let sql_select_namespaced_symbols =
  "SELECT name, kind, filename_hash FROM symbols WHERE ('\\' || name) LIKE ? ESCAPE '\\' LIMIT ?"

let sql_check_alive = "SELECT name FROM symbols LIMIT 1"

let sql_select_namespaces = "SELECT namespace FROM namespaces"

(* Add escape characters when necessary *)
let sqlite_escape_str (str : string) : string =
  String.fold str ~init:"" ~f:(fun acc char ->
      match char with
      | '%' -> acc ^ "\\%"
      | '_' -> acc ^ "\\_"
      | '\\' -> acc ^ "\\\\"
      | _ -> acc ^ String.make 1 char)

(* Attempt to fetch this file *)
let find_saved_symbolindex ~(ignore_hh_version : bool) :
    (string, string) Core_kernel.result =
  try
    let repo = Path.make (Relative_path.path_of_prefix Relative_path.Root) in
    let res =
      Future.get_exn
        (State_loader_futures.load
           ~repo
           ~ignore_hh_version
           ~saved_state_type:Saved_state_loader.Symbol_index)
    in
    match res with
    | Ok (info, _) ->
      Ok
        (Path.to_string
           info
             .Saved_state_loader.Symbol_index_saved_state_info.symbol_index_path)
    | Error load_error ->
      Error (Saved_state_loader.long_user_message_of_error load_error)
  with _ -> Error "Exception searching for saved state"

(* Determine the correct filename to use for the db_path or build it *)
let find_or_build_sqlite_file
    (workers : MultiWorker.worker list option)
    (savedstate_file_opt : string option)
    ~(silent : bool)
    ~(ignore_hh_version : bool) : string =
  match savedstate_file_opt with
  | Some path -> path
  | None ->
    (* Can we get one from the saved state fetcher? *)
    (match find_saved_symbolindex ~ignore_hh_version with
    | Ok filename -> filename
    | Error errmsg ->
      let repo_path = Relative_path.to_absolute (Relative_path.from_root "") in
      if not silent then
        Hh_logger.log "Unable to fetch sqlite symbol index: %s" errmsg;
      let tempfilename = Filename.temp_file "symbolindex" ".db" in
      if not silent then
        Hh_logger.log
          "Generating [%s] from repository [%s]"
          tempfilename
          repo_path;
      let ctxt =
        {
          IndexBuilderTypes.repo_folder = repo_path;
          sqlite_filename = Some tempfilename;
          text_filename = None;
          json_filename = None;
          json_repo_name = None;
          json_chunk_size = 0;
          custom_service = None;
          custom_repo_name = None;
          set_paths_for_worker = false;
          hhi_root_folder = Some (Hhi.get_hhi_root ());
          silent;
        }
      in
      IndexBuilder.go ctxt workers;
      tempfilename)

(*
 * Ensure the database is available.
 * If the database is stored remotely, this will trigger it being
 * pulled local.  If no database is specified, this will generate
 * it.
 *)
let initialize
    ~(sienv : si_env)
    ~(workers : MultiWorker.worker list option)
    ~(ignore_hh_version : bool)
    ~(savedstate_file_opt : string option) : si_env =
  (* Find the database and open it *)
  let db_path =
    find_or_build_sqlite_file
      ~silent:sienv.sie_quiet_mode
      ~ignore_hh_version
      workers
      savedstate_file_opt
  in
  let db = Sqlite3.db_open db_path in
  (* Report that the database has been loaded *)
  if not sienv.sie_quiet_mode then
    Hh_logger.log "Initialized symbol index sqlite: [%s]" db_path;

  (* Here's the updated environment *)
  { sienv with sql_symbolindex_db = ref (Some db) }

(* Single function for reading results from an executed statement *)
let read_si_results (stmt : Sqlite3.stmt) : si_results =
  let results = ref [] in
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    let kindnum = to_int (Sqlite3.column stmt 1) in
    let kind = int_to_kind kindnum in
    let filehash = to_int64 (Sqlite3.column stmt 2) in
    results :=
      {
        si_name = name;
        si_kind = kind;
        si_filehash = filehash;
        si_fullname = name;
      }
      :: !results
  done;
  !results

let check_rc (sienv : si_env) =
  check_rc (Option.value_exn !(sienv.sql_symbolindex_db))

(* Find all symbols matching a specific kind *)
let search_symbols_by_kind
    ~(sienv : si_env)
    ~(query_text : string)
    ~(max_results : int)
    ~(kind_filter : si_kind) : si_results =
  let stmt =
    prepare_or_reset_statement
      sienv.sql_symbolindex_db
      sienv.sql_select_symbols_by_kind_stmt
      sql_select_symbols_by_kind
  in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc sienv;
  Sqlite3.bind
    stmt
    2
    (Sqlite3.Data.INT (Int64.of_int (kind_to_int kind_filter)))
  |> check_rc sienv;
  Sqlite3.bind stmt 3 (Sqlite3.Data.INT (Int64.of_int max_results))
  |> check_rc sienv;
  read_si_results stmt

(* Symbol search for all symbols. *)
let search_all_symbols
    ~(sienv : si_env) ~(query_text : string) ~(max_results : int) : si_results =
  let stmt =
    prepare_or_reset_statement
      sienv.sql_symbolindex_db
      sienv.sql_select_symbols_stmt
      sql_select_all_symbols
  in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc sienv;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results))
  |> check_rc sienv;
  read_si_results stmt

(* Symbol search for symbols containing a string. *)
let search_namespaced_symbols
    ~(sienv : si_env) ~(query_text : string) ~(max_results : int) : si_results =
  let stmt =
    prepare_or_reset_statement
      sienv.sql_symbolindex_db
      sienv.sql_select_namespaced_symbols_stmt
      sql_select_namespaced_symbols
  in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT ("%\\\\" ^ query_text ^ "%"))
  |> check_rc sienv;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results))
  |> check_rc sienv;
  read_si_results stmt

(* Symbol search for symbols valid in ACID context *)
let search_acid ~(sienv : si_env) ~(query_text : string) ~(max_results : int) :
    si_results =
  let stmt =
    prepare_or_reset_statement
      sienv.sql_symbolindex_db
      sienv.sql_select_acid_stmt
      sql_select_acid
  in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc sienv;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results))
  |> check_rc sienv;
  read_si_results stmt

(* Symbol search for symbols valid in ACNEW context *)
let search_acnew ~(sienv : si_env) ~(query_text : string) ~(max_results : int) :
    si_results =
  let stmt =
    prepare_or_reset_statement
      sienv.sql_symbolindex_db
      sienv.sql_select_acnew_stmt
      sql_select_acnew
  in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc sienv;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results))
  |> check_rc sienv;
  read_si_results stmt

(* Symbol search for symbols valid in ACTYPE context *)
let search_actype ~(sienv : si_env) ~(query_text : string) ~(max_results : int)
    : si_results =
  let stmt =
    prepare_or_reset_statement
      sienv.sql_symbolindex_db
      sienv.sql_select_actype_stmt
      sql_select_actype
  in
  Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")) |> check_rc sienv;
  Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results))
  |> check_rc sienv;
  read_si_results stmt

(* Main entry point *)
let sqlite_search
    ~(sienv : si_env)
    ~(query_text : string)
    ~(max_results : int)
    ~(context : autocomplete_type option)
    ~(kind_filter : SearchUtils.si_kind option) : si_results =
  let query_text = sqlite_escape_str query_text in
  match (context, kind_filter) with
  | (Some Acid, _) -> search_acid ~sienv ~query_text ~max_results
  | (Some Acnew, _) -> search_acnew ~sienv ~query_text ~max_results
  | (Some Actype, _) -> search_actype ~sienv ~query_text ~max_results
  | (Some Actrait_only, _) ->
    search_symbols_by_kind ~sienv ~query_text ~max_results ~kind_filter:SI_Trait
  | (None, Some kind) ->
    search_symbols_by_kind ~sienv ~query_text ~max_results ~kind_filter:kind
  | (Some Ac_workspace_symbol, _) ->
    search_namespaced_symbols ~sienv ~query_text ~max_results
  | _ -> search_all_symbols ~sienv ~query_text ~max_results

(* Fetch all known namespaces from the database *)
let fetch_namespaces ~(sienv : si_env) : string list =
  let stmt =
    prepare_or_reset_statement
      sienv.sql_symbolindex_db
      sienv.sql_select_namespaces_stmt
      sql_select_namespaces
  in
  let namespace_list = ref [] in
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    namespace_list := name :: !namespace_list
  done;
  !namespace_list
