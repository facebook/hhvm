(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

val to_string : ?path:Relative_path.t -> ?dump_symbol_refs:bool -> Hhas_program.t -> string
val string_of_instruction : Hhbc_ast.instruct -> string
val string_of_local_id : Local.t -> string
