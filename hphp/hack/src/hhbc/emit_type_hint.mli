(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

val fmt_hint :
  tparams: string list ->
  namespace: Namespace_env.env ->
  ?strip_tparams: bool ->
  Ast.hint ->
  string

type type_hint_kind =
| Property
| Return
| Param
| TypeDef

val hint_to_type_info :
  kind:type_hint_kind ->
  skipawaitable: bool ->
  nullable: bool ->
  tparams: string list ->
  namespace: Namespace_env.env ->
  Ast.hint ->
  Hhas_type_info.t

val hint_to_class :
  namespace: Namespace_env.env ->
  Ast.hint ->
  Hhbc_id.Class.t

val emit_type_constraint_for_native_function :
  string list ->
  Ast.hint option ->
  Hhas_type_info.t ->
  Hhas_type_info.t
