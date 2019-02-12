(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shallow_decl_defs

val get : TypecheckerOptions.t -> string -> shallow_class option

val class_naming_and_decl : TypecheckerOptions.t -> Ast.class_ -> shallow_class
