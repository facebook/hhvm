(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type t = Decl_ipc_ffi_externs.decl_client

let rpc_get_gconst (client : Decl_ipc_ffi_externs.decl_client) (name : string) :
    Typing_defs.decl_ty option =
  (* TODO: this is just a placeholder for now *)
  Printf.printf "GET GCONST... %s\n%!" name;
  let ty = Decl_ipc_ffi_externs.get_gconst_ffi client name in
  (* HACK: The decl service just stores a decl_ty, not a decl_ty option, so it
     either responds with a pointer to the decl_ty (when present) or the integer
     0 (otherwise). Turn that into None/Some here. *)
  if Int.equal 0 (Obj.magic ty) then (
    Printf.printf "NO GCONST %s\n%!" name;
    None
  ) else (
    Format.printf "GOT GCONST... %s = %a\n%!" name Pp_type.pp_decl_ty ty;
    Some ty
  )
