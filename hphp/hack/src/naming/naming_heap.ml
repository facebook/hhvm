(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type blocked_entry = Blocked

(** Gets an entry from shared memory, or falls back to SQLite if necessary. If data is returned by
    SQLite, we also cache it back to shared memory.

    @param map_result function that maps from the SQLite fallback value to the actual value type we
      want to cache and return.
    @param get_func function that retrieves a key from shared memory.
    @param check_block_func function that checks if a key is blocked from falling back to SQLite.
    @param fallback_get_func function to get a fallback value from SQLite.
    @param add_func function to cache a value back into shared memory.
    @param measure_name the name of the measure to use for tracking fallback stats. We write a 1.0
      if the request could be resolved entirely from shared memory, and 0.0 if we had to go to
      SQLite.
    @param key the key to request.
*)
let get_and_cache
    ~(map_result : 'fallback_value -> 'value option)
    ~(get_func : 'key -> 'value option)
    ~(check_block_func : 'key -> blocked_entry option)
    ~(fallback_get_func_opt : ('key -> 'fallback_value option) option)
    ~(add_func : 'key -> 'value -> unit)
    ~(measure_name : string)
    ~(key : 'key) : 'value option =
  match (get_func key, fallback_get_func_opt) with
  | (Some v, _) ->
    Measure.sample measure_name 1.0;
    Some v
  | (None, None) -> None
  | (None, Some fallback_get_func) ->
    (match check_block_func key with
    | Some Blocked ->
      (* We sample 1.0 here even though we're returning None because we didn't go to SQLite. *)
      Measure.sample measure_name 1.0;
      None
    | None ->
      Measure.sample measure_name 0.0;
      begin
        match fallback_get_func key with
        | Some res ->
          begin
            match map_result res with
            | Some pos ->
              add_func key pos;
              Some pos
            | None -> None
          end
        | None -> None
      end)

let check_valid key pos =
  if Relative_path.equal (FileInfo.get_pos_filename pos) Relative_path.default
  then (
    Hh_logger.log
      "WARNING: setting canonical position of %s to be in dummy file. If this happens in incremental mode, things will likely break later."
      key;
    Hh_logger.log
      "%s"
      (Caml.Printexc.raw_backtrace_to_string (Caml.Printexc.get_callstack 100))
  )

let canonize_set = SSet.map Naming_sqlite.to_canon_name_key

module type ReverseNamingTable = sig
  type pos

  val add : string -> pos -> unit

  val get_pos :
    Naming_sqlite.db_path option -> ?bypass_cache:bool -> string -> pos option

  val get_filename :
    Naming_sqlite.db_path option -> string -> Relative_path.t option

  val is_defined : Naming_sqlite.db_path option -> string -> bool

  val remove_batch : Naming_sqlite.db_path option -> string list -> unit

  module Position : Value.Type with type t = pos
end

(* The Types module records both class names and typedefs since they live in the
* same namespace. That is, one cannot both define a class Foo and a typedef Foo
* (or FOO or fOo, due to case insensitivity). *)
module Types = struct
  type pos = FileInfo.pos * Naming_types.kind_of_type

  module CanonName = struct
    type t = string

    let prefix = Prefix.make ()

    let description = "Naming_TypeCanon"
  end

  module Position = struct
    type t = pos

    let prefix = Prefix.make ()

    let description = "Naming_TypePos"
  end

  module TypeCanonHeap =
    SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey) (CanonName)

  module TypePosHeap =
    SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Position)
      (struct
        let capacity = 1000
      end)

  module BlockedEntries =
    SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey)
      (struct
        type t = blocked_entry

        let prefix = Prefix.make ()

        let description = "Naming_TypeBlocked"
      end)
      (struct
        let capacity = 1000
      end)

  let add id type_info =
    if not @@ TypePosHeap.LocalChanges.has_local_changes () then
      check_valid id (fst type_info);
    TypeCanonHeap.add (Naming_sqlite.to_canon_name_key id) id;
    TypePosHeap.write_around id type_info

  let get_pos db_path_opt ?(bypass_cache = false) id =
    let get_func =
      if bypass_cache then
        TypePosHeap.get_no_cache
      else
        TypePosHeap.get
    in
    let map_result (path, entry_type) =
      let name_type =
        match entry_type with
        | Naming_types.TClass -> FileInfo.Class
        | Naming_types.TTypedef -> FileInfo.Typedef
        | Naming_types.TRecordDef -> FileInfo.RecordDef
      in
      Some (FileInfo.File (name_type, path), entry_type)
    in
    let fallback_get_func_opt =
      Option.map db_path_opt ~f:Naming_sqlite.get_type_pos
    in
    get_and_cache
      ~map_result
      ~get_func
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~add_func:add
      ~measure_name:"Reverse naming table (types) cache hit rate"
      ~key:id

  let get_kind db_path_opt id =
    match get_pos db_path_opt id with
    | Some (_pos, kind) -> Some kind
    | None -> None

  let get_filename_and_kind db_path_opt id =
    match get_pos db_path_opt id with
    | Some (pos, kind) -> Some (FileInfo.get_pos_filename pos, kind)
    | None -> None

  let get_filename db_path_opt id =
    match get_pos db_path_opt id with
    | None -> None
    | Some (pos, _) -> Some (FileInfo.get_pos_filename pos)

  let is_defined db_path_opt id = Option.is_some (get_pos db_path_opt id)

  let get_canon_name ctx id =
    let map_result (path, entry_type) =
      let path_str = Relative_path.S.to_string path in
      match entry_type with
      | Naming_types.TClass ->
        begin
          match Ast_provider.find_iclass_in_file ctx path id with
          | Some cls -> Some (snd cls.Aast.c_name)
          | None ->
            Hh_logger.log
              "Failed to get canonical name for %s in file %s"
              id
              path_str;
            None
        end
      | Naming_types.TTypedef ->
        begin
          match Ast_provider.find_itypedef_in_file ctx path id with
          | Some typedef -> Some (snd typedef.Aast.t_name)
          | None ->
            Hh_logger.log
              "Failed to get canonical name for %s in file %s"
              id
              path_str;
            None
        end
      | Naming_types.TRecordDef ->
        begin
          match Ast_provider.find_irecord_def_in_file ctx path id with
          | Some cls -> Some (snd cls.Aast.rd_name)
          | None ->
            Hh_logger.log
              "Failed to get canonical name for %s in file %s"
              id
              path_str;
            None
        end
    in
    let db_path_opt =
      Db_path_provider.get_naming_db_path (Provider_context.get_backend ctx)
    in
    let fallback_get_func_opt =
      Option.map db_path_opt ~f:Naming_sqlite.get_itype_pos
    in
    get_and_cache
      ~map_result
      ~get_func:TypeCanonHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~add_func:TypeCanonHeap.add
      ~measure_name:"Canon naming table (types) cache hit rate"
      ~key:id

  let remove_batch db_path_opt types =
    let types = SSet.of_list types in
    let canon_key_types = canonize_set types in
    TypeCanonHeap.remove_batch canon_key_types;
    TypePosHeap.remove_batch types;
    match db_path_opt with
    | None -> ()
    | Some _ ->
      SSet.iter
        (fun id -> BlockedEntries.add id Blocked)
        (SSet.union types canon_key_types)
end

module Funs = struct
  type pos = FileInfo.pos

  module CanonName = struct
    type t = string

    let prefix = Prefix.make ()

    let description = "Naming_FunCanon"
  end

  module FunCanonHeap =
    SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey) (CanonName)

  module Position = struct
    type t = pos

    let prefix = Prefix.make ()

    let description = "Naming_FunPos"
  end

  module FunPosHeap =
    SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey) (Position)

  module BlockedEntries =
    SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey)
      (struct
        type t = blocked_entry

        let prefix = Prefix.make ()

        let description = "Naming_FunBlocked"
      end)

  let add id pos =
    if not @@ FunPosHeap.LocalChanges.has_local_changes () then
      check_valid id pos;
    FunCanonHeap.add (Naming_sqlite.to_canon_name_key id) id;
    FunPosHeap.add id pos

  let get_pos db_path_opt ?bypass_cache:(_ = false) id =
    let map_result path = Some (FileInfo.File (FileInfo.Fun, path)) in
    let fallback_get_func_opt =
      Option.map db_path_opt ~f:Naming_sqlite.get_fun_pos
    in
    get_and_cache
      ~map_result
      ~get_func:FunPosHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~add_func:add
      ~measure_name:"Reverse naming table (functions) cache hit rate"
      ~key:id

  let get_filename db_path_opt id =
    get_pos db_path_opt id
    |> Core_kernel.Option.map ~f:FileInfo.get_pos_filename

  let is_defined db_path_opt id = Option.is_some (get_pos db_path_opt id)

  let get_canon_name ctx name =
    let map_result path =
      match Ast_provider.find_ifun_in_file ctx path name with
      | Some f -> Some (snd f.Aast.fd_fun.Aast.f_name)
      | None ->
        let path_str = Relative_path.S.to_string path in
        Hh_logger.log
          "Failed to get canonical name for %s in file %s"
          name
          path_str;
        None
    in
    let db_path_opt =
      Db_path_provider.get_naming_db_path (Provider_context.get_backend ctx)
    in
    let fallback_get_func_opt =
      Option.map db_path_opt ~f:Naming_sqlite.get_ifun_pos
    in
    get_and_cache
      ~map_result
      ~get_func:FunCanonHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~add_func:FunCanonHeap.add
      ~measure_name:"Canon naming table (functions) cache hit rate"
      ~key:name

  let remove_batch db_path_opt funs =
    let funs = SSet.of_list funs in
    let canon_key_funs = canonize_set funs in
    FunCanonHeap.remove_batch canon_key_funs;
    FunPosHeap.remove_batch funs;
    match db_path_opt with
    | None -> ()
    | Some _ ->
      SSet.iter
        (fun id -> BlockedEntries.add id Blocked)
        (SSet.union funs canon_key_funs)
end

module Consts = struct
  type pos = FileInfo.pos

  module Position = struct
    type t = pos

    let prefix = Prefix.make ()

    let description = "Naming_ConstPos"
  end

  module ConstPosHeap =
    SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey) (Position)

  module BlockedEntries =
    SharedMem.NoCache (SharedMem.ProfiledImmediate) (StringKey)
      (struct
        type t = blocked_entry

        let prefix = Prefix.make ()

        let description = "Naming_ConstBlocked"
      end)

  let add id pos =
    if not @@ ConstPosHeap.LocalChanges.has_local_changes () then
      check_valid id pos;
    ConstPosHeap.add id pos

  let get_pos db_path_opt ?bypass_cache:(_ = false) id =
    let map_result path = Some (FileInfo.File (FileInfo.Const, path)) in
    let fallback_get_func_opt =
      Option.map db_path_opt ~f:(fun db_path ->
          Naming_sqlite.get_const_pos db_path)
    in
    get_and_cache
      ~map_result
      ~get_func:ConstPosHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~add_func:add
      ~measure_name:"Reverse naming table (consts) cache hit rate"
      ~key:id

  let get_filename db_path_opt id =
    get_pos db_path_opt id
    |> Core_kernel.Option.map ~f:FileInfo.get_pos_filename

  let is_defined db_path_opt id = Option.is_some (get_pos db_path_opt id)

  let remove_batch db_path_opt consts =
    let consts = SSet.of_list consts in
    ConstPosHeap.remove_batch consts;
    match db_path_opt with
    | None -> ()
    | Some _ -> SSet.iter (fun id -> BlockedEntries.add id Blocked) consts
end

let push_local_changes () =
  Types.TypePosHeap.LocalChanges.push_stack ();
  Types.TypeCanonHeap.LocalChanges.push_stack ();
  Types.BlockedEntries.LocalChanges.push_stack ();
  Funs.FunPosHeap.LocalChanges.push_stack ();
  Funs.FunCanonHeap.LocalChanges.push_stack ();
  Funs.BlockedEntries.LocalChanges.push_stack ();
  Consts.ConstPosHeap.LocalChanges.push_stack ();
  Consts.BlockedEntries.LocalChanges.push_stack ()

let pop_local_changes () =
  Types.TypePosHeap.LocalChanges.pop_stack ();
  Types.TypeCanonHeap.LocalChanges.pop_stack ();
  Types.BlockedEntries.LocalChanges.pop_stack ();
  Funs.FunPosHeap.LocalChanges.pop_stack ();
  Funs.FunCanonHeap.LocalChanges.pop_stack ();
  Funs.BlockedEntries.LocalChanges.pop_stack ();
  Consts.ConstPosHeap.LocalChanges.pop_stack ();
  Consts.BlockedEntries.LocalChanges.pop_stack ()

let has_local_changes () =
  Types.TypePosHeap.LocalChanges.has_local_changes ()
  || Types.TypeCanonHeap.LocalChanges.has_local_changes ()
  || Types.BlockedEntries.LocalChanges.has_local_changes ()
  || Funs.FunPosHeap.LocalChanges.has_local_changes ()
  || Funs.FunCanonHeap.LocalChanges.has_local_changes ()
  || Funs.BlockedEntries.LocalChanges.has_local_changes ()
  || Consts.ConstPosHeap.LocalChanges.has_local_changes ()
  || Consts.BlockedEntries.LocalChanges.has_local_changes ()
