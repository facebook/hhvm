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

(* Some SQL commands we'll need *)
let sql_begin_transaction =
  "BEGIN TRANSACTION"

let sql_commit_transaction =
  "COMMIT TRANSACTION"

let sql_create_symbols_table =
  "CREATE TABLE IF NOT EXISTS symbols ( " ^
  "    namespace_id INTEGER NOT NULL, " ^
  "    filename_hash INTEGER NOT NULL, " ^
  "    name TEXT NOT NULL, " ^
  "    kind INTEGER NOT NULL " ^
  ");"

let sql_create_kinds_table =
  "CREATE TABLE IF NOT EXISTS kinds ( " ^
  "    id INTEGER NOT NULL, " ^
  "    description TEXT NOT NULL " ^
  ");"

let sql_create_namespaces_table =
  "CREATE TABLE IF NOT EXISTS namespaces ( " ^
  "    namespace_id INTEGER NOT NULL, " ^
  "    namespace TEXT NOT NULL " ^
  ");"

let sql_insert_kinds =
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (1, 'Class');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (2, 'Interface');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (3, 'Enum');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (4, 'Trait');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (5, 'Unknown');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (6, 'Mixed');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (7, 'Function');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (8, 'Typedef');"

let sql_insert_symbol =
  "INSERT INTO symbols " ^
  " (namespace_id, filename_hash, name, kind)" ^
  " VALUES" ^
  " (?, ?, ?, ?);"

let sql_insert_namespace =
  "INSERT INTO namespaces " ^
  " (namespace_id, namespace)" ^
  " VALUES" ^
  " (?, ?);"

let sql_create_indexes =
  "CREATE INDEX IF NOT EXISTS ix_symbols_name ON symbols (name);" ^
  "CREATE INDEX IF NOT EXISTS ix_symbols_kindname ON symbols (kind, name);" ^
  "CREATE INDEX IF NOT EXISTS ix_symbols_namespace ON symbols (namespace_id, name);"

(* Capture responses and crash if database fails *)
let check_rc (rc: Sqlite3.Rc.t): unit =
  if rc <> Sqlite3.Rc.OK && rc <> Sqlite3.Rc.DONE
  then failwith (Printf.sprintf "SQLite operation failed: %s" (Sqlite3.Rc.to_string rc))

(* Begin the work of creating an SQLite index DB *)
let record_in_db
    (filename: string)
    (symbols: sic_results): unit =

  (* If the file exists, remove it before starting over *)
  if Sys.file_exists filename then begin
    Unix.unlink filename;
  end;

  (* Open the database and do basic prep *)
  let db = Sqlite3.db_open filename in
  Sqlite3.exec db "PRAGMA synchronous = OFF;" |> check_rc;
  Sqlite3.exec db "PRAGMA journal_mode = MEMORY;" |> check_rc;
  Sqlite3.exec db sql_create_symbols_table |> check_rc;
  Sqlite3.exec db sql_create_indexes |> check_rc;
  Sqlite3.exec db sql_create_kinds_table |> check_rc;
  Sqlite3.exec db sql_create_namespaces_table |> check_rc;
  Sqlite3.exec db sql_insert_kinds |> check_rc;
  Sqlite3.exec db sql_begin_transaction |> check_rc;

  (* Insert namespaces into the database and construct a map to their IDs *)
  let namespace_tbl = Caml.Hashtbl.create 0 in
  let ns_id = ref 1 in

  (* Insert symbols and link them to namespaces *)
  begin
    let stmt = Sqlite3.prepare db sql_insert_symbol in
    List.iter symbols ~f:(fun symbol -> begin

      (* Determine the namespace of this symbol, if any *)
      let (namespace, _name) = Utils.split_ns_from_name symbol.sic_name in
      let nsid_opt = Caml.Hashtbl.find_opt namespace_tbl namespace in
      let nsid = match nsid_opt with
      | Some id -> id
      | None ->
        let id = !ns_id in
        Caml.Hashtbl.add namespace_tbl namespace id;
        incr ns_id;
        id
      in

      (* Insert this symbol *)
      Sqlite3.reset stmt |> check_rc;
      Sqlite3.bind stmt 1 (Sqlite3.Data.INT (Int64.of_int nsid)) |> check_rc;
      Sqlite3.bind stmt 2 (Sqlite3.Data.INT symbol.sic_filehash) |> check_rc;
      Sqlite3.bind stmt 3 (Sqlite3.Data.TEXT symbol.sic_name) |> check_rc;
      Sqlite3.bind stmt 4 (Sqlite3.Data.INT
        (Int64.of_int (kind_to_int symbol.sic_kind))) |> check_rc;
      Sqlite3.step stmt |> check_rc;
    end);
    Sqlite3.finalize stmt |> check_rc;
  end;

  (* We've seen all namespaces, now insert them and their IDs *)
  begin
    let stmt = Sqlite3.prepare db sql_insert_namespace in
    Caml.Hashtbl.iter (fun ns id -> begin
      Caml.Hashtbl.add namespace_tbl ns id;
      Sqlite3.reset stmt |> check_rc;
      Sqlite3.bind stmt 1 (Sqlite3.Data.INT (Int64.of_int id)) |> check_rc;
      Sqlite3.bind stmt 2 (Sqlite3.Data.TEXT ns) |> check_rc;
      Sqlite3.step stmt |> check_rc;
    end) namespace_tbl;
    Sqlite3.finalize stmt |> check_rc;
  end;

  (* Finish up *)
  Sqlite3.exec db sql_commit_transaction |> check_rc;
  if not (Sqlite3.db_close db) then
    failwith ("Unable to close database " ^ filename)
;;
