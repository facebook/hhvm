(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Core
open Typing_defs
open String_utils

module Env = Typing_env
module TUtils = Typing_utils
module TURecursive = Typing_unify_recursive

(* Most code -- notably the cases in unify_ -- do *not* need to thread through
 * unwrappedToptionX, since for example just because we know an array<foo, bar>
 * can't itself be null, that doesn't mean that foo and bar can't be null.
 *)
let rec unify ?(opts=TUtils.default_unify_opt) env ty1 ty2 =
  unify_unwrapped ~opts env
    ~unwrappedToption1:false ty1 ~unwrappedToption2:false ty2

(* If result is (env', ty) then env' extends env,
 * and ty1 <: ty and ty2 <: ty under env'
 *
 * If unwrappedToptionX = true then elide Toption before recursing.
 *)
and unify_unwrapped ?(opts=TUtils.default_unify_opt) env
    ~unwrappedToption1 ty1 ~unwrappedToption2 ty2 =
  if ty1 == ty2 then env, ty1 else
  match ty1, ty2 with
  | (_, (Tany | Terr)), ty | ty, (_, (Tany | Terr)) -> env, ty
  | (r1, Tvar n1), (r2, Tvar n2) ->
    let r = unify_reason r1 r2 in
    let env, n1 = Env.get_var env n1 in
    let env, n2 = Env.get_var env n2 in
    if n1 = n2 then env, (r, Tvar n1) else
    let env, ty1 = Env.get_type_unsafe env n1 in
    let env, ty2 = Env.get_type_unsafe env n2 in
    let n' = Env.fresh() in
    let env = Env.rename env n1 n' in
    let env = Env.rename env n2 n' in
    let env, ty = unify_unwrapped ~opts env
        ~unwrappedToption1 ty1 ~unwrappedToption2 ty2 in
    let env = TURecursive.add env n' ty in
    env, (r, Tvar n')
  | (r, Tvar n), ty2
  | ty2, (r, Tvar n) ->
    let env, ty1 = Env.get_type env r n in
    let n' = Env.fresh() in
    let env = Env.rename env n n' in
    let env, ty = unify_unwrapped ~opts env
        ~unwrappedToption1 ty1 ~unwrappedToption2 ty2 in
    let env = TURecursive.add env n ty in
    env, (r, Tvar n')
  | (r1, Tunresolved tyl1), (r2, Tunresolved tyl2) ->
      let r = unify_reason r1 r2 in
      (* TODO this should probably pass through unwrappedToptions? *)
      let env, tyl = TUtils.normalize_inter env tyl1 tyl2 in
      env, (r, Tunresolved tyl)
  | (r, Tunresolved tyl), (_, ty_ as ty)
  | (_, ty_ as ty), (r, Tunresolved tyl) ->
      let p1 = TUtils.find_pos (Reason.to_pos r) tyl in
      let str_ty = Typing_print.error ty_ in
      let r = Reason.Rcoerced (p1, env.Env.pos, str_ty) in
      let env = List.fold_left tyl
        ~f:(fun env x -> TUtils.sub_type env x ty) ~init:env in
      env, (r, ty_)
  | (_, Toption ty1), _ when unwrappedToption1 ->
      unify_unwrapped env unwrappedToption1 ty1 unwrappedToption2 ty2
  | _, (_, Toption ty2) when unwrappedToption2 ->
    unify_unwrapped env unwrappedToption1 ty1 unwrappedToption2 ty2
  (* We are trying to solve ?ty1 against ?ty2, so recursively solve
   * ty1 against ty2, but strip any subsequent uses of Toption because
   * ??t is regarded as equivalent to ?t
   *)
  | (r1, Toption ty1), (r2, Toption ty2) ->
      let r = unify_reason r1 r2 in
      let env, ty = unify_unwrapped env ~unwrappedToption1:true ty1
                                        ~unwrappedToption2:true ty2 in
      env, (r, Toption ty)
  (* Mixed is nullable and we want it to unify with both ?T and T at
   * the same time. If we try to unify mixed with an option,
   * we peel of the ? and unify mixed with the underlying type. *)
  | (r2, Tmixed), (_, Toption ty1)
  | (_, Toption ty1), (r2, Tmixed) ->
    unify ~opts env ty1 (r2, Tmixed)
  | (r1, ty1), (r2, ty2) ->
      let r = unify_reason r1 r2 in
      let env, ty = unify_ ~opts env r1 ty1 r2 ty2 in
      env, (r, ty)

and unify_ ?(opts=TUtils.default_unify_opt) env r1 ty1 r2 ty2 =
  match ty1, ty2 with
  | Tprim x, Tprim y ->
    if x == y then env, Tprim x
    else
      let () = TUtils.uerror r1 ty1 r2 ty2 in
      env, Terr
  | Tarraykind AKempty, (Tarraykind _ as ty)
  | (Tarraykind _ as ty), Tarraykind AKempty
  | (Tarraykind AKany as ty), Tarraykind AKany ->
      env, ty
  | Tarraykind AKany, (Tarraykind _ as ty)
  | (Tarraykind _ as ty), Tarraykind AKany ->
      let safe_array =
        TypecheckerOptions.safe_array (Env.get_options env) in
      if safe_array then
        (TUtils.uerror r1 ty1 r2 ty2;
        env, Terr)
      else
        env, ty
  | Tarraykind AKvarray ty1, Tarraykind AKvarray ty2 ->
      let env, ty = unify env ty1 ty2 in
      env, Tarraykind (AKvarray ty)
  | Tarraykind (AKvarray ty1 | AKvec ty1),
    Tarraykind (AKvarray ty2 | AKvec ty2) ->
      let env, ty = unify env ty1 ty2 in
      env, Tarraykind (AKvec ty)
  | Tarraykind AKdarray (ty1, ty2), Tarraykind AKdarray (ty3, ty4) ->
      let env, ty1 = unify env ty1 ty3 in
      let env, ty2 = unify env ty2 ty4 in
      env, Tarraykind (AKdarray (ty1, ty2))
  | Tarraykind (AKdarray (ty1, ty2) | AKmap (ty1, ty2)),
    Tarraykind (AKdarray (ty3, ty4) | AKmap (ty3, ty4)) ->
      let env, ty1 = unify env ty1 ty3 in
      let env, ty2 = unify env ty2 ty4 in
      env, Tarraykind (AKmap (ty1, ty2))
  | Tarraykind (AKvarray_or_darray ty1) , Tarraykind (AKvarray_or_darray ty2) ->
      let env, ty = unify env ty1 ty2 in
      env, Tarraykind (AKvarray_or_darray ty)
  | Tarraykind (
      AKvarray _
      | AKvec _
      | AKdarray _
      | AKvarray_or_darray _
      | AKmap _
    ),
    Tarraykind (AKshape _ | AKtuple _)->
    unify_ ~opts env r2 ty2 r1 ty1
  | Tarraykind AKshape fdm1,
    Tarraykind (
      AKvarray _
      | AKvec _
      | AKdarray _
      | AKvarray_or_darray _
      | AKmap _
    ) ->
    Typing_arrays.fold_akshape_as_akmap_with_acc begin fun env ty2 (r1, ty1) ->
      unify_ env r1 ty1 r2 ty2
    end env ty2 r1 fdm1
  | Tarraykind AKtuple fields,
    Tarraykind (
      AKvarray _
      | AKvec _
      | AKdarray _
      | AKvarray_or_darray _
      | AKmap _
    ) ->
    Typing_arrays.fold_aktuple_as_akvec_with_acc begin fun env ty2 (r1, ty1) ->
      unify_ env r1 ty1 r2 ty2
    end env ty2 r1 fields
  | Tarraykind (AKshape fdm1), Tarraykind (AKshape fdm2) ->
    let env, fdm = Nast.ShapeMap.fold begin fun k (tk1, tv1) (env, fdm) ->
      match Nast.ShapeMap.get k fdm2 with
        | Some (tk2, tv2) ->
          let env, tk = unify env tk1 tk2 in
          let env, tv = unify env tv1 tv2 in
          env, (Nast.ShapeMap.add k (tk, tv) fdm)
        | None -> env, (Nast.ShapeMap.add k (tk1, tv1) fdm)
      end fdm1 (env, fdm2) in
    env, Tarraykind (AKshape fdm)
  (* We allow tuple-like arrays of different lengths to unify (unify the common
   * prefix, and append the remaining suffix), because the worst thing that can
   * happen if we don't do this is array get returning null. But that is true
   * for any PHP array get operation - if the key is not there, you will get
   * null even when the value type is not nullable, so we will allow it here
   * too. *)
  | Tarraykind (AKtuple fields1), Tarraykind (AKtuple fields2) ->
    let env, fields = IMap.fold begin fun k ty1 (env, fields) ->
      match IMap.get k fields2 with
        | Some ty2 ->
          let env, ty = unify env ty1 ty2 in
          env, (IMap.add k ty fields)
        | None -> env, (IMap.add k ty1 fields)
      end fields1 (env, fields2) in
    env, Tarraykind (AKtuple fields)
  | Tfun ft1, Tfun ft2 ->
      let env, ft = unify_funs env r1 ft1 r2 ft2 in
      env, Tfun ft
  | Tclass (((p1, x1) as id), argl1),
      Tclass ((p2, x2), argl2) when String.compare x1 x2 = 0 ->
        (* We handle the case where a generic A<T> is used as A *)
        let argl1 =
          if argl1 = [] && not (Env.is_strict env)
          then List.map argl2 (fun _ -> (r1, Tany))
          else argl1
        in
        let argl2 =
          if argl2 = [] && not (Env.is_strict env)
          then List.map argl1 (fun _ -> (r1, Tany))
          else argl2
        in
        if List.length argl1 <> List.length argl2
        then begin
          let n1 = soi (List.length argl1) in
          let n2 = soi (List.length argl2) in
          Errors.type_arity_mismatch p1 n1 p2 n2;
          env, Terr
        end
        else
          let env, argl = List.map2_env env argl1 argl2 unify in
          env, Tclass (id, argl)
  | Tabstract (AKnewtype (x1, argl1), tcstr1),
    Tabstract (AKnewtype (x2, argl2), tcstr2) when String.compare x1 x2 = 0 ->
        if List.length argl1 <> List.length argl2
        then begin
          let n1 = soi (List.length argl1) in
          let n2 = soi (List.length argl2) in
          let p1 = Reason.to_pos r1 in
          let p2 = Reason.to_pos r2 in
          Errors.type_arity_mismatch p1 n1 p2 n2;
          env, Terr
        end
        else
          let env, tcstr =
            match tcstr1, tcstr2 with
            | None, None -> env, None
            | Some x1, Some x2 ->
                let env, x = unify env x1 x2 in
                env, Some x
            | _ -> assert false
          in
          let env, argl = List.map2_env env argl1 argl2 unify in
          env, Tabstract (AKnewtype (x1, argl), tcstr)
  | Tabstract (AKgeneric x1, tcstr1),
    Tabstract (AKgeneric x2, tcstr2)
    when x1 = x2 && (Option.is_none tcstr1 = Option.is_none tcstr2) ->
      let env, tcstr = match Option.map2 tcstr1 tcstr2 ~f:(unify env) with
        | None -> env, None
        | Some (env, cstr) -> env, Some cstr in
      env, Tabstract (AKgeneric x1, tcstr)

  | Tabstract (ak1, tcstr1), Tabstract (ak2, tcstr2)
    when ak1 = ak2 && (Option.is_none tcstr1 = Option.is_none tcstr2) ->
      let env, tcstr = match Option.map2 tcstr1 tcstr2 ~f:(unify env) with
        | None -> env, None
        | Some (env, cstr) -> env, Some cstr in
      env, Tabstract (ak1, tcstr)

  | Tabstract (AKdependent (expr_dep, _),
      Some (_, Tclass ((_, x) as id, _) as ty)), _ ->
    let class_ = Env.get_class env x in
      (* For final class C, there is no difference between abstract<X> and X.
       * Two exceptions are for new types, because they are considered a
       * distinct type from X, and for variant classes, since we can't
       * statically guarantee their runtime type.
       *)
      (match class_ with
       | Some(class_ty)
          when TUtils.class_is_final_and_not_contravariant class_ty ->
          let env, ty = unify env ty (r2, ty2) in
          env, snd ty
      | _ ->
          (Errors.try_when
             (fun () -> TUtils.simplified_uerror env (r1, ty1) (r2, ty2))
             ~when_: begin fun () ->
               match ty2 with
               | Tclass ((_, y), _) -> y = x
               | Tany | Terr | Tmixed | Tarraykind _ | Tprim _
               | Toption _ | Tvar _ | Tabstract (_, _) | Ttuple _
               | Tanon (_, _) | Tfun _ | Tunresolved _ | Tobject
               | Tshape _ -> false
             end
             ~do_: begin fun error ->
               if expr_dep = `cls x then
                 Errors.exact_class_final id (Reason.to_pos r2) error
               else
                 Errors.this_final id (Reason.to_pos r2) error
             end
          );
          env, Terr
        )
  | _, Tabstract (AKdependent (_, _), Some (_, Tclass _)) ->
      unify_ env r2 ty2 r1 ty1

  | (Ttuple _ as ty), Tarraykind AKany
  | Tarraykind AKany, (Ttuple _ as ty) ->
      env, ty

  | Ttuple tyl1, Ttuple tyl2 ->
      let size1 = List.length tyl1 in
      let size2 = List.length tyl2 in
      if size1 <> size2
      then
        let p1 = Reason.to_pos r1 in
        let p2 = Reason.to_pos r2 in
        let n1 = soi size1 in
        let n2 = soi size2 in
        Errors.tuple_arity_mismatch p1 n1 p2 n2;
        env, Terr
      else
        let env, tyl = List.map2_env env tyl1 tyl2 unify in
        env, Ttuple tyl
  | Tmixed, Tmixed -> env, Tmixed
  | Tanon (_, id1), Tanon (_, id2) when id1 = id2 -> env, ty1
  | Tanon _, Tanon _ ->
      (* This could be smarter, but the only place where we currently compare
       * two anonymous functions is when trying to normalize intersection -
       * saying that they never unify will just keep the intersection
       * unchanged, which is always a valid option. *)
      TUtils.uerror r1 ty1 r2 ty2;
      env, Terr
  | Tfun ft, Tanon (anon_arity, id)
  | Tanon (anon_arity, id), Tfun ft ->
      (match Env.get_anonymous env id with
      | None ->
        Errors.anonymous_recursive_call (Reason.to_pos r1);
        env, Terr
      | Some anon ->
        let p1 = Reason.to_pos r1 in
        let p2 = Reason.to_pos r2 in
        if not (unify_arities ~ellipsis_is_variadic:true anon_arity ft.ft_arity)
        then Errors.fun_arity_mismatch p1 p2;
        let env, _, ret = anon env ft.ft_params in
        let env, _ = unify env ft.ft_ret ret in
        env, Tfun ft)
  | Tobject, Tobject
  | Tobject, Tclass _
  | Tclass _, Tobject -> env, Tobject
  | Tshape (fields_known1, fdm1), Tshape (fields_known2, fdm2)  ->
      (**
       * shape_field_type A and shape_field_type B are unifiable iff:
       *   1. A and B have the same optionality
       *   2. A's type and B's type are unifiable
       *)
      let on_common_field
          (env, acc)
          name
          { sft_optional = sft_optional1; sft_ty = ty1 }
          { sft_optional = sft_optional2; sft_ty = ty2 } =
        if sft_optional1 = sft_optional2
        then
          let env, sft_ty = unify env ty1 ty2 in
          let common_shape_field_type = {
            sft_optional = sft_optional1;
            sft_ty;
          } in
          env, Nast.ShapeMap.add name common_shape_field_type acc
        else
          let optional, required = if sft_optional1 then r1, r2 else r2, r1 in
          Errors.required_field_is_optional
            (Reason.to_pos optional)
            (Reason.to_pos required)
            (Env.get_shape_field_name name);
          env, acc in
      let on_missing_optional_field (env, acc) name missing_shape_field_type =
        env, Nast.ShapeMap.add name missing_shape_field_type acc in
      (* We do it both directions to verify that no field is missing *)
      let res = Nast.ShapeMap.empty in
      let env, res = TUtils.apply_shape
        ~on_common_field
        ~on_missing_optional_field
        (env, res) (r1, fields_known1, fdm1) (r2, fields_known2, fdm2) in
      let env, res = TUtils.apply_shape
        ~on_common_field
        ~on_missing_optional_field
        (env, res) (r2, fields_known2, fdm2) (r1, fields_known1, fdm1) in
        (* After doing apply_shape in both directions we can be sure that
         * fields_known1 = fields_known2 *)
      env, Tshape (fields_known1, res)
  | Tabstract (AKenum enum_name, _), Tclass ((post, class_name), tylist)
    when String.compare enum_name class_name = 0 ->
      env, Tclass ((post, class_name), tylist)
  | (Tclass ((post, class_name), tylist), Tabstract (AKenum enum_name, _))
    when String.compare enum_name class_name = 0 ->
    env, Tclass ((post, class_name), tylist)

  (* If we are trying to unify a type parameter T with another type t it's
     possible that we can get there through subtyping in both directions.
     For example we might have T as C, T super C, and we're asked
     to unify T with C. This should succeed. We don't apply this
     transitively, but assume that the type parameter environment is
     already closed under transitivity. This is ensured by
     Typing_subtype.add_constraint. *)

  | Tabstract (AKgeneric x, _), _
    when generic_param_matches ~opts env x (r2,ty2) ->
    env, ty2

  | _, Tabstract (AKgeneric x, _)
    when generic_param_matches ~opts env x (r1,ty1) ->
    env, ty1

  | (Terr | Tany | Tmixed | Tarraykind _ | Tprim _ | Toption _
      | Tvar _ | Tabstract (_, _) | Tclass (_, _) | Ttuple _ | Tanon (_, _)
      | Tfun _ | Tunresolved _ | Tobject | Tshape _), _ ->
        (* Make sure to add a dependency on any classes referenced here, even if
         * we're in an error state (i.e., where we are right now). The need for
         * this is extremely subtle. Consider this function:
         *
         * function f(): blah {
         *   // ...
         * }
         *
         * Suppose that "blah" isn't currently defined, and we send the result
         * of f() into a function that expects an int. We'll hit a unification
         * error here, as we should. But, we might later define "blah" to be a
         * type alias, "type blah = int", in another file. In that case, f()
         * needs to be rechecked with the new definition of "blah" present.
         *
         * Normally this isn't a problem. The presence of the error in f() in
         * the first place will cause it to be rechecked when "blah" pops into
         * existance anyways. (And in strict mode, or with assume_php=false, you
         * can't refer to the undefined "blah" anyways.) But there's one
         * important case where this does matter: the JS cross-compile of the
         * typechecker. The JS driver code uses the presence of dependencies to
         * figure out what code to pull into the browser, and it's pretty aggro
         * about not pulling in things it doesn't need. If this dep is missing,
         * it will never pull in "blah" -- which actually does exist, but is
         * "undefined" as far as the typechecker is concerned because the JS
         * driver hasn't pulled it into the browser *yet*. The presence of this
         * dep causes that to happen.
         *
         * Another way to do this might be to look up blah and see if it's
         * defined (and doing this will add the dep for us), and suppress the
         * error if it isn't. We typically say that undefined classes could live
         * in PHP and thus be anything -- but the only way it could unify with
         * a non-class is if it's a type alias, which isn't a PHP feature, so
         * the strictness (and subtlety) is warranted here.
         *
         * And the dep is correct anyways: if there weren't a unification error
         * like this, we'd be pulling in the declaration of "blah" (and adding
         * the dep) anyways.
         *)
        let add env = function
          | Tclass ((_, cid), _) -> Env.add_wclass env cid
          | _ -> () in
        add env ty1;
        add env ty2;
        if opts.TUtils.simplify_errors then
        TUtils.simplified_uerror env (r1, ty1) (r2, ty2)
        else
        TUtils.uerror r1 ty1 r2 ty2;
        env, Terr

(* Use unify to check if two types are the same. We use this in
 * generic_param_matches below, but we set follow_bounds=false so that we
 * don't end up recursing back through generic_param_matches from unify *)
and is_same_type ~opts env ty_sub ty_super =
  Errors.try_
    (fun () -> ignore(
        unify ~opts:{ opts with TUtils.follow_bounds = false }
          env ty_sub ty_super); true)
    (fun _ -> false)

(* This deals with the situation where we have an implied equality between
 * type parameters in the type parameter environment. We're trying to unify
 * x (a type parameter) with ty, and checking to see if we have ty as both
 * an upper and lower bound of x in the environment. We don't need to look
 * any further (e.g. consider a cycle T1 as T2 as T3 as T1) because
 * Typing_subtype.add_constraint already computes transitive closure.
 *)
and generic_param_matches ~opts env x ty =
  let lower = Env.get_lower_bounds env x in
  let upper = Env.get_upper_bounds env x in
  let mem_bounds = List.exists ~f:(fun ty' -> is_same_type ~opts env ty ty') in
  opts.TUtils.follow_bounds && mem_bounds lower && mem_bounds upper

and unify_arities ~ellipsis_is_variadic anon_arity func_arity : bool =
  match anon_arity, func_arity with
    | Fellipsis a_min, Fvariadic (f_min, _) when ellipsis_is_variadic ->
      (* we want to allow use the "..." syntax in the declaration of
       * anonymous function types to match named variadic arguments
       * of the "...$args" form as well as unnamed ones *)
      a_min = f_min
    | Fvariadic (a_min, _), Fvariadic (f_min, _)
    | Fellipsis a_min, Fellipsis f_min ->
      a_min = f_min
    | Fstandard (a_min, a_max), Fstandard (f_min, f_max) ->
      a_min = f_min && a_max = f_max
    | _, _ -> false

and unify_reason r1 r2 =
  if r1 = Reason.none then r2 else
    if r2 = Reason.none then r1 else
      let c = Reason.compare r1 r2 in
      if c <= 0 then r1
      else r2

and iunify env ty1 ty2 =
  let env, _ = unify env ty1 ty2 in
  env

(* This function is used to unify two functions *)
(* r1 is the reason for ft1 to have its type (witness) *)
and unify_funs env r1 ft1 r2 ft2 =
  let p = Reason.to_pos r2 in
  let p1 = Reason.to_pos r1 in
  if not (unify_arities ~ellipsis_is_variadic:false ft1.ft_arity ft2.ft_arity)
  then Errors.fun_arity_mismatch p p1;
  let env, var_opt, arity = match ft1.ft_arity, ft2.ft_arity with
    | Fvariadic (_, (n1, var_ty1)), Fvariadic (min, (_n2, var_ty2)) ->
      let env, var = unify env var_ty1 var_ty2 in
      env, Some (n1, var), Fvariadic (min, (n1, var))
    | ar1, _ar2 ->
      env, None, ar1
  in
  let env, params = unify_params env ft1.ft_params ft2.ft_params var_opt in
  let env, ret = unify env ft1.ft_ret ft2.ft_ret in
  env, { ft1 with
    ft_arity = arity;
    ft_params = params;
    ft_ret = ret;
  }

and unify_params env l1 l2 var1_opt =
  match l1, l2, var1_opt with
  | [], l, None -> env, l
  | [], (name2, x2) :: rl2, Some (name1, v1) ->
    let name = if name1 = name2 then name1 else None in
    let env = { env with Env.pos = Reason.to_pos (fst x2) } in
    let env, _ = unify env x2 v1 in
    let env, rl = unify_params env [] rl2 var1_opt in
    env, (name, x2) :: rl
  | l, [], _ -> env, l
  | (name1, x1) :: rl1, (name2, x2) :: rl2, _ ->
    let name = if name1 = name2 then name1 else None in
    let env = { env with Env.pos = Reason.to_pos (fst x1) } in
    let env, _ = unify env x2 x1 in
    let env, rl = unify_params env rl1 rl2 var1_opt in
    env, (name, x2) :: rl

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.unify_ref := unify
