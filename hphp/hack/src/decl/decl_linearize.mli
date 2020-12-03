(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type class_name = string

type linearizations = {
  lin_members: Decl_defs.linearization;
  lin_ancestors: Decl_defs.linearization;
}

val get_linearizations : Provider_context.t -> class_name -> linearizations
