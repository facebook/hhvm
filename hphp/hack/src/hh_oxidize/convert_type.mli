(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val core_type : Parsetree.core_type -> Rust_type.t

val tuple : ?seen_indirection:bool -> Parsetree.core_type list -> Rust_type.t

val is_copy : Rust_type.t -> bool

val is_primitive : string -> bool
