(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get :
  Provider_context.t ->
  string * Decl_defs.linearization_kind ->
  Decl_defs.linearization
