(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val adata_to_buffer : string Mutable_accumulator.t -> Typed_value.t -> unit

val attributes_to_strings : Hhas_attribute.t list -> string list

val get_array_identifier : Typed_value.t -> Hhbc_ast.adata_id

val get_adata : unit -> Hhas_adata.t list

val reset : unit -> unit

val rewrite_typed_values : Instruction_sequence.t -> Instruction_sequence.t
