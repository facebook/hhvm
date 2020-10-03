(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type class_env = {
  ctx: Provider_context.t;
  stack: SSet.t;
}

val class_decl_if_missing :
  sh:SharedMem.uses ->
  class_env ->
  Nast.class_ ->
  (string * Decl_defs.decl_class_type) option
