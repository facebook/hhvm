(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
include Typing_env_types_sig.S

open Typing_defs
open Typing_heap

val get_tcopt : env -> TypecheckerOptions.t
val fresh : unit -> int
val fresh_type : unit -> locl ty
val fresh_unresolved_type : env -> env * locl ty
val add_subst : env -> int -> int -> env
val get_var : env -> int -> env * int
val rename : env -> int -> int -> env
val add : env -> int -> locl ty -> env
val get_type : env -> Reason.t -> int -> env * locl ty
val get_type_unsafe : env -> int -> env * locl ty
val expand_type : env -> locl ty -> env * locl ty
val make_ft : Pos.t -> decl fun_params -> decl ty -> decl fun_type
val get_shape_field_name : Nast.shape_field_name -> string
val empty_fake_members : fake_members
val empty_local : tpenv -> local_env
val empty : TypecheckerOptions.t -> Relative_path.t ->
  droot: Typing_deps.Dep.variant option -> env
val is_typedef : Typedefs.key -> bool
val get_enum : env -> Classes.key -> Classes.t option
val is_enum : env -> Classes.key -> bool
val get_enum_constraint : env -> Classes.key -> decl ty option
val add_wclass : env -> string -> unit
val fresh_tenv : env -> (env -> 'a) -> 'a
val get_class : env -> Classes.key -> Classes.t option
val get_typedef : env -> Typedefs.key -> Typedefs.t option
val get_const : env -> class_type -> string -> class_const option
val get_typeconst : env -> class_type -> string -> typeconst_type option
val get_gconst : env -> GConsts.key -> GConsts.t option
val get_static_member : bool -> env -> class_type -> string -> class_elt option
val suggest_static_member :
  bool -> class_type -> string -> (Pos.t * string) option
val get_member : bool -> env -> class_type -> string -> class_elt option
val suggest_member : bool -> class_type -> string -> (Pos.t * string) option
val get_construct : env -> class_type -> class_elt option * bool
val check_todo : env -> env
val get_return : env -> locl ty
val set_return : env -> locl ty -> env
val with_return : env -> (env -> env * 'a) -> env * 'a
val is_static : env -> bool
val get_self : env -> locl ty
val get_self_id : env -> string
val is_outside_class : env -> bool
val get_parent_id : env -> string
val get_parent : env -> decl ty
val get_fn_kind : env -> Ast.fun_kind
val get_file : env -> Relative_path.t
val get_fun : env -> Funs.key -> Funs.t option
val set_fn_kind : env -> Ast.fun_kind -> env
val add_todo : env -> tfun -> env
val add_anonymous : env -> anon -> env * int
val set_anonymous : env -> int -> anon -> env
val get_anonymous : env -> int -> anon option
val set_self_id : env -> string -> env
val set_self : env -> locl ty -> env
val set_parent_id : env -> string -> env
val set_parent : env -> decl ty -> env
val set_static : env -> env
val set_mode : env -> FileInfo.mode -> env
val get_mode : env -> FileInfo.mode
val is_strict : env -> bool
val is_decl : env -> bool
val get_options: env -> TypecheckerOptions.t
val get_last_call : env -> Pos.t
val lost_info : string -> env -> locl ty -> env * locl ty
val forget_members : env -> Pos.t -> env
module FakeMembers :
  sig
    val make_id : Nast.expr -> string -> string
    val make_static_id : Nast.class_id -> string -> string
    val get : env -> Nast.expr -> string -> int option
    val is_invalid : env -> Nast.expr -> string -> bool
    val get_static : env -> Nast.class_id -> string -> int option
    val is_static_invalid : env -> Nast.class_id -> string -> bool
    val make : Pos.t -> env -> Nast.expr -> string -> env * Local_id.t
    val make_static : Pos.t -> env -> Nast.class_id -> string ->
      env * Local_id.t
  end
val unbind : env -> locl ty -> env * locl ty
val set_local : env -> Local_id.t -> locl ty -> env
val get_local : env -> Local_id.t -> env * locl ty
val set_local_expr_id : env -> Local_id.t -> expression_id -> env
val get_local_expr_id : env -> Local_id.t -> expression_id option
val get_lower_bounds : env -> string -> tparam_bounds
val get_upper_bounds : env -> string -> tparam_bounds
val add_upper_bound : env -> string -> locl ty -> env
val add_lower_bound : env -> string -> locl ty -> env
val get_equal_bounds : env -> string -> tparam_bounds
val union_global_tpenv : tpenv -> tpenv -> tpenv
val add_upper_bound_global : env -> string -> locl ty -> env
val add_lower_bound_global : env -> string -> locl ty -> env
val env_with_tpenv : env -> tpenv -> env
val add_generic_parameters : env -> Nast.tparam list -> env
val is_generic_parameter : env -> string -> bool
val get_generic_parameters : env -> string list
val add_fresh_generic_parameter : env -> string -> env * string
val get_tpenv_size : env -> int
val freeze_local_env : env -> env
val env_with_locals :
  env -> local_types -> local_history Local_id.Map.t -> env
val anon : local_env -> env -> (env -> env * Tast.expr * locl ty) -> env * Tast.expr * locl ty
val in_loop : env -> (env -> env * 'a) -> env * 'a
val merge_locals_and_history : local_env -> old_local Local_id.Map.t
val separate_locals_and_history :
  old_local Local_id.Map.t ->
  (local_types * local_history Local_id.Map.t)
