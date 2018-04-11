(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

val empty_symbol_refs : Hhas_symbol_refs.t
val get_symbol_refs : unit -> Hhas_symbol_refs.t
val add_include : Hhas_symbol_refs.include_path -> unit
val add_constant : string -> unit
val add_function : string -> unit
val add_class : string -> unit
val reset : unit -> unit
