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

val from_raw_client :
  Decl_ipc_ffi_externs.decl_client -> DeclParserOptions.t -> t

val rpc_get_fun : t -> string -> Typing_defs.fun_elt option

val rpc_get_class : t -> string -> Shallow_decl_defs.shallow_class option

val rpc_get_typedef : t -> string -> Typing_defs.typedef_type option

val rpc_get_gconst : t -> string -> Typing_defs.const_decl option

val rpc_get_module : t -> string -> Typing_defs.module_def_type option

val rpc_get_gconst_path :
  t -> string -> (FileInfo.pos * FileInfo.name_type) option

val rpc_get_fun_path : t -> string -> (FileInfo.pos * FileInfo.name_type) option

val rpc_get_type_path :
  t -> string -> (FileInfo.pos * FileInfo.name_type) option

val rpc_get_module_path :
  t -> string -> (FileInfo.pos * FileInfo.name_type) option

val rpc_get_fun_canon_name : t -> string -> string option

val rpc_get_type_canon_name : t -> string -> string option

val parse_and_cache_decls_in : t -> Relative_path.t -> string -> unit
