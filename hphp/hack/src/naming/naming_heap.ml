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
    ~(cache_func : 'key -> 'value -> unit)
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
              cache_func key pos;
              Some pos
            | None -> None
          end
        | None -> None
      end)

module type ReverseNamingTable = sig
  type pos

  module Position : SharedMem.Value with type t = pos

  module CanonName : SharedMem.Value with type t = string

  val add : string -> pos -> unit

  val get_pos : Naming_sqlite.db_path option -> string -> pos option

  val remove_batch : Naming_sqlite.db_path option -> string list -> unit

  val get_canon_name : Provider_context.t -> string -> string option

  val hash : string -> Typing_deps.Dep.t

  val canon_hash : string -> Typing_deps.Dep.t
end

(* The Types module records both class names and typedefs since they live in the
* same namespace. That is, one cannot both define a class Foo and a typedef Foo
* (or FOO or fOo, due to case insensitivity). *)
module Types = struct
  type pos = FileInfo.pos * Naming_types.kind_of_type

  module Position = struct
    type t = pos

    let description = "Naming_TypePos"
  end

  module CanonName = struct
    type t = string

    let description = "Naming_TypeCanon"
  end

  module TypePosHeap =
    SharedMem.WithCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (Position)
      (struct
        let capacity = 1000
      end)

  module TypeCanonHeap =
    SharedMem.NoCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (CanonName)

  module BlockedEntries =
    SharedMem.WithCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (struct
        type t = blocked_entry

        let description = "Naming_TypeBlocked"
      end)
      (struct
        let capacity = 1000
      end)

  let hash name =
    Typing_deps.Dep.Type name |> Typing_deps.Dep.make Typing_deps.Mode.Hash64Bit

  let canon_hash name =
    Typing_deps.Dep.Type (Naming_sqlite.to_canon_name_key name)
    |> Typing_deps.Dep.make Typing_deps.Mode.Hash64Bit

  let add id pos =
    TypePosHeap.write_around (hash id) pos;
    TypeCanonHeap.add (canon_hash id) id;
    ()

  let get_pos db_path_opt id =
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
      Option.map db_path_opt ~f:(fun db_path hash ->
          match Naming_sqlite.get_path_by_64bit_dep db_path hash with
          | None -> None
          | Some (file, Naming_types.Type_kind type_kind) ->
            Some (file, type_kind)
          | Some (_, _) ->
            failwith "passed in Type dephash, but got non-type out")
    in
    get_and_cache
      ~map_result
      ~get_func:TypePosHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~cache_func:TypePosHeap.write_around
      ~measure_name:"Reverse naming table (types) cache hit rate"
      ~key:(hash id)

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
      Option.map db_path_opt ~f:(fun db_path _canon_hash ->
          Naming_sqlite.get_itype_path_by_name db_path id)
    in
    get_and_cache
      ~map_result
      ~get_func:TypeCanonHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~cache_func:TypeCanonHeap.add
      ~measure_name:"Canon naming table (types) cache hit rate"
      ~key:(canon_hash id)

  let remove_batch db_path_opt types =
    let hashes = types |> List.map ~f:hash in
    let canon_hashes = types |> List.map ~f:canon_hash in
    TypeCanonHeap.remove_batch (TypeCanonHeap.KeySet.of_list canon_hashes);
    TypePosHeap.remove_batch (TypePosHeap.KeySet.of_list hashes);
    match db_path_opt with
    | None -> ()
    | Some _ ->
      List.iter hashes ~f:(fun hash -> BlockedEntries.add hash Blocked);
      List.iter canon_hashes ~f:(fun canon_hash ->
          BlockedEntries.add canon_hash Blocked);
      ()
end

module Funs = struct
  type pos = FileInfo.pos

  module Position = struct
    type t = pos

    let description = "Naming_FunPos"
  end

  module CanonName = struct
    type t = string

    let description = "Naming_FunCanon"
  end

  module FunPosHeap =
    SharedMem.NoCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (Position)
  module FunCanonHeap =
    SharedMem.NoCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (CanonName)

  module BlockedEntries =
    SharedMem.NoCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (struct
        type t = blocked_entry

        let description = "Naming_FunBlocked"
      end)

  let hash name =
    Typing_deps.Dep.Fun name |> Typing_deps.Dep.make Typing_deps.Mode.Hash64Bit

  let canon_hash name =
    Typing_deps.Dep.Fun (Naming_sqlite.to_canon_name_key name)
    |> Typing_deps.Dep.make Typing_deps.Mode.Hash64Bit

  let add id pos =
    FunCanonHeap.add (canon_hash id) id;
    FunPosHeap.add (hash id) pos;
    ()

  let get_pos db_path_opt (id : string) =
    let map_result path = Some (FileInfo.File (FileInfo.Fun, path)) in
    let fallback_get_func_opt =
      Option.map db_path_opt ~f:(fun db_path dep ->
          Naming_sqlite.get_path_by_64bit_dep db_path dep |> Option.map ~f:fst)
    in
    get_and_cache
      ~map_result
      ~get_func:FunPosHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~cache_func:FunPosHeap.add
      ~measure_name:"Reverse naming table (functions) cache hit rate"
      ~key:(hash id)

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
      Option.map db_path_opt ~f:(fun db_path _canon_hash ->
          Naming_sqlite.get_ifun_path_by_name db_path name)
    in
    get_and_cache
      ~map_result
      ~get_func:FunCanonHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~cache_func:FunCanonHeap.add
      ~measure_name:"Canon naming table (functions) cache hit rate"
      ~key:(canon_hash name)

  let remove_batch db_path_opt funs =
    let hashes = funs |> List.map ~f:hash in
    let canon_hashes = funs |> List.map ~f:canon_hash in
    FunCanonHeap.remove_batch (FunCanonHeap.KeySet.of_list canon_hashes);
    FunPosHeap.remove_batch (FunPosHeap.KeySet.of_list hashes);
    match db_path_opt with
    | None -> ()
    | Some _ ->
      List.iter hashes ~f:(fun hash -> BlockedEntries.add hash Blocked);
      List.iter canon_hashes ~f:(fun canon_hash ->
          BlockedEntries.add canon_hash Blocked);
      ()
end

module Consts = struct
  type pos = FileInfo.pos

  module Position = struct
    type t = pos

    let description = "Naming_ConstPos"
  end

  (** This module isn't actually used. It's here only for uniformity with the other ReverseNamingTables. *)
  module CanonName = struct
    type t = string

    let description = "Naming_ConstCanon"
  end

  module ConstPosHeap =
    SharedMem.NoCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (Position)

  module BlockedEntries =
    SharedMem.NoCache (SharedMem.ProfiledBackend) (Typing_deps.DepHashKey)
      (struct
        type t = blocked_entry

        let description = "Naming_ConstBlocked"
      end)

  let hash name =
    Typing_deps.Dep.GConst name
    |> Typing_deps.Dep.make Typing_deps.Mode.Hash64Bit

  let canon_hash name =
    Typing_deps.Dep.GConst (Naming_sqlite.to_canon_name_key name)
    |> Typing_deps.Dep.make Typing_deps.Mode.Hash64Bit

  let add id pos = ConstPosHeap.add (hash id) pos

  let get_pos db_path_opt id =
    let map_result path = Some (FileInfo.File (FileInfo.Const, path)) in
    let fallback_get_func_opt =
      Option.map db_path_opt ~f:(fun db_path hash ->
          Naming_sqlite.get_path_by_64bit_dep db_path hash |> Option.map ~f:fst)
    in
    get_and_cache
      ~map_result
      ~get_func:ConstPosHeap.get
      ~check_block_func:BlockedEntries.get
      ~fallback_get_func_opt
      ~cache_func:ConstPosHeap.add
      ~measure_name:"Reverse naming table (consts) cache hit rate"
      ~key:(hash id)

  (* This function isn't even used, because the only callers who wish to obtain canonical
     names are "class fOo isn't defined; did you mean Foo?" and "error class Foobar differs
     from class FooBar only in capitalization", and they don't do their checks for constants.
     Nevertheless, we maintain consistent behavior: get_canon_name only returns a name if
     that name is defined in the repository. *)
  let get_canon_name ctx name =
    let db_path_opt =
      Db_path_provider.get_naming_db_path (Provider_context.get_backend ctx)
    in
    match get_pos db_path_opt name with
    | Some _ -> Some name
    | None -> None

  let remove_batch db_path_opt consts =
    let hashes = consts |> List.map ~f:hash in
    ConstPosHeap.remove_batch (ConstPosHeap.KeySet.of_list hashes);
    match db_path_opt with
    | None -> ()
    | Some _ ->
      List.iter hashes ~f:(fun hash -> BlockedEntries.add hash Blocked);
      ()
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
