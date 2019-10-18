(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** an opaque type, the pointer to the base of shared memory *)
type sharedmem_base_address

type t = {
  rpc_get_gconst: string -> (string, Marshal_tools.error) result;
      (** fetches a global const *)
}

val init : string -> sharedmem_base_address -> (t, Marshal_tools.error) result
