(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type decl_client

let get_decl _ _ _ = failwith "decl_ipc not implemented"

let get_const_path _ _ = failwith "decl_ipc not implemented"

let get_fun_path _ _ = failwith "decl_ipc not implemented"

let get_type_path_and_kind _ _ = failwith "decl_ipc not implemented"

let get_fun_canon_name _ _ = failwith "decl_ipc not implemented"

let get_type_canon_name _ _ = failwith "decl_ipc not implemented"
