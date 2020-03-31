(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* `t` represents a decl service, i.e. something where you can make requests
for decls and get back answers. Often these requests will block upon IO. *)
type t

val from_raw_client : Decl_ipc_ffi_externs.decl_client -> t

val rpc_get_fun : t -> string -> Typing_defs.fun_elt option

val rpc_get_class : t -> string -> Shallow_decl_defs.shallow_class option

val rpc_get_typedef : t -> string -> Typing_defs.typedef_type option

val rpc_get_record_def : t -> string -> Typing_defs.record_def_type option

val rpc_get_gconst : t -> string -> Typing_defs.decl_ty option
