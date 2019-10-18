(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type sharedmem_base_address

type t = { rpc_get_gconst: string -> (string, Marshal_tools.error) result }

let _rpc_get_gconst
    (_fd : Unix.file_descr) (_base : sharedmem_base_address) (name : string) :
    (string, Marshal_tools.error) result =
  Error (Marshal_tools.Rpc_malformed ("not implemented", Utils.Callstack name))

let init (decl_sock_file : string) (_base : sharedmem_base_address) :
    (t, Marshal_tools.error) result =
  Error
    (Marshal_tools.Rpc_malformed
       ("not implemented", Utils.Callstack decl_sock_file))
