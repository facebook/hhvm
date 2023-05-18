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
    (on_error : Typing_error.Reasons_callback.t) : env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  let ety_env = empty_expand_env in
  (* We check constraint entailment and contravariant parameter/covariant result
   * subtyping in the context of the ft_super constraints. But we'd better
   * restore tpenv afterwards *)
  let old_tpenv = Env.get_tpenv env in
  let old_global_tpenv = Env.get_global_tpenv env in
  let env = Env.open_tyvars env Pos.none in

  (* First extend the environment with the type parameters from the supertype, along
   * with their bounds and the function's where constraints
   *)
  let (env, ty_err1) =
    Phase.localize_and_add_generic_parameters_and_where_constraints
      ~ety_env
      env
      ft_super.ft_tparams
      ft_super.ft_where_constraints
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err1;
  (* Reified type parameters should match those in the overridden method (TODO).
   * For non-reified parameters, we allow a generic method to override
   * a method whose signature is an instance of the generic type. For example,
   *   foo(int):int
   * can be overridden by
   *   foo<T>(T):T
   * To implement this, we use localize_targs to generate fresh type variables for the
   * generic parameters, and add bounds and where constraints.
   * A more complex example is the following:
   *   interface I { abstract const type TC as num; }
   * Now consider
   * class B {
   *   function foo(num):void;
   * }
   * overridden by
   * class C<T as I> extends B {
   *   function foo<TF>(TF):void where TF super T::TC;
   * }
   * We need to find type variable #1 with T::TC <: #1 such that
   * (by contravariance of parameter types) num <: #1.
   * Clearly #1:=num is a solution, as T::TC <: num from the type constant bound.
   * Dually, suppose we have
   * class C<T as I> extends B {
   *   function foo<TF>(TF):void where TF as T::TC;
   * }
   * This time, we need to find type variable #1 with #1 <: T::TC
   * such that num <: #1.
   * There is no solution, as by transitivity we need num <: T::TC which does not hold.
   *)
  let non_reified_tparams =
    List.filter ft_sub.ft_tparams ~f:(fun tp ->
        match tp.tp_reified with
        | Aast.Erased -> true
        | _ -> false)
  in
  let ((env, ty_err2), explicit_targs) =
    Phase.localize_targs
      ~check_well_kinded:true
      ~is_method:true
      ~def_pos:p_sub
      ~use_pos:Pos.none
      ~use_name:"overriding"
      ~check_explicit_targs:false
      env
      non_reified_tparams
      []
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err2;
  let tvarl = List.map ~f:fst explicit_targs in
  let substs = Decl_subst.make_locl non_reified_tparams tvarl in
  let (env, ty_err3) =
    Typing_phase.check_tparams_constraints
      ~use_pos:Pos.none
      ~ety_env:{ ety_env with substs }
      env
      ft_sub.ft_tparams
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err3;

  (* Localize the function type for the method in the subtype.
   * Non-reified generic method parameters will be replaced by the type
   * variables generated by localize_targs *)
  let ((env, ty_err4), locl_ft_sub) =
    Phase.localize_ft ~ety_env:{ ety_env with substs } ~def_pos:p_sub env ft_sub
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err4;
  (* Localize the function type for the method in the supertype.
   * Generic method parameters will be left alone.
   *)
  let ((env, ty_err5), locl_ft_super) =
    Phase.localize_ft ~ety_env ~def_pos:p_super env ft_super
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err5;
  let add_where_constraints env (cstrl : locl_where_constraint list) =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
        Typing_subtype.add_constraint env ck ty1 ty2 (Some on_error))
  in
  (* Now extend the environment with `where` constraints from the supertype *)
  let env = add_where_constraints env locl_ft_super.ft_where_constraints in
  (* Check the where constraints from the subtype. *)
  (* Localize the where constraints here rather than using locl_ft_sub.ft_where_constraints,
   * as localize_ft loses changes to tpenv (see comments in code for localize_ft) *)
  let check_where_constraints ~ety_env env cstrl =
    let (env, ty_errs) =
      List.fold_left
        cstrl
        ~init:(env, [])
        ~f:(fun (env, ty_errs) (ty, ck, cstr_ty) ->
          let ((env, e1), ty) = Phase.localize ~ety_env env ty in
          let ((env, e2), cstr_ty) = Phase.localize ~ety_env env cstr_ty in
          let (env, e3) =
            Typing_generic_constraint.check_constraint env ck ~cstr_ty ty
            @@ Some on_error
          in
          let ty_err_opt =
            Typing_error.multiple_opt @@ List.filter_map ~f:Fn.id [e1; e2; e3]
          in
          let ty_errs =
            Option.value_map
              ~default:ty_errs
              ~f:(fun e -> e :: ty_errs)
              ty_err_opt
          in
          (env, ty_errs))
    in
    (env, Typing_error.multiple_opt ty_errs)
  in
  let (env, ty_err6) =
    check_where_constraints
      ~ety_env:{ ety_env with substs }
      env
      ft_sub.ft_where_constraints
  in

  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err6;
  (* Finally apply ordinary contra/co subtyping on the function types *)
  let (env, ty_err7) =
    subtype_funs
      ~on_error:(Some on_error)
      ~for_override:true
      ~check_return
      r_sub
      locl_ft_sub
      r_super
      locl_ft_super
      env
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err7;
  let (env, ty_err8) = Typing_solver.close_tyvars_and_solve env in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err8;
  (* Restore the type parameter environment *)
  Env.env_with_tpenv (Env.env_with_global_tpenv env old_global_tpenv) old_tpenv
