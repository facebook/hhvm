(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val should_print : user_mode:Ifc_types.mode -> phase:Ifc_types.mode -> bool

val check :
  Ifc_types.options ->
  ( Aast.pos * Typing_defs.locl_ty,
    unit,
    Tast.saved_env,
    Typing_defs.locl_ty )
  Aast.def
  list ->
  Provider_context.t ->
  unit

val get_solver_result :
  Ifc_types.callable_result list -> Ifc_types.callable_result SMap.t

val simplify :
  Ifc_types.callable_result ->
  Ifc_types.callable_result * Ifc_types.pos_flow list * Ifc_types.prop

val check_valid_flow :
  Ifc_types.options ->
  'a ->
  Ifc_types.callable_result
  * (Ifc_types.PosSet.t * Ifc_types.policy * Ifc_types.policy) list
  * Ifc_types.prop ->
  unit

val analyse_callable :
  ?class_name:Ifc_types.purpose ->
  pos:Pos.t ->
  opts:Ifc_types.options ->
  decl_env:Ifc_types.decl_env ->
  is_static:bool ->
  saved_env:Tast.saved_env ->
  ctx:Provider_context.t ->
  Ifc_types.purpose ->
  ( Aast.pos * Typing_defs.locl_ty,
    unit,
    Tast.saved_env,
    Typing_defs.locl_ty )
  Aast.fun_param
  list ->
  ( Aast.pos * Typing_defs.locl_ty,
    unit,
    Tast.saved_env,
    Typing_defs.locl_ty )
  Aast.func_body ->
  Typing_defs.locl_ty ->
  Ifc_types.callable_result option
