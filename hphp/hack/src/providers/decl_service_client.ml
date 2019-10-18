(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type sharedmem_base_address

type t = { rpc_get_gconst: string -> (string, Marshal_tools.error) result }

external get_gconst_ffi :
  Unix.file_descr -> sharedmem_base_address -> string -> int = "get_gconst"

let rpc_get_gconst
    (fd : Unix.file_descr) (base : sharedmem_base_address) (name : string) :
    (string, Marshal_tools.error) result =
  (* TODO: this is just a placeholder for now *)
  Printf.printf "GET GCONST... %s\n%!" name;
  let i = get_gconst_ffi fd base name in
  Printf.printf "GOT GCONST... %s = %d\n%!" name i;
  Ok (Printf.sprintf "%d" i)

let init (decl_sock_file : string) (base : sharedmem_base_address) :
    (t, Marshal_tools.error) result =
  let sockaddr = Unix.ADDR_UNIX decl_sock_file in
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
    Ok { rpc_get_gconst = rpc_get_gconst fd base }
