(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Decl_cache_entry = struct
  (* NOTE: we can't simply use a string as a key. In the case of a name
     conflict, we may put e.g. a function named 'foo' into the cache whose value is
     one type, and then later try to withdraw a class named 'foo' whose value is
     another type. *)
  type _ t =
    | Fun_decl : string -> Typing_defs.fun_elt t
    | Class_decl : string -> Typing_class_types.class_t t
    | Typedef_decl : string -> Typing_defs.typedef_type t
    | Gconst_decl : string -> Typing_defs.const_decl t
    | Module_decl : string -> Typing_defs.module_def_type t

  type 'a key = 'a t

  type 'a value = 'a

  let get_size ~key:_ ~value:_ = 1

  let key_to_log_string : type a. a key -> string =
   fun key ->
    match key with
    | Fun_decl s -> "FunDecl" ^ s
    | Class_decl s -> "ClassDecl" ^ s
    | Typedef_decl s -> "TypedefDecl" ^ s
    | Gconst_decl s -> "GconstDecl" ^ s
    | Module_decl s -> "ModuleDecl" ^ s
end

module Cache (Entry : Lfu_cache.Entry) = Lfu_cache.Cache (Entry)
module Decl_cache = Cache (Decl_cache_entry)

module Shallow_decl_cache_entry = struct
  type _ t = Shallow_class_decl : string -> Shallow_decl_defs.shallow_class t

  type 'a key = 'a t

  type 'a value = 'a

  let get_size ~key:_ ~value:_ = 1

  let key_to_log_string : type a. a key -> string =
   (fun (Shallow_class_decl key) -> "ClasssShallow" ^ key)
end

module Shallow_decl_cache = Cache (Shallow_decl_cache_entry)

module Folded_class_cache_entry = struct
  type _ t = Folded_class_decl : string -> Decl_defs.decl_class_type t

  type 'a key = 'a t

  type 'a value = 'a

  let get_size ~key:_ ~value:_ = 1

  let key_to_log_string : type a. a key -> string =
   (fun (Folded_class_decl key) -> "ClassFolded" ^ key)
end

module Folded_class_cache = Cache (Folded_class_cache_entry)

module Linearization_cache_entry = struct
  type _ t = Linearization : string -> Decl_defs.lin t

  type 'a key = 'a t

  type 'a value = 'a

  let get_size ~key:_ ~value:_ = 1

  let key_to_log_string : type a. a key -> string =
   fun key ->
    match key with
    | Linearization c -> "Linearization" ^ c
end

module Linearization_cache = Cache (Linearization_cache_entry)

type fixme_map = Pos.t IMap.t IMap.t [@@deriving show]

module Fixme_store = struct
  type t = fixme_map Relative_path.Map.t ref

  let empty = ref Relative_path.Map.empty

  let get t filename = Relative_path.Map.find_opt !t filename

  let add t filename fixmes =
    t := Relative_path.Map.add !t ~key:filename ~data:fixmes

  let remove t filename = t := Relative_path.Map.remove !t filename

  let remove_batch t filenames =
    t :=
      Relative_path.Set.fold filenames ~init:!t ~f:(fun filename map ->
          Relative_path.Map.remove map filename)
end

module Fixmes = struct
  type t = {
    hh_fixmes: Fixme_store.t;
    decl_hh_fixmes: Fixme_store.t;
    disallowed_fixmes: Fixme_store.t;
  }

  let get_telemetry ~(key : string) (t : t) (telemetry : Telemetry.t) :
      Telemetry.t =
    let hh_fixme_files = Relative_path.Map.cardinal !(t.hh_fixmes) in
    let decl_hh_fixme_files = Relative_path.Map.cardinal !(t.decl_hh_fixmes) in
    let disallowed_fixme_files =
      Relative_path.Map.cardinal !(t.disallowed_fixmes)
    in
    if hh_fixme_files + decl_hh_fixme_files + disallowed_fixme_files = 0 then
      telemetry
    else
      let sub_telemetry =
        Telemetry.create ()
        |> Telemetry.int_ ~key:"hh_fixme_files" ~value:hh_fixme_files
        |> Telemetry.int_ ~key:"decl_hh_fixme_files" ~value:decl_hh_fixme_files
        |> Telemetry.int_
             ~key:"disallowed_fixme_files"
             ~value:disallowed_fixme_files
      in
      Telemetry.object_ telemetry ~key ~value:sub_telemetry
end

let empty_fixmes =
  Fixmes.
    {
      hh_fixmes = Fixme_store.empty;
      decl_hh_fixmes = Fixme_store.empty;
      disallowed_fixmes = Fixme_store.empty;
    }

module Reverse_naming_table_delta = struct
  type pos = FileInfo.name_type * Relative_path.t

  type pos_or_deleted =
    | Pos of pos * pos list
    | Deleted

  type t = {
    consts: pos_or_deleted SMap.t ref;
    funs: pos_or_deleted SMap.t ref;
    types: pos_or_deleted SMap.t ref;
    modules: pos_or_deleted SMap.t ref;
    funs_canon_key: pos_or_deleted SMap.t ref;
    types_canon_key: pos_or_deleted SMap.t ref;
  }

  let make () : t =
    {
      consts = ref SMap.empty;
      funs = ref SMap.empty;
      types = ref SMap.empty;
      modules = ref SMap.empty;
      funs_canon_key = ref SMap.empty;
      types_canon_key = ref SMap.empty;
    }

  let get_telemetry ~(key : string) (t : t) (telemetry : Telemetry.t) :
      Telemetry.t =
    let consts = SMap.cardinal !(t.consts) in
    let funs = SMap.cardinal !(t.funs) in
    let types = SMap.cardinal !(t.types) in
    let modules = SMap.cardinal !(t.modules) in
    if consts + funs + types + modules = 0 then
      telemetry
    else
      let sub_telemetry =
        Telemetry.create ()
        |> Telemetry.int_ ~key:"consts" ~value:consts
        |> Telemetry.int_ ~key:"funs" ~value:funs
        |> Telemetry.int_ ~key:"types" ~value:types
        |> Telemetry.int_ ~key:"modules" ~value:modules
      in
      Telemetry.object_ telemetry ~key ~value:sub_telemetry
end

type local_memory = {
  shallow_decl_cache: Shallow_decl_cache.t;
  folded_class_cache: Folded_class_cache.t;
  decl_cache: Decl_cache.t;
  linearization_cache: Linearization_cache.t;
  reverse_naming_table_delta: Reverse_naming_table_delta.t;
  fixmes: Fixmes.t;
  naming_db_path_ref: Naming_sqlite.db_path option ref;
}

type t =
  | Shared_memory
  | Local_memory of local_memory
  | Decl_service of {
      decl: Decl_service_client.t;
      fixmes: Fixmes.t;
    }
  | Rust_provider_backend of Rust_provider_backend.t
  | Analysis

let t_to_string (t : t) : string =
  match t with
  | Shared_memory -> "Shared_memory"
  | Local_memory _ -> "Local_memory"
  | Decl_service _ -> "Decl_service"
  | Rust_provider_backend _ -> "Rust_provider_backend"
  | Analysis -> "Analysis"

let backend_ref = ref Shared_memory

let set_analysis_backend () : unit = backend_ref := Analysis

let set_shared_memory_backend () : unit =
  backend_ref := Shared_memory;
  Decl_store.set Decl_store.shared_memory_store;
  ()

let set_rust_backend popt : unit =
  backend_ref := Rust_provider_backend (Rust_provider_backend.make popt)

let make_decl_store_from_local_memory
    ({ decl_cache; folded_class_cache; _ } : local_memory) :
    Decl_store.decl_store =
  {
    Decl_store.add_class =
      (fun k v ->
        Folded_class_cache.add
          folded_class_cache
          ~key:(Folded_class_cache_entry.Folded_class_decl k)
          ~value:v);
    get_class =
      (fun k : Decl_defs.decl_class_type option ->
        Folded_class_cache.find_or_add
          folded_class_cache
          ~key:(Folded_class_cache_entry.Folded_class_decl k)
          ~default:(fun _ -> None));
    add_typedef =
      (fun k v ->
        Decl_cache.add
          decl_cache
          ~key:(Decl_cache_entry.Typedef_decl k)
          ~value:v);
    get_typedef =
      (fun k ->
        Decl_cache.find_or_add
          decl_cache
          ~key:(Decl_cache_entry.Typedef_decl k)
          ~default:(fun _ -> None));
    add_module =
      (fun k v ->
        Decl_cache.add decl_cache ~key:(Decl_cache_entry.Module_decl k) ~value:v);
    get_module =
      (fun k ->
        Decl_cache.find_or_add
          decl_cache
          ~key:(Decl_cache_entry.Module_decl k)
          ~default:(fun _ -> None));
    add_fun =
      (fun k v ->
        Decl_cache.add decl_cache ~key:(Decl_cache_entry.Fun_decl k) ~value:v);
    get_fun =
      (fun k ->
        Decl_cache.find_or_add
          decl_cache
          ~key:(Decl_cache_entry.Fun_decl k)
          ~default:(fun _ -> None));
    add_gconst =
      (fun k v ->
        Decl_cache.add decl_cache ~key:(Decl_cache_entry.Gconst_decl k) ~value:v);
    get_gconst =
      (fun k ->
        Decl_cache.find_or_add
          decl_cache
          ~key:(Decl_cache_entry.Gconst_decl k)
          ~default:(fun _ -> None));
    add_method = (fun _ _ -> ());
    get_method = (fun _ -> None);
    add_static_method = (fun _ _ -> ());
    get_static_method = (fun _ -> None);
    add_prop = (fun _ _ -> ());
    get_prop = (fun _ -> None);
    add_static_prop = (fun _ _ -> ());
    get_static_prop = (fun _ -> None);
    add_constructor = (fun _ _ -> ());
    get_constructor = (fun _ -> None);
    push_local_changes = (fun () -> ());
    pop_local_changes = (fun () -> ());
  }

let set_local_memory_backend_internal
    ~(max_num_decls : int)
    ~(max_num_shallow_decls : int)
    ~(max_num_linearizations : int) : unit =
  let local_memory =
    {
      decl_cache = Decl_cache.make ~max_size:max_num_decls;
      folded_class_cache = Folded_class_cache.make ~max_size:max_num_decls;
      shallow_decl_cache =
        Shallow_decl_cache.make ~max_size:max_num_shallow_decls;
      linearization_cache =
        Linearization_cache.make ~max_size:max_num_linearizations;
      reverse_naming_table_delta = Reverse_naming_table_delta.make ();
      fixmes = empty_fixmes;
      naming_db_path_ref = ref None;
    }
  in
  backend_ref := Local_memory local_memory;
  Decl_store.set @@ make_decl_store_from_local_memory local_memory;
  ()

let set_local_memory_backend
    ~(max_num_decls : int)
    ~(max_num_shallow_decls : int)
    ~(max_num_linearizations : int) =
  Hh_logger.log
    "Provider_backend.Local_memory cache sizes: max_num_decls=%d max_num_shallow_decls=%d max_num_linearizations=%d"
    max_num_decls
    max_num_shallow_decls
    max_num_linearizations;
  set_local_memory_backend_internal
    ~max_num_decls
    ~max_num_shallow_decls
    ~max_num_linearizations

let set_local_memory_backend_with_defaults_for_test () : unit =
  (* These are all arbitrary, so that test can spin up the backend easily;
     they haven't been tuned and shouldn't be used in production. *)
  set_local_memory_backend_internal
    ~max_num_decls:5000
    ~max_num_shallow_decls:(140 * 1024 * 1024)
    ~max_num_linearizations:10000

let set_decl_service_backend (decl : Decl_service_client.t) : unit =
  backend_ref := Decl_service { decl; fixmes = empty_fixmes }

let get () : t = !backend_ref

let supports_eviction (t : t) : bool =
  match t with
  | Analysis -> false
  | Local_memory _
  | Decl_service _
  | Rust_provider_backend _
  | Shared_memory ->
    true
