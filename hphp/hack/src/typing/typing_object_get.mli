(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Check member access, both static and instance.
 *   [obj_pos] is position of the object expression i.e. expr in expr->m
 *   [is_method] is true if this is a method invocation rather than property access
 *   [inst_meth] is true if this is an inst_meth expression
 *   [nullsafe] is Some r for null-safe calls such as expr?->m
 *   [parent_ty] is the type of the parent, in the case of a parent call (class_id = CIparent)
 *   [explicit_targs]  is a list of explicit type argument expressions, if present
 *   [member_id] is positioned identifier for the member i.e. m in expr->m
 *   [on_error] is an error callback
 *)
val obj_get :
  obj_pos:Ast_defs.pos ->
  is_method:bool ->
  inst_meth:bool ->
  meth_caller:bool ->
  nullsafe:Typing_reason.t option ->
  coerce_from_ty:
    (Ast_defs.pos * Typing_reason.ureason * Typing_defs.locl_ty) option ->
  explicit_targs:Nast.targ list ->
  class_id:Nast.class_id_ ->
  member_id:Nast.sid ->
  on_error:Errors.typing_error_callback ->
  ?parent_ty:Typing_defs.locl_ty ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * (Typing_defs.locl_ty * Tast.targ list)

(** As above but also return the types at which coercion of `coerce_from_ty`
    to the object type failed, if at all *)
val obj_get_with_err :
  obj_pos:Ast_defs.pos ->
  is_method:bool ->
  inst_meth:bool ->
  meth_caller:bool ->
  nullsafe:Typing_reason.t option ->
  coerce_from_ty:
    (Ast_defs.pos * Typing_reason.ureason * Typing_defs.locl_ty) option ->
  explicit_targs:Nast.targ list ->
  class_id:Nast.class_id_ ->
  member_id:Nast.sid ->
  on_error:Errors.typing_error_callback ->
  ?parent_ty:Typing_defs.locl_ty ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env
  * (Typing_defs.locl_ty * Tast.targ list)
  * (Typing_defs.locl_ty * Typing_defs.locl_ty) option

val smember_not_found :
  Ast_defs.pos ->
  is_const:bool ->
  is_method:bool ->
  is_function_pointer:bool ->
  Decl_provider.Class.t ->
  string ->
  Errors.typing_error_callback ->
  unit
