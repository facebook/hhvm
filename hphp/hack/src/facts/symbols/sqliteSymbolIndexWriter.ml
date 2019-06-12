(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel
open IndexBuilderTypes
open SearchUtils
open Sqlite_utils

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
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (8, 'Typedef');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (9, 'Constant');" ^
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (10, 'XHP Class');"

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

(* Begin the work of creating an SQLite index DB *)
let record_in_db
    (filename: string)
    (symbols: si_scan_result): unit =

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

  (* Insert symbols and link them to namespaces *)
  begin
    let stmt = Sqlite3.prepare db sql_insert_symbol in
    List.iter symbols.sisr_capture ~f:(fun symbol -> begin

      (* Find nsid and filehash *)
      let (ns, _) = Utils.split_ns_from_name symbol.sif_name in
      let nsid = Caml.Hashtbl.find symbols.sisr_namespaces ns in
      let file_hash = Caml.Hashtbl.find symbols.sisr_filepaths symbol.sif_filepath in

      (* Insert this symbol *)
      Sqlite3.reset stmt |> check_rc;
      Sqlite3.bind stmt 1 (Sqlite3.Data.INT (Int64.of_int nsid)) |> check_rc;
      Sqlite3.bind stmt 2 (Sqlite3.Data.INT file_hash) |> check_rc;
      Sqlite3.bind stmt 3 (Sqlite3.Data.TEXT symbol.sif_name) |> check_rc;
      Sqlite3.bind stmt 4 (Sqlite3.Data.INT
        (Int64.of_int (kind_to_int symbol.sif_kind))) |> check_rc;
      Sqlite3.step stmt |> check_rc;
    end);
    Sqlite3.finalize stmt |> check_rc;
  end;

  (* We've seen all namespaces, now insert them and their IDs *)
  begin
    let stmt = Sqlite3.prepare db sql_insert_namespace in
    Caml.Hashtbl.iter (fun ns id -> begin
      Sqlite3.reset stmt |> check_rc;
      Sqlite3.bind stmt 1 (Sqlite3.Data.INT (Int64.of_int id)) |> check_rc;
      Sqlite3.bind stmt 2 (Sqlite3.Data.TEXT ns) |> check_rc;
      Sqlite3.step stmt |> check_rc;
    end) symbols.sisr_namespaces;
    Sqlite3.finalize stmt |> check_rc;
  end;

  (* Finish up *)
  Sqlite3.exec db sql_commit_transaction |> check_rc;

  (* Reduces database size by 10% even when we have only one transaction *)
  Sqlite3.exec db "VACUUM;" |> check_rc;

  (* We are done *)
  if not (Sqlite3.db_close db) then
    failwith ("Unable to close database " ^ filename)
;;
