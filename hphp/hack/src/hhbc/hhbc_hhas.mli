(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

val to_string : ?path:Relative_path.t -> Hhas_program.t -> string
val string_of_instruction : Hhbc_ast.instruct -> string
val string_of_local_id : Local.t -> string
val string_of_label : Label.t -> string
