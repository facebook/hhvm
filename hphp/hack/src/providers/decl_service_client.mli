(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  hhi_root: Path.t;  (** where does the decl-service keep its hhi files *)
  rpc_get_gconst: string -> (string, Marshal_tools.error) result;
      (** fetches a global const *)
}

val init :
  decl_sock:Path.t ->
  base_addr:Decl_ipc_ffi_externs.sharedmem_base_address ->
  hhi_root:Path.t ->
  (t, Marshal_tools.error) result
