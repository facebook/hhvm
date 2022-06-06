(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
module LSTable = Lazy_string_table

type inherited_members = Typing_class_types.inherited_members

val make :
  Provider_context.t ->
  string ->
  Decl_defs.linearization ->
  (string -> decl_ty option) ->
  inherited_members
