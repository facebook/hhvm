(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_class_requirements : Decl_env.env -> Nast.class_ ->
  Typing_defs.requirement list * SSet.t
