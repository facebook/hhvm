(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

 open Typing_defs
 open Type_parameter_env
 module TySet = Typing_set
 module type S = sig

 type fake_members = {
   last_call : Pos.t option;
   invalid   : SSet.t;
   valid     : SSet.t;
 }
 (* Along with a type, each local variable has a expression id associated with
  * it. This is used when generating expression dependent types for the 'this'
  * type. The idea is that if two local variables have the same expression_id
  * then they refer to the same late bound type, and thus have compatible
  * 'this' types.
  *)
 type expression_id = Ident.t
 type local = locl ty * expression_id
 type old_local = locl ty list * locl ty * expression_id
 type local_id_map = local Local_id.Map.t
 type local_types = local_id_map Typing_continuations.Map.t

 (* Local environment includes types of locals and bounds on type parameters. *)
 type local_env = {
   fake_members       : fake_members;
   local_types        : local_types;
   local_mutability   : Typing_mutability_env.mutability_env;
   local_reactive : reactivity;
   (* Local variables that were assigned in a `using` clause *)
   local_using_vars   : Local_id.Set.t;
   (* Type parameter environment
    * Lower and upper bounds on generic type parameters and abstract types
    * For constraints of the form Tu <: Tv where both Tu and Tv are type
    * parameters, we store an upper bound for Tu and a lower bound for Tv.
    * Contrasting with tenv and subst, bounds are *assumptions* for type
    * inference, not conclusions.
    *)
   tpenv              : tpenv;
 }

 type env = {
   (* position of the function/method being checked *)
   function_pos: Pos.t  ;
   pos     : Pos.t      ;
   (* Position and reason information on entry to a subtype or unification check *)
   outer_pos : Pos.t;
   outer_reason : Typing_reason.ureason;
   tenv    : locl ty IMap.t ;
   subst   : int IMap.t ;
   lenv    : local_env  ;
   genv    : genv       ;
   decl_env: Decl_env.env;
   todo    : tfun list  ;
   checking_todos : bool;
   in_loop : bool       ;
   in_try  : bool       ;
   in_case : bool       ;
   inside_constructor: bool;
   inside_ppl_class: bool;
   (* A set of constraints that are global to a given method *)
   global_tpenv : tpenv ;
 }
and genv = {
  tcopt   : TypecheckerOptions.t;
  return  : Typing_env_return_info.t;
  (* For each function parameter, its type and calling convention. *)
  params  : (locl ty * param_mode) Local_id.Map.t;
  (* condition types associated with parameters.
     For every mayberx parameter that has condition type we create
     fresh type parameter (see: make_local_param_ty) and store mapping
     fresh type name -> condition type in env so it can be retrieved later *)
  condition_types: decl ty SMap.t;
  parent_id : string;
  parent  : decl ty;
  (* Identifier of the enclosing class *)
  self_id : string;
  (* Type of the enclosing class, instantiated at its generic parameters *)
  self    : locl ty;
  static  : bool;
  fun_kind : Ast.fun_kind;
  fun_mutable : bool;
  anons   : anon IMap.t;
  file    : Relative_path.t;
}

(* A type-checker for an anonymous function
 * Parameters are
 * - the environment
 * - types of the parameters under which the body should be checked
 * - the arity of the function
 * - the expected return type of the body (optional)
 *)
and anon_log = locl ty list * locl ty list
and anon =
  reactivity *
  Nast.is_coroutine *
  anon_log ref *
  Pos.t *
  (?el:Nast.expr list ->
  ?ret_ty: locl ty ->
  env ->
  locl fun_params ->
  locl fun_arity ->
  env * Tast.expr * locl ty)

(* A deferred check; return true if the check should now be removed from the list *)
 and tfun = env -> env * bool
end
