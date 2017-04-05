(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

val elaborate_id : Namespace_env.env ->
                   Ast.ns_kind ->
                   Ast.id ->
                   Ast.id
val elaborate_defs : ParserOptions.t -> Ast.program -> Ast.program

val renamespace_if_aliased : ?reverse:bool ->
                             (string * string) list ->
                             string ->
                             string
