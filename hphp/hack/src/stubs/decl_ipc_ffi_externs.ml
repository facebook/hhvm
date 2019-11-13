(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type sharedmem_base_address

let get_gconst_ffi _ _ _ = failwith "decl_ipc not implemented"

type inproc_state

let inproc_init_ffi _ _ _ = failwith "inproc_init not implemented"

let inproc_request_ffi _ _ _ = failwith "inproc_request not implemented"
