(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections

module CacheKey = struct
  type t = Typing_deps.Dep.t

  let compare = Typing_deps.Dep.compare

  let to_string = Typing_deps.Dep.to_hex_string
end

module SymbolMap = Reordered_argument_map (WrappedMap.Make (CacheKey))

type decl = Shallow_decl_defs.decl =
  | Class of Shallow_decl_defs.class_decl
  | Fun of Shallow_decl_defs.fun_decl
  | Record of Shallow_decl_defs.record_decl
  | Typedef of Shallow_decl_defs.typedef_decl
  | Const of Shallow_decl_defs.const_decl

module Decls =
  SharedMem.LocalCache
    (CacheKey)
    (struct
      type t = decl option

      let prefix = Prefix.make ()

      let description = "Decl_service_client_Decls"
    end)
    (struct
      let capacity = 10000
    end)

type t = {
  client: Decl_ipc_ffi_externs.decl_client;
  opts: DeclParserOptions.t;
  mutable current_file_decls: decl SymbolMap.t;
  gconst_path_cache: Relative_path.t option String.Table.t;
  fun_path_cache: Relative_path.t option String.Table.t;
  type_path_and_kind_cache:
    (Relative_path.t * Naming_types.kind_of_type) option String.Table.t;
}

let from_raw_client
    (client : Decl_ipc_ffi_externs.decl_client) (opts : DeclParserOptions.t) : t
    =
  {
    client;
    opts;
    current_file_decls = SymbolMap.empty;
    gconst_path_cache = String.Table.create ();
    fun_path_cache = String.Table.create ();
    type_path_and_kind_cache = String.Table.create ();
  }

let get_and_cache_decl (t : t) (symbol_hash : Typing_deps.Dep.t) =
  let decl_opt = Decl_ipc_ffi_externs.get_decl t.client symbol_hash in
  Decls.add symbol_hash decl_opt;
  decl_opt

let rpc_get_fun (t : t) (name : string) : Typing_defs.fun_elt option =
  let key = Typing_deps.(Dep.make Mode.Hash64Bit (Dep.Fun name)) in
  match SymbolMap.find_opt t.current_file_decls key with
  | Some (Fun decl) -> Some decl
  | _ ->
    (match Decls.get key with
    | Some (Some (Fun decl)) -> Some decl
    | Some _ -> None
    | None ->
      (match get_and_cache_decl t key with
      | Some (Fun f) -> Some f
      | _ -> None))

let rpc_get_class (t : t) (name : string) :
    Shallow_decl_defs.shallow_class option =
  let key = Typing_deps.(Dep.make Mode.Hash64Bit (Dep.Type name)) in
  match SymbolMap.find_opt t.current_file_decls key with
  | Some (Class decl) -> Some decl
  | _ ->
    (match Decls.get key with
    | Some (Some (Class decl)) -> Some decl
    | Some _ -> None
    | None ->
      (match get_and_cache_decl t key with
      | Some (Class c) -> Some c
      | _ -> None))

let rpc_get_typedef (t : t) (name : string) : Typing_defs.typedef_type option =
  let key = Typing_deps.(Dep.make Mode.Hash64Bit (Dep.Type name)) in
  match SymbolMap.find_opt t.current_file_decls key with
  | Some (Typedef decl) -> Some decl
  | _ ->
    (match Decls.get key with
    | Some (Some (Typedef decl)) -> Some decl
    | Some _ -> None
    | None ->
      (match get_and_cache_decl t key with
      | Some (Typedef t) -> Some t
      | _ -> None))

let rpc_get_record_def (t : t) (name : string) :
    Typing_defs.record_def_type option =
  let key = Typing_deps.(Dep.make Mode.Hash64Bit (Dep.Type name)) in
  match SymbolMap.find_opt t.current_file_decls key with
  | Some (Record decl) -> Some decl
  | _ ->
    (match Decls.get key with
    | Some (Some (Record decl)) -> Some decl
    | Some _ -> None
    | None ->
      (match get_and_cache_decl t key with
      | Some (Record r) -> Some r
      | _ -> None))

let rpc_get_gconst (t : t) (name : string) : Typing_defs.const_decl option =
  let key = Typing_deps.(Dep.make Mode.Hash64Bit (Dep.GConst name)) in
  match SymbolMap.find_opt t.current_file_decls key with
  | Some (Const decl) -> Some decl
  | _ ->
    (match Decls.get key with
    | Some (Some (Const decl)) -> Some decl
    | Some _ -> None
    | None ->
      (match get_and_cache_decl t key with
      | Some (Const c) -> Some c
      | _ -> None))

let rpc_get_gconst_path (t : t) (name : string) : Relative_path.t option =
  match String.Table.find t.gconst_path_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_const_path t.client name in
    String.Table.add_exn t.gconst_path_cache ~key:name ~data:opt;
    let key = Typing_deps.(Dep.make Mode.Hash64Bit (Dep.GConst name)) in
    if Option.is_none opt then Decls.add key None;
    opt

let rpc_get_fun_path (t : t) (name : string) : Relative_path.t option =
  match String.Table.find t.fun_path_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_fun_path t.client name in
    String.Table.add_exn t.fun_path_cache ~key:name ~data:opt;
    let key = Typing_deps.(Dep.make Mode.Hash64Bit (Dep.Fun name)) in
    if Option.is_none opt then Decls.add key None;
    opt

let rpc_get_type_path_and_kind (t : t) (name : string) :
    (Relative_path.t * Naming_types.kind_of_type) option =
  match String.Table.find t.type_path_and_kind_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_type_path_and_kind t.client name in
    String.Table.add_exn t.type_path_and_kind_cache ~key:name ~data:opt;
    opt

let rpc_get_fun_canon_name (t : t) (name : string) : string option =
  Decl_ipc_ffi_externs.get_fun_canon_name t.client name

let rpc_get_type_canon_name (t : t) (name : string) : string option =
  Decl_ipc_ffi_externs.get_type_canon_name t.client name

let parse_and_cache_decls_in
    (t : t) (filename : Relative_path.t) (contents : string) : unit =
  let decls = Direct_decl_parser.parse_decls_ffi t.opts filename contents in
  let hash dep = Typing_deps.(Dep.make Mode.Hash64Bit dep) in
  t.current_file_decls <-
    List.fold decls ~init:SymbolMap.empty ~f:(fun map (name, decl) ->
        let key =
          match decl with
          | Fun _ -> hash (Typing_deps.Dep.Fun name)
          | Class _ -> hash (Typing_deps.Dep.Type name)
          | Typedef _ -> hash (Typing_deps.Dep.Type name)
          | Record _ -> hash (Typing_deps.Dep.Type name)
          | Const _ -> hash (Typing_deps.Dep.GConst name)
        in
        SymbolMap.add map ~key ~data:decl)
