(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Utils
open Typing_defs

module Reason = Typing_reason
module Unify = Typing_unify
module Env = Typing_env
module Subst = Decl_subst
module TUtils = Typing_utils
module TUEnv = Typing_unification_env
module SN = Naming_special_names
module Phase = Typing_phase

(* Decomposition and subtyping for arrays *)
let decompose_array
  (env : Env.env)
  (r : Reason.t)
  (ak_sub : array_kind)
  (ak_super : array_kind)
  (sub : Env.env -> locl ty -> locl ty -> Env.env)
  (fail : Env.env -> Env.env) : Env.env =
  match ak_sub, ak_super with
  (* An array of any kind is a subtype of an array of AKany *)
  | _, AKany ->
    env

  (* array is a subtype of varray_or_darray *)
  | AKany, AKvarray_or_darray (_, Tany) ->
    env

  (* varray_or_darray<ty1> <: varray_or_darray<ty2> iff t1 <: ty2
     But, varray_or_darray<ty1> is never a subtype of a vect-like array *)
  | AKvarray_or_darray ty_sub, AKvarray_or_darray ty_super ->
    sub env ty_sub ty_super

  | (AKvarray ty_sub | AKvec ty_sub),
    (AKvarray ty_super | AKvec ty_super | AKvarray_or_darray ty_super) ->
    sub env ty_sub ty_super

  | (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
    (AKdarray (tk_super, tv_super) | AKmap (tk_super, tv_super)) ->
    let env = sub env tk_sub tk_super in
    sub env tv_sub tv_super

  | (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub)),
    (AKvarray_or_darray tv_super) ->
    let tk_super =
      Reason.Rvarray_or_darray_key (Reason.to_pos r),
      Tprim Nast.Tarraykey in
    let env = sub env tk_sub tk_super in
    sub env tv_sub tv_super

  | _ ->
    fail env

(* Given two types that we know are in a subtype relationship
 *   ty_sub <: ty_super
 * add to env.tpenv any bounds on generic type parameters that must
 * hold for ty_sub <: ty_super to be valid.
 *
 * For example, suppose we know Cov<T> <: Cov<D> for a covariant class Cov.
 * Then it must be the case that T <: D so we add an upper bound D to the
 * bounds for T.
 *
 * Although some of this code is similar to that for subtype_with_uenv, its
 * purpose is different. subtype_with_uenv takes two types t and u and makes
 * updates to the substitution of type variables (through unification) to
 * make t <: u true.
 *
 * decompose_subtype takes two types t and u for which t <: u is *assumed* to
 * hold, and makes updates to bounds on generic parameters that *necessarily*
 * hold in order for t <: u.
 *
 * If it turns out that there is no situation in which t <: u (for example, we
 * are given string and int, or Cov<Derived> <: Cov<Base>) then evaluate the
 * failure continuation `fail`
 *)
let rec decompose_subtype
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  (fail : Env.env -> Env.env) : Env.env =
  let env, ty_super = Env.expand_type env ty_super in
  let env, ty_sub = Env.expand_type env ty_sub in
  let again env ty_sub =
    decompose_subtype env ty_sub ty_super fail in
  match ty_sub, ty_super with
  (* name_sub <: ty_super so add an upper bound on name_sub *)
  | (_, Tabstract (AKgeneric name_sub, _)), _ when ty_sub != ty_super ->
    let tys = Env.get_upper_bounds env name_sub in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_super tys then env
    else Env.add_upper_bound env name_sub ty_super

  (* ty_sub <: name_super so add a lower bound on name_super *)
  | _, (_, Tabstract (AKgeneric name_super, _)) when ty_sub != ty_super ->
    let tys = Env.get_lower_bounds env name_super in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_sub tys then env
    else Env.add_lower_bound env name_super ty_sub

  (* If ?ty_sub' <: ?ty_super' then must have ty_sub <: ty_super *)
  | (_, Toption ty_sub'), (_, Toption ty_super') ->
    decompose_subtype env ty_sub' ty_super' fail

  (* Arrays *)
  | (r, Tarraykind ak_sub), (_, Tarraykind ak_super) ->
    decompose_array env r ak_sub ak_super (fun env ty_sub' ty_super' ->
      decompose_subtype env ty_sub' ty_super' fail) fail

  (* Singleton union *)
  | _, (_, Tunresolved [ty_super']) ->
    decompose_subtype env ty_sub ty_super' fail

  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tv_super]))
    when (coll = SN.Collections.cTraversable ||
          coll = SN.Collections.cContainer) ->
      (match akind with
      | AKany -> env
      | AKempty -> env
      (* If vec<tv> <: Traversable<tv_super>
       * then it must be the case that tv <: tv_super
       * Likewise for vec<tv> <: Container<tv_super>
       *          and map<_,tv> <: Traversable<tv_super>
       *          and map<_,tv> <: Container<tv_super>
       *)
      | AKvarray tv
      | AKvec tv
      | AKdarray (_, tv)
      | AKvarray_or_darray tv
      | AKmap (_, tv) -> decompose_subtype env tv tv_super fail
      | AKshape fdm ->
        Typing_arrays.fold_akshape_as_akmap again env r fdm
      | AKtuple fields ->
        Typing_arrays.fold_aktuple_as_akvec again env r fields
    )

  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tk_super; tv_super]))
    when (coll = SN.Collections.cKeyedTraversable
         || coll = SN.Collections.cKeyedContainer
         || coll = SN.Collections.cIndexish) ->
      (match akind with
      | AKany -> env
      | AKempty -> env
      | AKvarray tv
      | AKvec tv ->
        let env' = decompose_subtype env (r, Tprim Nast.Tint) tk_super fail in
        decompose_subtype env' tv tv_super fail
      | AKvarray_or_darray tv ->
        let tk_sub =
          Reason.Rvarray_or_darray_key (Reason.to_pos r),
          Tprim Nast.Tarraykey in
        let env' = decompose_subtype env tk_sub tk_super fail in
        decompose_subtype env' tv tv_super fail
      | AKdarray (tk, tv)
      | AKmap (tk, tv) ->
        let env' = decompose_subtype env tk tk_super fail in
        decompose_subtype env' tv tv_super fail
      | AKshape fdm ->
        Typing_arrays.fold_akshape_as_akmap again env r fdm
      | AKtuple fields ->
        Typing_arrays.fold_aktuple_as_akvec again env r fields
      )

  (* If C<ts> <: ?D<ts'> then must have C<ts> <: D<ts'> *)
  | (_, Tclass _), (_, Toption ((_, Tclass _) as ty_super')) ->
    decompose_subtype env ty_sub ty_super' fail

  | (_, (Tclass (x_sub, tyl_sub))), (_, (Tclass (x_super, tyl_super)))->
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    begin match Env.get_class env cid_sub with
    | None ->
      env

    | Some class_sub ->
      (* If cid_sub = cid_super = C and C<tyl_sub> <: C<tyl_super>
       * then it must be the case that tyl_sub and tyl_super are
       * pointwise related according to the variance annotations on
       * the type parameters of C
       *)
      if cid_super = cid_sub
      then decompose_tparams env class_sub.tc_tparams tyl_sub tyl_super fail
      else

      (* Let's just do a sanity check on arity. We don't expect it to fail *)
      if List.length class_sub.tc_tparams <> List.length tyl_sub
      then env
      else

      (* Otherwise, let's look at the class it extends *)
        begin match SMap.get cid_super class_sub.tc_ancestors with
        | Some open_extends_ty ->
          let ety_env = {
            type_expansions = [];
            substs = Subst.make class_sub.tc_tparams tyl_sub;
            this_ty = ty_sub;
            from_class = None;
          } in
          (* Substitute type arguments for formal generic parameters *)
          let env, extends_ty = Phase.localize ~ety_env env open_extends_ty in

          (* Now recurse with the new assumption extends_ty <: ty_super *)
          decompose_subtype env extends_ty ty_super fail

        | None ->
          begin match Env.get_class env cid_super with
            | None -> env
            | Some class_super ->
              if class_super.tc_kind = Ast.Cnormal
              then fail env
              else env
          end
        end
    end

  | _, _ ->
    env

and decompose_tparams
  (env : Env.env)
  (tparams : decl tparam list)
  (children_tyl : locl ty list)
  (super_tyl : locl ty list)
  (fail : Env.env -> Env.env) : Env.env =
  match tparams, children_tyl, super_tyl with
  | [], _, _
  | _, [], _
  | _, _, [] -> env
  | (variance,_,_) :: tparams, child :: childrenl, super :: superl ->
    let ck =
      match variance with
      | Ast.Covariant -> Ast.Constraint_as
      | Ast.Contravariant -> Ast.Constraint_super
      | Ast.Invariant -> Ast.Constraint_eq in
    let env' = decompose_constraint env ck child super fail in
    decompose_tparams env' tparams childrenl superl fail

(* Decompose a general constraint *)
and decompose_constraint
  (env : Env.env)
  (ck : Ast.constraint_kind)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  (fail : Env.env -> Env.env) : Env.env =
  match ck with
  | Ast.Constraint_as ->
    decompose_subtype env ty_sub ty_super fail
  | Ast.Constraint_super ->
    decompose_subtype env ty_super ty_sub fail
  | Ast.Constraint_eq ->
    let env' = decompose_subtype env ty_sub ty_super fail in
    decompose_subtype env' ty_super ty_sub fail

(* Given a constraint ty1 ck ty2 where ck is AS, SUPER or =,
 * add bounds to type parameters in the environment that necessarily
 * must hold in order for ty1 ck ty2.
 *
 * First, we invoke decompose_constraint to add initial bounds to
 * the environment. Then we iterate, decomposing constraints that
 * arise through transitivity across bounds.
 *
 * For example, suppose that env already contains
 *   C<T1> <: T2
 * for some covariant class C. Now suppose we add the
 * constraint "T2 as C<T3>" i.e. we end up with
 *   C<T1> <: T2 <: C<T3>
 * Then by transitivity we know that T1 <: T3 so we add this to the
 * environment also.
 *
 * We repeat this process until no further bounds are added to the
 * environment, or some limit is reached. (It's possible to construct
 * types that expand forever under inheritance.)
 *
 * If the constraint turns out to be unsatisfiable, invoke
 * the failure continuation fail.
*)
let constraint_iteration_limit = 20
let add_constraint_with_fail
  (env : Env.env)
  (ck : Ast.constraint_kind)
  (ty_sub : locl ty)
  (ty_super : locl ty)
  (fail : Env.env -> Env.env) : Env.env =
  let oldsize = Env.get_tpenv_size env in
  let env' = decompose_constraint env ck ty_sub ty_super fail in
  if Env.get_tpenv_size env' = oldsize
  then env'
  else
  let rec iter n env =
    if n > constraint_iteration_limit then env
    else
      let oldsize = Env.get_tpenv_size env in
      let env' =
        List.fold_left (Env.get_generic_parameters env) ~init:env
          ~f:(fun env x ->
            List.fold_left (Typing_set.elements (Env.get_lower_bounds env x)) ~init:env
              ~f:(fun env ty_sub' ->
                List.fold_left (Typing_set.elements (Env.get_upper_bounds env x)) ~init:env
                  ~f:(fun env ty_super' ->
                    decompose_subtype env ty_sub' ty_super' fail))) in
      if Env.get_tpenv_size env' = oldsize
      then env'
      else iter (n+1) env'
  in
    iter 0 env'

(* Default is to ignore unsatisfiable constraints; in future we might
 * want to produce an error. (For example, if after instantiation a
 * constraint becomes C<string> as C<int>)
 *)
let add_constraint
  (p : Pos.Map.key)
  (env : Env.env)
  (ck : Ast.constraint_kind)
  (ty_sub : locl ty)
  (ty_super : locl ty) : Env.env =
  Typing_log.log_types 2 p env
    [Typing_log.Log_sub ("Typing_subtype.add_constraint",
       [Typing_log.Log_type ("ty_sub", ty_sub);
        Typing_log.Log_type ("ty_super", ty_super)])];
  add_constraint_with_fail env ck ty_sub ty_super (fun env -> env)

type reactivity_extra_info = {
  method_info: ((* method_name *) string * (* is_static *) bool) option;
  class_ty: phase_ty option;
  parent_class_ty: phase_ty option
}

let empty_extra_info = {
  method_info = None;
  class_ty = None;
  parent_class_ty = None
}

let rec subtype_params
  ?(is_method : bool = false)
  (env : Env.env)
  (subl : locl fun_param list)
  (superl : locl fun_param list)
  (variadic_sub_ty : locl ty option)
  (variadic_super_ty : locl ty option) : Env.env =

  match subl, superl with
  (* When either list runs out, we still have to typecheck that
  the remaining portion sub/super types with the other's variadic.
  For example, if
  ChildClass {
    public function a(int $x = 0, string ... $args) // superl = [int], super_var = string
  }
  overrides
  ParentClass {
    public function a(string ... $args) // subl = [], sub_var = string
  }
  , there should be an error because the first argument will be checked against
  int, not string that is, ChildClass::a("hello") would crash,
  but ParentClass::a("hello") wouldn't.

  Similarly, if the other list is longer, aka
  ChildClass  extends ParentClass {
    public function a(mixed ... $args) // superl = [], super_var = mixed
  }
  overrides
  ParentClass {
    //subl = [string], sub_var = string
    public function a(string $x = 0, string ... $args)
  }
  It should also check that string is a subtype of mixed.
  *)
  | [], _ -> (match variadic_super_ty with
    | None -> env
    | Some ty -> supertype_params_with_variadic env superl ty)
  | _, [] -> (match variadic_sub_ty with
    | None -> env
    | Some ty -> subtype_params_with_variadic env subl ty)
  | sub :: subl, super :: superl ->
    let { fp_type = ty_sub; _ } = sub in
    let { fp_type = ty_super; _ } = super in
    (* Check that the calling conventions of the params are compatible.
     * We don't currently raise an error for reffiness because function
     * hints don't support '&' annotations (enforce_ctpbr = false). *)
    Unify.unify_param_modes ~enforce_ctpbr:is_method sub super;
    Unify.unify_accept_disposable sub super;
    let env = { env with Env.pos = Reason.to_pos (fst ty_sub) } in
    let env = match sub.fp_kind, super.fp_kind with
    | FPinout, FPinout ->
      (* Inout parameters are invariant wrt subtyping for function types. *)
      let env, _ = Unify.unify env ty_super ty_sub in
      env
    | _ ->
      sub_type env ty_sub ty_super in
    let env = subtype_params ~is_method env subl superl variadic_sub_ty variadic_super_ty in
    env

and subtype_params_with_variadic
  (env : Env.env)
  (subl : locl fun_param list)
  (variadic_ty : locl ty) : Env.env =
  match subl with
  | [] -> env
  | { fp_type = sub; _ } :: subl ->
    let env = { env with Env.pos = Reason.to_pos (fst sub) } in
    let env = sub_type env sub variadic_ty in
    subtype_params_with_variadic env subl variadic_ty

and supertype_params_with_variadic
  (env : Env.env)
  (superl : locl fun_param list)
  (variadic_ty : locl ty) : Env.env =
  match superl with
  | [] -> env
  | { fp_type = super; _ } :: superl ->
    let env = { env with Env.pos = Reason.to_pos (fst super) } in
    let env = sub_type env variadic_ty super in
    supertype_params_with_variadic env superl variadic_ty

and subtype_reactivity
  ?(extra_info: reactivity_extra_info option)
  ?(is_call_site = false)
  (env : Env.env)
  (r_sub : reactivity)
  (r_super : reactivity) : bool =
  let ety_env = Phase.env_with_self env in
  let localize ty =
    let _, t = Phase.localize ~ety_env env ty in
    t in
  let maybe_localize ty =
    match ty with
    | LoclTy t -> t
    | DeclTy t -> localize t in
  (* for methods compute effective reactivity
     with respect to containing classes:
     interface Rx {
       <<__Rx>>
       public function f(): int;
     }
     class A {
       <<__Rx, __OnlyRxIfImpl(Rx::class)>>
       public function f(): int { return 1; }
     }
     class B extends A implements Rx {
     }
     When checking if B properly implements Rx we try to verify
     that A::f that B inherits from A is a subtype of Rx::f.
     A::f is conditionally reactive with condition type Rx and B states
     that it implements Rx so in context of B A::f will be unconditionally reactive
  *)
  let rec effective_reactivity r containing_ty =
    match r, containing_ty with
    | Local (Some t), Some containing_ty
      when is_sub_type env (maybe_localize containing_ty) (localize t) -> Local None
    | Shallow (Some t), Some containing_ty
      when is_sub_type env (maybe_localize containing_ty) (localize t) -> Shallow None
    | Reactive (Some t), Some containing_ty
      when is_sub_type env (maybe_localize containing_ty) (localize t) -> Reactive None
    | MaybeReactive r, _ -> MaybeReactive (effective_reactivity r containing_ty)
    | _ -> r in
  let r_sub =
    Option.bind extra_info (fun { class_ty = c; _ } -> c)
    |> effective_reactivity r_sub in
  let r_super =
    Option.bind extra_info (fun { parent_class_ty = c; _ } -> c)
    |> effective_reactivity r_super in
  (* for method declarations check if condition type for r_super includes
     reactive method with a matching name. If yes - then it will act as a guarantee
     that derived class will have to redefine the method with a shape required
     by condition type (reactivity of redefined method must be subtype of reactivity
     of method in interface) *)
  let condition_type_has_matching_reactive_method condition_type_super (method_name, is_static) =
    let condition_type_opt =
      Option.bind
        (TUtils.try_unwrap_class_type condition_type_super)
        (fun (_, (_, x), _) -> Env.get_class env x) in
    begin match condition_type_opt with
    | None -> false
    | Some cls ->
      let m = if is_static then cls.tc_smethods else cls.tc_methods in
      begin match SMap.get method_name m with
      | Some { ce_type = lazy (_, Typing_defs.Tfun f); _  } ->
        (* check that reactivity of interface method (effectively a promised
           reactivity of a method in derived class) is a subtype of r_super.
           NOTE: we check only for unconditional reactivity since conditional
           version does not seems to yield a lot and will requre implementing
           cycle detection for condition types *)
        begin match f.ft_reactive with
        | Reactive None | Shallow None | Local None ->
          (* adjust r_super since now we are doing the check assuming
             that dervied class implements the condition type interface *)
          let r_super = effective_reactivity r_super (Some (DeclTy condition_type_super)) in
          subtype_reactivity env f.ft_reactive r_super
        | _ -> false
        end
      | _ -> false
      end
    end in
  match r_sub, r_super, extra_info with
  | MaybeReactive sub, MaybeReactive super, _ ->
    subtype_reactivity ?extra_info env sub super
  | _, MaybeReactive _, _ -> true
  (* anything is a subtype of nonreactive functions *)
  | _, Nonreactive, _ -> true
  (* unconditional local/shallow/reactive functions are subtypes of Local *.
      reason: if condition does not hold Local becomes non-reactive which is
      still ok *)
  | (Local None | Shallow None | Reactive None), Local _, _ -> true
  (* unconditional shallow / reactive functions are subtypes of Shallow *.
     Reason: same as above *)
  | (Shallow None | Reactive None), Shallow _, _ -> true
  (* unconditional reactive functions are subtype of reactive *.
     Reason: same as above *)
  | Reactive None, Reactive _, _ -> true
  (* function type TSub of method M with arbitrary reactivity in derive class
     can be subtype of conditionally reactive function type TSuper of method M
     defined in base class when condition type has reactive method M.
     interface Rx {
       <<__Rx>>
       public function f(): int;
     }
     class A {
       <<__RxIfImplements(Rx::class)>>
       public function f(): int { ... }
     }
     class B extends A {
       public function f(): int { ... }
     }
     This should be OK because:
     - B does not implement Rx (B::f is not compatible with Rx::f) which means
     that calling ($b : B)->f() will not be treated as reactive
     - if one of subclasses of B will decide to implement B - they will be forced
     to redeclare f which now will shadow B::f. Note that B::f will still be
     accessible as parent::f() but will be treated as non-reactive call.
     *)
  | _, (Reactive (Some t) | Shallow (Some t) | Local (Some t)), Some { method_info = Some mi; _ }
    when condition_type_has_matching_reactive_method t mi ->
    true
  (* conditionally reactive function A is a subtype of conditionally reactive
     function B only if condition type of B is a subtype of condition type of A *)
  | Reactive (Some t), Reactive (Some t1), _
  | (Shallow (Some t) | Reactive (Some t)), Shallow (Some t1), _
  | (Local (Some t) | Shallow (Some t) | Reactive (Some t)), Local (Some t1), _ ->
    is_sub_type env (localize t1) (localize t)

  (* call_site specific cases *)
  (* shallow can call into local *)
  | Local None, Shallow None, _ when is_call_site -> true
  | Local (Some t), Shallow (Some t1), _ when is_call_site ->
    is_sub_type env (localize t1) (localize t)
  (* local can call into non-reactive *)
  | Nonreactive, Local _, _ when is_call_site -> true
  | _ -> false


(* This function checks that the method ft_sub can be used to replace
 * (is a subtype of) ft_super.
 *
 * It's used for two purposes:
 * (1) checking that one method can validly override another
 * (2) checking that one function type is a subtype of another
 * For (1) there are a number of features (generics, bounds, Fvariadic)
 * that don't apply in (2)
 * For (2) we apply contravariance on function parameter types, for (1)
 * we treat them invariantly (because of runtime restrictions)
 *
 * The rules must take account of arity,
 * generic parameters and their constraints, parameter types, and return type.
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
 * Note in particular the generali form of the 'where' constraint.
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
and subtype_funs_generic
  ~(check_return : bool)
  ?(extra_info: reactivity_extra_info option)
  (env : Env.env)
  (r_sub : Reason.t)
  (ft_sub : locl fun_type)
  (r_super : Reason.t)
  (ft_super : locl fun_type) : Env.env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  if not (subtype_reactivity ?extra_info env ft_sub.ft_reactive ft_super.ft_reactive) then
    Errors.fun_reactivity_mismatch
      p_super (TUtils.reactivity_to_string env ft_super.ft_reactive)
      p_sub (TUtils.reactivity_to_string env ft_sub.ft_reactive);
  if ft_sub.ft_is_coroutine <> ft_super.ft_is_coroutine
  then Errors.coroutinness_mismatch ft_super.ft_is_coroutine p_super p_sub;
  if ft_sub.ft_return_disposable <> ft_super.ft_return_disposable
  then Errors.return_disposable_mismatch ft_super.ft_return_disposable p_super p_sub;
  (* it is ok for subclass to return mutably owned value and treat it as immutable -
  the fact that value is mutably owned guarantees it has only single reference so
  as a result this single reference will be immutable. However if super type
  returns mutable value and subtype yields immutable value - this is not safe.
  NOTE: error is not reported if child is non-reactive since it does not have
  immutability-by-default behavior *)
  if (ft_sub.ft_returns_mutable <> ft_super.ft_returns_mutable)
    && ft_super.ft_returns_mutable
    && ft_sub.ft_reactive <> Nonreactive
  then Errors.mutable_return_result_mismatch ft_super.ft_returns_mutable p_super p_sub;
  if (arity_min ft_sub.ft_arity) > (arity_min ft_super.ft_arity)
  then Errors.fun_too_many_args p_sub p_super;
  (match ft_sub.ft_arity, ft_super.ft_arity with
    | Fellipsis _, Fvariadic _ ->
      (* The HHVM runtime ignores "..." entirely, but knows about
       * "...$args"; for contexts for which the runtime enforces method
       * compatibility (currently, inheritance from abstract/interface
       * methods), letting "..." override "...$args" would result in method
       * compatibility errors at runtime. *)
      Errors.fun_variadicity_hh_vs_php56 p_sub p_super;
    | Fstandard (_, sub_max), Fstandard (_, super_max) ->
      if sub_max < super_max
      then Errors.fun_too_few_args p_sub p_super;
    | Fstandard _, _ -> Errors.fun_unexpected_nonvariadic p_sub p_super;
    | _, _ -> ()
  );

  let env  = match ft_sub.ft_arity, ft_super.ft_arity with
    | Fvariadic (_, fp_super), Fvariadic (_, { fp_type = var_sub; _ }) ->
      let {fp_type = var_super; _ } = fp_super in
      sub_type env var_sub var_super
    | _ -> env
  in
  (* This is (1) above *)
  let env =
    let variadic_subtype = match ft_sub.ft_arity with
      | Fvariadic (_, {fp_type = var_sub; _ }) -> Some var_sub
      | _ -> None in
    let variadic_supertype =  match ft_super.ft_arity with
      | Fvariadic (_, {fp_type = var_super; _ }) -> Some var_super
      | _ -> None in
    let is_method =
      (Option.map extra_info (fun i -> i.method_info <> None)) = Some true in
    subtype_params
      ~is_method
      env ft_super.ft_params ft_sub.ft_params variadic_subtype variadic_supertype
  in

  (* We check constraint entailment and invariant parameter/covariant result
   * subtyping in the context of the ft_super constraints. But we'd better
   * restore tpenv afterwards *)
  let old_tpenv = env.Env.lenv.Env.tpenv in
  let add_tparams_constraints env (tparams: locl tparam list) =
    let add_bound env (_, (pos, name), cstrl) =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
        let tparam_ty = (Reason.Rwitness pos,
          Tabstract(AKgeneric name, None)) in
        add_constraint pos env ck tparam_ty ty) in
    List.fold_left tparams ~f:add_bound ~init: env in

  let add_where_constraints env (cstrl: locl where_constraint list) =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
      add_constraint p_sub env ck ty1 ty2) in

  let env =
    add_tparams_constraints env ft_super.ft_tparams in
  let env =
    add_where_constraints env ft_super.ft_where_constraints in

  (* Checking that if the return type was defined in the parent class, it
   * is defined in the subclass too (requested by Gabe Levi).
   *)
  (* We agreed this was too painful for now, breaks too many things *)
  (*  (match ft_super.ft_ret, ft_sub.ft_ret with
      | (_, Tany), _ -> ()
      | (r_super, ty), (r_sub, Tany) ->
      let p_super = Reason.to_pos r_super in
      let p_sub = Reason.to_pos r_sub in
      error_l [p_sub, "Please add a return type";
      p_super, "Because we want to be consistent with this annotation"]
      | _ -> ()
      );
  *)
  (* This is (2) above *)
  let env =
    if check_return
    then sub_type env ft_sub.ft_ret ft_super.ft_ret
    else env in

  (* This is (3) above *)
  let check_tparams_constraints env tparams =
  let check_tparam_constraints env (_var, (p, name), cstrl) =
    List.fold_left cstrl ~init:env ~f:begin fun env (ck, cstr_ty) ->
      let tgeneric = (Reason.Rwitness p, Tabstract (AKgeneric name, None)) in
      Typing_generic_constraint.check_constraint env ck cstr_ty tgeneric
    end in
  List.fold_left tparams ~init:env ~f:check_tparam_constraints in

  let check_where_constraints env cstrl =
    List.fold_left cstrl ~init:env ~f:begin fun env (ty1, ck, ty2) ->
      Typing_generic_constraint.check_constraint env ck ty2 ty1
    end in

  (* We only do this if the ft_tparam lengths match. Currently we don't even
   * report this as an error, indeed different names for type parameters.
   * TODO: make it an error to override with wrong number of type parameters
  *)
  let env =
    if List.length ft_sub.ft_tparams <> List.length ft_super.ft_tparams
    then env
    else check_tparams_constraints env ft_sub.ft_tparams in
  let env =
    check_where_constraints env ft_sub.ft_where_constraints in

  Env.env_with_tpenv env old_tpenv

(* Checking subtyping for methods is different than normal functions. Since
 * methods are declarations we do not want to instantiate their function type
 * parameters as unresolved, instead it should stay as a Tgeneric.
 *)
and subtype_method
  ~(check_return : bool)
  ~(extra_info: reactivity_extra_info)
  (env : Env.env)
  (r_sub : Reason.t)
  (ft_sub : decl fun_type)
  (r_super : Reason.t)
  (ft_super : decl fun_type) : Env.env =
  if not ft_super.ft_abstract && ft_sub.ft_abstract then
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See override_abstract_concrete.php test case for example. *)
    Errors.abstract_concrete_override ft_sub.ft_pos ft_super.ft_pos `method_;
  let ety_env =
    Phase.env_with_self env in
  let env, ft_super_no_tvars =
    Phase.localize_ft ~use_pos:ft_super.ft_pos ~ety_env ~instantiate_tparams:false env ft_super in
  let env, ft_sub_no_tvars =
    Phase.localize_ft ~use_pos:ft_sub.ft_pos ~ety_env ~instantiate_tparams:false env ft_sub in
  subtype_funs_generic
    ~check_return env
    ~extra_info
    r_sub ft_sub_no_tvars
    r_super ft_super_no_tvars

and subtype_tparams
  (env : Env.env)
  (c_name : string)
  (variancel : Ast.variance list)
  (children_tyl : locl ty list)
  (super_tyl : locl ty list) : Env.env =
  match variancel, children_tyl, super_tyl with
  | [], [], [] -> env
  | [], _, _
  | _, [], _
  | _, _, [] -> env
  | variance :: variancel, child :: childrenl, super :: superl ->
      let env = subtype_tparam env c_name variance child super in
      subtype_tparams env c_name variancel childrenl superl

and invariance_suggestion (c_name : string) : string =
  let open Naming_special_names.Collections in
  if c_name = cVector then
    "\nDid you mean ConstVector instead?"
  else if c_name = cMap then
    "\nDid you mean ConstMap instead?"
  else if c_name = cSet then
    "\nDid you mean ConstSet instead?"
  else ""

(* TODO: subtype_tparam ignores part of super argument *)
and subtype_tparam
  (env : Env.env)
  (c_name : string)
  (variance : Ast.variance)
  (child : locl ty)
  (super : locl ty) : Env.env =
  let r_super, _ = super in
  match variance with
  | Ast.Covariant -> sub_type env child super
  | Ast.Contravariant ->
      Errors.try_
        (fun () -> sub_type env super child)
        (fun err ->
          let pos = Reason.to_pos r_super in
          Errors.explain_contravariance pos c_name err; env)
  | Ast.Invariant ->
      Errors.try_
        (fun () -> fst (Unify.unify env super child))
        (fun err ->
          let suggestion =
            let s = invariance_suggestion c_name in
            let try_fun = (fun () ->
              subtype_tparam env c_name Ast.Covariant child super) in
            if not (s = "") && Errors.has_no_errors try_fun then s else "" in
          let pos = Reason.to_pos r_super in
          Errors.explain_invariance pos c_name suggestion err; env)

(* Distinction b/w sub_type and sub_type_with_uenv similar to unify and
 * unify_with_uenv, see comment there. *)
and sub_type
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty) : Env.env =
  sub_type_with_uenv env (TUEnv.empty, ty_sub) (TUEnv.empty, ty_super)

(**
 * Checks that ty_sub is a subtype of ty_super, and returns an env.
 *
 * E.g. sub_type env ?int int   => env
 *      sub_type env int alpha  => env where alpha==int
 *      sub_type env ?int alpha => env where alpha==?int
 *      sub_type env int string => error
*)
and sub_type_with_uenv
  (env : Env.env)
  ((uenv_sub, ty_sub) : TUEnv.uenv * locl ty)
  ((uenv_super, ty_super) : TUEnv.uenv * locl ty) : Env.env =
  Typing_log.log_types 2 (Reason.to_pos (fst ty_sub)) env
    [Typing_log.Log_sub (Printf.sprintf
        "Typing_subtype.sub_type_with_uenv uenv_sub.unwrappedToption=%b uenv_super.unwrappedToption=%b"
        uenv_sub.TUEnv.unwrappedToption uenv_super.TUEnv.unwrappedToption,
      [Typing_log.Log_type ("ty_sub", ty_sub);
       Typing_log.Log_type ("ty_super", ty_super)])];
  let env, ety_super =
    Env.expand_type env ty_super in
  let env, ety_sub =
    Env.expand_type env ty_sub in

  match ety_sub, ety_super with
  | (_, Terr), _
  | _, (_, Terr) -> env

  | (_, Tunresolved _), (_, Tunresolved _) ->
      let env, _ =
        Unify.unify_unwrapped env uenv_super.TUEnv.unwrappedToption ty_super
                                  uenv_sub.TUEnv.unwrappedToption ty_sub in
      env
(****************************************************************************)
(* ### Begin Tunresolved madness ###
 * If the supertype is a
 * Tunresolved, we allow it to keep growing, which is the desired behavior for
 * e.g. figuring out the type of a generic, but if the subtype is a
 * Tunresolved, then we check that all the members are indeed subtypes of the
 * given supertype, which is the desired behavior for e.g. checking function
 * return values. In general, if a supertype is Tunresolved, then we
 * consider it to be "not yet finalized", but if a subtype is Tunresolved
 * and the supertype isn't, we've probably hit a type annotation
 * and should consider the supertype to be definitive.
 *
 * However, sometimes we want this behavior reversed, e.g. when the type
 * annotation has a contravariant generic parameter or a `super` constraint --
 * now the definitive type is the subtype.
 *
 * I considered splitting this out into a separate function and swapping the
 * order of the super / sub types passed to it, so we would only have to handle
 * one set of cases, but it doesn't look much better since that function still
 * has to recursively call sub_type and therefore needs to remember whether its
 * arguments had been swapped.
 *)
(****************************************************************************)
  | (r_sub, _), (_, Tunresolved _) ->
      let ty_sub = (r_sub, Tunresolved [ty_sub]) in
      let env, _ =
        Unify.unify_unwrapped
          env ~unwrappedToption1:uenv_super.TUEnv.unwrappedToption ty_super
              ~unwrappedToption2:uenv_sub.TUEnv.unwrappedToption ty_sub in
      env
  | (_, Tunresolved _), (_, Tany) ->
      (* This branch is necessary in the following case:
       * function foo<T as I>(T $x)
       * if I call foo with an intersection type, T is a Tvar,
       * it's expanded version (ety_super in this case) is Tany and what
       * we end up doing is unifying all the elements of the intersection
       * together ...
       * Thanks to this branch, the type variable unifies with the intersection
       * type.
       *)
    fst (Unify.unify_unwrapped
        env ~unwrappedToption1:uenv_super.TUEnv.unwrappedToption ty_super
            ~unwrappedToption2:uenv_sub.TUEnv.unwrappedToption ty_sub)

    (* If the subtype is a type variable bound to an empty unresolved, record
     * this in the todo list to be checked later. But make sure that we wrap
     * any error using the position and reason information that was supplied
     * at the entry point to subtyping.
     *)
  | (_, Tunresolved []), _ ->
    begin match ty_sub with
    | (_, Tvar _) ->
      if not
        (TypecheckerOptions.experimental_feature_enabled
        (Env.get_options env)
        TypecheckerOptions.experimental_unresolved_fix)
      then env
      else
        let outer_pos = env.Env.outer_pos in
        let outer_reason = env.Env.outer_reason in
        Typing_log.log_types 2 outer_pos env
        [Typing_log.Log_sub
          ("Typing_subtype.add_todo",
           [Typing_log.Log_type ("ty_sub", ty_sub);
           Typing_log.Log_type ("ty_super", ty_super)])];
        Env.add_todo env begin fun env' ->
          Errors.try_add_err outer_pos (Reason.string_of_ureason outer_reason)
          (fun () ->
            sub_type env' ty_sub ty_super, false)
          (fun _ ->
            env', true (* Remove from todo list if there was an error *)
          )
          end

    | _ ->
      env
    end

  | (_, Tunresolved tyl), _ ->
    let env =
      List.fold_left tyl ~f:begin fun env x ->
        sub_type_with_uenv env (uenv_sub, x) (uenv_super, ty_super)
      end ~init:env in
    env

(****************************************************************************)
(* ### End Tunresolved madness ### *)
(****************************************************************************)
  | (_, Tany), _ -> env
  (* This case is for when Tany comes from expanding an empty Tvar - it will
   * result in binding the type variable to the other type. *)
  | _, (_, Tany) -> fst (Unify.unify env ty_super ty_sub)
  | (_,   Tabstract (AKdependent d_sub, Some ty_sub)),
    (_, Tabstract (AKdependent d_super, Some ty_super))
        when d_sub = d_super ->
      let uenv_sub = TUEnv.update_this_if_unset uenv_sub ety_sub in
      let uenv_super = TUEnv.update_this_if_unset uenv_super ety_super in
      sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  (* This is sort of a hack because our handling of Toption is highly
   * dependent on how the type is structured. When we see a bare
   * dependent type we strip it off at this point since it shouldn't be
   * relevant to subtyping any more.
   *)
  | (_, Tabstract (AKdependent (`expr _, []), Some ty_sub)), _ ->
      let uenv_sub = TUEnv.update_this_if_unset uenv_sub ety_sub  in
      sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  | (_, Tabstract (AKdependent d_sub, Some sub)),
    (_,     Tabstract (AKdependent d_super, _)) when d_sub <> d_super ->
      let uenv_sub = TUEnv.update_this_if_unset uenv_sub ety_sub in
      (* If an error occurred while subtyping, we produce a unification error
       * so we get the full information on how the dependent type was
       * generated
       *)
      Errors.try_when
        (fun () ->
          sub_type_with_uenv env (uenv_sub, sub) (uenv_super, ty_super))
        ~when_: begin fun () ->
          match sub, TUtils.get_base_type env ty_super with
          | (_, Tclass ((_, y), _)), (_, Tclass ((_, x), _)) when x = y -> false
          | _, _ -> true
        end
        ~do_: (fun _ -> TUtils.simplified_uerror env ty_super ty_sub)

  | (p_sub, (Tclass (x_sub, tyl_sub) as ty_sub_)),
    (p_super, (Tclass (x_super, tyl_super) as ty_super_)) ->
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    if cid_super = cid_sub then
      if tyl_super <> [] && List.length tyl_super = List.length tyl_sub
      then
        match Env.get_class env cid_super with
        | None -> fst (Unify.unify env ety_super ety_sub)
        | Some { tc_tparams; _} ->
            let variancel =
              List.map tc_tparams (fun (variance, _, _) -> variance)
            in
            subtype_tparams env cid_super variancel tyl_sub tyl_super
      else fst (Unify.unify env ety_super ety_sub)
    else begin
      let class_ = Env.get_class env cid_sub in
      (match class_ with
        | None -> env
        | Some class_ ->
          let ety_env =
            (* We handle the case where a generic A<T> is used as A *)
            let tyl_sub =
              if tyl_sub = [] && not (Env.is_strict env)
              then List.map class_.tc_tparams (fun _ -> (p_sub, Tany))
              else tyl_sub
            in
            if List.length class_.tc_tparams <> List.length tyl_sub
            then
              Errors.expected_tparam
                (Reason.to_pos p_sub) (List.length class_.tc_tparams);
            (* NOTE: We rely on the fact that we fold all ancestors of
             * ty_sub in its class_type so we will never hit this case
             * again. If this ever changes then we would need to store
             * ty_sub as the 'this_ty' in the uenv and be careful to
             * thread it through.
             *
             * This is covered by test/typecheck/this_tparam2.php
            *)
            {
              type_expansions = [];
              substs = Subst.make class_.tc_tparams tyl_sub;
              this_ty = Option.value uenv_sub.TUEnv.this_ty ~default:ty_sub;
              from_class = None;
            } in
          let subtype_req_ancestor =
            if class_.tc_kind = Ast.Ctrait || class_.tc_kind = Ast.Cinterface then
              (* a trait is never the runtime type, but it can be used
               * as a constraint if it has requirements for its using
               * classes *)
              let _, ret = List.fold_left ~f:begin fun acc (_p, req_type) ->
                match acc with
                  | _, Some _ -> acc
                  | env, None ->
                    Errors.try_ begin fun () ->
                      let env, req_type =
                        Phase.localize ~ety_env env req_type in
                      let _, req_ty = req_type in
                      env, Some (sub_type env (p_sub, req_ty) ty_super)
                    end (fun _ -> acc)
              end class_.tc_req_ancestors ~init:(env, None) in
              ret
            else None in
          (match subtype_req_ancestor with
            | Some ret -> ret
            | None ->
              let up_obj = SMap.get cid_super class_.tc_ancestors in
              match up_obj with
                | Some up_obj ->
                  let env, up_obj =
                      Phase.localize ~ety_env env up_obj in
                  sub_type env up_obj ty_super
                | None ->
                  TUtils.uerror p_super ty_super_ p_sub ty_sub_;
                  env
           )
      )
    end
  | _, (_, Tmixed) -> env
  | _, (_, Tdynamic) -> env
  | _, (_, Toption (_, Tnonnull)) -> env
  | (_,
     (Tprim Nast.(Tint | Tbool | Tfloat | Tstring
                  | Tresource | Tnum | Tarraykey | Tnoreturn)
      | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _
      | Tobject | Tclass _ | Tarraykind _ | Tabstract (AKenum _, _))),
    (_, Tnonnull) -> env
  | (_, (Tprim Nast.Tvoid | Tmixed | Tdynamic | Toption _ | Tvar _
         | Tabstract ((AKnewtype _ | AKdependent _), None))),
    (_, Tnonnull) -> fst (Unify.unify env ty_super ty_sub)
  | (_, Tprim (Nast.Tint | Nast.Tfloat)), (_, Tprim Nast.Tnum) -> env
  | (_, Tprim (Nast.Tint | Nast.Tstring)), (_, Tprim Nast.Tarraykey) -> env
  | (_, Tabstract ((AKenum _), _)), (_, Tprim Nast.Tarraykey) -> env
  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tv_super]))
    when (coll = SN.Collections.cTraversable ||
        coll = SN.Collections.cContainer) ->
      (match akind with
      | AKany -> env
      | AKempty -> env
      | AKvarray tv
      | AKvec tv
      | AKdarray (_, tv)
      | AKvarray_or_darray tv
      | AKmap (_, tv) ->
          sub_type env tv tv_super
      | AKshape fdm ->
          Typing_arrays.fold_akshape_as_akmap begin fun env ty_sub ->
            sub_type env ty_sub ty_super
          end env r fdm
      | AKtuple fields ->
          Typing_arrays.fold_aktuple_as_akvec begin fun env ty_sub ->
            sub_type env ty_sub ty_super
          end env r fields
      )
  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tk_super; tv_super]))
    when (coll = SN.Collections.cKeyedTraversable
         || coll = SN.Collections.cKeyedContainer
         || coll = SN.Collections.cIndexish) ->
      (match akind with
      | AKany -> env
      | AKempty -> env
      | AKvarray tv
      | AKvec tv ->
        let env = sub_type env (r, Tprim Nast.Tint) tk_super in
        sub_type env tv tv_super
      | AKvarray_or_darray tv ->
        let tk_sub =
          Reason.Rvarray_or_darray_key (Reason.to_pos r),
          Tprim Nast.Tarraykey in
        let env = sub_type env tk_sub tk_super in
        sub_type env tv tv_super
      | AKdarray (tk, tv)
      | AKmap (tk, tv) ->
        let env = sub_type env tk tk_super in
        sub_type env tv tv_super
      | AKshape fdm ->
        Typing_arrays.fold_akshape_as_akmap begin fun env ty_sub ->
          sub_type env ty_sub ty_super
        end env r fdm
      | AKtuple fields ->
          Typing_arrays.fold_aktuple_as_akvec begin fun env ty_sub ->
            sub_type env ty_sub ty_super
          end env r fields
      )

  | (_, Tprim Nast.Tstring), (_, Tclass ((_, stringish), _))
    when stringish = SN.Classes.cStringish -> env
  | (_, Tarraykind _), (_, Tclass ((_, xhp_child), _))
  | (_, Tprim (Nast.Tarraykey | Nast.Tint | Nast.Tfloat | Nast.Tstring | Nast.Tnum)), (_, Tclass ((_, xhp_child), _))
    when xhp_child = SN.Classes.cXHPChild -> env
  | (_, Tarraykind (AKdarray (tk_sub, tv_sub) | AKmap (tk_sub, tv_sub))),
    (r, Tarraykind (AKvarray_or_darray tv_super)) ->
      let tk_super =
        Reason.Rvarray_or_darray_key (Reason.to_pos r),
        Tprim Nast.Tarraykey in
      let env = sub_type env tk_sub tk_super in
      sub_type env tv_sub tv_super
  | (reason, Tarraykind (AKvarray elt_ty | AKvec elt_ty)),
    (_, Tarraykind (AKdarray _ | AKmap _))
    when not (TypecheckerOptions.safe_vector_array (Env.get_options env)) ->
      let int_reason = Reason.Ridx (Reason.to_pos reason, Reason.Rnone) in
      let int_type = int_reason, Tprim Nast.Tint in
      sub_type env (reason, Tarraykind (AKmap (int_type, elt_ty))) ty_super
  | (r, Tarraykind AKshape fdm_sub), _ ->
      Typing_arrays.fold_akshape_as_akmap begin fun env ty_sub ->
        sub_type env ty_sub ty_super
      end env r fdm_sub
  | (r, Tarraykind AKtuple fields_sub), _ ->
      Typing_arrays.fold_aktuple_as_akvec begin fun env ty_sub ->
        sub_type env ty_sub ty_super
      end env r fields_sub

  | (r, Tarraykind ak_sub), (_, Tarraykind ak_super) ->
    decompose_array env r ak_sub ak_super sub_type (fun env -> fst (Unify.unify env ty_super ty_sub))

  (* If ?t1 <: ?t2, then from t1 <: ?t1 (widening) and transitivity
   * of <: it follows that t1 <: ?t2.  Conversely, if t1 <: ?t2, then
   * by covariance and idempotence of ?, we have ?t1 <: ??t2 <: ?t2.
   * Therefore, this step preserves the set of solutions.
   *)
  | (_, Toption ty_sub), (_, Toption _) ->
    let uenv_sub = {uenv_sub with TUEnv.unwrappedToption = true} in
    sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  (* void is the type of null and is a subtype of any option type. *)
  | (_, Tprim Nast.Tvoid), (_, Toption _)
    when TUtils.is_void_type_of_null env -> env

  (* If the nonnull type is not enabled, mixed <: ?t is equivalent
   * to mixed <: t.  Otherwise, we should not encounter mixed
   * because by this time it should have been desugared into ?nonnull.
   *)
  | (_, (Tmixed | Tdynamic)), (_, Toption ty_super) ->
    let uenv_super = {uenv_super with TUEnv.unwrappedToption = true} in
    sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  (* If t1 <: ?t2, where t1 is guaranteed not to contain null, then
   * t1 <: t2, and the converse is obviously true as well.
   *)
  | (_, (Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ |
         Tobject | Tclass _ | Tarraykind _ |
         Tabstract ((AKdependent _ | AKnewtype _| AKenum _), None))),
    (_, Toption ty_super) ->
    let uenv_super = {uenv_super with TUEnv.unwrappedToption = true} in
    sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
   * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
   * true as well.  We can fold the case where t1 is unconstrained
   * into the case analysis below.
   *)
  | (_, Tabstract ((AKnewtype _ | AKenum _), Some ty)),
    (_, Toption arg_ty_super) ->
    Errors.try_
      (fun () ->
        let uenv_super = {uenv_super with TUEnv.unwrappedToption = true} in
        sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, arg_ty_super))
      (fun _ ->
        sub_type_with_uenv env (uenv_sub, ty) (uenv_super, ty_super))

  | (_, Tabstract (AKdependent _, Some ty)), (_, Toption arg_ty_super) ->
    Errors.try_
      (fun () ->
        let uenv_super = {uenv_super with TUEnv.unwrappedToption = true} in
        sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, arg_ty_super))
      (fun _ ->
        let uenv_sub = TUEnv.update_this_if_unset uenv_sub ety_sub in
        sub_type_with_uenv env (uenv_sub, ty) (uenv_super, ty_super))

  | (_, Tabstract (AKgeneric _, _)), (_, Toption arg_ty_super) ->
    Errors.try_
      (fun () ->
        let uenv_super = {uenv_super with TUEnv.unwrappedToption = true} in
        sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, arg_ty_super))
      (fun _ ->
        sub_generic_params SSet.empty env
          (uenv_sub, ty_sub) (uenv_super, ty_super))

  | (r_sub, Tvar v), (_, Toption _) ->
    let env, ty_sub = Env.get_type env r_sub v in
    sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  | (_, Ttuple tyl_sub), (_, Ttuple tyl_super)
    when List.length tyl_super = List.length tyl_sub ->
    wfold_left2 sub_type env tyl_sub tyl_super
  | (r_sub, Tfun ft_sub), (r_super, Tfun ft_super) ->
    subtype_funs_generic  ~check_return:true
      env r_sub ft_sub r_super ft_super
  | (r_sub, Tanon (anon_arity, id)), (r_super, Tfun ft)  ->
      (match Env.get_anonymous env id with
      | None ->
          Errors.anonymous_recursive_call (Reason.to_pos r_sub);
          env
      | Some (reactivity, is_coroutine, counter, _, anon) ->
          let p_super = Reason.to_pos r_super in
          let p_sub = Reason.to_pos r_sub in
          if not (subtype_reactivity env reactivity ft.ft_reactive)
          then Errors.fun_reactivity_mismatch
            p_super (TUtils.reactivity_to_string env reactivity)
            p_sub (TUtils.reactivity_to_string env ft.ft_reactive);
          if is_coroutine <> ft.ft_is_coroutine
          then Errors.coroutinness_mismatch ft.ft_is_coroutine p_super p_sub;
          if not (Unify.unify_arities
                    ~ellipsis_is_variadic:true anon_arity ft.ft_arity)
          then Errors.fun_arity_mismatch p_super p_sub;
          counter := !counter + 1;
          let env, _, ret = anon env ft.ft_params ft.ft_arity in
          let env = sub_type env ret ft.ft_ret in
          env
      )
  | (r_sub, Tshape (fields_known_sub, fdm_sub)),
    (r_super, Tshape (fields_known_super, fdm_super)) ->
      (**
       * shape_field_type A <: shape_field_type B iff:
       *   1. A is no more optional than B
       *   2. A's type <: B.type
       *)
      let on_common_field
          (env, acc)
          name
          { sft_optional = optional_super; sft_ty = ty_super }
          { sft_optional = optional_sub; sft_ty = ty_sub } =
        match optional_super, optional_sub with
          | true, _ | false, false ->
            sub_type env ty_sub ty_super, acc
          | false, true ->
            Errors.required_field_is_optional
              (Reason.to_pos r_sub)
              (Reason.to_pos r_super)
              (Env.get_shape_field_name name);
            env, acc in
      fst (TUtils.apply_shape
        ~on_common_field
        ~on_missing_optional_field:(fun acc _ _ -> acc)
        (env, None)
        (r_super, fields_known_super, fdm_super)
        (r_sub, fields_known_sub, fdm_sub))
  | (_, Tabstract (AKnewtype (name_sub, tyl_sub), _)),
    (_, Tabstract (AKnewtype (name_super, tyl_super), _))
    when name_super = name_sub ->
      let td = Env.get_typedef env name_super in
      begin match td with
        | Some {td_tparams; _} ->
          let variancel =
            List.map td_tparams (fun (variance, _, _) -> variance) in
          subtype_tparams env name_super variancel tyl_sub tyl_super
        | _ -> env
      end

  (* Supertype is generic parameter *and* subtype is a newtype with bound.
   * We need to make this a special case because there is a *choice*
   * of subtyping rule to apply. See details in the case of dependent type
   * against generic parameter which is similar
   *)
  | (_, Tabstract (AKnewtype (_, _), Some ty)),
    (_, Tabstract (AKgeneric _, _)) ->
     Errors.try_
       (fun () -> fst (Unify.unify env ty_super ty_sub))
       (fun _ ->
          Errors.try_
           (fun () ->
             sub_type_with_uenv env (uenv_sub, ty) (uenv_super, ty_super))
           (fun _ ->
              sub_generic_params SSet.empty env (uenv_sub, ty_sub)
                (uenv_super, ty_super)))

  | (_, Tabstract ((AKnewtype (_, _) | AKenum _), Some ty)), _ ->
    Errors.try_
      (fun () ->
         fst @@ Unify.unify env ty_super ty_sub
      )
      (fun _ ->  sub_type_with_uenv env (uenv_sub, ty) (uenv_super, ty_super))

  (* Supertype is generic parameter *and* subtype is dependent.
   * We need to make this a special case because there is a *choice*
   * of subtyping rule to apply.
   *
   * Example. First suppose that we have the definition
   *
   *     abstract const type TC as C
   *
   * (1) Now suppose we have to check
   *       this::TC <: Tu
   *     where we have the constraint
   *       <Tu super C>.
   *     Then it's necessary to apply the rule for AKdependent first, so we
   *     reduce this problem to
   *       C <: Tu
   *     and then call sub_generic_params to deal with the type parameter.
   * (2) Alternatively, suppose we again have to check
   *       this::TC <: Tu
   *     but this time we have the constraint
   *       <Tu super this::TC>.
   *    Then if we first reduce the problem to C <: Tu we fail;
   *    but if we also try sub_generic_params then we succeed, because
   *    we end up checking this::TC <: this::TC.
  *)
  | (_, Tabstract (AKdependent _, Some ty)), (_, Tabstract (AKgeneric _, _)) ->
    Errors.try_
      (fun () -> fst (Unify.unify env ty_super ty_sub))
      (fun _ ->
         Errors.try_
          (fun () ->
            let uenv_sub = TUEnv.update_this_if_unset uenv_sub ety_sub in
            sub_type_with_uenv env (uenv_sub, ty) (uenv_super, ty_super))
          (fun _ ->
             sub_generic_params SSet.empty env (uenv_sub, ty_sub)
               (uenv_super, ty_super)))

  | (_, Tabstract (AKdependent _, Some ty)), _ ->
      Errors.try_
        (fun () -> fst (Unify.unify env ty_super ty_sub))
        (fun _ ->
          let uenv_sub = TUEnv.update_this_if_unset uenv_sub ety_sub in
          sub_type_with_uenv env (uenv_sub, ty) (uenv_super, ty_super))

  (* Subtype is generic parameter
   * We delegate this case to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
   *)
  | (_, Tabstract (AKgeneric _, _)), _ ->
    sub_generic_params SSet.empty env (uenv_sub, ty_sub) (uenv_super, ty_super)

  (* Supertype is generic parameter
   * We delegate this case to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
  *)
  | _, (_, Tabstract (AKgeneric _, _)) ->
    sub_generic_params SSet.empty env (uenv_sub, ty_sub) (uenv_super, ty_super)

  | _, (_, (Tarraykind _ | Tprim _ | Tvar _
    | Tabstract (_, _) | Ttuple _ | Tanon (_, _) | Tfun _
    | Tobject | Tshape _ | Tclass (_, _))
    ) -> fst (Unify.unify env ty_super ty_sub)

and sub_generic_params
  (seen : SSet.t)
  (env : Env.env)
  ((uenv_sub, ty_sub) : TUEnv.uenv * locl ty)
  ((uenv_super, ty_super) : TUEnv.uenv * locl ty) : Env.env =
  Typing_log.log_types 2 (Reason.to_pos (fst ty_sub)) env
    [Typing_log.Log_sub ("Typing_subtype.sub_generic_params",
      [Typing_log.Log_type ("ty_sub", ty_sub);
       Typing_log.Log_type ("ty_super", ty_super)])];
  let env, ety_super = Env.expand_type env ty_super in
  let env, ety_sub = Env.expand_type env ty_sub in
  match ety_sub, ety_super with
  (* If subtype and supertype are the same generic parameter, we're done *)
  | (_, Tabstract (AKgeneric name_sub, _)),
    (_, Tabstract (AKgeneric name_super, _))
       when name_sub = name_super
    -> env

  (* Subtype is generic parameter *)
  | (r_sub, Tabstract (AKgeneric name_sub, opt_sub_cstr)), _ ->
    (* If the generic is actually an expression dependent type,
      we need to update the Unification environment's this_ty
    *)
    let uenv_sub = if AbstractKind.is_generic_dep_ty name_sub then
    TUEnv.update_this_if_unset uenv_sub ety_sub else uenv_sub in
    (* If we've seen this type parameter before then we must have gone
     * round a cycle so we fail
     *)
    if SSet.mem name_sub seen
    then fst (Unify.unify env ty_super ty_sub)
    else
      let seen = SSet.add name_sub seen in
      (* Otherwise, we collect all the upper bounds ("as" constraints) on
         the generic parameter, and check each of these in turn against
         ty_super until one of them succeeds
       *)
      let rec try_bounds tyl =
        match tyl with
        | [] ->
          (* There are no bounds so force an error *)
           fst (Unify.unify env ty_super ty_sub)

        | ty::tyl ->
          Errors.try_
            (fun () ->
              sub_generic_params seen env (uenv_sub, ty)
                                          (uenv_super, ty_super))
            (fun l ->
              (* Right now we report constraint failure based on the last
               * error. This should change when we start supporting
               * multiple constraints *)
              if List.is_empty tyl
              then (Reason.explain_generic_constraint
                env.Env.pos r_sub name_sub l; env)
              else try_bounds tyl)
      in try_bounds (Option.to_list opt_sub_cstr @
           Typing_set.elements (Env.get_upper_bounds env name_sub))

  (* Supertype is generic parameter *)
  | _, (r_super, Tabstract (AKgeneric name_super, _)) ->
    (* If the generic is actually an expression dependent type,
      we need to update the Unification environment's this_ty
    *)
    let uenv_super =  if AbstractKind.is_generic_dep_ty name_super then
    TUEnv.update_this_if_unset uenv_super ety_super else uenv_super in
    (* If we've seen this type parameter before then we must have gone
     * round a cycle so we fail
     *)
    if SSet.mem name_super seen
    then fst (Unify.unify env ty_super ty_sub)
    else
      let seen = SSet.add name_super seen in
      (* Collect all the lower bounds ("super" constraints) on the
       * generic parameter, and check ty_sub against each of them in turn
       * until one of them succeeds *)
      let rec try_bounds tyl =
        match tyl with
        | [] ->
          (* There are no bounds so force an error *)
          fst (Unify.unify env ty_super ty_sub)

        | ty::tyl ->
          Errors.try_
            (fun () -> sub_generic_params seen env (uenv_sub, ty_sub)
                                                   (uenv_super, ty))
          (fun l ->
           (* Right now we report constraint failure based on the last
            * error. This should change when we start supporting
              multiple constraints *)
             if List.is_empty tyl
             then (Reason.explain_generic_constraint
                 env.Env.pos r_super name_super l; env)
             else try_bounds tyl)
      in try_bounds (Typing_set.elements (Env.get_lower_bounds env name_super))

  | _, _ ->
    sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

(* BEWARE: hack upon hack here.
 * To implement a predicate that tests whether `ty_sub` is a subtype of
 * `ty_super`, we call sub_type but handle any unification errors and
 * turn them into `false` result. Unfortunately HH_FIXME might end up
 * hiding the "error", and so we need to disable the fixme mechanism
 * before calling sub_type and then re-enable it afterwards.
 *)
and is_sub_type
  (env : Env.env)
  (ty_sub : locl ty)
  (ty_super : locl ty) : bool =
  let f = !Errors.is_hh_fixme in
  Errors.is_hh_fixme := (fun _ _ -> false);
  let result =
    Errors.try_
      (fun () -> ignore(sub_type env ty_sub ty_super); true)
      (fun _ -> false) in
  Errors.is_hh_fixme := f;
  result

and sub_string
  (p : Pos.Map.key)
  (env : Env.env)
  (ty2 : locl ty) : Env.env =
  let env, ety2 = Env.expand_type env ty2 in
  match ety2 with
  | (_, Toption ty2) -> sub_string p env ty2
  | (_, Tunresolved tyl) ->
      List.fold_left tyl ~f:(sub_string p) ~init:env
  | (_, Tprim _) ->
      env
  | (_, Tabstract (AKenum _, _)) ->
      (* Enums are either ints or strings, and so can always be used in a
       * stringish context *)
      env
  | (_, Tabstract _) ->
    begin match TUtils.get_concrete_supertypes env ty2 with
      | env, [] ->
        fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)
      | env, tyl ->
        List.fold_left tyl ~f:(sub_string p) ~init:env
    end
  | (r2, Tclass (x, _)) ->
      let class_ = Env.get_class env (snd x) in
      (match class_ with
      | None -> env
      | Some tc
          (* A Stringish is a string or an object with a __toString method
           * that will be converted to a string *)
          when tc.tc_name = SN.Classes.cStringish
          || SMap.mem SN.Classes.cStringish tc.tc_ancestors ->
        env
      | Some _ ->
        Errors.object_string p (Reason.to_pos r2);
        env
      )
  | _, (Tany | Terr | Tdynamic) ->
    env (* Tany, Terr and Tdynamic are considered Stringish by default *)
  | _, Tobject -> env
  | _, (Tmixed | Tnonnull | Tarraykind _ | Tvar _
    | Ttuple _ | Tanon (_, _) | Tfun _ | Tshape _) ->
      fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type
let () = Typing_utils.add_constraint_ref := add_constraint
