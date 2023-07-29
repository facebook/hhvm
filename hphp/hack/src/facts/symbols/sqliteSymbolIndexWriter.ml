(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open IndexBuilderTypes
open SearchUtils
open Sqlite_utils

(* Some SQL commands we'll need *)
let sql_begin_transaction = "BEGIN TRANSACTION"

let sql_commit_transaction = "COMMIT TRANSACTION"

let sql_create_symbols_table =
  "CREATE TABLE IF NOT EXISTS symbols ( "
  ^ "    namespace_id INTEGER NOT NULL, "
  ^ "    filename_hash INTEGER NOT NULL, "
  ^ "    name TEXT NOT NULL, "
  ^ "    kind INTEGER NOT NULL, "
  ^ "    valid_for_acid INTEGER NOT NULL, "
  ^ "    valid_for_acnew INTEGER NOT NULL, "
  ^ "    valid_for_actype INTEGER NOT NULL, "
  ^ "    is_abstract INTEGER NOT NULL, "
  ^ "    is_final INTEGER NOT NULL "
  ^ ");"

let sql_create_kinds_table =
  "CREATE TABLE IF NOT EXISTS kinds ( "
  ^ "    id INTEGER NOT NULL PRIMARY KEY, "
  ^ "    description TEXT NOT NULL "
  ^ ");"

let sql_create_namespaces_table =
  "CREATE TABLE IF NOT EXISTS namespaces ( "
  ^ "    namespace_id INTEGER NOT NULL PRIMARY KEY, "
  ^ "    namespace TEXT NOT NULL "
  ^ ");"

let sql_insert_kinds =
  "INSERT OR IGNORE INTO kinds (id, description) VALUES (1, 'Class');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (2, 'Interface');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (3, 'Enum');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (4, 'Trait');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (5, 'Unknown');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (6, 'Mixed');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (7, 'Function');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (8, 'Typedef');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (9, 'Constant');"
  ^ "INSERT OR IGNORE INTO kinds (id, description) VALUES (10, 'XHP Class');"

let sql_insert_symbol =
  "INSERT INTO symbols "
  ^ " (namespace_id, filename_hash, name, kind, valid_for_acid, "
  ^ " valid_for_acnew, valid_for_actype, is_abstract, is_final)"
  ^ " VALUES"
  ^ " (?, ?, ?, ?, ?, ?, ?, ?, ?);"

let sql_insert_namespace =
  "INSERT INTO namespaces "
  ^ " (namespace_id, namespace)"
  ^ " VALUES"
  ^ " (?, ?);"

(*
 * TS 2019-06-17 - Based on testing, creating ANY indexes slows down
 * the performance of sqlite autocomplete.  By eliminating all indexes,
 * we get average query time down to ~20 ms.
 *
 * This certainly isn't what we expected, but please be careful and run
 * performance tests before you add indexes back.
 *
 * CREATE INDEX IF NOT EXISTS ix_symbols_name ON symbols (name);
 *)
let sql_create_indexes = ""

(* Begin the work of creating an SQLite index DB *)
let record_in_db (filename : string) (symbols : si_scan_result) : unit =
  (* If the file exists, remove it before starting over *)
  if Sys.file_exists filename then Unix.unlink filename;

  (* Open the database and do basic prep *)
  let db = Sqlite3.db_open filename in
  Sqlite3.exec db "PRAGMA synchronous = OFF;" |> check_rc db;
  Sqlite3.exec db "PRAGMA journal_mode = MEMORY;" |> check_rc db;
  Sqlite3.exec db sql_create_symbols_table |> check_rc db;
  Sqlite3.exec db sql_create_indexes |> check_rc db;
  Sqlite3.exec db sql_create_kinds_table |> check_rc db;
  Sqlite3.exec db sql_create_namespaces_table |> check_rc db;
  Sqlite3.exec db sql_insert_kinds |> check_rc db;
  Sqlite3.exec db sql_begin_transaction |> check_rc db;

  (* Insert symbols and link them to namespaces *)
  begin
    let stmt = Sqlite3.prepare db sql_insert_symbol in
    List.iter symbols.sisr_capture ~f:(fun symbol ->
        (* Find nsid and filehash *)
        let (ns, _) = Utils.split_ns_from_name symbol.sif_name in
        let nsid = Caml.Hashtbl.find symbols.sisr_namespaces ns in
        let file_hash =
          Caml.Hashtbl.find symbols.sisr_filepaths symbol.sif_filepath
        in
        (* Insert this symbol *)
        Sqlite3.reset stmt |> check_rc db;
        Sqlite3.bind stmt 1 (Sqlite3.Data.INT (Int64.of_int nsid))
        |> check_rc db;
        Sqlite3.bind stmt 2 (Sqlite3.Data.INT file_hash) |> check_rc db;
        Sqlite3.bind stmt 3 (Sqlite3.Data.TEXT symbol.sif_name) |> check_rc db;
        Sqlite3.bind
          stmt
          4
          (Sqlite3.Data.INT
             (Int64.of_int (SearchTypes.kind_to_int symbol.sif_kind)))
        |> check_rc db;
        Sqlite3.bind
          stmt
          5
          (bool_to_sqlite
             (SearchTypes.valid_for_acid symbol.SearchUtils.sif_kind))
        |> check_rc db;
        Sqlite3.bind
          stmt
          6
          (bool_to_sqlite
             (SearchTypes.valid_for_acnew symbol.SearchUtils.sif_kind
             && not symbol.SearchUtils.sif_is_abstract))
        |> check_rc db;
        Sqlite3.bind
          stmt
          7
          (bool_to_sqlite
             (SearchTypes.valid_for_actype symbol.SearchUtils.sif_kind))
        |> check_rc db;
        Sqlite3.bind stmt 8 (bool_to_sqlite symbol.sif_is_abstract)
        |> check_rc db;
        Sqlite3.bind stmt 9 (bool_to_sqlite symbol.sif_is_final) |> check_rc db;
        Sqlite3.step stmt |> check_rc db);
    Sqlite3.finalize stmt |> check_rc db
  end;

  (* We've seen all namespaces, now insert them and their IDs *)
  begin
    let stmt = Sqlite3.prepare db sql_insert_namespace in
    Caml.Hashtbl.iter
      (fun ns id ->
        Sqlite3.reset stmt |> check_rc db;
        Sqlite3.bind stmt 1 (Sqlite3.Data.INT (Int64.of_int id)) |> check_rc db;
        Sqlite3.bind stmt 2 (Sqlite3.Data.TEXT ns) |> check_rc db;
        Sqlite3.step stmt |> check_rc db)
      symbols.sisr_namespaces;
    Sqlite3.finalize stmt |> check_rc db
  end;

  (* Finish up *)
  Sqlite3.exec db sql_commit_transaction |> check_rc db;

  (* Reduces database size by 10% even when we have only one transaction *)
  Sqlite3.exec db "VACUUM;" |> check_rc db;

  (* We are done *)
  if not (Sqlite3.db_close db) then
    failwith ("Unable to close database " ^ filename)
