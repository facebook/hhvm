(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  client: Decl_ipc_ffi_externs.decl_client;
  fun_cache: Typing_defs.fun_elt option String.Table.t;
  class_cache: Shallow_decl_defs.shallow_class option String.Table.t;
  typedef_cache: Typing_defs.typedef_type option String.Table.t;
  record_cache: Typing_defs.record_def_type option String.Table.t;
  gconst_cache: Typing_defs.decl_ty option String.Table.t;
  gconst_path_cache: Relative_path.t option String.Table.t;
  fun_path_cache: Relative_path.t option String.Table.t;
  type_path_and_kind_cache:
    (Relative_path.t * Naming_types.kind_of_type) option String.Table.t;
}

let from_raw_client (client : Decl_ipc_ffi_externs.decl_client) : t =
  {
    client;
    fun_cache = String.Table.create ();
    class_cache = String.Table.create ();
    typedef_cache = String.Table.create ();
    record_cache = String.Table.create ();
    gconst_cache = String.Table.create ();
    gconst_path_cache = String.Table.create ();
    fun_path_cache = String.Table.create ();
    type_path_and_kind_cache = String.Table.create ();
  }

(* HACK: The decl service just stores the decl (rather than a decl option),
   so it either responds with a pointer to the decl_ty (when present) or the
   integer 0 (otherwise). Turn that into None/Some here. *)
let pointer_to_option (ptr : 'a) : 'a option =
  if Obj.is_int (Obj.repr ptr) then
    None
  else
    Some ptr

(* When writing information from a decl request into a naming cache or vice
   versa, we don't care whether an entry already exists, so we ignore the
   returned value (which indicates whether the added entry was a duplicate).  *)
let add_to_cache cache key value = ignore (String.Table.add cache key value)

let rpc_get_fun (t : t) (name : string) : Typing_defs.fun_elt option =
  match String.Table.find t.fun_cache name with
  | Some opt -> opt
  | None ->
    let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Fun name in
    let fun_elt_opt = pointer_to_option ptr in
    String.Table.add_exn t.fun_cache name fun_elt_opt;
    let path_opt =
      Option.map fun_elt_opt ~f:(fun fun_elt ->
          Pos.filename fun_elt.Typing_defs.fe_pos)
    in
    add_to_cache t.fun_path_cache name path_opt;
    fun_elt_opt

let rpc_get_class (t : t) (name : string) :
    Shallow_decl_defs.shallow_class option =
  match String.Table.find t.class_cache name with
  | Some opt -> opt
  | None ->
    let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Class name in
    let class_decl_opt = pointer_to_option ptr in
    String.Table.add_exn t.class_cache name class_decl_opt;
    let path_and_kind_opt =
      Option.map class_decl_opt ~f:(fun sc ->
          (Pos.filename (fst sc.Shallow_decl_defs.sc_name), Naming_types.TClass))
    in
    add_to_cache t.type_path_and_kind_cache name path_and_kind_opt;
    class_decl_opt

let rpc_get_typedef (t : t) (name : string) : Typing_defs.typedef_type option =
  match String.Table.find t.typedef_cache name with
  | Some opt -> opt
  | None ->
    let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Typedef name in
    let typedef_decl_opt = pointer_to_option ptr in
    String.Table.add_exn t.typedef_cache name typedef_decl_opt;
    let path_and_kind_opt =
      Option.map typedef_decl_opt ~f:(fun td ->
          (Pos.filename td.Typing_defs.td_pos, Naming_types.TTypedef))
    in
    add_to_cache t.type_path_and_kind_cache name path_and_kind_opt;
    typedef_decl_opt

let rpc_get_record_def (t : t) (name : string) :
    Typing_defs.record_def_type option =
  match String.Table.find t.record_cache name with
  | Some opt -> opt
  | None ->
    let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.RecordDef name in
    let record_decl_opt = pointer_to_option ptr in
    String.Table.add_exn t.record_cache name record_decl_opt;
    let path_and_kind_opt =
      Option.map record_decl_opt ~f:(fun rdt ->
          (Pos.filename rdt.Typing_defs.rdt_pos, Naming_types.TRecordDef))
    in
    add_to_cache t.type_path_and_kind_cache name path_and_kind_opt;
    record_decl_opt

let rpc_get_gconst (t : t) (name : string) : Typing_defs.decl_ty option =
  match String.Table.find t.gconst_cache name with
  | Some opt -> opt
  | None ->
    let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Const name in
    let gconst_ty_opt = pointer_to_option ptr in
    String.Table.add_exn t.gconst_cache name gconst_ty_opt;
    let path_opt =
      Option.map gconst_ty_opt ~f:(fun ty ->
          Pos.filename (Typing_defs.get_pos ty))
    in
    add_to_cache t.gconst_path_cache name path_opt;
    gconst_ty_opt

let rpc_get_gconst_path (t : t) (name : string) : Relative_path.t option =
  match String.Table.find t.gconst_path_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_const_path t.client name in
    String.Table.add_exn t.gconst_path_cache name opt;
    if Option.is_none opt then add_to_cache t.gconst_cache name None;
    opt

let rpc_get_fun_path (t : t) (name : string) : Relative_path.t option =
  match String.Table.find t.fun_path_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_fun_path t.client name in
    String.Table.add_exn t.fun_path_cache name opt;
    if Option.is_none opt then add_to_cache t.fun_cache name None;
    opt

let rpc_get_type_path_and_kind (t : t) (name : string) :
    (Relative_path.t * Naming_types.kind_of_type) option =
  match String.Table.find t.type_path_and_kind_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_type_path_and_kind t.client name in
    String.Table.add_exn t.type_path_and_kind_cache name opt;
    let kind_opt = Option.map opt snd in
    let ( <> ) a b = not (Option.equal Naming_types.equal_kind_of_type a b) in
    if kind_opt <> Some Naming_types.TClass then
      add_to_cache t.class_cache name None;
    if kind_opt <> Some Naming_types.TTypedef then
      add_to_cache t.typedef_cache name None;
    if kind_opt <> Some Naming_types.TRecordDef then
      add_to_cache t.record_cache name None;
    opt

let rpc_get_fun_canon_name (t : t) (name : string) : string option =
  Decl_ipc_ffi_externs.get_fun_canon_name t.client name

let rpc_get_type_canon_name (t : t) (name : string) : string option =
  Decl_ipc_ffi_externs.get_type_canon_name t.client name
