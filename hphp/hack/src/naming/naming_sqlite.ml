(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Sqlite_utils

type db_path = Db_path of string [@@deriving show]

type save_result = {
  files_added: int;
  symbols_added: int;
}

let empty_save_result = { files_added = 0; symbols_added = 0 }

type 'a forward_naming_table_delta =
  | Modified of 'a
  | Deleted
[@@deriving show]

type file_deltas = FileInfo.t forward_naming_table_delta Relative_path.Map.t

let pp_file_deltas =
  Relative_path.Map.make_pp
    Relative_path.pp
    (pp_forward_naming_table_delta FileInfo.pp)

type local_changes = {
  file_deltas: file_deltas;
  base_content_version: string;
}
[@@deriving show]

let _ = show_forward_naming_table_delta

let _ = show_local_changes

let make_relative_path ~prefix_int ~suffix =
  let prefix =
    Core_kernel.Option.value_exn
      (Relative_path.prefix_of_enum (Int64.to_int prefix_int))
  in
  let full_suffix =
    Filename.concat (Relative_path.path_of_prefix prefix) suffix
  in
  Relative_path.create prefix full_suffix

let to_canon_name_key = String.lowercase_ascii

let fold_sqlite stmt ~f ~init =
  let rec helper acc =
    match Sqlite3.step stmt with
    | Sqlite3.Rc.DONE -> acc
    | Sqlite3.Rc.ROW -> helper (f stmt acc)
    | rc ->
      failwith
        (Printf.sprintf "SQLite operation failed: %s" (Sqlite3.Rc.to_string rc))
  in
  helper init

(* It's kind of weird to just dump this into the SQLite database, but removing
 * entries one at a time during a normal incremental update leads to too many
 * table scans, and I don't want to put this in a separate file because then
 * the naming table and dep table in a given SQLite database might not agree
 * with each other, and that just feels weird. *)
module LocalChanges = struct
  let table_name = "NAMING_LOCAL_CHANGES"

  let create_table_sqlite =
    Printf.sprintf
      "
          CREATE TABLE IF NOT EXISTS %s(
            ID INTEGER PRIMARY KEY,
            LOCAL_CHANGES BLOB NOT NULL,
            BASE_CONTENT_VERSION TEXT
          );
        "
      table_name

  let insert_sqlite =
    Printf.sprintf
      "
          INSERT INTO %s (ID, LOCAL_CHANGES, BASE_CONTENT_VERSION) VALUES (0, ?, ?);
        "
      table_name

  let update_sqlite =
    Printf.sprintf
      "
          UPDATE %s SET LOCAL_CHANGES = ? WHERE ID = 0;
        "
      table_name

  let get_sqlite =
    Printf.sprintf
      "
          SELECT LOCAL_CHANGES, BASE_CONTENT_VERSION FROM %s;
        "
      table_name

  let prime db base_content_version =
    let insert_stmt = Sqlite3.prepare db insert_sqlite in
    let empty =
      Marshal.to_string Relative_path.Map.empty [Marshal.No_sharing]
    in
    Sqlite3.bind insert_stmt 1 (Sqlite3.Data.BLOB empty) |> check_rc db;
    Sqlite3.bind insert_stmt 2 (Sqlite3.Data.TEXT base_content_version)
    |> check_rc db;
    Sqlite3.step insert_stmt |> check_rc db;
    Sqlite3.finalize insert_stmt |> check_rc db

  let update db (local_changes : local_changes) =
    let local_changes_saved =
      Relative_path.Map.map local_changes.file_deltas ~f:(fun delta ->
          match delta with
          | Modified fi -> Modified (FileInfo.to_saved fi)
          | Deleted -> Deleted)
    in
    let local_changes_blob =
      Marshal.to_string local_changes_saved [Marshal.No_sharing]
    in
    let update_stmt = Sqlite3.prepare db update_sqlite in
    Sqlite3.bind update_stmt 1 (Sqlite3.Data.BLOB local_changes_blob)
    |> check_rc db;
    Sqlite3.step update_stmt |> check_rc db;
    Sqlite3.finalize update_stmt |> check_rc db

  let get db =
    let get_stmt = Sqlite3.prepare db get_sqlite in
    match Sqlite3.step get_stmt with
    (* We don't include Sqlite3.Rc.DONE in this match because we always expect
     * exactly one row. *)
    | Sqlite3.Rc.ROW ->
      let local_changes_blob = column_blob get_stmt 0 in
      let local_changes_saved = Marshal.from_string local_changes_blob 0 in
      let base_content_version = column_str get_stmt 1 in
      let file_deltas =
        Relative_path.Map.mapi local_changes_saved ~f:(fun path delta ->
            match delta with
            | Modified saved -> Modified (FileInfo.from_saved path saved)
            | Deleted -> Deleted)
      in
      { base_content_version; file_deltas }
    | rc ->
      failwith
        (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))
end

(* These are just done as modules to keep the SQLite for related tables close together. *)
module FileInfoTable = struct
  let table_name = "NAMING_FILE_INFO"

  let create_table_sqlite =
    Printf.sprintf
      "
      CREATE TABLE IF NOT EXISTS %s(
        FILE_INFO_ID INTEGER PRIMARY KEY AUTOINCREMENT,
        PATH_PREFIX_TYPE INTEGER NOT NULL,
        PATH_SUFFIX TEXT NOT NULL,
        TYPE_CHECKER_MODE INTEGER,
        DECL_HASH BLOB,
        CLASSES TEXT,
        CONSTS TEXT,
        FUNS TEXT,
        RECS TEXT,
        TYPEDEFS TEXT
      );
      "
      table_name

  let create_index_sqlite =
    Printf.sprintf
      "
      CREATE UNIQUE INDEX IF NOT EXISTS FILE_INFO_PATH_IDX ON %s (PATH_SUFFIX, PATH_PREFIX_TYPE);
      "
      table_name

  let insert_sqlite =
    Printf.sprintf
      "
        INSERT INTO %s(
          PATH_PREFIX_TYPE,
          PATH_SUFFIX,
          TYPE_CHECKER_MODE,
          DECL_HASH,
          CLASSES,
          CONSTS,
          FUNS,
          RECS,
          TYPEDEFS
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
      "
      table_name

  let get_sqlite =
    Printf.sprintf
      "
        SELECT
          TYPE_CHECKER_MODE, DECL_HASH, CLASSES, CONSTS, FUNS, RECS, TYPEDEFS
        FROM %s
        WHERE PATH_PREFIX_TYPE = ? AND PATH_SUFFIX = ?;
      "
      table_name

  let iter_sqlite =
    Printf.sprintf
      "
        SELECT
          PATH_PREFIX_TYPE,
          PATH_SUFFIX,
          TYPE_CHECKER_MODE,
          DECL_HASH,
          CLASSES,
          CONSTS,
          FUNS,
          RECS,
          TYPEDEFS
        FROM %s
        ORDER BY PATH_PREFIX_TYPE, PATH_SUFFIX;
      "
      table_name

  let insert
      db
      relative_path
      ~(type_checker_mode : FileInfo.mode option)
      ~(decl_hash : OpaqueDigest.t option)
      ~(classes : FileInfo.id list)
      ~(consts : FileInfo.id list)
      ~(funs : FileInfo.id list)
      ~(recs : FileInfo.id list)
      ~(typedefs : FileInfo.id list) =
    let prefix_type =
      Sqlite3.Data.INT
        (Int64.of_int
           (Relative_path.prefix_to_enum (Relative_path.prefix relative_path)))
    in
    let suffix = Sqlite3.Data.TEXT (Relative_path.suffix relative_path) in
    let type_checker_mode =
      match type_checker_mode with
      | Some type_checker_mode ->
        Sqlite3.Data.INT
          (Int64.of_int (FileInfo.mode_to_enum type_checker_mode))
      | None -> Sqlite3.Data.NULL
    in
    let decl_hash =
      match decl_hash with
      | Some decl_hash ->
        Sqlite3.Data.BLOB (OpaqueDigest.to_raw_contents decl_hash)
      | None -> Sqlite3.Data.NULL
    in
    let names_to_data_type names =
      let open Core_kernel in
      let names = String.concat ~sep:"|" (List.map names ~f:snd) in
      match String.length names with
      | 0 -> Sqlite3.Data.NULL
      | _ -> Sqlite3.Data.TEXT names
    in
    let insert_stmt = Sqlite3.prepare db insert_sqlite in
    Sqlite3.bind insert_stmt 1 prefix_type |> check_rc db;

    Sqlite3.bind insert_stmt 2 suffix |> check_rc db;

    Sqlite3.bind insert_stmt 3 type_checker_mode |> check_rc db;

    Sqlite3.bind insert_stmt 4 decl_hash |> check_rc db;
    Sqlite3.bind insert_stmt 5 (names_to_data_type classes) |> check_rc db;
    Sqlite3.bind insert_stmt 6 (names_to_data_type consts) |> check_rc db;
    Sqlite3.bind insert_stmt 7 (names_to_data_type funs) |> check_rc db;
    Sqlite3.bind insert_stmt 8 (names_to_data_type recs) |> check_rc db;
    Sqlite3.bind insert_stmt 9 (names_to_data_type typedefs) |> check_rc db;
    Sqlite3.step insert_stmt |> check_rc db;
    Sqlite3.finalize insert_stmt |> check_rc db

  let read_row ~stmt ~path ~base_index =
    let file_mode =
      FileInfo.mode_of_enum (Int64.to_int (column_int64 stmt base_index))
    in
    let hash =
      OpaqueDigest.from_raw_contents (column_blob stmt (base_index + 1))
    in
    let to_ids ~value ~name_type =
      match value with
      | Sqlite3.Data.TEXT s ->
        Core_kernel.(
          List.map (String.split s ~on:'|') ~f:(fun name ->
              (FileInfo.File (name_type, path), name)))
      | Sqlite3.Data.NULL -> []
      | _ -> failwith "Unexpected column type when retrieving names"
    in
    let classes =
      to_ids
        ~value:(Sqlite3.column stmt (base_index + 2))
        ~name_type:FileInfo.Class
    in
    let consts =
      to_ids
        ~value:(Sqlite3.column stmt (base_index + 3))
        ~name_type:FileInfo.Const
    in
    let funs =
      to_ids
        ~value:(Sqlite3.column stmt (base_index + 4))
        ~name_type:FileInfo.Fun
    in
    let record_defs =
      to_ids
        ~value:(Sqlite3.column stmt (base_index + 5))
        ~name_type:FileInfo.RecordDef
    in
    let typedefs =
      to_ids
        ~value:(Sqlite3.column stmt (base_index + 6))
        ~name_type:FileInfo.Typedef
    in
    FileInfo.
      {
        hash;
        file_mode;
        funs;
        classes;
        record_defs;
        typedefs;
        consts;
        comments = None;
      }

  let get_file_info db path =
    let get_stmt = Sqlite3.prepare db get_sqlite in
    let prefix_type =
      Relative_path.prefix_to_enum (Relative_path.prefix path)
    in
    let suffix = Relative_path.suffix path in
    Sqlite3.bind get_stmt 1 (Sqlite3.Data.INT (Int64.of_int prefix_type))
    |> check_rc db;
    Sqlite3.bind get_stmt 2 (Sqlite3.Data.TEXT suffix) |> check_rc db;
    match Sqlite3.step get_stmt with
    | Sqlite3.Rc.DONE -> None
    | Sqlite3.Rc.ROW -> Some (read_row ~stmt:get_stmt ~path ~base_index:0)
    | rc ->
      failwith
        (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))

  let fold db ~init ~f =
    let iter_stmt = Sqlite3.prepare db iter_sqlite in
    let f iter_stmt acc =
      let prefix_int = column_int64 iter_stmt 0 in
      let suffix = column_str iter_stmt 1 in
      let path = make_relative_path ~prefix_int ~suffix in
      let fi = read_row ~stmt:iter_stmt ~path ~base_index:2 in
      f path fi acc
    in
    fold_sqlite iter_stmt ~f ~init
end

module TypesTable = struct
  let table_name = "NAMING_TYPES"

  let class_flag =
    Int64.of_int (Naming_types.kind_of_type_to_enum Naming_types.TClass)

  let typedef_flag =
    Int64.of_int (Naming_types.kind_of_type_to_enum Naming_types.TTypedef)

  let create_table_sqlite =
    Printf.sprintf
      "
        CREATE TABLE IF NOT EXISTS %s(
          HASH INTEGER PRIMARY KEY NOT NULL,
          CANON_HASH INTEGER NOT NULL,
          FLAGS INTEGER NOT NULL,
          FILE_INFO_ID INTEGER NOT NULL
        );
      "
      table_name

  let create_index_sqlite =
    Printf.sprintf
      "
      CREATE INDEX IF NOT EXISTS TYPES_CANON ON %s (CANON_HASH);
      "
      table_name

  let insert_sqlite =
    Printf.sprintf
      "
        INSERT INTO %s(
          HASH,
          CANON_HASH,
          FLAGS,
          FILE_INFO_ID)
        VALUES (?, ?, ?, ?);
      "
      table_name

  let (get_sqlite, get_sqlite_case_insensitive) =
    let base =
      Str.global_replace
        (Str.regexp "{table_name}")
        table_name
        "
        SELECT
          NAMING_FILE_INFO.PATH_PREFIX_TYPE,
          NAMING_FILE_INFO.PATH_SUFFIX,
          {table_name}.FLAGS
        FROM {table_name}
        LEFT JOIN NAMING_FILE_INFO ON
          {table_name}.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE {table_name}.{hash} = ?"
    in
    ( Str.global_replace (Str.regexp "{hash}") "HASH" base,
      Str.global_replace (Str.regexp "{hash}") "CANON_HASH" base )

  let insert db ~name ~flags ~file_info_id =
    let hash = SharedMem.get_hash name in
    let canon_hash = SharedMem.get_hash (to_canon_name_key name) in
    let insert_stmt = Sqlite3.prepare db insert_sqlite in
    Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc db;
    Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT canon_hash) |> check_rc db;
    Sqlite3.bind insert_stmt 3 (Sqlite3.Data.INT flags) |> check_rc db;
    Sqlite3.bind insert_stmt 4 (Sqlite3.Data.INT file_info_id) |> check_rc db;
    Sqlite3.step insert_stmt |> check_rc db;
    Sqlite3.finalize insert_stmt |> check_rc db

  let insert_class db ~name ~file_info_id =
    insert db ~name ~flags:class_flag ~file_info_id

  let insert_typedef db ~name ~file_info_id =
    insert db ~name ~flags:typedef_flag ~file_info_id

  let get db ~name ~case_insensitive =
    let name =
      if case_insensitive then
        String.lowercase_ascii name
      else
        name
    in
    let hash = SharedMem.get_hash name in
    let get_sqlite =
      if case_insensitive then
        get_sqlite_case_insensitive
      else
        get_sqlite
    in
    let get_stmt = Sqlite3.prepare db get_sqlite in
    Sqlite3.bind get_stmt 1 (Sqlite3.Data.INT hash) |> check_rc db;
    match Sqlite3.step get_stmt with
    | Sqlite3.Rc.DONE -> None
    | Sqlite3.Rc.ROW ->
      let prefix_type = column_int64 get_stmt 0 in
      let suffix = column_str get_stmt 1 in
      let flags = Int64.to_int (column_int64 get_stmt 2) in
      let class_type =
        Core_kernel.Option.value_exn (Naming_types.kind_of_type_of_enum flags)
      in
      Some (make_relative_path prefix_type suffix, class_type)
    | rc ->
      failwith
        (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))
end

module FunsTable = struct
  let table_name = "NAMING_FUNS"

  let create_table_sqlite =
    Printf.sprintf
      "
        CREATE TABLE IF NOT EXISTS %s(
          HASH INTEGER PRIMARY KEY NOT NULL,
          CANON_HASH INTEGER NOT NULL,
          FILE_INFO_ID INTEGER NOT NULL
        );
      "
      table_name

  let create_index_sqlite =
    Printf.sprintf
      "
      CREATE INDEX IF NOT EXISTS FUNS_CANON ON %s (CANON_HASH);
      "
      table_name

  let insert_sqlite =
    Printf.sprintf
      "
        INSERT INTO %s (HASH, CANON_HASH, FILE_INFO_ID) VALUES (?, ?, ?);
      "
      table_name

  let (get_sqlite, get_sqlite_case_insensitive) =
    let base =
      Str.global_replace
        (Str.regexp "{table_name}")
        table_name
        "
        SELECT
          NAMING_FILE_INFO.PATH_PREFIX_TYPE,
          NAMING_FILE_INFO.PATH_SUFFIX
        FROM {table_name}
        LEFT JOIN NAMING_FILE_INFO ON
          {table_name}.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE {table_name}.{hash} = ?"
    in
    ( Str.global_replace (Str.regexp "{hash}") "HASH" base,
      Str.global_replace (Str.regexp "{hash}") "CANON_HASH" base )

  let insert db ~name ~file_info_id =
    let hash = SharedMem.get_hash name in
    let canon_hash = SharedMem.get_hash (to_canon_name_key name) in
    let insert_stmt = Sqlite3.prepare db insert_sqlite in
    Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc db;
    Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT canon_hash) |> check_rc db;
    Sqlite3.bind insert_stmt 3 (Sqlite3.Data.INT file_info_id) |> check_rc db;
    Sqlite3.step insert_stmt |> check_rc db;
    Sqlite3.finalize insert_stmt |> check_rc db

  let get db ~name ~case_insensitive =
    let name =
      if case_insensitive then
        String.lowercase_ascii name
      else
        name
    in
    let hash = SharedMem.get_hash name in
    let get_sqlite =
      if case_insensitive then
        get_sqlite_case_insensitive
      else
        get_sqlite
    in
    let get_stmt = Sqlite3.prepare db get_sqlite in
    Sqlite3.bind get_stmt 1 (Sqlite3.Data.INT hash) |> check_rc db;
    match Sqlite3.step get_stmt with
    | Sqlite3.Rc.DONE -> None
    | Sqlite3.Rc.ROW ->
      let prefix_type = column_int64 get_stmt 0 in
      let suffix = column_str get_stmt 1 in
      Some (make_relative_path prefix_type suffix)
    | rc ->
      failwith
        (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))
end

module ConstsTable = struct
  let table_name = "NAMING_CONSTS"

  let create_table_sqlite =
    Printf.sprintf
      "
        CREATE TABLE IF NOT EXISTS %s(
          HASH INTEGER PRIMARY KEY NOT NULL,
          FILE_INFO_ID INTEGER NOT NULL
        );
      "
      table_name

  let insert_sqlite =
    Printf.sprintf
      "
        INSERT INTO %s (HASH, FILE_INFO_ID) VALUES (?, ?);
      "
      table_name

  let get_sqlite =
    Str.global_replace
      (Str.regexp "{table_name}")
      table_name
      "
        SELECT
          NAMING_FILE_INFO.PATH_PREFIX_TYPE,
          NAMING_FILE_INFO.PATH_SUFFIX
        FROM {table_name}
        LEFT JOIN NAMING_FILE_INFO ON
          {table_name}.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE {table_name}.HASH = ?
      "

  let insert db ~name ~file_info_id =
    let hash = SharedMem.get_hash name in
    let insert_stmt = Sqlite3.prepare db insert_sqlite in
    Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc db;
    Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT file_info_id) |> check_rc db;
    Sqlite3.step insert_stmt |> check_rc db;
    Sqlite3.finalize insert_stmt |> check_rc db

  let get db ~name =
    let hash = SharedMem.get_hash name in
    let get_stmt = Sqlite3.prepare db get_sqlite in
    Sqlite3.bind get_stmt 1 (Sqlite3.Data.INT hash) |> check_rc db;
    match Sqlite3.step get_stmt with
    | Sqlite3.Rc.DONE -> None
    | Sqlite3.Rc.ROW ->
      let prefix_type = column_int64 get_stmt 0 in
      let suffix = column_str get_stmt 1 in
      Some (make_relative_path prefix_type suffix)
    | rc ->
      failwith
        (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))
end

module Database_handle : sig
  val get_db_path : unit -> string option

  val set_db_path : string option -> unit

  val is_connected : unit -> bool

  val get_db : unit -> Sqlite3.db option
end = struct
  module Shared_db_settings =
    SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey)
      (struct
        type t = string

        let prefix = Prefix.make ()

        let description = "NamingTableDatabaseSettings"
      end)

  let open_db () =
    match Shared_db_settings.get "database_path" with
    | None -> None
    | Some path ->
      let db = Sqlite3.db_open path in
      Sqlite3.exec db "PRAGMA synchronous = OFF;" |> check_rc db;
      Sqlite3.exec db "PRAGMA journal_mode = MEMORY;" |> check_rc db;
      Some db

  let db = ref (lazy (open_db ()))

  let get_db_path () : string option = Shared_db_settings.get "database_path"

  let set_db_path path =
    Shared_db_settings.remove_batch (SSet.singleton "database_path");

    (match path with
    | Some path -> Shared_db_settings.add "database_path" path
    | None -> ());

    (* Force this immediately so that we can get validation errors in master. *)
    db := Lazy.from_val (open_db ())

  let get_db () = Lazy.force !db

  let is_connected () = get_db () <> None
end

let save_file_info db relative_path file_info =
  Core_kernel.(
    FileInfoTable.insert
      db
      relative_path
      ~type_checker_mode:file_info.FileInfo.file_mode
      ~decl_hash:file_info.FileInfo.hash
      ~consts:file_info.FileInfo.consts
      ~classes:file_info.FileInfo.classes
      ~funs:file_info.FileInfo.funs
      ~recs:file_info.FileInfo.record_defs
      ~typedefs:file_info.FileInfo.typedefs;
    let file_info_id = ref None in
    Sqlite3.exec_not_null_no_headers
      db
      "SELECT last_insert_rowid()"
      ~cb:(fun row ->
        match row with
        | [| row_id |] -> file_info_id := Some (Int64.of_string row_id)
        | [||] -> failwith "Got no columns when querying last inserted row ID"
        | _ ->
          failwith "Got too many columns when querying last inserted row ID")
    |> check_rc db;
    let file_info_id =
      match !file_info_id with
      | Some id -> id
      | None -> failwith "Could not get last inserted row ID"
    in
    let symbols_inserted = 0 in
    let symbols_inserted =
      List.fold
        ~init:symbols_inserted
        ~f:(fun acc (_, name) ->
          FunsTable.insert db ~name ~file_info_id;
          acc + 1)
        file_info.FileInfo.funs
    in
    let symbols_inserted =
      List.fold
        ~init:symbols_inserted
        ~f:(fun acc (_, name) ->
          TypesTable.insert_class db ~name ~file_info_id;
          acc + 1)
        file_info.FileInfo.classes
    in
    let symbols_inserted =
      List.fold
        ~init:symbols_inserted
        ~f:(fun acc (_, name) ->
          TypesTable.insert_typedef db ~name ~file_info_id;
          acc + 1)
        file_info.FileInfo.typedefs
    in
    let symbols_inserted =
      List.fold
        ~init:symbols_inserted
        ~f:(fun acc (_, name) ->
          ConstsTable.insert db ~name ~file_info_id;
          acc + 1)
        file_info.FileInfo.consts
    in
    symbols_inserted)

let get_db_path = Database_handle.get_db_path

let set_db_path = Database_handle.set_db_path

let is_connected = Database_handle.is_connected

let save_file_infos db_name file_info_map ~base_content_version =
  let db = Sqlite3.db_open db_name in
  Sqlite3.exec db "BEGIN TRANSACTION;" |> check_rc db;
  try
    Sqlite3.exec db LocalChanges.create_table_sqlite |> check_rc db;
    Sqlite3.exec db FileInfoTable.create_table_sqlite |> check_rc db;
    Sqlite3.exec db ConstsTable.create_table_sqlite |> check_rc db;
    Sqlite3.exec db TypesTable.create_table_sqlite |> check_rc db;
    Sqlite3.exec db FunsTable.create_table_sqlite |> check_rc db;

    (* Incremental updates only update the single row in this table, so we need
     * to write in some dummy data to start. *)
    LocalChanges.prime db base_content_version;
    let save_result =
      Relative_path.Map.fold
        file_info_map
        ~init:empty_save_result
        ~f:(fun path fi acc ->
          {
            files_added = acc.files_added + 1;
            symbols_added = acc.symbols_added + save_file_info db path fi;
          })
    in
    Sqlite3.exec db FileInfoTable.create_index_sqlite |> check_rc db;
    Sqlite3.exec db TypesTable.create_index_sqlite |> check_rc db;
    Sqlite3.exec db FunsTable.create_index_sqlite |> check_rc db;
    Sqlite3.exec db "END TRANSACTION;" |> check_rc db;
    if not @@ Sqlite3.db_close db then
      failwith @@ Printf.sprintf "Could not close database at %s" db_name;
    Database_handle.set_db_path None;
    save_result
  with e ->
    Sqlite3.exec db "END TRANSACTION;" |> check_rc db;
    raise e

let update_file_infos db_name local_changes =
  Database_handle.set_db_path (Some db_name);
  let db =
    match Database_handle.get_db () with
    | Some db -> db
    | None -> failwith @@ Printf.sprintf "Could not connect to %s" db_name
  in
  LocalChanges.update db local_changes

let get_local_changes () =
  let db =
    match Database_handle.get_db () with
    | Some db -> db
    | None ->
      failwith @@ Printf.sprintf "Attempted to access non-connected database"
  in
  LocalChanges.get db

let fold ~init ~f ~file_deltas =
  (* We depend on [Relative_path.Map.bindings] returning results in increasing
   * order here. *)
  let sorted_changes = Relative_path.Map.bindings file_deltas in
  (* This function looks kind of funky, but it's not too terrible when
   * explained. We essentially have two major inputs:
   *   1. a path and file info that we got from SQLite. In order for this
   *      function to work properly, we require that consecutive invocations
   *      of this function within the same [fold] call have paths that are
   *      strictly increasing.
   *   2. the list of local changes. This list is also required to be in
   *      increasing sorted order.
   * In order to make sure that we a) return results in sorted order, and
   * b) properly handle files that have been added (and therefore will never
   * show up in the [path] and [fi] args), we essentially walk both lists in
   * sorted order (walking the SQLite list through successive invocations and
   * walking the local changes list through recursion), at each step folding
   * over whichever entry has the lowest sort order. If both paths are
   * identical, we use the one from our local changes. In this way we get O(n)
   * iteration while handling additions, modifications, and deletions
   * properly (again, given our input sorting restraints). *)
  let rec consume_sorted_changes path fi (sorted_changes, acc) =
    match sorted_changes with
    | hd :: tl when fst hd < path ->
      begin
        match snd hd with
        | Modified local_fi ->
          consume_sorted_changes path fi (tl, f (fst hd) local_fi acc)
        | Deleted -> consume_sorted_changes path fi (tl, acc)
      end
    | hd :: tl when fst hd = path ->
      begin
        match snd hd with
        | Modified fi -> (tl, f path fi acc)
        | Deleted -> (tl, acc)
      end
    | _ -> (sorted_changes, f path fi acc)
  in
  let db =
    match Database_handle.get_db () with
    | Some db -> db
    | None -> failwith "Attempted to access non-connected database"
  in
  let (remaining_changes, acc) =
    FileInfoTable.fold db ~init:(sorted_changes, init) ~f:consume_sorted_changes
  in
  List.fold_left
    begin
      fun acc (path, delta) ->
      match delta with
      | Modified fi -> f path fi acc
      | Deleted -> (* This probably shouldn't happen? *) acc
    end
    acc
    remaining_changes

let get_file_info path =
  match Database_handle.get_db () with
  | None -> None
  | Some db -> FileInfoTable.get_file_info db path

let get_type_pos name ~case_insensitive =
  match Database_handle.get_db () with
  | None -> None
  | Some db -> TypesTable.get db ~name ~case_insensitive

let get_fun_pos name ~case_insensitive =
  match Database_handle.get_db () with
  | None -> None
  | Some db -> FunsTable.get db ~name ~case_insensitive

let get_const_pos name =
  match Database_handle.get_db () with
  | None -> None
  | Some db -> ConstsTable.get db ~name
