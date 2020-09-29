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
  type t = FileInfo.name_type * string [@@deriving show, ord]

  let to_string = show
end

type decl =
  | Fun_decl of Typing_defs.fun_elt
  | Class_decl of Shallow_decl_defs.shallow_class
  | Typedef_decl of Typing_defs.typedef_type
  | Record_decl of Typing_defs.record_def_type
  | Gconst_decl of Typing_defs.decl_ty

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
  mutable current_file_decls: Direct_decl_parser.decls;
  gconst_path_cache: Relative_path.t option String.Table.t;
  fun_path_cache: Relative_path.t option String.Table.t;
  type_path_and_kind_cache:
    (Relative_path.t * Naming_types.kind_of_type) option String.Table.t;
}

let from_raw_client (client : Decl_ipc_ffi_externs.decl_client) : t =
  {
    client;
    current_file_decls = Direct_decl_parser.empty_decls;
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
  match SMap.find_opt t.current_file_decls.Direct_decl_parser.funs name with
  | Some decl -> Some decl
  | None ->
    (match Decls.get (FileInfo.Fun, name) with
    | Some (Some (Fun_decl decl)) -> Some decl
    | Some (Some _) -> assert false
    | Some None -> None
    | None ->
      let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Fun name in
      let fun_elt_opt = pointer_to_option ptr in
      Decls.add
        (FileInfo.Fun, name)
        (Option.map fun_elt_opt ~f:(fun x -> Fun_decl x));
      let path_opt =
        Option.map fun_elt_opt ~f:(fun fun_elt ->
            Pos.filename fun_elt.Typing_defs.fe_pos)
      in
      add_to_cache t.fun_path_cache name path_opt;
      fun_elt_opt)

let rpc_get_class (t : t) (name : string) :
    Shallow_decl_defs.shallow_class option =
  match SMap.find_opt t.current_file_decls.Direct_decl_parser.classes name with
  | Some decl -> Some decl
  | None ->
    (match Decls.get (FileInfo.Class, name) with
    | Some (Some (Class_decl decl)) -> Some decl
    | Some (Some _) -> assert false
    | Some None -> None
    | None ->
      let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Class name in
      let class_decl_opt = pointer_to_option ptr in
      Decls.add
        (FileInfo.Class, name)
        (Option.map class_decl_opt ~f:(fun x -> Class_decl x));
      Option.iter class_decl_opt ~f:(fun sc ->
          add_to_cache
            t.type_path_and_kind_cache
            name
            (Some
               ( Pos.filename (fst sc.Shallow_decl_defs.sc_name),
                 Naming_types.TClass )));
      class_decl_opt)

let rpc_get_typedef (t : t) (name : string) : Typing_defs.typedef_type option =
  match SMap.find_opt t.current_file_decls.Direct_decl_parser.typedefs name with
  | Some decl -> Some decl
  | None ->
    (match Decls.get (FileInfo.Typedef, name) with
    | Some (Some (Typedef_decl decl)) -> Some decl
    | Some (Some _) -> assert false
    | Some None -> None
    | None ->
      let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Typedef name in
      let typedef_decl_opt = pointer_to_option ptr in
      Decls.add
        (FileInfo.Typedef, name)
        (Option.map typedef_decl_opt ~f:(fun x -> Typedef_decl x));
      Option.iter typedef_decl_opt ~f:(fun td ->
          add_to_cache
            t.type_path_and_kind_cache
            name
            (Some (Pos.filename td.Typing_defs.td_pos, Naming_types.TTypedef)));
      typedef_decl_opt)

let rpc_get_record_def (t : t) (name : string) :
    Typing_defs.record_def_type option =
  match Decls.get (FileInfo.RecordDef, name) with
  | Some (Some (Record_decl decl)) -> Some decl
  | Some (Some _) -> assert false
  | Some None -> None
  | None ->
    let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.RecordDef name in
    let record_decl_opt = pointer_to_option ptr in
    Decls.add
      (FileInfo.RecordDef, name)
      (Option.map record_decl_opt ~f:(fun x -> Record_decl x));
    Option.iter record_decl_opt ~f:(fun rdt ->
        add_to_cache
          t.type_path_and_kind_cache
          name
          (Some (Pos.filename rdt.Typing_defs.rdt_pos, Naming_types.TRecordDef)));
    record_decl_opt

let rpc_get_gconst (t : t) (name : string) : Typing_defs.decl_ty option =
  match SMap.find_opt t.current_file_decls.Direct_decl_parser.consts name with
  | Some decl -> Some decl.Typing_defs.cd_type
  | None ->
    (match Decls.get (FileInfo.Const, name) with
    | Some (Some (Gconst_decl decl)) -> Some decl
    | Some (Some _) -> assert false
    | Some None -> None
    | None ->
      let ptr = Decl_ipc_ffi_externs.get_decl t.client FileInfo.Const name in
      let gconst_decl_opt = pointer_to_option ptr in
      let gconst_ty_opt =
        Option.map gconst_decl_opt (fun c -> c.Typing_defs.cd_type)
      in
      Decls.add
        (FileInfo.Const, name)
        (Option.map gconst_ty_opt ~f:(fun x -> Gconst_decl x));
      let path_opt =
        Option.map gconst_ty_opt ~f:(fun ty ->
            Pos.filename (Typing_defs.get_pos ty))
      in
      add_to_cache t.gconst_path_cache name path_opt;
      gconst_ty_opt)

let rpc_get_gconst_path (t : t) (name : string) : Relative_path.t option =
  match String.Table.find t.gconst_path_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_const_path t.client name in
    String.Table.add_exn t.gconst_path_cache name opt;
    if Option.is_none opt then Decls.add (FileInfo.Const, name) None;
    opt

let rpc_get_fun_path (t : t) (name : string) : Relative_path.t option =
  match String.Table.find t.fun_path_cache name with
  | Some opt -> opt
  | None ->
    let opt = Decl_ipc_ffi_externs.get_fun_path t.client name in
    String.Table.add_exn t.fun_path_cache name opt;
    if Option.is_none opt then Decls.add (FileInfo.Fun, name) None;
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
      Decls.add (FileInfo.Class, name) None;
    if kind_opt <> Some Naming_types.TTypedef then
      Decls.add (FileInfo.Typedef, name) None;
    if kind_opt <> Some Naming_types.TRecordDef then
      Decls.add (FileInfo.RecordDef, name) None;
    opt

let rpc_get_fun_canon_name (t : t) (name : string) : string option =
  Decl_ipc_ffi_externs.get_fun_canon_name t.client name

let rpc_get_type_canon_name (t : t) (name : string) : string option =
  Decl_ipc_ffi_externs.get_type_canon_name t.client name

let parse_and_cache_decls_in
    (t : t) (filename : Relative_path.t) (contents : string) : unit =
  let decls = Direct_decl_parser.parse_decls_ffi filename contents in
  t.current_file_decls <- decls
