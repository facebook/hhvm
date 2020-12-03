(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type class_name = string

val get_linearization :
  Provider_context.t ->
  class_name * Decl_defs.linearization_kind ->
  Decl_defs.linearization
