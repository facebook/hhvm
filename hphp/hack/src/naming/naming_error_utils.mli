(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val unbound_name_kind : Name_context.t -> string

val to_user_diagnostic :
  Naming_error.t ->
  Custom_error_config.t ->
  (Pos.t, Pos_or_decl.t) User_diagnostic.t
