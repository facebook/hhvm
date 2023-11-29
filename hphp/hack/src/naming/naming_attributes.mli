(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val mem : Hh_prelude.String.t -> ('a, 'b) Aast.user_attribute list -> bool

val mem2 :
  Hh_prelude.String.t ->
  Hh_prelude.String.t ->
  ('a, 'b) Aast.user_attribute list ->
  bool

val find :
  Hh_prelude.String.t ->
  ('a, 'b) Aast.user_attribute list ->
  ('a, 'b) Aast.user_attribute option

val find2 :
  Hh_prelude.String.t ->
  Hh_prelude.String.t ->
  ('a, 'b) Aast.user_attribute list ->
  ('a, 'b) Aast.user_attribute option

val mem_pos :
  Hh_prelude.String.t ->
  ('a, 'b) Aast.user_attribute list ->
  Ast_defs.pos option
