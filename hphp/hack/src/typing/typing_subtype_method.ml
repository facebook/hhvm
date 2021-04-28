(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Typing_defs
open Typing_env_types
open Typing_subtype
module Reason = Typing_reason
module Env = Typing_env
module Phase = Typing_phase
module MakeType = Typing_make_type

(* Helper method for subtype_method_decl. See below *)
let subtype_method
    ~(check_return : bool)
    (env : env)
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    (on_error : Errors.error_from_reasons_callback) : env =
  (* This is (1) and (2) below *)
  let env =
    subtype_funs ~on_error ~check_return r_sub ft_sub r_super ft_super env
  in
  (* This is (3) below *)
  let check_tparams_constraints env tparams =
    let check_tparam_constraints
        env { tp_name = (p, name); tp_constraints = cstrl; _ } =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, cstr_ty) ->
          (* TODO(T70068435) Revisit this when implementing bounds on HK generic vars.
             For now it's safe to produce a Tgeneric with empty args here, because
             if [name] were higher-kinded, then the constraints must be empty. *)
          let tgeneric = MakeType.generic (Reason.Rwitness_from_decl p) name in
          Typing_generic_constraint.check_constraint
            env
            ck
            tgeneric
            ~cstr_ty
            on_error)
    in
    List.fold_left tparams ~init:env ~f:check_tparam_constraints
  in
  let check_where_constraints env cstrl =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
        Typing_generic_constraint.check_constraint
          env
          ck
          ty1
          ~cstr_ty:ty2
          on_error)
  in
  (* We only do this if the ft_tparam lengths match. Currently we don't even
   * report this as an error, indeed different names for type parameters.
   * TODO: make it an error to override with wrong number of type parameters
   *)
  let env =
    if
      Int.( <> )
        (List.length ft_sub.ft_tparams)
        (List.length ft_super.ft_tparams)
    then
      env
    else
      check_tparams_constraints env ft_sub.ft_tparams
  in
  let env = check_where_constraints env ft_sub.ft_where_constraints in
  env

(** Check that the method decl with signature ft_sub can be used to override
 * (is a subtype of) method decl with signature ft_super.
 *
 * This goes beyond subtyping on function types because methods can have
 * generic parameters with bounds, and `where` constraints.
 *
 * Suppose ft_super is of the form
 *    <T1 csuper1, ..., Tn csupern>(tsuper1, ..., tsuperm) : tsuper where wsuper
 * and ft_sub is of the form
 *    <T1 csub1, ..., Tn csubn>(tsub1, ..., tsubm) : tsub where wsub
 * where csuperX and csubX are constraints on type parameters and wsuper and
 * wsub are 'where' constraints. Note that all types in the superclass,
 * including constraints (so csuperX, tsuperX, tsuper and wsuper) have had
 * any class type parameters instantiated appropriately according to
 * the actual arguments of the superclass. For example, suppose we have
 *
 *   class Super<T> {
 *     function foo<Tu as A<T>>(T $x) : B<T> where T super C<T>
 *   }
 *   class Sub extends Super<D> {
 *     ...override of foo...
 *   }
 * then the actual signature in the superclass that we need to check is
 *     function foo<Tu as A<D>>(D $x) : B<D> where D super C<D>
 * Note in particular the general form of the 'where' constraint.
 *
 * (Currently, this instantiation happens in
 *   Typing_extends.check_class_implements which in turn calls
 *   Decl_instantiate.instantiate_ce)
 *
 * Then for ft_sub to be a subtype of ft_super it must be the case that
 * (1) tsuper1 <: tsub1, ..., tsupern <: tsubn (under constraints
 *     T1 csuper1, ..., Tn csupern and wsuper).
 *
 *     This is contravariant subtyping on parameter types.
 *
 * (2) tsub <: tsuper (under constraints T1 csuper1, ..., Tn csupern and wsuper)
 *     This is covariant subtyping on result type. For constraints consider
 *       e.g. consider ft_super = <T super I>(): T
 *                 and ft_sub = <T>(): I
 *
 * (3) The constraints for ft_super entail the constraints for ft_sub, because
 *     we might be calling the function having checked that csuperX are
 *     satisfied but the definition of the function (e.g. consider an override)
 *     has been checked under csubX.
 *     More precisely, we must assume constraints T1 csuper1, ..., Tn csupern
 *     and wsuper, and check that T1 satisfies csub1, ..., Tn satisfies csubn
 *     and that wsub holds under those assumptions.
 *)
let subtype_method_decl
    ~(check_return : bool)
    (env : env)
    (r_sub : Reason.t)
    (ft_sub : decl_fun_type)
    (r_super : Reason.t)
    (ft_super : decl_fun_type)
    (on_error : Errors.error_from_reasons_callback) : env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  let ety_env = empty_expand_env in
  (* We check constraint entailment and contravariant parameter/covariant result
   * subtyping in the context of the ft_super constraints. But we'd better
   * restore tpenv afterwards *)
  let old_tpenv = Env.get_tpenv env in
  let old_global_tpenv = Env.get_global_tpenv env in
  (* First extend the environment with the type parameters from the supertype, along
   * with their bounds
   *)
  let env =
    Phase.localize_and_add_generic_parameters ~ety_env env ft_super.ft_tparams
  in
  (* Localize the function type itself *)
  let (env, locl_ft_sub) =
    Phase.localize_ft ~ety_env ~def_pos:p_sub env ft_sub
  in
  let (env, locl_ft_super) =
    Phase.localize_ft ~ety_env ~def_pos:p_super env ft_super
  in
  let add_where_constraints env (cstrl : locl_where_constraint list) =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
        Typing_subtype.add_constraint env ck ty1 ty2 on_error)
  in
  (* Now extend the environment with `where` constraints from the supertype *)
  let env = add_where_constraints env locl_ft_super.ft_where_constraints in
  (* Finally apply contra/co subtyping on the function type, and check that
   * the constraints on the supertype entail the constraints on the subtype *)
  let env =
    subtype_method
      ~check_return
      env
      r_sub
      locl_ft_sub
      r_super
      locl_ft_super
      on_error
  in
  (* Restore the type parameter environment *)
  Env.env_with_tpenv (Env.env_with_global_tpenv env old_global_tpenv) old_tpenv
