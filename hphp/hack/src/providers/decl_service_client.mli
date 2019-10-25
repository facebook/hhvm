(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  rpc_get_gconst: string -> (string, Marshal_tools.error) result;
      (** fetches a global const *)
}

val init :
  string ->
  Decl_ipc_ffi_externs.sharedmem_base_address ->
  (t, Marshal_tools.error) result
