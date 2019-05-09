(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open SearchUtils

(* Some SQL commands we'll need *)
let sql_begin_transaction =
  "BEGIN TRANSACTION"

let sql_commit_transaction =
  "COMMIT TRANSACTION"

let sql_create_symbols_table =
  "CREATE TABLE IF NOT EXISTS symbols ( " ^
  "    name TEXT NOT NULL, " ^
  "    kind INTEGER NOT NULL " ^
  ");"

let sql_create_kinds_table =
  "CREATE TABLE IF NOT EXISTS kinds ( " ^
  "    id INTEGER NOT NULL, " ^
  "    description TEXT NOT NULL " ^
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
  " (name, kind)" ^
  " VALUES" ^
  " (?, ?);"

let sql_create_indexes =
  "CREATE INDEX IF NOT EXISTS ix_symbols_name ON symbols (name);" ^
  "CREATE INDEX IF NOT EXISTS ix_symbols_kindname ON symbols (kind, name);"

(* Gather a database and prepared statement into a tuple *)
let prepare_or_reset_statement (db) (stmt_ref) (sql_command_text) =
  let stmt = match !stmt_ref with
    | Some s ->
      let _ = Sqlite3.reset s in
      s
    | None ->
      let s = Sqlite3.prepare db sql_command_text in
      stmt_ref := Some s;
      s
  in
  stmt
;;

(* Begin the work of creating an SQLite index DB *)
let record_in_db
    (filename: string)
    (symbols: si_results): unit =

  (* Open the database and do basic prep *)
  let db = Sqlite3.db_open filename in
  let _ = Sqlite3.exec db sql_create_symbols_table in
  let _ = Sqlite3.exec db sql_create_indexes in
  let _ = Sqlite3.exec db sql_create_kinds_table in
  let _ = Sqlite3.exec db sql_insert_kinds in
  let _ = Sqlite3.exec db sql_begin_transaction in

  (* Insert records *)
  let insert_symbol_stmt = ref None in
  Core_kernel.List.iter symbols ~f:(fun symbol -> begin
        let stmt = prepare_or_reset_statement db insert_symbol_stmt sql_insert_symbol in
        let _ = Sqlite3.bind stmt 1 (Sqlite3.Data.TEXT symbol.si_name) in
        let _ = Sqlite3.bind stmt 2 (Sqlite3.Data.INT
                                       (Int64.of_int (kind_to_int symbol.si_kind))) in
        let _ = Sqlite3.step stmt in
        ()
      end);

  (* Finish up *)
  let _ = Sqlite3.exec db sql_commit_transaction in
  let _ = Sqlite3.db_close db in
  ()
;;
