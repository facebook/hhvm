(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val prim_to_string : Aast.tprim -> string

val fmt_hint :
  tparams:string list ->
  ?strip_tparams:bool ->
  (* Tast.hint *) Aast.hint ->
  string

type type_hint_kind =
  | Property
  | Return
  | Param
  | TypeDef
  | UpperBound

val hint_to_type_info :
  kind:type_hint_kind ->
  skipawaitable:bool ->
  nullable:bool ->
  tparams:string list ->
  Aast.hint ->
  Hhas_type_info.t

val hint_to_class : Aast.hint -> Hhbc_id.Class.t

val emit_type_constraint_for_native_function :
  string list -> Aast.hint option -> Hhas_type_info.t -> Hhas_type_info.t
