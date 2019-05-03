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

type type_of_type =
  | TClass
  | TTypedef
  [@@deriving enum]

module Sqlite : sig
  val create_database :
    string ->
    ((int64 -> Relative_path.t -> FileInfo.t -> int) -> unit) ->
    unit
  val set_db_path : string -> unit
  val get_type_pos : string -> case_insensitive:bool -> (Relative_path.t * type_of_type) option
  val get_fun_pos : string -> case_insensitive:bool -> Relative_path.t option
  val get_const_pos : string -> Relative_path.t option
end = struct
  let check_rc rc =
    if rc <> Sqlite3.Rc.OK && rc <> Sqlite3.Rc.DONE
    then failwith (Printf.sprintf "SQLite operation failed: %s" (Sqlite3.Rc.to_string rc))

  let column_str stmt idx =
    match Sqlite3.column stmt idx with
    | Sqlite3.Data.TEXT s -> s
    | data ->
      let msg =
        Printf.sprintf "Expected a string at column %d, but was %s"
          idx
          (Sqlite3.Data.to_string_debug data)
      in
      failwith msg

  let column_int stmt idx =
    match Sqlite3.column stmt idx with
    | Sqlite3.Data.INT i -> i
    | data ->
      let msg =
        Printf.sprintf "Expected an int at column %d, but was %s"
          idx
          (Sqlite3.Data.to_string_debug data)
      in
      failwith msg

  let make_relative_path ~prefix_int ~suffix =
    let prefix =
      Core_kernel.Option.value_exn (Relative_path.prefix_of_enum (Int64.to_int prefix_int))
    in
    let full_suffix = Filename.concat (Relative_path.path_of_prefix prefix) suffix in
    Relative_path.create prefix full_suffix


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
    let class_flag = Int64.of_int (type_of_type_to_enum TClass)
    let typedef_flag = Int64.of_int (type_of_type_to_enum TTypedef)

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

    let (get_sqlite, get_sqlite_case_insensitive) =
      let base = Str.global_replace (Str.regexp "{table_name}") table_name "
        SELECT
          NAMING_FILE_INFO.PATH_PREFIX_TYPE,
          NAMING_FILE_INFO.PATH_SUFFIX,
          {table_name}.FLAGS
        FROM {table_name}
        LEFT JOIN NAMING_FILE_INFO ON
          {table_name}.FILE_INFO_PK = NAMING_FILE_INFO.PRIMARY_KEY
        WHERE {table_name}.{hash} = ?"
      in
      (Str.global_replace (Str.regexp "{hash}") "HASH" base,
        Str.global_replace (Str.regexp "{hash}") "CANON_HASH" base)

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

    let get db ~name ~case_insensitive =
      let name = if case_insensitive then String.lowercase_ascii name else name in
      let hash = SharedMem.get_hash name in
      let get_sqlite = if case_insensitive then get_sqlite_case_insensitive else get_sqlite in
      let get_stmt = Sqlite3.prepare db get_sqlite in
      Sqlite3.bind get_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      match Sqlite3.step get_stmt with
      | Sqlite3.Rc.DONE -> None
      | Sqlite3.Rc.ROW ->
        let prefix_type = column_int get_stmt 0 in
        let suffix = column_str get_stmt 1 in
        let flags = Int64.to_int (column_int get_stmt 2) in
        let class_type = Core_kernel.Option.value_exn (type_of_type_of_enum flags) in
        Some (make_relative_path prefix_type suffix, class_type)
      | rc -> failwith (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))
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

    let (get_sqlite, get_sqlite_case_insensitive) =
      let base = Str.global_replace (Str.regexp "{table_name}") table_name "
        SELECT
          NAMING_FILE_INFO.PATH_PREFIX_TYPE,
          NAMING_FILE_INFO.PATH_SUFFIX
        FROM {table_name}
        LEFT JOIN NAMING_FILE_INFO ON
          {table_name}.FILE_INFO_PK = NAMING_FILE_INFO.PRIMARY_KEY
        WHERE {table_name}.{hash} = ?"
      in
      (Str.global_replace (Str.regexp "{hash}") "HASH" base,
        Str.global_replace (Str.regexp "{hash}") "CANON_HASH" base)

    let insert db ~name ~file_info_pk =
      let hash = SharedMem.get_hash name in
      let canon_hash = SharedMem.get_hash (to_canon_name_key name) in
      let insert_stmt = Sqlite3.prepare db insert_sqlite in
      Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT canon_hash) |> check_rc;
      Sqlite3.bind insert_stmt 3 (Sqlite3.Data.INT file_info_pk) |> check_rc;
      Sqlite3.step insert_stmt |> check_rc;
      Sqlite3.finalize insert_stmt |> check_rc

    let get db ~name ~case_insensitive =
      let name = if case_insensitive then String.lowercase_ascii name else name in
      let hash = SharedMem.get_hash name in
      let get_sqlite = if case_insensitive then get_sqlite_case_insensitive else get_sqlite in
      let get_stmt = Sqlite3.prepare db (get_sqlite) in
      Sqlite3.bind get_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      match Sqlite3.step get_stmt with
      | Sqlite3.Rc.DONE -> None
      | Sqlite3.Rc.ROW ->
        let prefix_type = column_int get_stmt 0 in
        let suffix = column_str get_stmt 1 in
        Some (make_relative_path prefix_type suffix)
      | rc -> failwith (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))
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

    let get_sqlite =
      Str.global_replace (Str.regexp "{table_name}") table_name "
        SELECT
          NAMING_FILE_INFO.PATH_PREFIX_TYPE,
          NAMING_FILE_INFO.PATH_SUFFIX
        FROM {table_name}
        LEFT JOIN NAMING_FILE_INFO ON
          {table_name}.FILE_INFO_PK = NAMING_FILE_INFO.PRIMARY_KEY
        WHERE {table_name}.HASH = ?
      "

    let insert db ~name ~file_info_pk =
      let hash = SharedMem.get_hash name in
      let insert_stmt = Sqlite3.prepare db insert_sqlite in
      Sqlite3.bind insert_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      Sqlite3.bind insert_stmt 2 (Sqlite3.Data.INT file_info_pk) |> check_rc;
      Sqlite3.step insert_stmt |> check_rc;
      Sqlite3.finalize insert_stmt |> check_rc

    let get db ~name =
      let hash = SharedMem.get_hash name in
      let get_stmt = Sqlite3.prepare db get_sqlite in
      Sqlite3.bind get_stmt 1 (Sqlite3.Data.INT hash) |> check_rc;
      match Sqlite3.step get_stmt with
      | Sqlite3.Rc.DONE -> None
      | Sqlite3.Rc.ROW ->
        let prefix_type = column_int get_stmt 0 in
        let suffix = column_str get_stmt 1 in
        Some (make_relative_path prefix_type suffix)
      | rc -> failwith (Printf.sprintf "Failure retrieving row: %s" (Sqlite3.Rc.to_string rc))
  end


  module DatabaseSettings : sig
    val set_db_path : string -> unit
    val get_db : unit -> Sqlite3.db option
  end  = struct
    include SharedMem.NoCache
      (SharedMem.ProfiledImmediate)
      (StringKey)
      (struct
        type t = string
        let prefix = Prefix.make()
        let description = "DatabaseSettings"
      end)

    let check_table_sqlite =
      "SELECT NAME FROM SQLITE_MASTER WHERE TYPE='table' AND NAME='NAMING_FILE_INFO'"

    let open_db () =
      match get "database_path" with
      | None -> None
      | Some path ->
        let db = Sqlite3.db_open path in
        let has_table = ref false in
        Sqlite3.exec db ~cb:(fun _ _ -> has_table := true) check_table_sqlite |> check_rc;
        if !has_table then Some db else None

    let db = ref (lazy (open_db ()))

    let set_db_path path =
      add "database_path" path;
      db := lazy (open_db ())

    let get_db () =
      Lazy.force !db
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

  let set_db_path = DatabaseSettings.set_db_path

  let get_type_pos name ~case_insensitive =
    match DatabaseSettings.get_db () with
    | None -> None
    | Some db ->
      TypesTable.get db ~name ~case_insensitive

  let get_fun_pos name ~case_insensitive =
    match DatabaseSettings.get_db () with
    | None -> None
    | Some db ->
      FunsTable.get db ~name ~case_insensitive

  let get_const_pos name =
    match DatabaseSettings.get_db () with
    | None -> None
    | Some db ->
      ConstsTable.get db ~name
end

let set_sqlite_fallback_path = Sqlite.set_db_path


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

let get_and_cache ~map_result ~get_func ~fallback_get_func ~add_func ~measure_name ~name =
  match get_func name with
  | Some v ->
    Measure.sample measure_name 1.0;
    Some v
  | None ->
    Measure.sample measure_name 0.0;
    begin match fallback_get_func name with
    | Some res ->
      let pos = map_result res in
      add_func name pos;
      Some pos
    | None ->
      None
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
    let get_func =
      if bypass_cache
      then TypePosHeap.get_no_cache
      else TypePosHeap.get
    in
    let map_result (path, entry_type) =
      let name_type = match entry_type with
        | TClass -> FileInfo.Class
        | TTypedef -> FileInfo.Typedef
      in
      (FileInfo.File (name_type, path), entry_type)
    in
    get_and_cache
      ~map_result
      ~get_func
      ~fallback_get_func:(Sqlite.get_type_pos ~case_insensitive:false)
      ~add_func:add
      ~measure_name:"Reverse naming table (types) cache hit rate"
      ~name:id

  let get_canon_name id =
    let open Core_kernel in
    let map_result (path, entry_type) =
      let _path_str = Relative_path.S.to_string path in
      let id = match entry_type with
        | TClass ->
          let class_opt = Ast_provider.find_class_in_file ~case_insensitive:true path id in
          (Option.value_exn class_opt).Ast.c_name
        | TTypedef ->
          let typedef_opt = Ast_provider.find_typedef_in_file ~case_insensitive:true path id in
          (Option.value_exn typedef_opt).Ast.t_id
      in
      snd id
    in
    get_and_cache
      ~map_result
      ~get_func:TypeCanonHeap.get
      ~fallback_get_func:(Sqlite.get_type_pos ~case_insensitive:true)
      ~add_func:TypeCanonHeap.add
      ~measure_name:"Canon naming table (types) cache hit rate"
      ~name:id

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
    let map_result path = FileInfo.File (FileInfo.Fun, path) in
    get_and_cache
      ~map_result
      ~get_func:FunPosHeap.get
      ~fallback_get_func:(Sqlite.get_fun_pos ~case_insensitive:false)
      ~add_func:add
      ~measure_name:"Reverse naming table (functions) cache hit rate"
      ~name:id

  let get_canon_name name =
    let open Core_kernel in
    let map_result path =
      let fun_opt = Ast_provider.find_fun_in_file ~case_insensitive:true path name in
      snd (Option.value_exn fun_opt).Ast.f_name
    in
    get_and_cache
      ~map_result
      ~get_func:FunCanonHeap.get
      ~fallback_get_func:(Sqlite.get_fun_pos ~case_insensitive:true)
      ~add_func:FunCanonHeap.add
      ~measure_name:"Canon naming table (functions) cache hit rate"
      ~name

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
    let map_result path = FileInfo.File (FileInfo.Const, path) in
    get_and_cache
      ~map_result
      ~get_func:ConstPosHeap.get
      ~fallback_get_func:Sqlite.get_const_pos
      ~add_func:add
      ~measure_name:"Reverse naming table (consts) cache hit rate"
      ~name:id

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
