(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = FileInfo.t Relative_path.Map.t
type fast = FileInfo.names Relative_path.Map.t
type saved_state_info = FileInfo.saved Relative_path.Map.t

(* The canon name (and assorted *Canon heaps) store the canonical name for a
   symbol, keyed off of the lowercase version of its name. We use the canon
   heaps to check for symbols which are redefined using different
   capitalizations so we can throw proper Hack errors for them. *)
let to_canon_name_key = String.lowercase_ascii
let canonize_set = SSet.map to_canon_name_key


module Sqlite : sig
  val create_database :
    string ->
    ((int64 -> Relative_path.t -> FileInfo.t -> int) -> unit) ->
    unit
end = struct
  let check_rc rc =
    if rc <> Sqlite3.Rc.OK && rc <> Sqlite3.Rc.DONE
    then failwith (Printf.sprintf "SQLite operation failed: %s" (Sqlite3.Rc.to_string rc))

  (* These are just done as modules to keep the SQLite for related tables close together. *)
  module FileInfoTable = struct
    let table_name = "NAMING_FILE_INFO"

    let create_table_sqlite =
      Printf.sprintf "
      CREATE TABLE IF NOT EXISTS %s(
        PRIMARY_KEY INTEGER PRIMARY KEY NOT NULL,
        PATH_PREFIX_TYPE INTEGER NOT NULL,
        PATH_SUFFIX TEXT NOT NULL
      );
      " table_name

    let insert_sqlite =
      Printf.sprintf "
        INSERT INTO %s (PRIMARY_KEY, PATH_PREFIX_TYPE, PATH_SUFFIX) VALUES (?, ?, ?);
      " table_name

    let insert db primary_key relative_path =
      let prefix_type = Relative_path.prefix_to_enum (Relative_path.prefix relative_path) in
      let suffix = Relative_path.suffix relative_path in
      let insert_stmt = Sqlite3.prepare db insert_sqlite in
      Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT primary_key) |> check_rc;
      Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT (Int64.of_int prefix_type)) |> check_rc;
      Sqlite3.bind insert_stmt 3 (Sqlite3.Data.TEXT suffix) |> check_rc;
      Sqlite3.step insert_stmt |> check_rc;
      Sqlite3.finalize insert_stmt |> check_rc
  end

  module TypesTable = struct
    let table_name = "NAMING_TYPES"
    let class_flag = Int64.zero
    let typedef_flag = Int64.one

    let create_table_sqlite =
      Printf.sprintf "
        CREATE TABLE IF NOT EXISTS %s(
          HASH INTEGER PRIMARY KEY NOT NULL,
          CANON_HASH INTEGER NOT NULL,
          FLAGS INTEGER NOT NULL,
          FILE_INFO_PK INTEGER NOT NULL
        );
      " table_name

    let insert_sqlite =
      Printf.sprintf "
        INSERT INTO %s (HASH, CANON_HASH, FLAGS, FILE_INFO_PK) VALUES (?, ?, ?, ?);
      " table_name

    let insert db ~name ~flags ~file_info_pk =
      let hash = SharedMem.get_hash name in
      let canon_hash = SharedMem.get_hash (to_canon_name_key name) in
      let insert_stmt = Sqlite3.prepare db insert_sqlite in
      Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT canon_hash) |> check_rc;
      Sqlite3.bind insert_stmt 3 (Sqlite3.Data.INT flags) |> check_rc;
      Sqlite3.bind insert_stmt 4 (Sqlite3.Data.INT file_info_pk) |> check_rc;
      Sqlite3.step insert_stmt |> check_rc;
      Sqlite3.finalize insert_stmt |> check_rc

    let insert_class db ~name ~file_info_pk =
      insert db ~name ~flags:class_flag ~file_info_pk

    let insert_typedef db ~name ~file_info_pk =
      insert db ~name ~flags:typedef_flag ~file_info_pk
  end

  module FunsTable = struct
    let table_name = "NAMING_FUNS"

    let create_table_sqlite =
      Printf.sprintf "
        CREATE TABLE IF NOT EXISTS %s(
          HASH INTEGER PRIMARY KEY NOT NULL,
          CANON_HASH INTEGER NOT NULL,
          FILE_INFO_PK INTEGER NOT NULL
        );
      " table_name

    let insert_sqlite =
      Printf.sprintf "
        INSERT INTO %s (HASH, CANON_HASH, FILE_INFO_PK) VALUES (?, ?, ?);
      " table_name

    let insert db ~name ~file_info_pk =
      let hash = SharedMem.get_hash name in
      let canon_hash = SharedMem.get_hash (to_canon_name_key name) in
      let insert_stmt = Sqlite3.prepare db insert_sqlite in
      Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT canon_hash) |> check_rc;
      Sqlite3.bind insert_stmt 3 (Sqlite3.Data.INT file_info_pk) |> check_rc;
      Sqlite3.step insert_stmt |> check_rc;
      Sqlite3.finalize insert_stmt |> check_rc
  end

  module ConstsTable = struct
    let table_name = "NAMING_CONSTS"

    let create_table_sqlite =
      Printf.sprintf "
        CREATE TABLE IF NOT EXISTS %s(
          HASH INTEGER PRIMARY KEY NOT NULL,
          FILE_INFO_PK INTEGER NOT NULL
        );
      " table_name

    let insert_sqlite =
      Printf.sprintf "
        INSERT INTO %s (HASH, FILE_INFO_PK) VALUES (?, ?);
      " table_name

    let insert db ~name ~file_info_pk =
      let hash = SharedMem.get_hash name in
      let insert_stmt = Sqlite3.prepare db insert_sqlite in
      Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT file_info_pk) |> check_rc;
      Sqlite3.step insert_stmt |> check_rc;
      Sqlite3.finalize insert_stmt |> check_rc
  end

  let save_file_info db file_info_pk relative_path file_info =
    let open Core_kernel in
    FileInfoTable.insert db file_info_pk relative_path;
    let symbols_inserted = 0 in
    let symbols_inserted = List.fold
      ~init:symbols_inserted
      ~f:(fun acc (_, name) -> FunsTable.insert db ~name ~file_info_pk; acc + 1)
      file_info.FileInfo.funs
    in
    let symbols_inserted = List.fold
      ~init:symbols_inserted
      ~f:(fun acc (_, name) -> TypesTable.insert_class db ~name ~file_info_pk; acc + 1)
      file_info.FileInfo.classes
    in
    let symbols_inserted = List.fold
      ~init:symbols_inserted
      ~f:(fun acc (_, name) -> TypesTable.insert_typedef db ~name ~file_info_pk; acc + 1)
      file_info.FileInfo.typedefs
    in
    let symbols_inserted = List.fold
      ~init:symbols_inserted
      ~f:(fun acc (_, name) -> ConstsTable.insert db ~name ~file_info_pk; acc + 1)
      file_info.FileInfo.consts
    in
    symbols_inserted


  let create_database db_name f =
    let db = Sqlite3.db_open db_name in
    Sqlite3.exec db "BEGIN TRANSACTION;" |> check_rc;
    Sqlite3.exec db FileInfoTable.create_table_sqlite |> check_rc;
    Sqlite3.exec db ConstsTable.create_table_sqlite |> check_rc;
    Sqlite3.exec db TypesTable.create_table_sqlite |> check_rc;
    Sqlite3.exec db FunsTable.create_table_sqlite |> check_rc;
    let () = f (save_file_info db) in
    Sqlite3.exec db "END TRANSACTION;" |> check_rc;
    if not (Sqlite3.db_close db)
    then failwith (Printf.sprintf "Could not close database '%s'" db_name)
end


(*****************************************************************************)
(* Forward naming table functions *)
(*****************************************************************************)


let combine a b = Relative_path.Map.union a b
let create a = a
let empty = Relative_path.Map.empty
let filter = Relative_path.Map.filter
let fold = Relative_path.Map.fold
let get_files = Relative_path.Map.keys
let get_file_info = Relative_path.Map.get
let get_file_info_unsafe = Relative_path.Map.find_unsafe
let has_file = Relative_path.Map.mem
let iter = Relative_path.Map.iter
let update a key data = Relative_path.Map.add a ~key ~data

let save naming_table db_name =
  let t = Unix.gettimeofday() in
  let files_added = ref 0 in
  let symbols_added = ref 0 in
  Sqlite.create_database db_name begin fun save_file_info ->
    let get_next_file_info_primary_key () =
      incr files_added;
      !files_added
    in
    iter naming_table begin fun path file_info ->
      let file_info_primary_key = Int64.of_int (get_next_file_info_primary_key ()) in
      let new_symbol_count = save_file_info file_info_primary_key path file_info in
      symbols_added := !symbols_added + new_symbol_count
    end
  end;
  let _ : float =
    Hh_logger.log_duration
      (Printf.sprintf "Inserted %d files and %d symbols" !files_added !symbols_added)
      t
  in
  ()


(*****************************************************************************)
(* Conversion functions *)
(*****************************************************************************)


let from_saved saved =
  Relative_path.Map.fold saved ~init:Relative_path.Map.empty ~f:(fun fn saved acc ->
    let file_info = FileInfo.from_saved fn saved in
    Relative_path.Map.add acc fn file_info
  )

let to_saved a =
  Relative_path.Map.map a FileInfo.to_saved

let to_fast a =
  Relative_path.Map.map a FileInfo.simplify

let saved_to_fast saved =
  Relative_path.Map.map saved FileInfo.saved_to_names


(*****************************************************************************)
(* Reverse naming table functions *)
(*****************************************************************************)


let check_valid key pos =
  if FileInfo.get_pos_filename pos = Relative_path.default then begin
    Hh_logger.log
      ("WARNING: setting canonical position of %s to be in dummy file. If this \
      happens in incremental mode, things will likely break later.") key;
    Hh_logger.log "%s"
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100));
  end

module type ReverseNamingTable = sig
  type pos

  val add : string -> pos -> unit
  val get_pos : ?bypass_cache:bool -> string -> pos option
  val remove_batch : SSet.t -> unit

  val heap_string_of_key : string -> string
end

(* The Types module records both class names and typedefs since they live in the
* same namespace. That is, one cannot both define a class Foo and a typedef Foo
* (or FOO or fOo, due to case insensitivity). *)
type type_of_type =
  | TClass
  | TTypedef
  [@@deriving enum]
module Types = struct
  type pos = FileInfo.pos * type_of_type

  module TypeCanonHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = string
      let prefix = Prefix.make()
      let description = "TypeCanon"
    end)

  module TypePosHeap = SharedMem.WithCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = pos
      let prefix = Prefix.make ()
      let description = "TypePos"
    end)

  let add id type_info =
    if not @@ TypePosHeap.LocalChanges.has_local_changes () then check_valid id (fst type_info);
    TypeCanonHeap.add (to_canon_name_key id) id;
    TypePosHeap.write_around id type_info

  let get_pos ?(bypass_cache=false) id =
    if bypass_cache
    then TypePosHeap.get_no_cache id
    else TypePosHeap.get id

  let get_canon_name id =
    TypeCanonHeap.get id

  let remove_batch types =
    TypeCanonHeap.remove_batch (canonize_set types);
    TypePosHeap.remove_batch types

  let heap_string_of_key = TypePosHeap.string_of_key
end

module Funs = struct
  type pos = FileInfo.pos

  module FunCanonHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = string
      let prefix = Prefix.make()
      let description = "FunCanon"
    end)

  module FunPosHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = pos
      let prefix = Prefix.make()
      let description = "FunPos"
    end)

  let add id pos =
    if not @@ FunPosHeap.LocalChanges.has_local_changes () then check_valid id pos;
    FunCanonHeap.add (to_canon_name_key id) id;
    FunPosHeap.add id pos

  let get_pos ?bypass_cache:(_=false) id =
    FunPosHeap.get id

  let get_canon_name name =
    FunCanonHeap.get name

  let remove_batch funs =
    FunCanonHeap.remove_batch (canonize_set funs);
    FunPosHeap.remove_batch funs

  let heap_string_of_key = FunPosHeap.string_of_key
end

module Consts = struct
  type pos = FileInfo.pos

  module ConstPosHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = pos
      let prefix = Prefix.make()
      let description = "ConstPos"
    end)

  let add id pos =
    if not @@ ConstPosHeap.LocalChanges.has_local_changes () then check_valid id pos;
    ConstPosHeap.add id pos

  let get_pos ?bypass_cache:(_=false) id =
    ConstPosHeap.get id

  let remove_batch consts =
    ConstPosHeap.remove_batch consts

  let heap_string_of_key = ConstPosHeap.string_of_key
end

let push_local_changes () =
  Types.TypePosHeap.LocalChanges.push_stack ();
  Types.TypeCanonHeap.LocalChanges.push_stack ();
  Funs.FunPosHeap.LocalChanges.push_stack ();
  Funs.FunCanonHeap.LocalChanges.push_stack ();
  Consts.ConstPosHeap.LocalChanges.push_stack ()

let pop_local_changes () =
  Types.TypePosHeap.LocalChanges.pop_stack ();
  Types.TypeCanonHeap.LocalChanges.pop_stack ();
  Funs.FunPosHeap.LocalChanges.pop_stack ();
  Funs.FunCanonHeap.LocalChanges.pop_stack ();
  Consts.ConstPosHeap.LocalChanges.pop_stack ()

let has_local_changes () =
  Types.TypePosHeap.LocalChanges.has_local_changes ()
  || Types.TypeCanonHeap.LocalChanges.has_local_changes ()
  || Funs.FunPosHeap.LocalChanges.has_local_changes ()
  || Funs.FunCanonHeap.LocalChanges.has_local_changes ()
  || Consts.ConstPosHeap.LocalChanges.has_local_changes ()
