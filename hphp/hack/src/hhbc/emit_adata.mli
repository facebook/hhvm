(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

val adata_to_string_seq : Typed_value.t -> String_sequence.t
val attributes_to_strings : Hhas_attribute.t list -> string list
val get_adata : unit -> Hhas_adata.t list
val reset : unit -> unit
val rewrite_typed_value : Typed_value.t -> Hhbc_ast.instruct
