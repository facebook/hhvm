(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open SearchUtils
open Core_kernel

let select_symbols_stmt = ref None
let select_symbols_by_kind_stmt = ref None
let sqlite_file_path = ref None

(* SQL statements used by the autocomplete system *)
let sql_select_symbols_by_kind =
  "SELECT name FROM symbols WHERE name LIKE ? AND kind = ? LIMIT ?"
let sql_select_all_symbols =
  "SELECT name, kind FROM symbols WHERE name LIKE ? LIMIT ?"
let sql_check_alive =
  "SELECT name FROM symbols LIMIT 1"

(* Determine the correct filename to use for the db_path or build it *)
let find_or_build_sqlite_file
    (workers: MultiWorker.worker list option): string =
  match !sqlite_file_path with
  | Some path -> path
  | None ->
    (* Launch index builder task *)
    let repo_path = Relative_path.to_absolute
      (Relative_path.from_root "/") in
    let timestamp = string_of_int (int_of_float (Unix.gettimeofday ())) in

    (* Clean the path string *)
    let cleanpath = Path.slash_escaped_string_of_path (Path.make repo_path) in
    let tempdir = (Path.make (Filename.get_temp_dir_name ())) in
    let temppath =
      Path.concat tempdir ("autocomplete." ^ cleanpath ^ "." ^ timestamp ^ ".db") in
    let tempfilename = Path.to_string temppath in
    Hh_logger.log "Sqlite saved state not specified, generating on the fly";
    Hh_logger.log "Generating [%s] from repository [%s]" tempfilename repo_path;
    let ctxt = {
      IndexBuilder.repo_folder = repo_path;
      IndexBuilder.sqlite_filename = Some tempfilename;
      IndexBuilder.text_filename = None;
      IndexBuilder.json_filename = None;
      IndexBuilder.json_chunk_size = 0;
      IndexBuilder.custom_service = None;
      IndexBuilder.custom_repo_name = None;
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
  let db_path = find_or_build_sqlite_file workers in
  let db = Sqlite3.db_open db_path in
  symbolindex_db := (Some db);
  let stmt = Sqlite3.prepare db sql_check_alive in
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    Hh_logger.log "Sqlite database initialized: [%s]" name;
  done

(* Ensure that sqlite gave a valid response to an operation *)
let check_rc (rc: Sqlite3.Rc.t): unit =
  if rc <> Sqlite3.Rc.OK && rc <> Sqlite3.Rc.DONE
  then begin
    failwith (Printf.sprintf "SQLite operation failed: %s"
                (Sqlite3.Rc.to_string rc))
  end

(* Gather a database and prepared statement into a tuple *)
let prepare_or_reset_statement
    (stmt_ref: Sqlite3.stmt option ref)
    (sql_command_text: string) =
  let stmt = match !stmt_ref with
    | Some s ->
      check_rc (Sqlite3.reset s);
      s
    | None ->
      let db = Option.value_exn !symbolindex_db in
      let s = Sqlite3.prepare db sql_command_text in
      stmt_ref := Some s;
      s
  in
  stmt

(* This method should have been included in the sqlite3 package, but wasn't *)
let to_int = function
  | Sqlite3.Data.INT i ->
    begin
      match Int64.to_int i with
      | Some num -> num
      | None -> 0
    end
  | _ -> 0

(*
 * Symbol search for a specific kind.
 *)
let search_symbols_by_kind
    (query_text: string)
    (max_results: int)
    (kind_filter: si_kind)
  : si_results =
  let results = ref [] in
  let stmt = prepare_or_reset_statement select_symbols_by_kind_stmt sql_select_symbols_by_kind in
  check_rc (Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")));
  check_rc (Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int (kind_to_int kind_filter))));
  check_rc (Sqlite3.bind stmt 3 (Sqlite3.Data.INT (Int64.of_int max_results)));
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    results := {
      si_name = name;
      si_kind = kind_filter;
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
  let stmt = prepare_or_reset_statement select_symbols_stmt sql_select_all_symbols in
  check_rc (Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT (query_text ^ "%")));
  check_rc (Sqlite3.bind stmt 2 (Sqlite3.Data.INT (Int64.of_int max_results)));
  while Sqlite3.step stmt = Sqlite3.Rc.ROW do
    let name = Sqlite3.Data.to_string (Sqlite3.column stmt 0) in
    let kindnum = to_int (Sqlite3.column stmt 1) in
    let kind = int_to_kind kindnum in
    results := {
      si_name = name;
      si_kind = kind;
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
