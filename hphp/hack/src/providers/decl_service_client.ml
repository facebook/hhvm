(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type t = Decl_ipc_ffi_externs.decl_client

(* HACK: The decl service just stores the decl (rather than a decl option),
   so it either responds with a pointer to the decl_ty (when present) or the
   integer 0 (otherwise). Turn that into None/Some here. *)
let pointer_to_option (ptr : 'a) (caller : string) : 'a option =
  let ptr_as_int : int = Obj.magic ptr in
  if Int.equal 0 ptr_as_int then
    None
  else if Int.equal 1 ptr_as_int then
    failwith
      (Printf.sprintf "Decl_service_client.%s: error retrieving decl" caller)
  else
    Some ptr

let rpc_get_fun (client : Decl_ipc_ffi_externs.decl_client) (name : string) :
    Typing_defs.fun_elt option =
  let ty = Decl_ipc_ffi_externs.get_decl client FileInfo.Fun name in
  pointer_to_option ty "rpc_get_fun"

let rpc_get_typedef (client : Decl_ipc_ffi_externs.decl_client) (name : string)
    : Typing_defs.typedef_type option =
  let ty = Decl_ipc_ffi_externs.get_decl client FileInfo.Typedef name in
  pointer_to_option ty "rpc_get_typedef"

let rpc_get_record_def
    (client : Decl_ipc_ffi_externs.decl_client) (name : string) :
    Typing_defs.record_def_type option =
  let ty = Decl_ipc_ffi_externs.get_decl client FileInfo.RecordDef name in
  pointer_to_option ty "rpc_get_record_def"

let rpc_get_gconst (client : Decl_ipc_ffi_externs.decl_client) (name : string) :
    Typing_defs.decl_ty option =
  let ty = Decl_ipc_ffi_externs.get_decl client FileInfo.Const name in
  pointer_to_option ty "rpc_get_gconst"
