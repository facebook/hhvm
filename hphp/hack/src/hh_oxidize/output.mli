(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val with_output_context :
  module_name:string -> (unit -> unit) -> Oxidized_module.t

val add_extern_use : string -> unit

val add_glob_use : string -> unit

val add_alias : string -> string -> unit

val add_include : string -> unit

val add_ty_reexport : string -> unit

val add_decl : string -> string -> unit
