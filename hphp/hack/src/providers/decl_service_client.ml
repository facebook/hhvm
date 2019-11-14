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
  rpc_get_gconst: string -> (string, Marshal_tools.error) result;
}

let rpc_get_gconst
    (fd : Unix.file_descr)
    (base : Decl_ipc_ffi_externs.sharedmem_base_address)
    (name : string) : (string, Marshal_tools.error) result =
  (* TODO: this is just a placeholder for now *)
  Printf.printf "GET GCONST... %s\n%!" name;
  let s = Decl_ipc_ffi_externs.get_gconst_ffi fd base name in
  Printf.printf "GOT GCONST... %s = %s\n%!" name s;
  Ok s

let init
    ~(decl_sock : Path.t)
    ~(base_addr : Decl_ipc_ffi_externs.sharedmem_base_address)
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
    Ok { hhi_root; rpc_get_gconst = rpc_get_gconst fd base_addr }

let inproc_get_gconst
    (state : Decl_ipc_ffi_externs.inproc_state) (name : string) :
    (string, Marshal_tools.error) result =
  Printf.printf "INPROC GET GCONST... %s\n%!" name;
  let s = Decl_ipc_ffi_externs.inproc_request_ffi state 1 name in
  Printf.printf "INPROC GOT GCONST... %s = %s\n%!" name s;
  Ok s

let init_inproc ~(naming_table : Path.t) ~(root : Path.t) ~(hhi_root : Path.t) :
    t =
  let state =
    Decl_ipc_ffi_externs.inproc_init_ffi
      (Path.to_string naming_table)
      (Path.to_string root)
      (Path.to_string hhi_root)
  in
  { hhi_root; rpc_get_gconst = inproc_get_gconst state }
