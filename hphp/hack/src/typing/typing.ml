(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(* This module implements the typing.
 *
 * Given an Nast.program, it infers the type of all the local
 * variables, and checks that all the types are correct (aka
 * consistent) *)
open Utils
open Nast
open Typing_defs
open Silent
open Autocomplete

module TUtils       = Typing_utils
module Reason       = Typing_reason
module Inst         = Typing_instantiate
module Type         = Typing_ops
module Env          = Typing_env
module LEnv         = Typing_lenv
module Dep          = Typing_deps.Dep
module Async        = Typing_async
module DynamicYield = Typing_dynamic_yield
module SubType      = Typing_subtype
module Unify        = Typing_unify
module TGen         = Typing_generic

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

let debug = ref false

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let suggest env p ty =
  let ty = Typing_expand.fully_expand env ty in
  (match Typing_print.suggest ty with
  | "..." -> Utils.error p "Was expecting a type hint"
  | ty -> Utils.error p ("Was expecting a type hint (what about: "^ty^")")
  )

let suggest_return env p ty =
  let ty = Typing_expand.fully_expand env ty in
  (match Typing_print.suggest ty with
  | "..." -> Utils.error p "Was expecting a return type hint"
  | ty -> Utils.error p ("Was expecting a return type hint (what about: ': "^ty^"')")
  )

let any = Reason.Rnone, Tany

let compare_field_kinds x y =
  match x, y with
  | Nast.AFvalue (p1, _), Nast.AFkvalue ((p2, _), _)
  | Nast.AFkvalue ((p2, _), _), Nast.AFvalue (p1, _) ->
      error_l [p1, "You cannot use this kind of field (value)";
               p2, "Mixed with this kind of field (key => value)"]
  | _ -> ()

let check_consistent_fields x l =
  List.iter (compare_field_kinds x) l

let is_array = function _, Tarray _ -> true | _ -> false

let unbound_name env (pos, name)=
  match env.Env.genv.Env.mode with
  | Ast.Mstrict ->
      error pos ("Unbound name, Typing: "^name)
  | Ast.Mdecl | Ast.Mpartial ->
      env, (Reason.Rnone, Tany)

(*****************************************************************************)
(* Global constants typing *)
(*****************************************************************************)

let gconst_decl cst =
   let env = Typing_env.empty (Pos.filename (fst cst.cst_name)) in
   let env = Env.set_mode env cst.cst_mode in
   let env = Env.set_root env (Dep.GConst (snd cst.cst_name)) in
   let env, hint_ty =
     match cst.cst_type with
     | None -> env, (Reason.Rnone, Tany)
     | Some h -> Typing_hint.hint env h
   in
   Typing_env.GConsts.add (snd cst.cst_name) hint_ty

(*****************************************************************************)
(* Handling function/method arguments *)
(*****************************************************************************)

let rec fun_decl f =
  let env = Typing_env.empty (Pos.filename (fst f.f_name)) in
  let env = Env.set_mode env f.f_mode in
  let env = Env.set_root env (Dep.Fun (snd f.f_name)) in
  let _, ft = fun_decl_in_env env f in
  Env.add_fun (snd f.f_name) ft;
  ()

and fun_decl_in_env env f =
  let env, arity, params = make_params env true 0 f.f_params in
  let env, ret_ty = match f.f_ret with
    (* If there is no return type annotation, we clearly should make it Tany
     * but also want a witness so that we can point *somewhere* in event of
     * error. The function name itself isn't great, but is better than
     * nothing. *)
    | None -> env, (Reason.Rwitness (fst f.f_name), Tany)
    | Some ty -> Typing_hint.hint env ty in
  let arity_max =
    if f.f_ddd then 1000 else
    List.length f.f_params
  in
  let env, tparams = lfold type_param env f.f_tparams in
  let ft = {
    ft_pos = fst f.f_name;
    ft_unsafe = false;
    ft_abstract = false;
    ft_arity_min = arity;
    ft_arity_max = arity_max;
    ft_tparams = tparams;
    ft_params = params;
    ft_ret = ret_ty;
  } in
  env, ft

and type_param env (x, y) =
  let env, y = opt Typing_hint.hint env y in
  env, (x, y)

and check_default p mandatory e =
  if not mandatory && e = None
  then error p
      ("A previous parameter has a default value.\n"^
       "Remove all the default values for the preceding parameters,\n"^
       "or add a default value to this one.")
  else ()

(* Functions building the types for the parameters of a function *)
(* It's not completely trivial because of optional arguments  *)
and make_param env mandatory arity param =
  let env, ty = make_param_type (fun() -> any) env param in
  if Env.is_decl env || !is_silent_mode then () else begin
    check_default (fst param.param_id) mandatory param.param_expr;
  end;
  let mandatory = mandatory && param.param_expr = None in
  let arity = if mandatory then arity + 1 else arity in
  env, arity, mandatory, ty

and make_params env mandatory arity paraml =
  match paraml with
  | [] -> env, arity, []
  | param :: rl ->
      let env, arity, mandatory, ty = make_param env mandatory arity param in
      let env, arity, rest = make_params env mandatory arity rl in
      env, arity, ty :: rest

(* This function is used to determine the type of an argument.
 * When we want to type-check the body of a function, we need to
 * introduce the type of the arguments of the function in the environment
 * Let's take an example, we want to check the code of foo:
 *
 * function foo(int $x): int {
 *   // CALL TO make_arg_type on (int $x)
 *   // Now we know that the type of $x is int
 *
 *   return $x; // in the environment $x is an int, the code is correct
 * }
 *)
and make_param_type default env param =
  let env, ty =
    match param.param_hint with
      (* if the type is missing, use the default one (an unbound type variable) *)
    | None ->
        let default_type = default() in
        let r = Reason.Rwitness (fst param.param_id) in
        env, (r, snd default_type)
    | Some (p, (Hprim Tvoid)) -> error p "Cannot have a void parameter"
          (* if the code is strict, use the type-hint *)
    | Some x when Env.is_strict env  -> Typing_hint.hint env x
          (* This code is there because we use to be more tolerant in partial-mode
           * we use to allow (A $x = null) as an argument instead of (?A $x = null)
           * for the transition, we give this error message, that explains what's
           * going on, that dispite the the (= null) users are now required to
           * use the optional type (write ?A instead of A).
           *)
    | Some (_, (Hoption _ | Hmixed) as x) -> Typing_hint.hint env x
    | Some x ->
        match (param.param_expr) with
        | Some (null_pos, Null) when !is_silent_mode ->
            Typing_suggest.save_qm (fst x);
            let env, ty = Typing_hint.hint env x in
            env, (Reason.Rwitness null_pos, Toption ty)
        | Some (_, Null) when not (Env.is_decl env) ->
            error (fst x) "Please add a ?, this argument can be null"
        | Some _ | None -> Typing_hint.hint env x
  in
  let ty =
    match ty with
    | r, Tarray (_, x1, x2) ->
        (* if an array is passed by reference, we don't want to trigger
         * the copy on write
         *)
        r, Tarray (param.param_is_reference, x1, x2)
    | x -> x
  in
  TUtils.save_infer env (fst param.param_id) ty;
  env, (Some param.param_name, ty)

(* In strict mode, we force you to give a type declaration on a parameter *)
(* But the type checker is nice: it makes a suggestion :-) *)
and check_param env param (_, ty) =
  match (param.param_hint) with
  | None -> suggest env (fst param.param_id) ty
  | Some _ -> ()

and bind_param env (_, ty1) param =
  let env, ty2 = opt expr env param.param_expr in
  let ty2 =
    match ty2 with
    | None    -> Reason.none, Tany
    | Some ty -> ty
  in
  Typing_suggest.save_param (param.param_name) env ty1 ty2;
  let env = Type.sub_type (fst param.param_id) Reason.URhint env ty1 ty2 in
  Env.set_local env (snd param.param_id) ty1

(*****************************************************************************)
(* Now we are actually checking stuff! *)
(*****************************************************************************)
and fun_def env _ f =
  try fun_def_ env f with Ignore -> ()

and fun_def_ env f =
  if f.f_mode = Ast.Mdecl then () else begin
    NastCheck.fun_ env f;
    (* Fresh type environment is actually unnecessary, but I prefer to have
     * a garantee that we are using a clean typing enviroment.
     *)
    Env.fresh_tenv env (
    fun env_up ->
      let env = { env_up with Env.lenv = Env.empty_local } in
      let env = Env.set_mode env f.f_mode in
      let env = Env.set_root env (Dep.Fun (snd f.f_name)) in
      let env, hret =
        match f.f_ret with
        | None -> env, (Reason.Rwitness (fst f.f_name), Tany)
        | Some ret -> Typing_hint.hint env ret in
      let env, params = lfold (make_param_type Env.fresh_type) env f.f_params in
      let env = List.fold_left2 bind_param env params f.f_params in
      let env = fun_ env f.f_unsafe (f.f_ret <> None) hret (fst f.f_name) f.f_body f.f_type in
      let env = solve_todos env in
      if Env.is_strict env then begin
        List.iter2 (check_param env) f.f_params params;
        match f.f_ret with
        | None -> suggest_return env (fst f.f_name) hret
        | Some _ -> ()
      end
   )
  end

and solve_todos env =
  List.fold_left (fun env f -> f env) env (Env.get_todo env)


(*****************************************************************************)
(* function used to type closures, functions and methods *)
(*****************************************************************************)
and fun_ ?(abstract=false) env unsafe has_ret hret pos b fun_type =
  Env.with_return env begin fun env ->
    let env = Env.set_return env hret in
    let env = Env.set_fn_type env fun_type in
    let env = block env b in
    let ret = Env.get_return env in
    if Nast_terminality.Terminal.block b || abstract || unsafe || !auto_complete
    then env
    else fun_implicit_return env pos ret b fun_type
  end

and fun_implicit_return env pos ret b fun_type =
  (* an implicit return means the return value can be null *)
  if Env.has_yield env then env
  else if fun_type = Ast.FSync
  then implicit_return_noasync pos env ret
  else implicit_return_async b pos env ret (Reason.Rno_return_async pos)

(* A function without a terminal block has an implicit return null *)
and implicit_return_noasync p env ret =
  let rty = Reason.Rno_return p, Tprim Nast.Tvoid in
  Typing_suggest.save_return env ret rty;
  Type.sub_type p Reason.URreturn env ret rty

(* An async function without a yield result() has an implicit
 * yield result(null) if it is a generator or an implicit
 * return null if it is an async function
 *)
and implicit_return_async b p env ret reason =
  let type_var = Env.fresh_type () in
  let rty_core = Toption type_var in
  let suggest_core =
    (* OK, this is pretty obnoxious. In principle, a function with no call to
     * yield result() can be either Awaitable<void> or Awaitable<?anything> due
     * to the special case around yield result(null). In practice, you almost
     * certainly meant Awaitable<void> and we should suggest that. IMO we
     * should extend this HasYieldResult check beyond just type suggestions,
     * or come up with something better than yield result(null) for
     * Awaitable<void>, but the former is a ton of cleanup in www and I have
     * no ideas on the latter right now, so let's just use this heuristic.
     *
     * These semantics also unfortunately got extended to async functions --
     * normally an omitted return will mean the function has a return type of
     * void, but for async functions we implicitly assume return null.
     *
     * The check for is_suggest_mode is superflouous (save_return will do it)
     * and is just to avoid the potentially expensive HasYieldResult call if
     * we know we won't ever care. *)
    if !is_suggest_mode && NastVisitor.HasReturn.block b
    then rty_core
    else Tprim Nast.Tvoid in
  let mk_rty core = reason, Tapply ((p, "Awaitable"), [reason, core]) in
  Typing_suggest.save_return env ret (mk_rty suggest_core);
  Type.sub_type p Reason.URreturn env ret (mk_rty rty_core)

and block env stl =
  List.fold_left stmt env stl

and stmt env = function
  | Fallthrough
  | Noop ->
      env
  | Expr e ->
      let env, ty = expr env e in
      (* NB: this check does belong here and not in expr, even though it only
       * applies to expressions -- we actually want to perform the check on
       * statements that are expressions, e.g., "foo();" we want to check, but
       * "return foo();" we do not even though the expression "foo()" is a
       * subexpression of the statement "return foo();". *)
       Typing_async.enforce_not_awaitable env e ty;
      env
  | If (e, b1, b2)  ->
      let env, ty = expr env e in
      Typing_async.enforce_not_awaitable env e ty;
      let parent_lenv = env.Env.lenv in
      let env  = condition env true e in
      let env  = block env b1 in
      let lenv1 = env.Env.lenv in
      let env = { env with Env.lenv = parent_lenv } in
      let env  = condition env false e in
      let env  = block env b2 in
      let lenv2 = env.Env.lenv in
      let terminal1 = Nast_terminality.Terminal.block b1 in
      let terminal2 = Nast_terminality.Terminal.block b2 in
      if terminal1 && terminal2
      then
        let env = LEnv.integrate env parent_lenv lenv1 in
        let env = LEnv.integrate env env.Env.lenv lenv2 in
        LEnv.integrate env env.Env.lenv parent_lenv
      else if terminal1
      then begin
        let env = LEnv.integrate env parent_lenv lenv1 in
        LEnv.integrate env env.Env.lenv lenv2
      end
      else if terminal2
      then begin
        let env = LEnv.integrate env parent_lenv lenv2 in
        LEnv.integrate env env.Env.lenv lenv1
      end
      else LEnv.intersect env parent_lenv lenv1 lenv2
  | Return (p, None) ->
      let rty = match Env.get_fn_type env with
        | Ast.FSync -> (Reason.Rwitness p, Tprim Tvoid)
        | Ast.FAsync -> (Reason.Rwitness p, Tapply ((p, "Awaitable"), [(Reason.Rwitness p, Toption (Env.fresh_type ()))])) in
      let expected_return = Env.get_return env in
      Typing_suggest.save_return env expected_return rty;
      let env = Type.sub_type p Reason.URreturn env expected_return rty in
      env
  | Return (p, Some e) ->
      let pos = fst e in
      let env, rty = expr env e in
      let rty = match Env.get_fn_type env with
        | Ast.FSync -> rty
        | Ast.FAsync -> (Reason.Rwitness p), Tapply ((p, "Awaitable"), [rty]) in
      let expected_return = Env.get_return env in
      (match snd (Env.expand_type env expected_return) with
      | _, Tunresolved _ ->
          (* we allow return types to grow for anonymous functions *)
          let env, rty = TUtils.unresolved env rty in
          let env, _ = Type.unify pos Reason.URreturn env expected_return rty in
          env
      | _ ->
          Typing_suggest.save_return env expected_return rty;
          let env = Type.sub_type pos Reason.URreturn env expected_return rty in
          env
      )
  | Do (b, e) as st ->
      (* NOTE: leaks scope as currently implemented; this matches
         the behavior in naming (cf. `do_stmt` in naming/naming.ml).
       *)
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let env = block env b in
      let (env, _) = expr env e in
      let after_block = env.Env.lenv in
      let alias_depth = Typing_alias.get_depth st in
      let env = iter_n_acc alias_depth begin fun env ->
        let env = condition env true e in
        let env = block env b in
        env
      end env in
      let env =
        if NastVisitor.HasContinue.block b
        then LEnv.fully_integrate env parent_lenv
        else
          let env = LEnv.integrate env parent_lenv env.Env.lenv in
          let env = { env with Env.lenv = after_block } in
          env
      in
      condition env false e
  | While (e, b) as st ->
      let (env, _) = expr env e in
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let alias_depth = Typing_alias.get_depth st in
      let env = iter_n_acc alias_depth begin fun env ->
        let env = condition env true e in
        let env = block env b in
        env
      end env in
      let env = LEnv.fully_integrate env parent_lenv in
      condition env false e
  | For (e1, e2, e3, b) as st ->
      (* For loops leak their initalizer, but nothing that's defined in the
         body
       *)
      let (env, _) = expr env e1 in      (* initializer *)
      let (env, _) = expr env e2 in
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let alias_depth = Typing_alias.get_depth st in
      let env = iter_n_acc alias_depth begin fun env ->
        let env = condition env true e2 in (* iteration 0 *)
        let env = block env b in
        let (env, _) = expr env e3 in
        env
      end env in
      let env = LEnv.fully_integrate env parent_lenv in
      condition env false e2
  | Switch (e, cl) ->
      Nast_terminality.SafeCase.check (fst e) cl;
      let env, ty = expr env e in
      let parent_lenv = env.Env.lenv in
      let env, cl = case_list parent_lenv ty env cl in
      let terml, lenvl = List.split cl in
      LEnv.intersect_list env parent_lenv lenvl terml
  | Foreach (e1, e2, b) as st ->
      let env, ty1 = expr env e1 in
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, ety1 = Env.expand_type env ty1 in
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let env, ty2 = as_expr env (fst e1) e2 in
      let env = Type.sub_type (fst e1) Reason.URforeach env ty2 ty1 in
      let alias_depth = Typing_alias.get_depth st in
      let env = iter_n_acc alias_depth begin fun env ->
        let env = bind_as_expr env ty2 e2 in
        let env = block env b in
        env
      end env in
      let env = LEnv.fully_integrate env parent_lenv in
      env
  | Try (tb, cl, fb) ->
    let env = try_catch (tb, cl) env in
    let env = block env fb in
    env
  | Static_var el ->
    let env = List.fold_left begin fun env e ->
      match e with
        | _, Binop (Ast.Eq _, (_, Lvar (p, x)), _) ->
          Env.add_todo env (TGen.no_generic p x)
        | _ -> env
    end env el in
    let env, _ = lfold expr env el in
    env
  | Throw (_, e) ->
    let p = fst e in
    let env, ty = expr env e in
    let exn_ty = Reason.Rthrow p, Tapply ((p, "Exception"), []) in
    Type.sub_type p (Reason.URthrow) env exn_ty ty
  | Continue
  | Break -> env

and case_list parent_lenv ty env cl =
  let env = { env with Env.lenv = parent_lenv } in
  case_list_ parent_lenv ty env cl

and try_catch (tb, cl) env =
  let parent_lenv = env.Env.lenv in
  let env = Env.freeze_local_env env in
  let env = block env tb in
  let after_try = env.Env.lenv in
  let env, catchl = lfold (catch parent_lenv after_try) env cl in
  let terml = List.map (fun (_, _, b) -> Nast_terminality.Terminal.block b) cl in
  let lenvl = after_try :: catchl in
  let terml = Nast_terminality.Terminal.block tb :: terml in
  LEnv.intersect_list env parent_lenv lenvl terml

and case_list_ parent_lenv ty env = function
  | [] -> env, []
  | Default b :: _ ->
      (* TODO this is wrong, should continue on to the other cases, but it
       * doesn't matter in practice since our parser won't parse default
       * anywhere but in the last position :) Should fix all of this as well
       * as totality detection for switch. *)
    let env = block env b in
    env, [Nast_terminality.Terminal.case (Default b), env.Env.lenv]
  | Case (e, b) :: rl ->
    (* TODO - we should consider handling the comparisons the same
     * way as Binop Ast.EqEq, since case statements work using ==
     * comparison rules *)

    (* The way we handle terminal/nonterminal here is not quite right, you
     * can still break the type system with things like P3131824. *)
    let ty_num = (Reason.Rnone, Tprim Nast.Tnum) in
    if Nast_terminality.Terminal.block b then
      let env, ty2 = expr env e in
      let env, _ =
        if (SubType.is_sub_type env ty_num ty) &&
          (SubType.is_sub_type env ty_num ty2)
        then env, ty
        else Type.unify (fst e) Reason.URnone env ty ty2 in
      let env = block env b in
      let lenv = env.Env.lenv in
      let env, rl = case_list parent_lenv ty env rl in
      env, (Nast_terminality.Terminal.case (Case (e, b)), lenv) :: rl
    else
      let env, ty2 = expr env e in
      let env, _ =
        if (SubType.is_sub_type env ty_num ty) &&
          (SubType.is_sub_type env ty_num ty2)
        then env, ty
        else Type.unify (fst e) Reason.URnone env ty ty2 in
      let env = block env b in
      case_list_ parent_lenv ty env rl

and catch parent_lenv after_try env (ety, exn, b) =
  let env = { env with Env.lenv = after_try } in
  let env = LEnv.fully_integrate env parent_lenv in
  let env, ety = Typing_hint.hint env (fst ety, Happly (ety, [])) in
  let env = Env.set_local env (snd exn) ety in
  let env = block env b in
  (* Only keep the local bindings if this catch is non-terminal *)
  env, env.Env.lenv

and as_expr env pe  = function
    (* note that we don't call as_expr for arrays, so the only time
       this happens should be for vectors *)
  | As_id e ->
      let ty = Env.fresh_type() in
      let tvector = Tapply ((pe, "Traversable"), [ty]) in
      env, (Reason.Rforeach pe, tvector)
  | As_kv (e1, e2) ->
      let ty1 = Env.fresh_type() in
      let ty2 = Env.fresh_type() in
      let tmap = Tapply ((pe, "KeyedTraversable"), [ty1; ty2]) in
      env, (Reason.Rforeach pe, tmap)

and bind_as_expr env ty aexpr =
  let env, ety = Env.expand_type env ty in
  match ety with
  | _, Tapply ((p, class_id), [ty2]) ->
      (match aexpr with
      | As_id (_, Lvar (_, x)) ->
          Env.set_local env x ty2
      | As_kv ((_, Lvar (_, x)), (_, Lvar (_, y))) ->
          let env = Env.set_local env x (Reason.Rnone, Tmixed) in
          Env.set_local env y ty2
      | _ -> (* TODO Probably impossible, should check that *)
          env
      )
  | _, Tapply ((p, class_id), [ty1; ty2]) ->
      (match aexpr with
      | As_id (_, Lvar (_, x)) ->
          Env.set_local env x ty2
      | As_kv ((_, Lvar (_, x)), (_, Lvar (_, y))) ->
          let env = Env.set_local env x ty1 in
          Env.set_local env y ty2
      | _ -> (* TODO Probably impossible, should check that *)
          env
      )
  | _ -> assert false

and expr env e =
  let env, ty = expr_ false env e in
  if !accumulate_types
  then begin
    type_acc := (fst e, Typing_expand.fully_expand env ty) :: !type_acc;
  end;
  TUtils.save_infer env (fst e) ty;
  env, ty

and lvalue env e =
  expr_ true env e

and expr_ is_lvalue env (p, e) =
  match e with
  | Array [] -> env, (Reason.Rwitness p, Tarray (true, None, None))
  | Array (x :: rl as l) ->
      if not !is_silent_mode
      then check_consistent_fields x rl;
      let env, value = TUtils.in_var env (Reason.Rnone, Tunresolved []) in
      let env, values = lfold field_value env l in
      let has_unknown = List.exists (fun (_, ty) -> ty = Tany) values in
      let env, values = lfold TUtils.unresolved env values in
      let unify_value = Type.unify p Reason.URarray_value in
      let env, value =
        if has_unknown (* If one of the values comes from PHP land,
                        * we have to be conservative and consider that
                        * we don't know what the type of the values are.
                        *)
        then env, (Reason.Rnone, Tany)
        else fold_left_env unify_value env value values
      in
      (match x with
      | Nast.AFvalue _ ->
          env, (Reason.Rwitness p, Tarray (true, Some value, None))
      | Nast.AFkvalue _ ->
          let env, key = TUtils.in_var env (Reason.Rnone, Tunresolved []) in
          let env, keys = lfold field_key env l in
          let env, keys = lfold TUtils.unresolved env keys in
          let unify_key = Type.unify p Reason.URarray_key in
          let env, key = fold_left_env unify_key env key keys in
          env, (Reason.Rwitness p, Tarray (true, Some key, Some value))
      )
  | ValCollection (name, el) ->
      let env, x = TUtils.in_var env (Reason.Rnone, Tunresolved []) in
      let env, tyl = lmap expr env el in
      let env, tyl = lfold TUtils.unresolved env tyl in
      let env, v = fold_left_env (Type.unify p Reason.URvector) env x tyl in
      let tvector = Tapply ((p, name), [v]) in
      let ty = Reason.Rwitness p, tvector in
      env, ty
  | KeyValCollection (name, l) ->
      let kl, vl = List.split l in
      let env, kl = lfold expr env kl in
      let env, vl = lfold expr env vl in
      let env, k = TUtils.in_var env (Reason.Rnone, Tunresolved []) in
      let env, v = TUtils.in_var env (Reason.Rnone, Tunresolved []) in
      let env, kl = lfold TUtils.unresolved env kl in
      let env, k = fold_left_env (Type.unify p Reason.URkey) env k kl in
      let env, vl = lfold TUtils.unresolved env vl in
      let env, v = fold_left_env (Type.unify p Reason.URvalue) env v vl in
      let ty = Tapply ((p, name), [k; v])
      in
      env, (Reason.Rwitness p, ty)
  | Clone e -> expr env e
  | This when Env.is_static env && not !is_silent_mode ->
      error p "Don't use $this in a static method"
  | This ->
      let r, _ = Env.get_self env in
      if r = Reason.Rnone
      then error p "Can't use $this outside of a class";
      let env, (_, ty) = Env.get_local env this in
      let r = Reason.Rwitness p in
      let ty = (r, ty) in
      let ty = r, Tgeneric ("this", Some ty) in
      env, ty
  | Assert (AE_assert e) ->
      let env = condition env true e in
      env, (Reason.Rwitness p, Tprim Tvoid)
  | Assert (AE_invariant_violation (e, el)) ->
      let env, _ = lfold expr env el in
      let env, ty = expr env e in
      let string = Reason.Rwitness (fst e), Tprim Tstring in
      let env, k = Type.unify (fst e) Reason.URparam env ty string in
      env, (Reason.Rwitness p, Tprim Tvoid)
  | Assert (AE_invariant (e1, e2, el)) ->
      let env, _ = lfold expr env el in
      let env, ty = expr env e1 in
      let env = condition env true e1 in
      let env, ty2 = expr env e2 in
      let string = Reason.Rwitness (fst e2), Tprim Tstring in
      let env, k = Type.unify (fst e2) Reason.URparam env ty2 string in
      env, ty
  | True
  | False ->
      env, (Reason.Rwitness p, Tprim Tbool)
  | Int _ ->
      env, (Reason.Rwitness p, Tprim Tint)
  | Float _ ->
      env, (Reason.Rwitness p, Tprim Tfloat)
  | Null ->
      let ty = Env.fresh_type() in
      env, (Reason.Rwitness p, Toption ty)
  | String _ ->
      env, (Reason.Rwitness p, Tprim Tstring)
  | String2 (idl, _) ->
      let env = string2 env idl in
      env, (Reason.Rwitness p, Tprim Tstring)
  | Fun_id x ->
      auto_complete_id x;
      fun_type_of_id env x
  | Id (cst_pos, cst_name) ->
      auto_complete_id (cst_pos, cst_name);
      (match Env.get_gconst env cst_name with
      | None when Env.is_strict env ->
          error cst_pos "Unbound global constant (Typing)"
      | None ->
          env, (Reason.Rnone, Tany)
      | Some ty ->
          env, ty
      )
  | Method_id (instance, meth) ->
    (* Method_id is used when creating a "method pointer" using the magic
     * inst_meth function.
     *
     * Typing this is pretty simple, we just need to check that instance->meth is
     * public+not static and then return its type.
     *)
    let env, ty1 = expr env instance in
    let env, result, vis = obj_get_with_visibility true env ty1 meth (fun x -> x) in
    let has_lost_info = Env.FakeMembers.is_invalid env instance (snd meth) in
    if has_lost_info
    then
      let name = "the method "^snd meth in
      let env, result = Env.lost_info name ISet.empty env result in
      env, result
    else (match vis with
    | Some (method_pos, Vprivate _) ->
      error_l [method_pos, "This is a private method";
               p, "you cannot use it with inst_meth (whether you are in the same class or not)."]
    | Some (method_pos, Vprotected _) ->
      error_l [method_pos, "This is a protected method";
               p, "you cannot use it with inst_meth (whether you are in the same class hierarchy or not)."]
    | _ -> env, result
    )
  | Method_caller ((pos, class_name) as pos_cname, meth_name) ->
    (* meth_caller('X', 'foo') desugars to:
     * $x ==> $x->foo()
     *)
    let env, class_ = Env.get_class env class_name in
    (match class_ with
    | None -> unbound_name env pos_cname
    | Some class_ ->
        let env, params = lfold begin fun env x ->
          TUtils.in_var env (Reason.Rwitness p, Tunresolved [])
        end env class_.tc_tparams in
        let obj_type = Reason.Rwitness p, Tapply (pos_cname, params) in
        (* We need to instantiate the object because it could have
         * type parameters.
         *)
        let env, fty = obj_get true env obj_type meth_name (fun x -> x) in
        (match fty with
        | reason, Tfun fty ->
            (* We are creating a fake closure:
             * function<T as Class>(T $x): return_type_of(Class:meth_name)
             *)
            let tparam = pos_cname, Some obj_type in
            let param = Tgeneric (class_name, Some obj_type) in
            let param = Reason.Rwitness pos, param in
            let fty = { fty with
                        ft_tparams = [tparam];
                        ft_params = [None, param] } in
            let env, fty = Inst.instantiate_ft env fty in
            let caller = {
              ft_pos = pos;
              ft_unsafe = false;
              ft_abstract = false;
              ft_arity_min = 1;
              ft_arity_max = 1;
              ft_tparams = [];
              ft_params = fty.ft_params;
              ft_ret = fty.ft_ret;
            } in
            env, (reason, Tfun caller)
        | x ->
            (* This can happen if the method lives in PHP *)
            env, (Reason.Rwitness pos, Tany)
        )
    )
  | Smethod_id (c, meth) ->
    (* Smethod_id is used when creating a "method pointer" using the magic
     * class_meth function.
     *
     * Typing this is pretty simple, we just need to check that c::meth is public+static
     * and then return its type.
     *)
    let env, class_ = Env.get_class env (snd c) in
    (match class_ with
    | None ->
      (* The class given as a static string was not found. *)
      unbound_name env c
    | Some class_ ->
      let env, smethod = Env.get_static_member true env class_ (snd meth) in
      (match smethod with
      | None -> (* The static method wasn't found. *)
        smember_not_found p ~is_const:false ~is_method:true env class_ (snd meth)
      | Some smethod ->
        (match smethod.ce_type, smethod.ce_visibility with
        | (r, (Tfun _ as ty)), Vpublic ->
          env, (r, ty)
        | (r, Tfun _), Vprivate _ ->
          error_l [Reason.to_pos r, "This is a private method";
                   p, "you cannot use it with class_meth (whether you are in the same class or not)."]
        | (r, Tfun _), Vprotected _ ->
          error_l [Reason.to_pos r, "This is a protected method";
                   p, "you cannot use it with class_meth (whether you are in the same class hierarchy or not)."]
        | _, _ ->
          (* If this assert fails, we have a method which isn't callable. *)
          assert false
        )
      )
    )
  | Lvar (_, x) ->
      let env, x = Env.get_local env x in
      env, x
  | List el ->
      let env, tyl = lmap expr env el in
      let ty = Reason.Rwitness p, Ttuple tyl in
      env, ty
  | Pair (e1, e2) ->
      let env, ty1 = expr env e1 in
      let env, ty2 = expr env e2 in
      let ty = Reason.Rwitness p, Tapply ((p, "Pair"), [ty1; ty2]) in
      env, ty
  | Expr_list el ->
      let env, tyl = lmap expr env el in
      let ty = Reason.Rwitness p, Ttuple tyl in
      env, ty
  | Array_get (e, None) ->
      let env, ty1 = expr env e in
      array_append is_lvalue p env ty1
  | Array_get (e1, Some e2) ->
      let env, ty1 = expr env e1 in
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, ety1 = Env.expand_type env ty1 in
      let env, ty2 = expr env e2 in
      array_get is_lvalue p env ty1 ety1 e2 ty2
  | Call (Cnormal, (_, Id (_, "copy")), [x]) ->
      let env, ty = expr env x in
      let env, ety = Env.expand_type env ty in
      (match ety with
      | r, Tarray (_, x1, x2) ->
          (* We consider the array as local now *)
          TUtils.in_var env (r, Tarray (true, x1, x2))
      | r, ety ->
          TUtils.in_var env (r, ety)
      )
  | Call (Cnormal, (_, Id (_, "hh_show")), [x]) when !debug ->
      let env, ty = expr env x in
      Env.debug env ty;
      env, Env.fresh_type()
  | Call (call_type, (_, fun_expr as e), el) ->
      let env, result = dispatch_call p env call_type e el in
      let env = Env.forget_members env p in
      env, result
  | Binop (Ast.Eq (Some op), e1, e2) ->
      let e2 = p, Binop (op, e1, e2) in
      let env, ty = expr env (p, Binop (Ast.Eq None, e1, e2)) in
      env, ty
  | Binop (Ast.Eq None, e1, e2) ->
      let env, ty2 = expr env e2 in
      assign p env e1 ty2
  | Binop ((Ast.AMpamp | Ast.BArbar as c), e1, e2) ->
      let c = c = Ast.AMpamp in
      let lenv = env.Env.lenv in
      let env = condition env c e1 in
      let env, ty2 = expr env e2 in
      let env = { env with Env.lenv = lenv } in
      env, (Reason.Rlogic_ret p, Tprim Tbool)
  | Binop (bop, e1, e2) ->
      let env, ty1 = expr env e1 in
      let env, ty2 = expr env e2 in
      let env, ty = binop p env bop (fst e1) ty1 (fst e2) ty2 in
      env, ty
  | Unop (uop, e) ->
      let env, ty = expr env e in
      unop p env uop ty
  | Eif (c, e1, e2) ->
      let env, tyc = expr env c in
      Typing_async.enforce_not_awaitable env c tyc;
      let lenv = env.Env.lenv in
      let env  = condition env true c in
      let env, ty1 = match e1 with
      | None ->
          non_null env tyc
      | Some e1 ->
          expr env e1
      in
      let env  = { env with Env.lenv = lenv } in
      let env  = condition env false c in
      let env, ty2 = expr env e2 in
      Unify.unify_nofail env ty1 ty2
  | Class_const (CIparent, mid) ->
      let env, cty = static_class_id p env CIparent in
      obj_get false env cty mid (fun x -> x)
  | Class_const (cid, mid) ->
      Typing_utils.process_static_find_ref cid mid;
      let env, cty = static_class_id p env cid in
      let env, cty = Env.expand_type env cty in
      class_get ~is_method:false ~is_const:true env cty mid cid
  | Class_get (x, (_, y))
      when Env.FakeMembers.get_static env x y <> None ->
        let env, local = Env.FakeMembers.make_static p env x y in
        let local = p, Lvar (p, local) in
        expr env local
  | Class_get (cid, mid) ->
      Typing_utils.process_static_find_ref cid mid;
      let env, cty = static_class_id p env cid in
      let env, cty = Env.expand_type env cty in
      let env, ty = class_get ~is_method:false ~is_const:false env cty mid cid in
      if Env.FakeMembers.is_static_invalid env cid (snd mid)
      then
        let fake_name = Env.FakeMembers.make_static_id cid (snd mid) in
        let env, ty = Env.lost_info fake_name ISet.empty env ty in
        env, ty
      else env, ty
  | Obj_get (e, (_, Id (_, y)))
      when Env.FakeMembers.get env e y <> None ->
        let env, local = Env.FakeMembers.make p env e y in
        let local = p, Lvar (p, local) in
        expr env local
  | Obj_get (e1, (_, Id m)) ->
      let env, ty1 = expr env e1 in
      let env, result = obj_get false env ty1 m (fun x -> x) in
      let has_lost_info = Env.FakeMembers.is_invalid env e1 (snd m) in
      if has_lost_info
      then
        let name = "the member "^snd m in
        let env, result = Env.lost_info name ISet.empty env result in
        env, result
      else env, result
  | Obj_get (e1, _) ->
      let env, _ = expr env e1 in
      env, (Reason.Rwitness p, Tany)
  | Yield_break ->
      env, (Reason.Rwitness p, Tany)
  | Yield e ->
      let env = Env.set_has_yield env in
      let r = Reason.Rwitness p in
      let env, rty = expr env e in
      (* If we are yielding WaitHandles, then we don't need to check the return
       * type, since result() does that. Furthermore, we know what type the
       * preparer will send to this continuation based on the type variable of
       * the WaitHandle
       *)
      let env, erty = Env.expand_type env rty in
      (match erty with
      | r, Tapply ((p, "_AsyncWaitHandle"), [rty]) ->
          env, rty
      | _ ->
          let rty = r, Tapply ((p, "Continuation"), [rty]) in
          let env = Type.sub_type (fst e) (Reason.URyield) env (Env.get_return env) rty in
          let env = Env.forget_members env p in
          (* the return type of yield could be anything, it depends on the value
           * sent to the continuation.
           *)
          env, (r, Tany)
      )
  | Await e ->
      let env, rty = expr env e in
      Async.overload_extract_from_awaitable env p rty
  | Special_func func -> special_func env p func
  | New (c, el) ->
      Typing_utils.process_static_find_ref c (p, "__construct");
      let check_not_abstract = true in
      let env, ty = new_object ~check_not_abstract p env c el in
      let env = Env.forget_members env p in
      env, ty
  | Cast ((_, Harray (None, None)), _) when Env.is_strict env ->
      error p "(array) cast forbidden in strict mode; arrays with unspecified \
      key and value types are not allowed"
  | Cast (ty, e) ->
      let env, _ = expr env e in
      Typing_hint.hint env ty
  | InstanceOf (e1, _) ->
      let env, _ = expr env e1 in
      env, (Reason.Rwitness p, Tprim Tbool)
  | Efun (f, idl) ->
      NastCheck.fun_ env f;
      let env, ft = fun_decl_in_env env f in
      (* check for recursive function calls *)
      let anon = anon_make env.Env.lenv p f in
      let env, anon_id = Env.add_anonymous env anon in
      if Env.is_strict env then () else ignore (anon env ft.ft_params);
      env, (Reason.Rwitness p, Tanon (ft.ft_arity_min, ft.ft_arity_max, anon_id))
  | Xml (sid, attrl, el) ->
      let env, obj = expr env (fst sid, New (CI sid, [])) in
      let env, attr_tyl = lfold expr env (List.map snd attrl) in
      let env, body = lfold expr env el in
      (* We don't perform any check on XHP body right now, because
       * it is unclear, what is allowed ...
       * I keep the code here, because one day, we might want to be
       * more restrictive.
       *)
      (*
        let env, _ = lfold2 begin fun env e ty ->
        let p = fst e in
        let r = Reason.Rxhp p in
        let xhp_ty = (r, (Toption (r, Tapply ((p, "XHP"), [])))) in
        let env = Type.sub_type p Reason.URxhp env xhp_ty ty in
        env, ()
        end env el body in
       *)
      env, obj
  | Shape fdm ->
      let env, fdm = smap_env expr env fdm in
      (* allow_inter adds a type-variable *)
      let env, fdm = smap_env TUtils.unresolved env fdm in
      env, (Reason.Rwitness p, Tshape fdm)

(*****************************************************************************)
(* Anonymous functions. *)
(*****************************************************************************)

and anon_bind_param params env (param_name, ty as pname_ty) =
  match !params with
  | [] ->
      (* This code cannot be executed normally, because the arity is wrong
       * and it should have been caught earlier. But in silent-mode, we
       * tolerate it, we bind as many parameters as we can and carry on.
       *)
      assert !is_silent_mode;
      env
  | param :: paraml ->
      params := paraml;
      match param.param_hint with
      | Some h ->
          let env, h = Typing_hint.hint env h in
          let pos = Reason.to_pos (fst ty) in
          let env = Type.sub_type pos Reason.URparam env h ty in
          (* Closures are allowed to have explicit type-hints. When
           * that is the case we should check that the argument passed
           * is compatible with the type-hint.
           * The body of the function should be type-checked with the
           * hint and not the type of the argument passed.
           * Otherwise it leads to strange results where
           * foo(?string $x = null) is called with a string and fails to
           * type-check. If $x is a string instead of ?string, null is not
           * subtype of string ...
           *)
          bind_param env (param_name, h) param
      | _ -> bind_param env pname_ty param

and anon_bind_opt_param env param =
  match param.param_expr with
  | None ->
      assert !is_silent_mode;
      let ty = Reason.Rnone, Tany in
      bind_param env (None, ty) param
  | Some default ->
      let env, ty = expr env default in
      bind_param env (None, ty) param

and anon_check_param env param =
  match param.param_hint with
  | None -> env
  | Some hty ->
      let env, hty = Typing_hint.hint env hty in
      let env, paramty = Env.get_local env (snd param.param_id) in
      let hint_pos = Reason.to_pos (fst hty) in
      let env = Type.sub_type hint_pos Reason.URhint env hty paramty in
      env

and anon_make anon_lenv p f =
  let is_typing_self = ref false in
  fun env tyl ->
    if !is_typing_self
    then error p "Anonymous functions cannot be recursive";
    is_typing_self := true;
    Env.anon anon_lenv env begin fun env ->
      let params = ref f.f_params in
      let env = List.fold_left (anon_bind_param params) env tyl in
      let env = List.fold_left anon_bind_opt_param env !params in
      let env = List.fold_left anon_check_param env f.f_params in
      let env, hret =
        match f.f_ret with
        | None -> TUtils.in_var env (Reason.Rnone, Tunresolved [])
        | Some x -> Typing_hint.hint env x in
      let env = Env.set_return env hret in
      let env = Env.set_fn_type env f.f_type in
      let env = block env f.f_body in
      let env =
        if Nast_terminality.Terminal.block f.f_body || f.f_unsafe || !auto_complete
        then env
        else fun_implicit_return env p hret f.f_body f.f_type
      in
      is_typing_self := false;
      env, hret
    end

(*****************************************************************************)
(* End of anonymous functions. *)
(*****************************************************************************)


and special_func env p func =
  let env, ty = (match func with
  | Gena e ->
      let env, ety = expr env e in
      Async.gena env p ety
  | Genva el ->
      let env, etyl = lmap expr env el in
      Async.genva env p etyl
  | Gen_array_rec e ->
      let env, ety = expr env e in
      Async.gen_array_rec env p ety
  | Gen_array_va_rec el ->
      let env, etyl = lmap expr env el in
      Async.gen_array_va_rec env p etyl
  ) in
  env, (Reason.Rwitness p, Tapply ((p, "Awaitable"), [ty]))

and new_object ~check_not_abstract p env c el =
  let env, class_ = class_id p env c in
  (match class_ with
  | None ->
      (match c with
      | CIstatic -> error p "Can't use new static() outside of a class"
      | CIself -> error p "Can't use new self() outside of a class"
      | _ -> ());
      (match env.Env.genv.Env.mode with
      | Ast.Mstrict ->
          raise Ignore
      | Ast.Mdecl | Ast.Mpartial ->
          let _ = lmap expr env el in
          env, (Reason.Runknown_class p, Tobject)
      )
  | Some (cname, class_) ->
      if check_not_abstract && class_.tc_abstract && c <> CIstatic &&
        not !is_silent_mode
      then error p ("Can't instantiate " ^ snd cname);
      let env, params = lfold begin fun env x ->
        TUtils.in_var env (Reason.Rnone, Tunresolved [])
      end env class_.tc_tparams in
      let env =
        if SSet.mem "XHP" class_.tc_extends then env else
        let env = call_construct p env class_ params el in
        env
      in
      let obj_type = Reason.Rwitness p, Tapply (cname, params) in
      match c with
      | CIstatic ->
        env, (Reason.Rwitness p, Tgeneric ("this", Some obj_type))
      | _ -> env, obj_type
  )

(* While converting code from PHP to Hack, some arrays are used
 * as tuples. Example: array('', 0). Since the elements have
 * incompatible types, it should be a tuple. However, while migrating
 * code, it is more flexible to allow it in partial.
 *)
and convert_array_as_tuple p env ty2 =
  let r2 = fst ty2 in
  let p2 = Reason.to_pos r2 in
  if TUtils.is_array_as_tuple env ty2
  then
    if Env.is_strict env
    then
      let msg_tuple =
        "This array has heterogeneous elements, you should use a tuple instead" in
      error_l [p, "Invalid assignment"; p2, msg_tuple]
    else env, (r2, Tany)
  else env, ty2

and assign p env e1 ty2 =
  let env, ty2 = convert_array_as_tuple p env ty2 in
  match e1 with
  | (_, Lvar (_, x)) ->
      let env = Env.set_local env x ty2 in
      env, ty2
  | (_, List el) ->
      let env, folded_ty2 = TUtils.fold_unresolved env ty2 in
      let env, folded_ety2 = Env.expand_type env folded_ty2 in
      (match folded_ety2 with
      | r, Tapply ((_, x), argl) when Typing_env.is_typedef env x ->
          let env, ty2 = Typing_tdef.expand_typedef SSet.empty env r x argl in
          assign p env e1 ty2
      | r, Tapply ((_, ("Vector" | "ImmVector")), [elt_type])
      | r, Tarray (_, Some elt_type, None) ->
          let env, _ = lfold begin fun env e ->
            assign (fst e) env e elt_type
          end env el in
          env, ty2
      | r, Tarray (_, None, None)
      | r, Tany ->
          let env, _ = lfold begin fun env e ->
            assign (fst e) env e (r, Tany)
          end env el in
          env, ty2
      | r, Tapply ((_, "Pair"), [ty1; ty2]) ->
          (match el with
          | [x1; x2] ->
              let env, _ = assign p env x1 ty1 in
              let env, _ = assign p env x2 ty2 in
              env, (Reason.Rwitness (fst e1), Tprim Tvoid)
          | _ ->
              error p "A pair has exactly 2 elements"
          )
      | r, Ttuple tyl ->
          let size1 = List.length el in
          let size2 = List.length tyl in
          let p1 = fst e1 in
          let p2 = Reason.to_pos r in
          if size1 <> size2
          then
            if !is_silent_mode
            then env, (Reason.Rnone, Tany)
            else
              error_l [p2, "This tuple has "^ string_of_int size2^" elements";
                       p1, string_of_int size1 ^ " were expected"]
          else
            let env = List.fold_left2 begin fun env lvalue ty2 ->
              fst (assign p env lvalue ty2)
            end env el tyl in
            env, (Reason.Rwitness p1, Tprim Tvoid)
      | _ -> assign_simple p env e1 ty2
      )
  | _, Class_get _
  | _, Obj_get _ ->
      let env, ty1 = lvalue env e1 in
      let env, ety1 = Env.expand_type env ty1 in
      let fake_members, locals as lenv = env.Env.lenv in
      let no_fakes = Env.empty_fake_members, locals in
      (* In this section, we check that the assignment is compatible with
       * the real type of a member. Remember that members can change
       * type (cf fake_members). But when we assign a value to $this->x,
       * we want to make sure that the type assign to $this->x is compatible
       * with the actual type hint. In this portion of the code, type-check
       * the assignment in an envrionment without fakes, and therefore
       * check that the assignment is compatible with the type of
       * the member.
       *)
      let env, real_type = expr { env with Env.lenv = no_fakes } e1 in
      let env, exp_real_type = Env.expand_type env real_type in
      let env = { env with Env.lenv = lenv } in
      let env, ety2 = Env.expand_type env ty2 in
      let real_type_list =
        match exp_real_type with
        | _, Tunresolved tyl -> tyl
        | ty -> [ty]
      in
      let env = List.fold_left begin fun env real_type ->
        Type.sub_type p (Reason.URassign) env real_type ety2
      end env real_type_list in
      (match e1 with
      | _, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name))) ->
          let env, local = Env.FakeMembers.make p env obj member_name in
          (match obj with
          | _, This ->
              Typing_suggest.save_member member_name env exp_real_type ty2;
          | _ -> ());
          let env = Env.set_local env local ty2 in
          env, ty2
      | _, Class_get (x, (_, y)) ->
          let env, local = Env.FakeMembers.make_static p env x y in
          let env = Env.set_local env local ty2 in
          (match x with
          | CIself
          | CIstatic ->
              Typing_suggest.save_member y env exp_real_type ty2;
          | _ -> ());
          env, ty2
      | _ -> env, ty2
      )
  | _, Array_get ((_, Lvar (_, lvar)) as shape, Some (_, String (_, field))) ->
      (* In the case of an assignment of the form $x['new_field'] = ...;
       * $x could be a shape where the field 'new_field' is not yet defined.
       * When that is the case we want to add the field to its type.
       *)
      let env, shape_ty = expr env shape in
      let env, shape_ty = TUtils.grow_shape p e1 field ty2 env shape_ty in
      let env = Env.set_local env lvar shape_ty in

      (* We still need to call assign_simple, because shape_ty could be more
       * than just a shape. It could be an unresolved type where some elements
       * are shapes and some others are not.
       *)
      assign_simple p env e1 ty2
  | _ ->
      assign_simple p env e1 ty2

and assign_simple p env e1 ty2 =
  let env, ty1 = lvalue env e1 in
  let env, ty2 = TUtils.unresolved env ty2 in
  let env = Type.sub_type p (Reason.URassign) env ty1 ty2 in
  env, ty2

and field_value env = function
  | Nast.AFvalue x
  | Nast.AFkvalue (_, x) -> expr env x

and field_key env = function
  | Nast.AFvalue (p, _) -> env, (Reason.Rwitness p, Tprim Tint)
  | Nast.AFkvalue (x, _) -> expr env x

and call_parent_construct p env el =
  match Env.get_parent env with
  | _, Tapply (cid, params) ->
      let check_not_abstract = false in
      let env, parent = new_object ~check_not_abstract p env CIparent el in
      let env, _ = Type.unify p (Reason.URnone) env (Env.get_parent env) parent in
      env, (Reason.Rwitness p, Tprim Tvoid)
  | _ -> missing_parent p env

and missing_parent pos env =
  let error_no_self () = error pos "parent is undefined outside of a class" in
  let self_class = TUtils.get_self_class env error_no_self in
  let default = env, (Reason.Rnone, Tany) in
  (* We don't know the entire hierarchy, assume it's correct *)
  if not self_class.tc_members_fully_known then default else
  (* We do know all the hierarchy, and we are dealing with a "normal" class
   * (not a trait) *)
  if self_class.tc_kind = Ast.Cnormal
  then error pos "The parent class is undefined"
  (* We are dealing with a trait, it only fails in strict. *)
  else if Env.is_strict env
  then error pos "Don't call parent::__construct from a trait"
  else default

(* Depending on the kind of expression we are dealing with
 * The typing of call is different.
 *)
and dispatch_call p env call_type (fpos, fun_expr as e) el =
  match fun_expr with
  | Id (_, "echo") ->
      let env, _ = lfold expr env el in
      env, (Reason.Rwitness p, Tprim Tvoid)
  | Id (_, "isset") ->
      let env, _ = lfold expr env el in
      if Env.is_strict env
      then error p "Don't use isset!";
      env, (Reason.Rwitness p, Tprim Tbool)
  | Id (_, x) when SSet.mem x Naming.predef_tests ->
      let env, ty = expr env (List.hd el) in
      env, (Reason.Rwitness p, Tprim Tbool)
  | Class_const (CIparent, (_, "__construct")) ->
      call_parent_construct p env el
  | Class_const (CIparent, m) ->
      let env, ty1 = static_class_id p env CIparent in
      if Env.is_static env
      then begin
        (* in static context, you can only call parent::foo() on static
         * methods *)
        let env, fty = class_get ~is_method:true ~is_const:false env ty1 m CIparent in
        let env, fty = Env.expand_type env fty in
        let env, fty = Inst.instantiate_fun env fty el in
        call p env fty el
      end
      else begin
        (* in instance context, you can call parent:foo() on static
         * methods as well as instance methods *)
        (match class_contains_smethod env ty1 m with
          | None ->
            (* parent::nonStaticFunc() is really weird. It's calling a method
             * defined on the parent class, but $this is still the child class.
             * We can deal with this by hijacking the continuation that
             * calculates the "this" type *)
            let k_lhs ty =
              Reason.Rwitness fpos, Tgeneric ("this", Some (Env.get_self env))
            in
            let env, method_, _ = obj_get_ true env ty1 m begin fun (env, fty, _) ->
              let env, fty = Env.expand_type env fty in
              let env, fty = Inst.instantiate_fun env fty el in
              let env, method_ = call p env fty el in
              env, method_, None
            end k_lhs in
            env, method_
        | Some _ ->
            let env, fty = class_get ~is_method:true ~is_const:false env ty1 m CIparent in
            let env, fty = Env.expand_type env fty in
            let env, fty = Inst.instantiate_fun env fty el in
            call p env fty el
        )
      end
  | Class_const(e1, m) ->
      Typing_utils.process_static_find_ref e1 m;
      let env, ty1 = static_class_id p env e1 in
      let env, fty = class_get ~is_method:true ~is_const:false env ty1 m e1 in
      let env, fty = Env.expand_type env fty in
      let env, fty = Inst.instantiate_fun env fty el in
      call p env fty el
  | Obj_get(e1, (_, Id m)) ->
      let is_method = call_type = Cnormal in
      let env, ty1 = expr env e1 in
      obj_get is_method env ty1 m begin fun (env, fty, _) ->
        let env, fty = Env.expand_type env fty in
        let env, fty = Inst.instantiate_fun env fty el in
        let env, method_ = call p env fty el in
        env, method_, None
      end
  | Fun_id x
  | Id x ->
      auto_complete_id x;
      let env, fty = fun_type_of_id env x in
      let env, fty = Env.expand_type env fty in
      let env, fty = Inst.instantiate_fun env fty el in
      call p env fty el
  | _ ->
      let env, fty = expr env e in
      let env, fty = Env.expand_type env fty in
      let env, fty = Inst.instantiate_fun env fty el in
      call p env fty el

and fun_type_of_id env x =
  Find_refs.process_find_refs None (snd x) (fst x);
  let env, fty = Env.get_fun env (snd x) in
  let env, fty =
    match fty with
    | None -> unbound_name env x
    | Some fty ->
        let env, fty = Inst.instantiate_ft env fty in
        env, (Reason.Rwitness fty.ft_pos, Tfun fty)
  in
  env, fty

and auto_complete_id x =
  if is_auto_complete (snd x)
  then begin
      argument_global_type := Some Acid;
      auto_complete_for_global := snd x
  end

(* Checking that the user explicitely made a copy *)
and array_cow p =
  (* Let's disable this for now. We have too many arrays in our codebase.
   * When the time is right and we want to switch to containers, we will
   * revive this.
   *)
  (*
    error p ("This assignement triggers a copy-on-write.\n"^
    "You should copy this array explicitely before modifying it.\n"^
    "Use the function copy.")
   *)
  ()

(*****************************************************************************)
(* Function type-checking expressions accessing an array (example: $x[...]).
 * The parameter is_lvalue is true when the expression is on the left hand
 * side of an assignment (example: $x[...] = 0).
 *)
(*****************************************************************************)
and array_get is_lvalue p env ty1 ety1 e2 ty2 =
  match snd ety1 with
  | Tunresolved tyl ->
      let env, tyl = lfold begin fun env ty1 ->
        let env, ety1 = Env.expand_type env ty1 in
        array_get is_lvalue p env ty1 ety1 e2 ty2
      end env tyl
      in
      env, (fst ety1, Tunresolved tyl)
  | Tarray (is_local, Some ty, None) ->
      if is_lvalue && not is_local
      then array_cow p;
      let ty1 = Reason.Ridx (fst e2), Tprim Tint in
      let env, _ = Type.unify p Reason.URarray_get env ty2 ty1 in
      env, ty
  | Tgeneric (_, Some (_, Tapply ((_, "Vector"), [ty])))
  | Tapply ((_, "Vector"), [ty]) ->
      let ty1 = Reason.Ridx_vector (fst e2), Tprim Tint in
      let env, _ = Type.unify p Reason.URvector_get env ty2 ty1 in
      env, ty
  | Tgeneric (_, Some (_, Tapply ((_, "Map"), [k; v])))
  | Tgeneric (_, Some (_, Tapply ((_, "StableMap"), [k; v])))
  | Tapply ((_, "Map"), [k; v])
  | Tapply ((_, "StableMap"), [k; v]) ->
      let env, ty2 = TUtils.unresolved env ty2 in
      let env, _ = Type.unify p Reason.URmap_get env k ty2 in
      env, v
  | Tgeneric (_, Some (_, Tapply ((_, ("ConstMap" | "ImmMap")), [k; v])))
  | Tapply ((_, ("ConstMap" | "ImmMap")), [k; v])
      when not is_lvalue ->
      let env, _ = Type.unify p Reason.URmap_get env k ty2 in
      env, v
  | Tgeneric (_, Some (_, Tapply ((_, ("ConstMap" | "ImmMap")), _)))
  | Tapply ((_, ("ConstMap" | "ImmMap")), _)
      when is_lvalue -> error_const_mutation p ety1
  | Tgeneric (_, Some (_, Tapply ((_, "Indexish"), [k; v])))
  | Tapply ((_, "Indexish"), [k; v]) ->
      let env, _ = Type.unify p Reason.URcontainer_get env k ty2 in
      env, v
  | Tgeneric (_, Some (_, Tapply ((_, ("ConstVector" | "ImmVector" as cn)), [ty])))
  | Tapply ((_, ("ConstVector" | "ImmVector" as cn)), [ty])
      when not is_lvalue ->
      let ty1 = Reason.Ridx (fst e2), Tprim Tint in
      let ur = (match cn with
                  "ConstVector"   -> Reason.URconst_vector_get
                | "ImmVector"  -> Reason.URimm_vector_get
                | _  -> failwith ("Unexpected collection name: " ^ cn)) in
      let env, _ = Type.unify p ur env ty2 ty1 in
      env, ty
  | Tgeneric (_, Some (_, Tapply ((_, ("ConstVector" | "ImmVector")), _)))
  | Tapply ((_, ("ConstVector" | "ImmVector")), _)
      when is_lvalue -> error_const_mutation p ety1
  | Tarray (is_local, Some k, Some v) ->
      if is_lvalue && not is_local
      then array_cow p;
      let env, ty2 = TUtils.unresolved env ty2 in
      let env, _ = Type.unify p Reason.URarray_get env k ty2 in
      (* The values in the array are not consistent
       * we just give up. Use Maps!
       *)
      let env, ev = TUtils.fold_unresolved env v in
      let env, ev = Env.expand_type env ev in
      (match ev with
      | _, Tunresolved _ -> env, (Reason.Rwitness p, Tany)
      | _ -> env, v
      )
  | Tany | Tarray (_, None, None) -> env, (Reason.Rnone, Tany)
  | Tprim Tstring ->
      let ty = Reason.Rwitness p, Tprim Tstring in
      let env, ty = Type.unify p Reason.URnone env ty1 ty in
      let int = Reason.Ridx (fst e2), Tprim Tint in
      let env, _ = Type.unify p Reason.URarray_get env ty2 int in
      env, ty
  | Ttuple tyl ->
      (match e2 with
      | p, Int n ->
          (try
            let idx = int_of_string (snd n) in
            let nth = List.nth tyl idx in
            env, nth
          with _ ->
            error p (Reason.string_of_ureason Reason.URtuple_get)
          )
      | p, _ ->
          error p (Reason.string_of_ureason Reason.URtuple_access)
      )
  | Tapply ((_, "Pair"), [ty1; ty2]) ->
      (match e2 with
      | p, Int n ->
          (try
            let idx = int_of_string (snd n) in
            let nth = List.nth [ty1; ty2] idx in
            env, nth
          with _ ->
            error p (Reason.string_of_ureason Reason.URpair_get)
          )
      | p, _ ->
          error p (Reason.string_of_ureason Reason.URpair_access)
      )
  | Tshape fdm ->
      (match e2 with
      | p, String (_, name) ->
          (match SMap.get name fdm with
          | None -> error p ("The field "^name^" is undefined")
          | Some ty -> env, ty
          )
      | p, _ ->
          error p "Was expecting a constant string (for shape access)"
      )
  | _ when !is_silent_mode ->
      env, (Reason.Rnone, Tany)
  | Toption _ ->
      error_l [p,
               "You are trying to access an element of this container"^
               " but the container could be null. ";
               Reason.to_pos (fst ety1),
               "This is what makes me believe it can be null"^
               Reason.to_string (fst ety1)]
  | Tobject ->
      if Env.is_strict env
      then error_array p ety1
      else env, (Reason.Rnone, Tany)
  | Tapply ((_, x), argl) when Typing_env.is_typedef env x ->
      let env, ty1 = Typing_tdef.expand_typedef SSet.empty env (fst ety1) x argl in
      let env, ety1 = Env.expand_type env ty1 in
      array_get is_lvalue p env ty1 ety1 e2 ty2
  | _ -> error_array p ety1

and array_append is_lvalue p env ty1 =
  let env, ty1 = TUtils.fold_unresolved env ty1 in
  let env, ety1 = Env.expand_type env ty1 in
  match snd ety1 with
  | Tany | Tarray (_, None, None) -> env, (Reason.Rnone, Tany)
  | Tgeneric (_, Some (_, Tapply ((_, "Vector"), [ty])))
  | Tgeneric (_, Some (_, Tapply ((_, "Set"), [ty])))
  | Tapply ((_, "Vector"), [ty])
  | Tapply ((_, "Set"), [ty]) ->
      env, ty
  | Tarray (is_local, Some ty, None) ->
      if is_lvalue && not is_local
      then array_cow p;
      env, ty
  | Tobject ->
      if Env.is_strict env
      then error_array_append p ety1
      else env, (Reason.Rnone, Tany)
  | Tapply ((_, x), argl) when Typing_env.is_typedef env x ->
      let env, ty1 = Typing_tdef.expand_typedef SSet.empty env (fst ety1) x argl in
      array_append is_lvalue p env ty1
  | _ when !is_silent_mode ->
      env, (Reason.Rnone, Tany)
  | _ ->
      error_array_append p ety1

and error_array p (r, ty) =
  error_l ((p, "This is not a container, this is "^
            Typing_print.error ty) ::
           let p2 = Reason.to_pos r in
           if p2 != Pos.none
           then [p2, "You might want to check this out"]
           else [])

and error_array_append p (r, ty) =
  error_l ((p, Typing_print.error ty^
            " does not allow array append") ::
           let p2 = Reason.to_pos r in
           if p2 != Pos.none
           then [p2, "You might want to check this out"]
           else [])

and error_const_mutation p (r, ty) =
  error_l ((p, "You cannot mutate this") ::
           let p2 = Reason.to_pos r in
           if p2 != Pos.none
           then [(p2, "This is " ^ Typing_print.error ty)]
           else [])

and deref_tuple p env tyl = function
  | p, Int (_, s) ->
      let n = safe_ios p s in
      if n < 0 then error p "You cannot use a negative value here";
      if n >= List.length tyl then error p "Cannot access this field";
      let res = List.nth tyl n in
      env, res
  | _ -> error p "Please use a static integer"

(**
 * Checks if a class (given by cty) contains a given static method.
 *
 * We could refactor this + class_get
 *)
and class_contains_smethod env cty (p, mid) =
  match cty with
  | _, Tgeneric (_, Some (_, Tapply ((_, c), paraml)))
  | _, Tapply ((_, c), paraml) ->
      let env, class_ = Env.get_class env c in
      (match class_ with
      | None -> None
      | Some class_ ->
          assert (!is_silent_mode || List.length class_.tc_tparams = List.length paraml);
          let env, smethod = Env.get_static_member true env class_ mid in
          smethod
      )
  | _ -> None

and class_get ~is_method ~is_const env cty (p, mid) cid =
  let env, ty = class_get_ ~is_method ~is_const env cty (p, mid) cid in
  (* Replace the "this" type in the resulting type *)
  Inst.instantiate_this env ty cty

and class_get_ ~is_method ~is_const env cty (p, mid) cid =
  match cty with
  | r, Tapply ((_, x), argl) when Typing_env.is_typedef env x ->
      let env, cty = Typing_tdef.expand_typedef SSet.empty env r x argl in
      class_get_ ~is_method ~is_const env cty (p, mid) cid
  | _, Tgeneric (_, Some (_, Tapply ((_, c), paraml)))
  | _, Tapply ((_, c), paraml) ->
      let env, class_ = Env.get_class env c in
      (match class_ with
      | None ->
          if Env.is_strict env
          then raise Ignore
          else env, (Reason.Rnone, Tany)
      | Some class_ ->
          let env, smethod =
            if is_const
            then Env.get_const env class_ mid
            else Env.get_static_member is_method env class_ mid in
            if !Typing_defs.accumulate_method_calls then
              Typing_defs.accumulate_method_calls_result :=
                  (p, (class_.tc_name^"::"^mid)) ::
                      !Typing_defs.accumulate_method_calls_result;
          Find_refs.process_find_refs (Some class_.tc_name) mid p;
          (match smethod with
          | None when !auto_complete ->
              if is_auto_complete mid
              then begin
                argument_global_type := Some Acclass_get;
                let filter = begin fun key x map ->
                  if class_.tc_members_fully_known then
                    match is_visible env x.ce_visibility (Some cid) with
                    | None -> SMap.add key x map
                    | _ -> map
                  else SMap.add key x map end in
                let results = SMap.fold SMap.add class_.tc_smethods
                    (SMap.fold SMap.add class_.tc_consts class_.tc_scvars) in
                TUtils.add_auto_result env
                  (SMap.fold filter results SMap.empty);
              end;
              env, (Reason.Rnone, Tany)
          | None ->
            (match Env.get_static_member is_method env class_ "__callStatic" with
              | env, None when !is_silent_mode ->
                  env, (Reason.Rnone, Tany)
              | env, None ->
                  smember_not_found p ~is_const ~is_method env class_ mid
              | env, Some { ce_visibility = vis; ce_type = r, Tfun ft } ->
                  check_visibility p env (Reason.to_pos r, vis) (Some cid);
                  let ft = {ft with
                            ft_arity_min = 0;
                            ft_arity_max = 1000;
                            ft_tparams = [];
                            ft_params = [];
                          } in env, (r, Tfun ft)
              | _ -> assert false)
          | Some { ce_visibility = vis; ce_type = method_ } ->
              check_visibility p env (Reason.to_pos (fst method_), vis) (Some cid);
              let arity_match =
                List.length class_.tc_tparams = List.length paraml
              in
              assert (!is_silent_mode || arity_match);
              let subst = Inst.make_subst class_.tc_tparams paraml in
              let env, method_ = Inst.instantiate subst env method_ in
              env, method_)
      )
  | _, Tany ->
      (match env.Env.genv.Env.mode with
      | Ast.Mstrict -> error p "Was expecting a class"
      | Ast.Mdecl | Ast.Mpartial -> env, (Reason.Rnone, Tany)
      )
  | _ -> error p "Was expecting a class"

and smember_not_found pos ~is_const ~is_method env class_ member_name =
  let kind =
    if is_const then "class constant "
    else if is_method then "static method "
    else "class variable " in
  let msg = "Could not find "^kind^member_name in
  match Env.suggest_static_member is_method class_ member_name with
    | None ->
      (match Env.suggest_member is_method class_ member_name with
        | None when not class_.tc_members_fully_known ->
          (* no error in this case ... the member might be present
           * in one of the parents of class_ that the typing cannot see *)
          env, (Reason.Rnone, Tany)
        | None ->
          error pos msg
        | Some (pos2, v) ->
          let msg2 = "The closest thing is "^v^" but it's not a static method"
          in error_l [pos, msg; pos2, msg2]
      )
    | Some (pos2, v) ->
      error_l [pos, msg; pos2, "Did you mean: "^v]

and member_not_found pos ~is_method env class_ member_name class_name =
  let kind = if is_method then "method " else "member " in
  let msg = "The "^kind^member_name^" is undefined "
    ^"in an object of type "^(snd class_name)
  in
  let errors = [pos, msg; (fst class_name), "Check this out"] in
  match Env.suggest_member is_method class_ member_name with
    | None ->
      (match Env.suggest_static_member is_method class_ member_name with
        | None when not class_.tc_members_fully_known ->
          (* no error in this case ... the member might be present
           * in one of the parents of class_ that the typing cannot see *)
          env, (Reason.Rnone, Tany), None
        | None ->
          error_l errors
        | Some (def_pos, v) ->
          let msg2 = "The closest thing is "^v^" but it's a static method" in
          error_l (errors @ [def_pos, msg2])
      )
    | Some (def_pos, v) ->
      error_l (errors @ [def_pos, "Did you mean: "^v])

and obj_get is_method env ty1 id k =
  let env, method_, _ = obj_get_with_visibility is_method env ty1 id k in
  env, method_

and obj_get_with_visibility is_method env ty1 (p, s as id) k =
  obj_get_ is_method env ty1 id k (fun ty -> ty)

and obj_get_ is_method env ty1 (p, s as id) k k_lhs =
  let env, ety1 = Env.expand_type env ty1 in
  match ety1 with
  | _, Tunresolved tyl ->
      let (env, vis), tyl =
        lfold
          (fun (env, vis) ty ->
            let env, ty, vis' = obj_get_ is_method env ty id k k_lhs in
            (* There is one special case where we need to expose the
             * visibility outside of obj_get (checkout inst_meth special
             * function).
             * We keep a witness of the "most restrictive" visibility
             * we encountered (position + visibility), to be able to
             * special case inst_meth.
             *)
            let vis = TUtils.min_vis_opt vis vis' in
            (env, vis), ty)
          (env, None)
          tyl in
      let env, method_ = TUtils.in_var env (fst ety1, Tunresolved (tyl)) in
      env, method_, vis
  | p, Tgeneric (x, Some ty) ->
      let k_lhs' ty = k_lhs (p, Tgeneric (x, Some ty)) in
      obj_get_ is_method env ty id k k_lhs'
  | p, Tapply ((_, x) as c, argl) when Typing_env.is_typedef env x ->
      let env, ty1 = Typing_tdef.expand_typedef SSet.empty env (fst ety1) x argl in
      let k_lhs' ty = k_lhs (p, Tapply (c, argl)) in
      obj_get_ is_method env ty1 id k k_lhs'
  | _ -> k begin match snd ety1 with
    | Tgeneric (_, Some (_, Tapply (x, paraml)))
    | Tapply (x, paraml) ->
        let env, class_ = Env.get_class env (snd x) in
        (match class_ with
        | None ->
            (match env.Env.genv.Env.mode with
            | Ast.Mstrict ->
                error p ("class "^snd x^" is undefined")
            | Ast.Mdecl | Ast.Mpartial ->
                env, (Reason.Rnone, Tany), None
            )
        | Some class_ ->
            let paraml =
              if List.length paraml = 0
              then List.map (fun _ -> Reason.Rwitness p, Tany) class_.tc_tparams
              else paraml
            in
            let env, method_ = Env.get_member is_method env class_ s in
            if !Typing_defs.accumulate_method_calls then
              Typing_defs.accumulate_method_calls_result :=
                  (p, (class_.tc_name^"::"^s)) ::
                      !Typing_defs.accumulate_method_calls_result;
            Find_refs.process_find_refs (Some class_.tc_name) s p;
            (match method_ with
            | None when !auto_complete ->
                if is_auto_complete s
                then begin
                  argument_global_type := Some Acclass_get;
                  let filter = begin fun key x map ->
                    if class_.tc_members_fully_known then
                      match is_visible env x.ce_visibility None with
                      | None -> SMap.add key x map
                      | _ -> map
                    else SMap.add key x map end in
                  let results =
                    SMap.fold SMap.add class_.tc_methods class_.tc_cvars in
                  TUtils.add_auto_result env
                    (SMap.fold filter results SMap.empty);
                end;
                env, (Reason.Rnone, Tany), None
            | None ->
                (match Env.get_member is_method env class_ "__call" with
                | env, None when !is_silent_mode ->
                    env, (Reason.Rnone, Tany), None
                | env, None ->
                    member_not_found p ~is_method env class_ s x
                | env, Some { ce_visibility = vis; ce_type = r, Tfun ft } ->
                    check_visibility p env (Reason.to_pos r, vis) None;
                    let pos = Reason.to_pos r in
                    let ft = {ft with
                              ft_arity_min = 0;
                              ft_arity_max = 1000;
                              ft_tparams = [];
                              ft_params = [];
                            } in env, (r, Tfun ft), Some (pos, vis)
                | _ -> assert false)
            | Some { ce_visibility = vis; ce_type = method_ } ->
                check_visibility p env (Reason.to_pos (fst method_), vis) None;
                let arity_match =
                  List.length class_.tc_tparams = List.length paraml
                in
                assert (!is_silent_mode || arity_match);

                let new_name = "alpha_varied_this" in

                (* Since a param might include a "this" type, let's alpha vary
                 * all the "this" types in the params*)
                let env, paraml = List.fold_right
                  (fun param (env, paraml) ->
                    let env, param =
                      Typing_generic.rename env "this" new_name param in
                    env, param::paraml)
                    paraml
                    (env, []) in

                let subst = Inst.make_subst class_.tc_tparams paraml in
                let env, method_ = Inst.instantiate subst env method_ in

                (* We must substitute out the this types separately
                 * from when the method is instantiated with its type
                 * variables. Consider Vector<this>::add(). It is
                 * declared with the return type this and argument
                 * T. We don't want to substitute "this" for T and then
                 * replace that "this" with Vector<this>. *)
                let this_ty = k_lhs ety1 in
                let env, method_ = Inst.instantiate_this env method_ this_ty in

                (* Now this has been substituted, we can de-alpha-vary *)
                let env, method_ =
                  Typing_generic.rename env new_name "this" method_ in
                let meth_pos = Reason.to_pos (fst method_)in
                env, method_, Some (meth_pos, vis)
            )
        )
    | Tobject
    | Tany -> env, (fst ety1, Tany), None
    | _ when !is_silent_mode ->
        env, (fst ety1, Tany), None
    | Toption _ ->
        error_l [p,
                 "You are trying to access the member "^s^
                 " but this object can be null. ";
                 Reason.to_pos (fst ety1),
                 "This is what makes me believe it can be null"^
                 Reason.to_string (fst ety1)]
    | ty ->
        error_l [p,
                 ("You are trying to access the member "^s^
                  " but this is not an object, it is "^
                  Typing_print.error ty);
                 Reason.to_pos (fst ety1),
                 "Check this out"]
  end

and class_id p env cid =
  let env, obj = static_class_id p env cid in
  match obj with
  | _, Tgeneric ("this", Some (_, Tapply ((_, cid as c), _)))
  | _, Tapply ((_, cid as c), _) ->
      let env, class_ = Env.get_class env cid in
      (match class_ with
      | None -> env, None
      | Some class_ ->
          env, Some (c, class_)
      )
  | _ -> env, None

(* To be a valid trait declaration, all of its 'require extends' must
 * match; since there's no multiple inheritance, it follows that all of
 * the 'require extends' must belong to the same inheritance hierarchy
 * and one of them should be the child of all the others *)
and trait_most_concrete_req_class trait env =
  SSet.fold (fun name acc ->
    let keep = match acc with
      | Some c -> SMap.mem name c.tc_ancestors
      | None -> false
    in
    if keep then acc
    else
      let env, class_ = Env.get_class env name in
      (match class_ with
        | None | Some { tc_kind = Ast.Cinterface } -> acc
        | Some c -> assert (c.tc_kind <> Ast.Ctrait); Some c
      )
  ) trait.tc_req_ancestors None

and static_class_id p env = function
  | CIparent ->
    let error_no_self () = error p "parent is undefined outside of a class" in
    (match TUtils.get_self_class env error_no_self with
      | {tc_kind = Ast.Ctrait; tc_req_ancestors ; tc_name} as trait ->
        (match trait_most_concrete_req_class trait env with
          | None ->
            error p ("parent:: inside a trait is undefined"
                     ^" without 'require extends' of a class defined in <?hh")
          | Some parent ->
            let r = Reason.Rwitness p in
            let self_ty = Env.get_self env in
            let ty = (match self_ty with
              | (_, Tapply (_, tyl)) ->
                Tapply ((p, parent.tc_name), tyl)
              | _ -> assert(false) ;
                error p ("Internal error; expected to find self as "^parent.tc_name)
            ) in
            (* in a trait, parent is "this", but with the type of the most
             * concrete class the trait 'require extend's *)
            env, (r, Tgeneric ("this", Some (r, ty)))
        )
      | _ ->
        let parent = Env.get_parent env in
        let parent_defined = snd parent <> Tany in
        if not parent_defined && not !is_silent_mode
        then error p "parent is undefined";
        let r = Reason.Rwitness p in
        (* parent is still technically the same object. *)
        env, (r, Tgeneric ("this", Some (r, snd parent)))
    )
  | CIstatic ->
    env, (Reason.Rwitness p, Tgeneric ("this", Some (Env.get_self env)))
  | CIself -> env, (Reason.Rwitness p, snd (Env.get_self env))
  | CI c ->
    let env, class_ = Env.get_class env (snd c) in
    (match class_ with
      | None ->
        (match env.Env.genv.Env.mode with
          | Ast.Mstrict ->
            raise Ignore
          | Ast.Mpartial | Ast.Mdecl ->
            env, (Reason.Rnone, Tany))
      | Some class_ ->
        let params = List.map (fun x -> Env.fresh_type()) class_.tc_tparams in
        env, (Reason.Rwitness p, Tapply (c, params))
    )

and call_construct p env class_ params el =
  let env, cstr = Env.get_construct env class_ in
  let mode = env.Env.genv.Env.mode in
  Find_refs.process_find_refs (Some class_.tc_name) "__construct" p;
  match cstr with
  | None ->
      if el <> [] &&
        (mode = Ast.Mstrict || mode = Ast.Mpartial) &&
        class_.tc_members_fully_known &&
        not !is_silent_mode
      then error p "This constructor expects no argument";
      env
  | Some { ce_visibility = vis; ce_type = m } ->
      check_visibility p env (Reason.to_pos (fst m), vis) None;
      let subst = Inst.make_subst class_.tc_tparams params in
      let env, m = Inst.instantiate subst env m in
      fst (call p env m el)

and check_visibility p env (p_vis, vis) cid =
  if !is_silent_mode then () else
  match is_visible env vis cid with
  | None -> ()
  | Some (msg1, msg2) -> error_l [(p, msg1); (p_vis, msg2)]

and is_visible env vis cid =
  let self_id = Env.get_self_id env in
  match vis with
  | Vpublic -> None
  | Vprivate _ when self_id = "" ->
    Some ("You cannot access this member", "This member is private")
  | Vprivate x ->
    (match cid with
      | Some CIstatic -> 
          Some ("Private members cannot be accessed"
          ^" with static:: since a child class may also have an identically"
          ^" named private member", "This member is private")
      | Some CIparent -> 
          Some (
            "You cannot access a private member with parent::", 
            "This member is private")
      | Some CIself -> None
      | Some (CI (_, called_ci)) when x <> self_id ->
          (match Env.get_class env called_ci with
          | _, Some {tc_kind = Ast.Ctrait} ->
              Some ("You cannot access private members"
              ^" using the trait's name (did you mean to use self::?)",
              "This member is private")
          | _ -> 
            Some ("You cannot access this member", "This member is private"))
      | None when x <> self_id -> 
        Some ("You cannot access this member", "This member is private")
      | Some (CI _) 
      | None -> None)
  | Vprotected x when x = self_id -> None
  | Vprotected _ when self_id = "" ->
    Some ("You cannot access this member", "This member is protected")
  | Vprotected x ->
    let env, my_class = Env.get_class env self_id in
    let env, their_class = Env.get_class env x in
    match cid, their_class with
      | Some CI _, Some {tc_kind = Ast.Ctrait} ->
        Some ("You cannot access protected members"
        ^" using the trait's name (did you mean to use static:: or self::?)",
        "This member is protected")
      | _ -> (
        match my_class, their_class with
          | Some my_class, Some their_class ->
              (* Children can call parent's protected methods and
               * parents can call children's protected methods (like a
               * constructor) *)
              if SSet.mem x my_class.tc_extends
                || SSet.mem self_id their_class.tc_extends
                || SSet.mem x my_class.tc_req_ancestors_extends
                || SSet.mem self_id their_class.tc_req_ancestors (* needed? *)
                || not my_class.tc_members_fully_known
              then None
              else Some (
                "Cannot access this protected member, you don't extend "^ x, 
                "This member is protected"
              )
            | _, _ -> None
        )

and check_arity env pos pos_def arity arity_min arity_max =
  if arity > arity_max && Env.get_mode env <> Ast.Mdecl && not !is_silent_mode
  then
    error_l [pos, "Too many arguments"; pos_def, "Definition is here"];
  if arity < arity_min && not !is_silent_mode then
    error_l  [pos, "Too few arguments"; pos_def, "Definition is here"];
  ()

and call pos env fty el =
  let env, ty = call_ pos env fty el in
  (* We need to solve the constraints after every single function call.
   * The type-checker is control-flow sensitive, the same value could
   * have different type depending on the branch that we are in.
   * When this is the case, a call could violate one of the constraints
   * in a branch.
   *)
  let env = solve_todos env in
  env, ty

and call_ pos env fty el =
  let env, efty = Env.expand_type env fty in
  (match efty with
  | r, Tapply ((_, x), argl) when Typing_env.is_typedef env x ->
      let env, fty = Typing_tdef.expand_typedef SSet.empty env r x argl in
      call_ pos env fty el
  | _, (Tany | Tunresolved []) ->
      let env, _ = lmap expr env el in
      env, (Reason.Rnone, Tany)
  | r, Tunresolved tyl ->
      let env, retl = lmap (fun env ty -> call pos env ty el) env tyl in
      TUtils.in_var env (r, Tunresolved retl)
  | r2, Tfun ft ->
      let pos_def = Reason.to_pos r2 in
      if not !is_silent_mode then
        check_arity env pos pos_def (List.length el) ft.ft_arity_min ft.ft_arity_max;
      let env, tyl = lfold expr env el in
      let pos_tyl = List.combine (List.map fst el) tyl in
      Typing_utils.process_arg_info ft.ft_params pos_tyl env;
      let env = wfold_left2 call_param env ft.ft_params pos_tyl in
      call_check_return env pos (Reason.to_pos r2) ft.ft_ret;
      env, ft.ft_ret
  | r2, Tanon (arity_min, arity_max, id) ->
      let env, tyl = lmap expr env el in
      let anon = Env.get_anonymous env id in
      let fpos = Reason.to_pos r2 in
      (match anon with
      | None -> error pos "recursive call to anonymous function"
      | Some anon ->
          check_arity env pos fpos (List.length tyl) arity_min arity_max;
          let tyl = List.map (fun x -> None, x) tyl in
          anon env tyl)
  | _, Tarray _ when not (Env.is_strict env) ->
      (* Relaxing call_user_func to work with an array is partial mode *)
      env, (Reason.Rnone, Tany)
  | _ when !is_silent_mode ->
      env, (Reason.Rnone, Tany)
  | _, ty -> bad_call pos ty
  )

and call_param env (name, x) (pos, arg) =
  (match name with
  | None -> ()
  | Some name -> Typing_suggest.save_param name env x arg
  );
  Type.sub_type pos Reason.URparam env x arg


and call_check_return env p1 p2 ty =
(*
  if Env.is_strict env then begin
  match ty with
  | _, Tany -> error_l [p1, "I don't know what this function returns";
  p2, "Add a return type hint to this function"]
  | _ -> ()
  end
  else ()
 *)
  ()

and bad_call p ty =
  error p
    ("This call is invalid, this is not a function, it is "^
     Typing_print.error ty)

and expr_list env el =
  List.fold_right (
  fun e (env, acc) ->
    let env, e = expr env e in
    env, e :: acc
 ) el (env, [])

and unop p env uop ty =
  match uop with
  | Ast.Unot ->
      (* !$x (logical not) works with any type, so we just return Tbool *)
      env, (Reason.Rlogic_ret p, Tprim Tbool)
  | Ast.Utild ->
      (* ~$x (bitwise not) only works with int *)
      Type.unify p Reason.URnone env (Reason.Rarith p, Tprim Tint) ty
  | Ast.Uincr
  | Ast.Upincr
  | Ast.Updecr
  | Ast.Udecr
  | Ast.Uplus
  | Ast.Uminus ->
      (* math operators work with int or floats, so we call sub_type *)
      let env = Type.sub_type p Reason.URnone env (Reason.Rarith p, Tprim Tnum) ty in
      env, ty

and binop p env bop p1 ty1 p2 ty2 =
  match bop with
  | Ast.Plus ->
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, ty2 = TUtils.fold_unresolved env ty2 in
      let env, ety1 = Env.expand_type env ty1 in
      let env, ety2 = Env.expand_type env ty2 in
      (match ety1, ety2 with
      | (_, Tarray _), (_, Tarray _)
      | (_, Tany), (_, Tarray _)
      | (_, Tarray _), (_, Tany) ->
          let env, ty = Type.unify p Reason.URnone env ty1 ty2 in
          env, ty
      | _ -> binop p env Ast.Minus p1 ty1 p2 ty2
      )
  | Ast.Minus | Ast.Star ->
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, ty2 = TUtils.fold_unresolved env ty2 in
      let env = Type.sub_type p1 Reason.URnone env (Reason.Rarith p1, Tprim Tnum) ty1 in
      let env = Type.sub_type p2 Reason.URnone env (Reason.Rarith p2, Tprim Tnum) ty2 in
      let env, ety1 = Env.expand_type env ty1 in
      let env, ety2 = Env.expand_type env ty2 in
      (match ety1, ety2 with
      | (r, Tprim Tfloat), _ | _, (r, Tprim Tfloat) ->
          (* if either side is a float then float: 1.0 - 1 -> float *)
          env, (r, Tprim Tfloat)
      | (r, Tprim Tnum), _ | _, (r, Tprim Tnum) ->
          (* if either side is a num, then num: (3 / x) - 1 -> num *)
          env, (r, Tprim Tnum)
      | (_, Tprim Tint), (_, Tprim Tint) ->
          (* Both sides are integers, then integer: 1 - 1 -> int *)
          env, (Reason.Rarith_ret p, Tprim Tint)
      | rty1, _ ->
          (* Either side is unknown, unknown *)
          env, rty1)
  | Ast.Slash ->
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, ty2 = TUtils.fold_unresolved env ty2 in
      let env = Type.sub_type p1 Reason.URnone env (Reason.Rarith p1, Tprim Tnum) ty1 in
      let env = Type.sub_type p2 Reason.URnone env (Reason.Rarith p2, Tprim Tnum) ty2 in
      let env, ety1 = Env.expand_type env ty1 in
      let env, ety2 = Env.expand_type env ty2 in
      (match ety1, ety2 with
      | (r, Tprim Tfloat), _ | _, (r, Tprim Tfloat) -> env, (r, Tprim Tfloat)
      | _ -> env, (Reason.Rret_div p, Tprim Tnum)
      )
  | Ast.Percent ->
      let env, ty1 = Type.unify p Reason.URnone env ty1 (Reason.Rarith p1, Tprim Tint) in
      let env, ty2 = Type.unify p Reason.URnone env ty2 (Reason.Rarith p1, Tprim Tint) in
      env, (Reason.Rarith_ret p, Tprim Tint)
  | Ast.Xor ->
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, ty2 = TUtils.fold_unresolved env ty2 in
      let env, ety1 = Env.expand_type env ty1 in
      let env, ety2 = Env.expand_type env ty2 in
      (match ety1, ety2 with
      | (_, Tprim Tbool), _ | _, (_, Tprim Tbool) ->
          let env, ty1 = Type.unify p Reason.URnone env ty1 (Reason.Rlogic_ret p1, Tprim Tbool) in
          let env, ty2 = Type.unify p Reason.URnone env ty2 (Reason.Rlogic_ret p1, Tprim Tbool) in
          env, (Reason.Rlogic_ret p, Tprim Tbool)
      | _ ->
          let env, ty1 = Type.unify p Reason.URnone env ty1 (Reason.Rarith p1, Tprim Tint) in
          let env, ty2 = Type.unify p Reason.URnone env ty2 (Reason.Rarith p1, Tprim Tint) in
          env, (Reason.Rarith_ret p, Tprim Tint)
      )
  | Ast.Eqeq  | Ast.Diff  | Ast.EQeqeq  | Ast.Lt
  | Ast.Lte  | Ast.Gt  | Ast.Gte  | Ast.Diff2 ->
      let ty_num = (Reason.Rnone, Tprim Nast.Tnum) in
      if (SubType.is_sub_type env ty_num ty1) && (SubType.is_sub_type env ty_num ty2)
      then env, (Reason.Rcomp p, Tprim Tbool)
      else
        (match bop with
        | Ast.Eqeq | Ast.Diff
        | Ast.EQeqeq | Ast.Diff2 ->
            env, (Reason.Rcomp p, Tprim Tbool)
        | _ ->
            let env, ty = Type.unify p Reason.URnone env ty1 ty2 in
            env, (Reason.Rcomp p, Tprim Tbool)
        )
  | Ast.Dot ->
      let env = SubType.sub_string p1 env ty1 in
      let env = SubType.sub_string p2 env ty2 in
      env, (Reason.Rconcat_ret p, Tprim Tstring)
  | Ast.AMpamp
  | Ast.BArbar ->
      env, (Reason.Rlogic_ret p, Tprim Tbool)
  | Ast.Amp  | Ast.Bar  | Ast.Ltlt  | Ast.Gtgt ->
      let env, ty1 = Type.unify p Reason.URnone env ty1 (Reason.Rbitwise p1, Tprim Tint) in
      let env, ty2 = Type.unify p Reason.URnone env ty2 (Reason.Rbitwise p2, Tprim Tint) in
      env, (Reason.Rbitwise_ret p, Tprim Tint)
  | Ast.Eq _ ->
      assert false


and dyn_option p x_ty =
  match x_ty with
  | _, Tmixed -> Reason.Rwitness p, Toption (Reason.Rwitness p, Tmixed)
  | _ -> x_ty

and non_null env ty =
  let env, ty = Env.expand_type env ty in
  match ty with
  | _, Toption ty ->
      let env, ty = Env.expand_type env ty in
      env, ty
  | r, Tunresolved tyl ->
      let env, tyl = lfold non_null env tyl in
      (* We need to flatten the unresolved types, otherwise we could
       * end up with "Tunresolved[Tunresolved _]" which is not supposed
       * to happen.
       *)
      let tyl = List.fold_right begin fun ty tyl ->
        match ty with
        | _, Tunresolved l -> l @ tyl
        | x -> x :: tyl
      end tyl [] in
      env, (r, Tunresolved tyl)
  | r, Tapply ((_, x), argl) when Typing_env.is_typedef env x ->
      let env, ty = Typing_tdef.expand_typedef SSet.empty env r x argl in
      non_null env ty
  | r, Tgeneric (x, Some ty) ->
      let env, ty = non_null env ty in
      env, (r, Tgeneric (x, Some ty))
  | ty ->
      env, ty

and condition_var_non_null env = function
  | _, Lvar (p, x) ->
      let env, x_ty = Env.get_local env x in
      let env, x_ty = Env.expand_type env x_ty in
      let x_ty = dyn_option p x_ty in
      let env, x_ty = non_null env x_ty in
      Env.set_local env x x_ty
  | p, Class_get (cname, (_, member_name)) as e ->
      let env, ty = expr env e in
      let env, local = Env.FakeMembers.make_static p env cname member_name in
      let env = Env.set_local env local ty in
      let local = p, Lvar (p, local) in
      condition_var_non_null env local
  | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name))) as e ->
      let env, ty = expr env e in
      let env, local = Env.FakeMembers.make p env obj member_name in
      let env = Env.set_local env local ty in
      let local = p, Lvar (p, local) in
      condition_var_non_null env local
  | _ -> env

and condition_isset env = function
  | _, Array_get (x, _) -> condition_isset env x
  | v -> condition_var_non_null env v

(**
 * Build an environment for the true or false branch
 * of conditional statements.
 *)
and condition env tparamet = function
  | _, Expr_list [] -> env
  | _, Expr_list [x] ->
      let env, _ = expr env x in
      condition env tparamet x
  | r, Expr_list (x::xs) ->
      let env, _ = expr env x in
      condition env tparamet (r, Expr_list xs)
  | _, Call (Cnormal, (_, Id (_, "isset")), [param])
    when tparamet && not (Env.is_strict env) ->
      condition_isset env param
  | _, Call (Cnormal, (_, Id (_, "is_null")), [e]) when not tparamet ->
      condition_var_non_null env e
  | r, Binop ((Ast.Eqeq | Ast.EQeqeq as bop),
              (_, Null), e)
  | r, Binop ((Ast.Eqeq | Ast.EQeqeq as bop),
              e, (_, Null)) when not tparamet ->
                let env, x_ty = expr env e in
                let env, x_ty = Env.expand_type env x_ty in
                if bop == Ast.Eqeq
                then check_null_wtf env r x_ty;
                condition_var_non_null env e
  | (p, (Lvar _ | Obj_get _ | Class_get _) as e) ->
      let env, ty = expr env e in
      let env, ety = Env.expand_type env ty in
      (match ety with
      | _, Tarray (_, None, None)
      | _, Tprim Tbool -> env
      | _ ->
          condition env (not tparamet) (p, Binop (Ast.Eqeq, e, (p, Null))))
  | r, Binop (Ast.Eq None, var, e) when tparamet ->
      let env, e_ty = expr env e in
      let env, e_ty = Env.expand_type env e_ty in
      check_null_wtf env r e_ty;
      condition_var_non_null env var
  | p1, Binop (Ast.Eq None, (_, (Lvar _ | Obj_get _) as lv), (p2, _)) ->
      let env, ty = expr env (p1, Binop (Ast.Eq None, lv, (p2, Null))) in
      condition env tparamet lv
  | p, Binop ((Ast.Diff | Ast.Diff2 as op), e1, e2) ->
      let op = if op = Ast.Diff then Ast.Eqeq else Ast.EQeqeq in
      condition env (not tparamet) (p, Binop (op, e1, e2))
  | _, Binop (Ast.AMpamp, e1, e2) when tparamet ->
      let env = condition env true e1 in
      let env = condition env true e2 in
      env
  | _, Binop (Ast.BArbar, e1, e2) when not tparamet ->
      let env = condition env false e1 in
      let env = condition env false e2 in
      env
  | _, Call (Cnormal, (_, Id (_, f)), [lv])
    when tparamet && f = Naming.is_array ->
      is_array env lv
  | _, Call (Cnormal, (_, Id (_, f)), [lv])
    when tparamet && f = Naming.is_int ->
      is_type env lv Tint
  | _, Call (Cnormal, (_, Id (_, f)), [lv])
    when tparamet && f = Naming.is_bool ->
      is_type env lv Tbool
  | _, Call (Cnormal, (_, Id (_, f)), [lv])
    when tparamet && f = Naming.is_float ->
      is_type env lv Tfloat
  | _, Call (Cnormal, (_, Id (_, f)), [lv])
    when tparamet && f = Naming.is_string ->
      is_type env lv Tstring
  | _, Call (Cnormal, (_, Id (_, f)), [lv])
    when tparamet && f = Naming.is_resource ->
      is_type env lv Tresource
  | _, Unop (Ast.Unot, e) ->
      condition env (not tparamet) e
  | _, InstanceOf (ivar, (_, Id (pc, c as cid)))
    when tparamet && is_instance_var ivar ->
      let env, (p, x) = get_instance_var env ivar in
      let env, x_ty = Env.get_local env x in
      let env, x_ty = Env.expand_type env x_ty in (* We don't want to modify x *)
      let env, class_ = Env.get_class env c in
      (match class_ with
      | None ->
          Env.set_local env x (Reason.Rwitness pc, Tany)
      | Some class_ ->
          let env, params = lfold begin fun env x ->
            TUtils.in_var env (Reason.Rnone, Tunresolved [])
          end env class_.tc_tparams in
          let obj_ty = Reason.Rwitness pc, Tapply (cid, params) in
          (* This is the case where you check that an object is
           * an instance of a super type. In this case, since we
           * already have a more specialized object, we don't touch
           * the original object. Checkout the unit test srecko.php if
           * this is unclear.
           *)
          if SubType.sub_type_ok env obj_ty x_ty
          then env
          else
            let env = Env.set_local env x obj_ty in
            (match x_ty with
            | _, Tapply ((_, c2), _) when c = c2 ->
                Type.sub_type p Reason.URnone env x_ty obj_ty
            | _ -> env)
      )
  | _, Binop ((Ast.Eqeq | Ast.EQeqeq), e, (_, Null))
  | _, Binop ((Ast.Eqeq | Ast.EQeqeq), (_, Null), e) ->
      let env, _ = expr env e in
      env
  | e ->
      let env, _ = expr env e in
      env

and is_instance_var = function
  | _, (Lvar _ | This) -> true
  | _, Obj_get ((_, This), (_, Id _)) -> true
  | _, Obj_get ((_, Lvar _), (_, Id _)) -> true
  | _, Class_get (_, _) -> true
  | _ -> false

and get_instance_var env = function
  | p, Class_get (cname, (_, member_name)) ->
      let env, local = Env.FakeMembers.make_static p env cname member_name in
      env, (p, local)
  | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name))) ->
      let env, local = Env.FakeMembers.make p env obj member_name in
      env, (p, local)
  | _, Lvar (p, x) -> env, (p, x)
  | p, This -> env, (p, this)
  | _ -> failwith "Should only be called when is_instance_var is true"

and check_null_wtf env p ty =
  if Env.is_strict env then
    match ty with
    | _, Toption ty ->
        let env, ty = Env.expand_type env ty in
        (match ty with
        | _, Tmixed
        | _, Tany ->
            error p ("You are using a sketchy null check ...\n"^
                     "Use is_null, or $x === null instead")
        | _, Tprim _ ->
            error p ("You are using a sketchy null check on a primitive type ...\n"^
                     "Use is_null, or $x === null instead")
        | _ -> ())
    | _ -> ()


and is_type env e tprim =
  match e with
  | p, Class_get (cname, (_, member_name)) ->
      let env, local = Env.FakeMembers.make_static p env cname member_name in
      Env.set_local env local (Reason.Rwitness p, Tprim tprim)
  | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name))) ->
      let env, local = Env.FakeMembers.make p env obj member_name in
      Env.set_local env local (Reason.Rwitness p, Tprim tprim)
  | _, Lvar (p, x) ->
      Env.set_local env x (Reason.Rwitness p, Tprim tprim)
  | _ -> env

and is_array env = function
  | p, Class_get (cname, (_, member_name)) ->
      let env, local = Env.FakeMembers.make_static p env cname member_name in
      Env.set_local env local (Reason.Rwitness p, Tarray (true, None, None))
  | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name))) ->
      let env, local = Env.FakeMembers.make p env obj member_name in
      Env.set_local env local (Reason.Rwitness p, Tarray (true, None, None))
  | _, Lvar (p, x) ->
      Env.set_local env x (Reason.Rwitness p, Tarray (true, None, None))
  | _ -> env

and string2 env idl =
  List.fold_left (
  fun env x ->
    let env, ty = expr env x in
    let p = fst x in
    let env = SubType.sub_string p env ty in
    env
 ) env idl

and get_implements ~with_checks ~this (env: Typing_env.env) ht =
  let env, ht = Typing_hint.hint env ht in
  match ht with
  | _, Tapply ((p, c), paraml) ->
      let env, class_ = Env.get_class_dep env c in
      (match class_ with
      | None ->
          (* The class lives in PHP land *)
          env, (SMap.add c ht SMap.empty, SMap.empty)
      | Some class_ ->
          let size1 = List.length class_.tc_tparams in
          let size2 = List.length paraml in
          if size1 <> size2 && not !is_silent_mode
          then error_l [p, "This class expects "^soi size1^ " arguments"];
          let this_ty = fst this, Tgeneric ("this", Some this) in
          let subst =
            Inst.make_subst_with_this ~this:this_ty class_.tc_tparams paraml in
          List.iter2 begin fun ((p, x), cstr) ty ->
            if with_checks
            then match cstr with
            | None -> ()
            | Some cstr ->
                let cstr = snd (Inst.instantiate subst env cstr) in
                ignore (Type.sub_type p Reason.URnone env cstr ty)
            else ()
          end class_.tc_tparams paraml;
          let sub_implements =
            SMap.map
              (fun ty -> snd (Inst.instantiate subst env ty))
              class_.tc_ancestors
          in
          let sub_dimplements =
            SMap.map
              (fun ty -> snd (Inst.instantiate subst env ty))
              class_.tc_ancestors_checked_when_concrete
          in
          let this_ty = (fst this, Tgeneric ("this", Some this)) in
          let env, ht = Inst.instantiate_this env ht this_ty in
          env, (SMap.add c ht sub_implements, sub_dimplements)
      )
  | _ ->
      (* The class lives in PHP land *)
      env, (SMap.empty, SMap.empty)

(* In order to type-check a class, we need to know what "parent"
 * refers to. Sometimes people write "parent::", when that happens,
 * we need to know the type of parent.
 *)
and class_def_parent env class_def class_type =
  match class_def.c_extends with
  | (_, Happly ((_, x), _) as parent_ty) :: _ ->
      let env, parent_type = Env.get_class_dep env x in
      (match parent_type with
      | Some parent_type ->
          check_parent env class_def class_type parent_type
      | None -> ());
      let env, parent_ty = Typing_hint.hint env parent_ty in
      env, parent_ty
  (* The only case where we have more than one parent class is when
   * dealing with interfaces and interfaces cannot use parent.
   *)
  | _ :: _
  | _ -> env, (Reason.Rnone, Tany)

and check_parent env class_def class_type parent_type =
  let position = fst class_def.c_name in
  (* Are all the parents in Hack? Do we know all their methods?
   * If so, let's check that the abstract methods have been implemented.
   *)
  if class_type.tc_members_fully_known
  then check_parent_abstract position parent_type class_type;
  if parent_type.tc_final
  then error position "You cannot extend a class declared as final"
  else ()

and check_parent_abstract position parent_type class_type =
  if parent_type.tc_kind = Ast.Cabstract &&
    class_type.tc_kind <> Ast.Cabstract &&
    not !is_silent_mode
  then begin
    check_extend_abstract position class_type.tc_methods;
    check_extend_abstract position class_type.tc_smethods;
  end
  else ()

and class_def env_up _ c =
  try
    if c.c_mode = Ast.Mdecl
    then ()
    else begin
      if not !auto_complete && not !is_silent_mode
      then begin
        Env.prefetch (snd c.c_name);
        NastCheck.class_ env_up c;
        NastInitCheck.class_ env_up c;
      end;
      let env_tmp = Env.set_root env_up (Dep.Class (snd c.c_name)) in
      let _, tc = Env.get_class env_tmp (snd c.c_name) in
      match tc with
      | None ->
          (* This can happen if there was an error during the declaration
           * of the class.
           *)
          ()
      | Some tc -> try class_def_ env_up c tc with Ignore -> ()
    end
  with Ignore -> ()

and get_self_from_c env c =
  let env, tparams = lfold type_param env c.c_tparams in
  let tparams = List.map begin fun ((p, s), param) ->
    Reason.Rwitness p, Tgeneric (s, param)
  end tparams in
  let ret = Reason.Rwitness (fst c.c_name), Tapply (c.c_name, tparams)in
  ret

and class_def_ env_up c tc =
  let env = Env.set_self_id env_up (snd c.c_name) in
  let env = Env.set_mode env c.c_mode in
  let env = Env.set_root env (Dep.Class (snd c.c_name)) in
  let pc, cid = c.c_name in
  let impl = c.c_extends @ c.c_implements @ c.c_uses in
  let self = get_self_from_c env c in
  let env, impl_dimpl =
    lfold (get_implements ~with_checks:true ~this:self) env impl in
  let _, dimpl = List.split impl_dimpl in
  let dimpl = List.fold_right (SMap.fold SMap.add) dimpl SMap.empty in
  let env, parent = class_def_parent env c tc in
  if not !is_silent_mode && tc.tc_kind = Ast.Cnormal && tc.tc_members_fully_known
  then begin
    check_extend_abstract pc tc.tc_methods;
    check_extend_abstract pc tc.tc_smethods;
  end;
  let env = Env.set_self env self in
  let env = Env.set_parent env parent in
  if tc.tc_final && c.c_kind <> Ast.Cnormal
  then begin
    let pos = fst c.c_name in
    match c.c_kind with
    | Ast.Cinterface -> error pos "Interfaces cannot be final"
    | Ast.Cabstract -> error pos "Abstract classes cannot be final"
    | Ast.Ctrait -> error pos "Traits cannot be final"
    | Ast.Cnormal -> assert false
  end;
  List.iter (class_implements env c) impl;
  SMap.iter (fun _ ty -> class_implements_type env c ty) dimpl;
  List.iter (class_var_def env false c) c.c_vars;
  List.iter (method_def env) c.c_methods;
  List.iter (class_const_def env) c.c_consts;
  class_constr_def env c;
  let env = Env.set_static env in
  List.iter (class_var_def env true c) c.c_static_vars;
  List.iter (method_def env) c.c_static_methods;
  if DynamicYield.contains_dynamic_yield tc.tc_extends
  then DynamicYield.check_yield_visibility env c

and check_extend_abstract p smap =
  SMap.iter begin fun x ce ->
    match ce.ce_type with
    | r, Tfun { ft_abstract = true; _ } ->
        error_l [
          p,
          "This class must provide an implementation for the abstract method "^x;
          Reason.to_pos r,
          "The abstract method "^x^" is defined here";
        ]
    | _ -> ()
  end smap

and class_const_def env (h, id, e) =
  let env, ty =
    match h with
    | None -> env, Env.fresh_type()
    | Some h -> Typing_hint.hint env h
  in
  let env, ty' = expr env e in
  ignore (Type.sub_type (fst id) Reason.URhint env ty ty')

and class_constr_def env c =
  match c.c_constructor with
  | None -> ()
  | Some m -> method_def env m

and class_implements env c1 h =
  let env, ctype2 = Typing_hint.hint env h in
  class_implements_type env c1 ctype2

and class_implements_type env c1 ctype2 =
  let env, params = lfold begin fun env ((_, s), param) ->
    let env, param = opt Typing_hint.hint env param in
    env, (Reason.Rnone, Tgeneric (s, param))
  end env c1.c_tparams
  in
  let r = Reason.Rwitness (fst (c1.c_name)) in
  let ctype1 = r, Tapply (c1.c_name, params) in
  Typing_extends.check_implements env ctype2 ctype1;
  ()

and class_var_def env is_static c cv =
  let env, ty =
    match cv.cv_expr with
    | None -> env, Env.fresh_type()
    | Some e -> expr env e in
  match cv.cv_type with
  | None when Env.is_strict env ->
      error (fst cv.cv_id) "Please add a type"
  | None ->
      let pos, name = cv.cv_id in
      let name = if is_static then "$"^name else name in
      let var_type = Reason.Rwitness pos, Tany in
      (match cv.cv_expr with
      | None ->
          Typing_suggest.uninitalized_member (snd c.c_name) name env var_type ty;
          ()
      | Some _ ->
          Typing_suggest.save_member name env var_type ty;
          ()
      )
  | Some (p, _ as cty) ->
      let env, cty = Typing_hint.hint env cty in
      let _ = Type.sub_type p Reason.URhint env cty ty in
      ()

and method_def env m =
  let env = { env with Env.lenv = Env.empty_local } in
  let env = Env.set_local env this (Env.get_self env) in
  let env, ret =
    match m.m_ret with
    | None -> env, (Reason.Rwitness (fst m.m_name), Tany)
    | Some ret -> Typing_hint.hint env ret in
  let env = DynamicYield.method_def env m.m_name ret in
  let env, params = lfold (make_param_type Env.fresh_type) env m.m_params in
  if Env.is_strict env then begin
    List.iter2 (check_param env) m.m_params params;
  end;
  let env  = List.fold_left2 bind_param env params m.m_params in
  let env = fun_ ~abstract:m.m_abstract env m.m_unsafe (m.m_ret <> None)
      ret (fst m.m_name) m.m_body m.m_type in
  let env = List.fold_left (fun env f -> f env) env (Env.get_todo env) in
  match m.m_ret with
  | None when Env.is_strict env && snd m.m_name <> "__destruct" ->
      (* if we are in strict mode, the only case where we don't want to enforce
       * a return type is when the method is a destructor
       *)
      suggest_return env (fst m.m_name) ret
  | None
  | Some _ -> ()
