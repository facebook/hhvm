(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type old_decl_client

let get_decls _ _ _ = failwith "get_decls not implemented"

let put_decls ~silent:_ _ _ = failwith "put_decls not implemented"

let get_decls_via_file_hashes _ _ _ =
  failwith "get_decls_via_file_hashes not implemented"

let initialize_client _ = failwith "initialize_client not implemented"
