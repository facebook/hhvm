(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type t = {
  hhi_root: Path.t;
  rpc_get_gconst: string -> Typing_defs.decl_ty option;
}

let rpc_get_gconst
    (fd : Unix.file_descr)
    (cache : Decl_ipc_ffi_externs.readonly_cache_view)
    (name : string) : Typing_defs.decl_ty option =
  (* TODO: this is just a placeholder for now *)
  Printf.printf "GET GCONST... %s\n%!" name;
  let ty = Decl_ipc_ffi_externs.get_gconst_ffi fd cache name in
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

let init
    ~(decl_sock : Path.t)
    ~(cache_view : Decl_ipc_ffi_externs.readonly_cache_view)
    ~(hhi_root : Path.t) : (t, Marshal_tools.error) result =
  let sockaddr = Unix.ADDR_UNIX (Path.to_string decl_sock) in
  let connect_res =
    try Ok (Timeout.open_connection sockaddr) with
    | Unix.Unix_error (Unix.ECONNREFUSED, _, _) as e ->
      Error (Marshal_tools.Rpc_absent (Exception.wrap e))
    | e -> Error (Marshal_tools.Rpc_disconnected (Exception.wrap e))
  in
  match connect_res with
  | Error error -> Error error
  | Ok (ic, _oc) ->
    (* ic and oc have the same underlying Unix.file_descr *)
    let fd = Timeout.descr_of_in_channel ic in
    Ok { hhi_root; rpc_get_gconst = rpc_get_gconst fd cache_view }

let inproc_get_gconst
    (state : Decl_ipc_ffi_externs.inproc_state) (name : string) :
    Typing_defs.decl_ty option =
  Printf.printf "INPROC GET GCONST... %s\n%!" name;
  let ty = Decl_ipc_ffi_externs.inproc_request_ffi state 1 name in
  (* HACK: The decl service just stores a decl_ty, not a decl_ty option, so it
     either responds with a pointer to the decl_ty (when present) or the integer
     0 (otherwise). Turn that into None/Some here. *)
  if Int.equal 0 (Obj.magic ty) then (
    Printf.printf "INPROC NO GCONST %s\n%!" name;
    None
  ) else (
    Format.printf "INPROC GOT GCONST... %s = %a\n%!" name Pp_type.pp_decl_ty ty;
    Some ty
  )

let init_inproc ~(naming_table : Path.t) ~(root : Path.t) ~(hhi_root : Path.t) :
    t =
  let state =
    Decl_ipc_ffi_externs.inproc_init_ffi
      (Path.to_string naming_table)
      (Path.to_string root)
      (Path.to_string hhi_root)
  in
  { hhi_root; rpc_get_gconst = inproc_get_gconst state }
