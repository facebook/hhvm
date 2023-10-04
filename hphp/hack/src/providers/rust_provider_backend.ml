(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t

external register_custom_types : unit -> unit
  = "hh_rust_provider_backend_register_custom_types"

let () = register_custom_types ()

external make_ffi :
  root:string -> hhi_root:string -> tmp:string -> ParserOptions.t -> t
  = "hh_rust_provider_backend_make"

external push_local_changes_ffi : t -> unit
  = "hh_rust_provider_backend_push_local_changes"

external pop_local_changes_ffi : t -> unit
  = "hh_rust_provider_backend_pop_local_changes"

external set_ctx_empty : t -> bool -> unit
  = "hh_rust_provider_backend_set_ctx_empty"
  [@@noalloc]

type find_symbol_fn = string -> (FileInfo.pos * FileInfo.name_type) option

type ctx_proxy = {
  get_entry_contents: Relative_path.t -> string option;
  is_pos_in_ctx: FileInfo.pos -> bool;
  find_fun_canon_name_in_context: string -> string option;
  find_type_canon_name_in_context: string -> string option;
  find_const_in_context: find_symbol_fn;
  find_fun_in_context: find_symbol_fn;
  find_type_in_context: find_symbol_fn;
  find_module_in_context: find_symbol_fn;
}

let ctx_proxy_ref : ctx_proxy option ref = ref None

let with_ctx_proxy_opt t ctx_proxy_opt f =
  assert (Option.is_none !ctx_proxy_ref);
  ctx_proxy_ref := ctx_proxy_opt;
  set_ctx_empty t (Option.is_none ctx_proxy_opt);
  try
    let result = f () in
    ctx_proxy_ref := None;
    set_ctx_empty t true;
    result
  with
  | e ->
    ctx_proxy_ref := None;
    set_ctx_empty t true;
    raise e

let get_entry_contents x =
  match !ctx_proxy_ref with
  | None -> None
  | Some { get_entry_contents; _ } -> get_entry_contents x

let is_pos_in_ctx x =
  match !ctx_proxy_ref with
  | None -> false
  | Some { is_pos_in_ctx; _ } -> is_pos_in_ctx x

let find_fun_canon_name_in_context x =
  match !ctx_proxy_ref with
  | None -> None
  | Some { find_fun_canon_name_in_context; _ } ->
    find_fun_canon_name_in_context x

let find_type_canon_name_in_context x =
  match !ctx_proxy_ref with
  | None -> None
  | Some { find_type_canon_name_in_context; _ } ->
    find_type_canon_name_in_context x

let find_const_in_context x =
  match !ctx_proxy_ref with
  | None -> None
  | Some { find_const_in_context; _ } -> find_const_in_context x

let find_fun_in_context x =
  match !ctx_proxy_ref with
  | None -> None
  | Some { find_fun_in_context; _ } -> find_fun_in_context x

let find_type_in_context x =
  match !ctx_proxy_ref with
  | None -> None
  | Some { find_type_in_context; _ } -> find_type_in_context x

let find_module_in_context x =
  match !ctx_proxy_ref with
  | None -> None
  | Some { find_module_in_context; _ } -> find_module_in_context x

let () =
  Callback.register
    "hh_rust_provider_backend_get_entry_contents"
    get_entry_contents;
  Callback.register "hh_rust_provider_backend_is_pos_in_ctx" is_pos_in_ctx;
  Callback.register
    "hh_rust_provider_backend_find_fun_canon_name_in_context"
    find_fun_canon_name_in_context;
  Callback.register
    "hh_rust_provider_backend_find_type_canon_name_in_context"
    find_type_canon_name_in_context;
  Callback.register
    "hh_rust_provider_backend_find_const_in_context"
    find_const_in_context;
  Callback.register
    "hh_rust_provider_backend_find_fun_in_context"
    find_fun_in_context;
  Callback.register
    "hh_rust_provider_backend_find_type_in_context"
    find_type_in_context;
  Callback.register
    "hh_rust_provider_backend_find_module_in_context"
    find_module_in_context;
  ()

module Decl = struct
  module type Store = sig
    type key

    type value

    val get : t -> key -> value option

    val clear_cache : unit -> unit
  end

  module StoreWithLocalCache
      (Key : SharedMem.Key)
      (Value : SharedMem.Value) (Ffi : sig
        val get : t -> Key.t -> Value.t option
      end) : Store with type key = Key.t and type value = Value.t = struct
    type key = Key.t

    type value = Value.t

    module Cache =
      SharedMem.FreqCache (Key) (Value)
        (struct
          let capacity = 1000
        end)

    let clear_cache = Cache.clear

    let log_hit_rate ~hit =
      let hit =
        if hit then
          1.
        else
          0.
      in
      Measure.sample (Value.description ^ " (ffi cache hit rate)") hit;
      Measure.sample "ALL ffi cache hit rate" hit

    let get t key =
      let v = Cache.get key in
      if SharedMem.SMTelemetry.hh_log_level () > 0 then
        log_hit_rate ~hit:(Option.is_some v);
      match v with
      | Some _ -> v
      | None ->
        let value_opt = Ffi.get t key in
        (match value_opt with
        | Some value -> Cache.add key value
        | None -> ());
        value_opt
  end

  module Funs =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.fun_decl

        let description = "Decl_Fun"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.fun_decl option
          = "hh_rust_provider_backend_get_fun"
      end)

  module ShallowClasses =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.class_decl

        let description = "Decl_ShallowClass"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.class_decl option
          = "hh_rust_provider_backend_get_shallow_class"
      end)

  module Typedefs =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.typedef_decl

        let description = "Decl_Typedef"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.typedef_decl option
          = "hh_rust_provider_backend_get_typedef"
      end)

  module GConsts =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.const_decl

        let description = "Decl_GConst"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.const_decl option
          = "hh_rust_provider_backend_get_gconst"
      end)

  module Modules =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.module_decl

        let description = "Decl_Module"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.module_decl option
          = "hh_rust_provider_backend_get_module"
      end)

  module FoldedClasses =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Decl_defs.decl_class_type

        let description = "Decl_Class"
      end)
      (struct
        external get : t -> string -> Decl_defs.decl_class_type option
          = "hh_rust_provider_backend_get_folded_class"
      end)

  let decl_store t =
    let noop_add _ _ = () in
    let noop () = () in
    (* Rely on lazy member lookup. *)
    let get_none _ = None in
    Decl_store.
      {
        add_prop = noop_add;
        get_prop = get_none;
        add_static_prop = noop_add;
        get_static_prop = get_none;
        add_method = noop_add;
        get_method = get_none;
        add_static_method = noop_add;
        get_static_method = get_none;
        add_constructor = noop_add;
        get_constructor = get_none;
        add_class = noop_add;
        get_class = FoldedClasses.get t;
        add_fun = noop_add;
        get_fun = Funs.get t;
        add_typedef = noop_add;
        get_typedef = Typedefs.get t;
        add_gconst = noop_add;
        get_gconst = GConsts.get t;
        add_module = noop_add;
        get_module = Modules.get t;
        pop_local_changes = noop;
        push_local_changes = noop;
      }

  let did_set_decl_store = ref false

  let set_decl_store t =
    if not !did_set_decl_store then (
      did_set_decl_store := true;
      Decl_store.set (decl_store t)
    )

  external direct_decl_parse_and_cache :
    t -> Relative_path.t -> string -> Direct_decl_parser.parsed_file_with_hashes
    = "hh_rust_provider_backend_direct_decl_parse_and_cache"

  let direct_decl_parse_and_cache t =
    set_decl_store t;
    direct_decl_parse_and_cache t

  external add_shallow_decls :
    t -> (string * Shallow_decl_defs.decl) list -> unit
    = "hh_rust_provider_backend_add_shallow_decls"

  let add_shallow_decls t =
    set_decl_store t;
    add_shallow_decls t

  let get_fun t ctx name =
    set_decl_store t;
    with_ctx_proxy_opt t ctx @@ fun () -> Funs.get t name

  let get_shallow_class t ctx name =
    set_decl_store t;
    with_ctx_proxy_opt t ctx @@ fun () -> ShallowClasses.get t name

  let get_typedef t ctx name =
    set_decl_store t;
    with_ctx_proxy_opt t ctx @@ fun () -> Typedefs.get t name

  let get_gconst t ctx name =
    set_decl_store t;
    with_ctx_proxy_opt t ctx @@ fun () -> GConsts.get t name

  let get_module t ctx name =
    set_decl_store t;
    with_ctx_proxy_opt t ctx @@ fun () -> Modules.get t name

  let get_folded_class t ctx name =
    set_decl_store t;
    with_ctx_proxy_opt t ctx @@ fun () -> FoldedClasses.get t name

  external oldify_defs_ffi : t -> FileInfo.names -> unit
    = "hh_rust_provider_backend_oldify_defs"

  external remove_old_defs_ffi : t -> FileInfo.names -> unit
    = "hh_rust_provider_backend_remove_old_defs"

  external remove_defs_ffi : t -> FileInfo.names -> unit
    = "hh_rust_provider_backend_remove_defs"

  external get_old_defs_ffi :
    t ->
    FileInfo.names ->
    Shallow_decl_defs.class_decl option SMap.t
    * Shallow_decl_defs.fun_decl option SMap.t
    * Shallow_decl_defs.typedef_decl option SMap.t
    * Shallow_decl_defs.const_decl option SMap.t
    * Shallow_decl_defs.module_decl option SMap.t
    = "hh_rust_provider_backend_get_old_defs"

  let clear_caches () =
    Funs.clear_cache ();
    ShallowClasses.clear_cache ();
    FoldedClasses.clear_cache ();
    Typedefs.clear_cache ();
    GConsts.clear_cache ();
    Modules.clear_cache ();
    ()

  let oldify_defs t names =
    set_decl_store t;
    oldify_defs_ffi t names;
    clear_caches ();
    ()

  let remove_old_defs t names =
    set_decl_store t;
    remove_old_defs_ffi t names;
    clear_caches ();
    ()

  let remove_defs t names =
    set_decl_store t;
    remove_defs_ffi t names;
    clear_caches ();
    ()

  let get_old_defs t names =
    set_decl_store t;
    get_old_defs_ffi t names

  external declare_folded_class : t -> string -> unit
    = "hh_rust_provider_backend_declare_folded_class"

  let declare_folded_class t ctx name =
    set_decl_store t;
    with_ctx_proxy_opt t ctx @@ fun () -> declare_folded_class t name
end

let make popt =
  let backend =
    make_ffi
      ~root:Relative_path.(path_of_prefix Root)
      ~hhi_root:Relative_path.(path_of_prefix Hhi)
      ~tmp:Relative_path.(path_of_prefix Tmp)
      popt
  in
  Decl.set_decl_store backend;
  backend

let set backend = Decl.set_decl_store backend

let push_local_changes t =
  Decl.clear_caches ();
  push_local_changes_ffi t

let pop_local_changes t =
  Decl.clear_caches ();
  pop_local_changes_ffi t

module File = struct
  external get_contents : t -> Relative_path.t -> string
    = "hh_rust_provider_backend_file_provider_get_contents"

  external provide_file_for_tests : t -> Relative_path.t -> string -> unit
    = "hh_rust_provider_backend_file_provider_provide_file_for_tests"

  external remove_batch : t -> Relative_path.Set.t -> unit
    = "hh_rust_provider_backend_file_provider_remove_batch"
end

module Naming = struct
  module type ReverseNamingTable = sig
    type pos

    val add : t -> string -> pos -> unit

    val get_pos : t -> ctx_proxy option -> string -> pos option

    val remove_batch : t -> string list -> unit
  end

  module Types = struct
    type pos = FileInfo.pos * Naming_types.kind_of_type

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_types_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_types_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_types_remove_batch"

    external get_canon_name : t -> string -> string option
      = "hh_rust_provider_backend_naming_types_get_canon_name"

    let get_pos t ctx name =
      with_ctx_proxy_opt t ctx @@ fun () -> get_pos t name

    let get_canon_name t ctx name =
      with_ctx_proxy_opt t ctx @@ fun () -> get_canon_name t name
  end

  module Funs = struct
    type pos = FileInfo.pos

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_funs_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_funs_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_funs_remove_batch"

    external get_canon_name : t -> string -> string option
      = "hh_rust_provider_backend_naming_funs_get_canon_name"

    let get_pos t ctx name =
      with_ctx_proxy_opt t ctx @@ fun () -> get_pos t name

    let get_canon_name t ctx name =
      with_ctx_proxy_opt t ctx @@ fun () -> get_canon_name t name
  end

  module Consts = struct
    type pos = FileInfo.pos

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_consts_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_consts_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_consts_remove_batch"

    let get_pos t ctx name =
      with_ctx_proxy_opt t ctx @@ fun () -> get_pos t name
  end

  module Modules = struct
    type pos = FileInfo.pos

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_modules_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_modules_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_modules_remove_batch"

    let get_pos t ctx name =
      with_ctx_proxy_opt t ctx @@ fun () -> get_pos t name
  end

  external get_db_path_ffi : t -> string option
    = "hh_rust_provider_backend_naming_get_db_path"

  let get_db_path t =
    get_db_path_ffi t |> Option.map ~f:(fun path -> Naming_sqlite.Db_path path)

  external set_db_path_ffi : t -> string -> unit
    = "hh_rust_provider_backend_naming_set_db_path"

  let set_db_path t (Naming_sqlite.Db_path path) = set_db_path_ffi t path

  external get_filenames_by_hash :
    t -> Typing_deps.DepSet.t -> Relative_path.Set.t
    = "hh_rust_provider_backend_naming_get_filenames_by_hash"
end
