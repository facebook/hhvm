(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type lifetime

val lifetime : string -> lifetime

type t

val rust_ref : lifetime -> t -> t

val rust_type : string -> lifetime list -> t list -> t

val rust_simple_type : string -> t

val rust_type_var : string -> t

val is_ref : t -> bool

val is_var : t -> bool

val deref : t -> t

val contains_ref : t -> bool

val rust_type_to_string : t -> string

val type_params_to_string : ?bound:string -> lifetime list -> t list -> string

val type_name_and_params : t -> string * t list
