(**
 * Copyright (c) 2015, Facebook, Inc.
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
open Autocomplete
open Core
open Decl_defs
open Nast
open Typing_defs
open Utils

module TUtils       = Typing_utils
module Reason       = Typing_reason
module Inst         = Decl_instantiate
module Type         = Typing_ops
module Env          = Typing_env
module LEnv         = Typing_lenv
module Dep          = Typing_deps.Dep
module Async        = Typing_async
module SubType      = Typing_subtype
module Unify        = Typing_unify
module TGen         = Typing_generic
module SN           = Naming_special_names
module TAccess      = Typing_taccess
module TI           = Typing_instantiability
module TVis         = Typing_visibility
module TNBody       = Typing_naming_body
module TS           = Typing_structure
module T            = Tast
module Phase        = Typing_phase
module Subst        = Decl_subst
module ExprDepTy    = Typing_dependent_type.ExprDepTy
module Conts        = Typing_continuations
module TCO          = TypecheckerOptions

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

(* A guess as to the last position we were typechecking, for use in debugging,
 * such as figuring out what a runaway hh_server thread is doing. Updated
 * only best-effort -- it's an approximation to point debugging in the right
 * direction, nothing more. *)
let debug_last_pos = ref Pos.none
let debug_print_last_pos _ = print_endline (Pos.string (Pos.to_absolute
  !debug_last_pos))

(****************************************************************************)
(* Hooks *)
(****************************************************************************)

let expr_hook = ref None

let with_expr_hook hook f = with_context
  ~enter: (fun () -> expr_hook := Some hook)
  ~exit: (fun () -> expr_hook := None)
  ~do_: f

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let suggest env p ty =
  let ty = Typing_expand.fully_expand env ty in
  (match Typing_print.suggest ty with
  | "..." -> Errors.expecting_type_hint p
  | ty -> Errors.expecting_type_hint_suggest p ty
  )

let suggest_return env p ty =
  let ty = Typing_expand.fully_expand env ty in
  (match Typing_print.suggest ty with
  | "..." -> Errors.expecting_return_type_hint p
  | ty -> Errors.expecting_return_type_hint_suggest p ty
  )

let err_witness p = Reason.Rwitness p, Terr
let err_none = Reason.Rnone, Terr

let expr_error env r =
  env, T.make_implicitly_typed_expr Pos.none T.Any, (r, Terr)

let expr_any env r =
  env, T.make_implicitly_typed_expr Pos.none T.Any, (r, Tany)

let compare_field_kinds x y =
  match x, y with
  | Nast.AFvalue (p1, _), Nast.AFkvalue ((p2, _), _)
  | Nast.AFkvalue ((p2, _), _), Nast.AFvalue (p1, _) ->
      Errors.field_kinds p1 p2;
      false
  | _ ->
      true

let check_consistent_fields x l =
  List.for_all l (compare_field_kinds x)

let unbound_name env (pos, name) =
  match Env.get_mode env with
  | FileInfo.Mstrict ->
    (Errors.unbound_name_typing pos name;
    expr_error env Reason.Rnone)

  | FileInfo.Mdecl | FileInfo.Mpartial | FileInfo.Mphp ->
    expr_any env Reason.Rnone

(* Try running function on each concrete supertype in turn. Return all
 * successful results
 *)
let try_over_concrete_supertypes env ty f =
  let env, tyl = TUtils.get_concrete_supertypes env ty in
  (* If there is just a single result then don't swallow errors *)
  match tyl with
  | [ty] -> [f env ty]
  | _ ->
    let rec iter_over_types env resl tyl =
      match tyl with
        [] -> resl
      | ty::tyl ->
        Errors.try_
          (fun () -> iter_over_types env (f env ty::resl) tyl)
          (fun _ -> iter_over_types env resl tyl) in
  iter_over_types env [] tyl

(*****************************************************************************)
(* Handling function/method arguments *)
(*****************************************************************************)

let rec wfold_left_default f (env, def1) l1 l2 =
  match l1, def1, l2 with
  | _, _, [] -> env
  | [], None, _ -> env
  | [], Some d1, x2 :: rl2 ->
    let env = f env d1 x2 in
    wfold_left_default f (env, def1) [] rl2
  | x1 :: rl1, _, x2 :: rl2 ->
    let env = f env x1 x2 in
    wfold_left_default f (env, def1) rl1 rl2

let rec check_memoizable env param ty =
  let env, ty = Env.expand_type env ty in
  let p = param.param_pos in
  match ty with
  | _, (Tprim (Tarraykey | Tbool | Tint | Tfloat | Tstring | Tnum)
       | Tmixed | Tany | Terr) ->
    ()
  | _, Tprim (Tvoid | Tresource | Tnoreturn) ->
    let ty_str = Typing_print.error (snd ty) in
    let msgl = Reason.to_string ("This is "^ty_str) (fst ty) in
    Errors.invalid_memoized_param p msgl
  | _, Toption ty ->
    check_memoizable env param ty
  | _, Tshape (_, fdm) ->
    ShapeMap.iter begin fun name _ ->
      match ShapeMap.get name fdm with
        | Some { sft_ty; _ } -> check_memoizable env param sft_ty
        | None ->
            let ty_str = Typing_print.error (snd ty) in
            let msgl = Reason.to_string ("This is "^ty_str) (fst ty) in
            Errors.invalid_memoized_param p msgl;
    end fdm
  | _, Ttuple tyl ->
    List.iter tyl begin fun ty ->
      check_memoizable env param ty
    end
  | _, Tabstract (AKenum _, _) ->
    ()
  | _, Tabstract (AKnewtype (_, _), _) ->
    let env, t', _ =
      let ety_env = Phase.env_with_self env in
      Typing_tdef.force_expand_typedef ~ety_env env ty in
    check_memoizable env param t'
  (* Just accept all generic types for now. Stricter checks to come later. *)
  | _, Tabstract (AKgeneric _, _) ->
    ()
  (* For parameter type 'this::TID' defined by 'type const TID as Bar' checks
   * Bar recursively.
   *)
  | _, Tabstract (AKdependent _, Some ty) ->
    check_memoizable env param ty
  (* Allow unconstrined dependent type `abstract type const TID` just as we
   * allow unconstrained generics. *)
  | _, Tabstract (AKdependent _, None) ->
    ()
  (* Handling Tunresolved case here for completeness, even though it
   * shouldn't be possible to have an unresolved type when checking
   * the method declaration. No corresponding test case for this.
   *)
  | _, Tunresolved tyl ->
    List.iter tyl begin fun ty ->
      check_memoizable env param ty
    end
  (* Allow untyped arrays. *)
  | _, Tarraykind AKany
  | _, Tarraykind AKempty ->
      ()
  | _,
    Tarraykind (
      AKvarray ty
      | AKvec ty
      | AKdarray(_, ty)
      | AKvarray_or_darray ty
      | AKmap(_, ty)
    ) ->
      check_memoizable env param ty
  | _, Tarraykind (AKshape fdm) ->
      ShapeMap.iter begin fun _ (_, tv) ->
        check_memoizable env param tv
      end fdm
  | _, Tarraykind (AKtuple fields) ->
      IMap.iter begin fun _ tv ->
        check_memoizable env param tv
      end fields
  | _, Tclass (_, _) ->
    let type_param = Env.fresh_type() in
    let container_type =
      Reason.none,
      Tclass ((Pos.none, SN.Collections.cContainer), [type_param]) in
    let env, is_container =
      Errors.try_
        (fun () ->
          SubType.sub_type env ty container_type, true)
        (fun _ -> env, false) in
    if is_container then
      check_memoizable env param type_param
    else
      let r, _ = ty in
      let memoizable_type =
        r, Tclass ((Pos.none, SN.Classes.cIMemoizeParam), []) in
      if SubType.is_sub_type env ty memoizable_type
      then ()
      else
        let ty_str = Typing_print.error (snd ty) in
        let msgl = Reason.to_string ("This is "^ty_str) (fst ty) in
        Errors.invalid_memoized_param p msgl;
  | _, Tfun _
  | _, Tvar _
  | _, Tanon (_, _)
  | _, Tobject ->
    let ty_str = Typing_print.error (snd ty) in
    let msgl = Reason.to_string ("This is "^ty_str) (fst ty) in
    Errors.invalid_memoized_param p msgl

(* This function is used to determine the type of an argument.
 * When we want to type-check the body of a function, we need to
 * introduce the type of the arguments of the function in the environment
 * Let's take an example, we want to check the code of foo:
 *
 * function foo(int $x): int {
 *   // CALL TO make_param_type on (int $x)
 *   // Now we know that the type of $x is int
 *
 *   return $x; // in the environment $x is an int, the code is correct
 * }
 *
 * When we localize, we want to resolve to "static" or "$this" depending on
 * the context. Even though we are passing in CIstatic, resolve_with_class_id
 * is smart enough to know what to do. Why do this? Consider the following
 *
 * abstract class C {
 *   abstract const type T;
 *
 *   private this::T $val;
 *
 *   final public function __construct(this::T $x) {
 *     $this->val = $x;
 *   }
 *
 *   public static function create(this::T $x): this {
 *     return new static($x);
 *   }
 * }
 *
 * class D extends C { const type T = int; }
 *
 * In __construct() we want to be able to assign $x to $this->val. The type of
 * $this->val will expand to '$this::T', so we need $x to also be '$this::T'.
 * We can do this soundly because when we construct a new class such as,
 * 'new D(0)' we can determine the late static bound type (D) and resolve
 * 'this::T' to 'D::T' which is int.
 *
 * A similar line of reasoning is applied for the static method create.
 *)
let make_param_local_ty env param =
  let ety_env =
    { (Phase.env_with_self env) with from_class = Some CIstatic; } in
  let env, ty =
    match param.param_hint with
    | None ->
      (* if the type is missing, use an unbound type variable *)
      let _r, ty = Env.fresh_type () in
      let r = Reason.Rwitness param.param_pos in
      env, (r, ty)
    | Some x ->
      let ty = Decl_hint.hint env.Env.decl_env x in
      Phase.localize ~ety_env env ty
  in
  let ty = match ty with
    | _, t when param.param_is_variadic ->
      (* when checking the body of a function with a variadic
       * argument, "f(C ...$args)", $args is an array<C> *)
      let r = Reason.Rvar_param param.param_pos in
      let arr_values = r, t in
      r, Tarraykind (AKvec arr_values)
    | x -> x
  in
  Typing_hooks.dispatch_infer_ty_hook ty param.param_pos env;
  env, ty

(* Given a localized parameter type and parameter information, infer
 * a type for the parameter default expression (if present) and check that
 * it is a subtype of the parameter type. Set the type of the parameter in
 * the locals environment *)
let rec bind_param env (ty1, param) =
  let env, param_te, ty2 =
    match param.param_expr with
    | None ->
        env, None, (Reason.none, Tany)
    | Some e ->
        let env, te, ty = expr env e in
        Typing_sequencing.sequence_check_expr e;
        env, Some te, ty
  in
  Typing_suggest.save_param (param.param_name) env ty1 ty2;
  let env = Type.sub_type param.param_pos Reason.URhint env ty2 ty1 in
  let tparam = {
    T.param_hint = param.param_hint;
    T.param_is_reference = param.param_is_reference;
    T.param_is_variadic = param.param_is_variadic;
    T.param_pos = param.param_pos;
    T.param_name = param.param_name;
    T.param_expr = param_te;
  } in
  Env.set_local env (Local_id.get param.param_name) ty1, tparam

(* In strict mode, we force you to give a type declaration on a parameter *)
(* But the type checker is nice: it makes a suggestion :-) *)
and check_param env param ty =
  match param.param_hint with
  | None -> suggest env param.param_pos ty
  | Some _ -> ()

(*****************************************************************************)
(* Now we are actually checking stuff! *)
(*****************************************************************************)
and fun_def tcopt f =
  (* reset the expression dependent display ids for each function body *)
  Reason.expr_display_id_map := IMap.empty;
  Typing_hooks.dispatch_enter_fun_def_hook f;
  let nb = TNBody.func_body tcopt f in
  let dep = Typing_deps.Dep.Fun (snd f.f_name) in
  let env = Env.empty tcopt (Pos.filename (fst f.f_name)) (Some dep) in
  NastCheck.fun_ env f nb;
  (* Fresh type environment is actually unnecessary, but I prefer to
   * have a guarantee that we are using a clean typing environment. *)
  let tfun_def, tenv = Env.fresh_tenv env (
    fun env ->
      let env = Env.set_mode env f.f_mode in
      let env, constraints =
        Phase.localize_generic_parameters_with_bounds env f.f_tparams
                  ~ety_env:(Phase.env_with_self env) in
      let env = add_constraints (fst f.f_name) env constraints in
      let env =
        localize_where_constraints
          ~ety_env:(Phase.env_with_self env) env f.f_where_constraints in
      let env, hret =
        match f.f_ret with
        | None -> env, (Reason.Rwitness (fst f.f_name), Tany)
        | Some ret ->
          let ty = TI.instantiable_hint env ret in
          Phase.localize_with_self env ty
      in
      TI.check_params_instantiable env f.f_params;
      TI.check_tparams_instantiable env f.f_tparams;
      let env, param_tys = List.map_env env f.f_params make_param_local_ty in
      let env, typed_params = List.map_env env (List.zip_exn param_tys f.f_params)
        bind_param in
      let env, t_variadic, v_ty = match f.f_variadic with
        | FVvariadicArg vparam ->
          TI.check_param_instantiable env vparam;
          let env, ty = make_param_local_ty env vparam in
          let env, t_vparam = bind_param env (ty, vparam) in
          env, T.FVvariadicArg t_vparam, Some ty
        | FVellipsis -> env, T.FVellipsis, None
        | FVnonVariadic -> env, T.FVnonVariadic, None in
      let env, tb = fun_ env hret (fst f.f_name) nb f.f_fun_kind in
      let env = Env.check_todo env in
      if Env.is_strict env then begin
        List.iter2_exn f.f_params param_tys (check_param env);
        match f.f_variadic, v_ty with
          | FVvariadicArg vparam, Some ty -> check_param env vparam ty;
          | _ -> ()
      end;
      begin match f.f_ret with
        | None when Env.is_strict env -> suggest_return env (fst f.f_name) hret
        | None -> Typing_suggest.save_fun_or_method f.f_name
        | Some _ -> ()
      end;
      {
        T.f_mode = f.f_mode;
        T.f_ret = f.f_ret;
        T.f_name = f.f_name;
        T.f_tparams = f.f_tparams;
        T.f_where_constraints = f.f_where_constraints;
        T.f_variadic = t_variadic;
        T.f_params = typed_params;
        T.f_fun_kind = f.f_fun_kind;
        T.f_user_attributes = List.map f.f_user_attributes (user_attribute env);
        T.f_body = T.NamedBody {
          T.fnb_nast = tb;
          T.fnb_unsafe = nb.fnb_unsafe;
        }
      },
      env
  ) in
  Typing_hooks.dispatch_exit_fun_def_hook f;
  tfun_def, tenv

(*****************************************************************************)
(* function used to type closures, functions and methods *)
(*****************************************************************************)

and fun_ ?(abstract=false) env hret pos named_body f_kind =
  Env.with_return env begin fun env ->
    debug_last_pos := pos;
    let env = Env.set_return env hret in
    let env = Env.set_fn_kind env f_kind in
    let env, tb = block env named_body.fnb_nast in
    Typing_sequencing.sequence_check_block named_body.fnb_nast;
    let ret = Env.get_return env in
    let env =
      if Nast_terminality.Terminal.block env named_body.fnb_nast ||
        abstract ||
        named_body.fnb_unsafe ||
        !auto_complete
      then env
      else fun_implicit_return env pos ret named_body.fnb_nast f_kind in
    debug_last_pos := Pos.none;
    env, tb
  end

and fun_implicit_return env pos ret _b = function
  | Ast.FGenerator | Ast.FAsyncGenerator -> env
  | Ast.FSync ->
    (* A function without a terminal block has an implicit return; the
     * "void" type *)
    let rty = Reason.Rno_return pos, Tprim Nast.Tvoid in
    Typing_suggest.save_return env ret rty;
    Type.sub_type pos Reason.URreturn env rty ret
  | Ast.FAsync ->
    (* An async function without a terminal block has an implicit return;
     * the Awaitable<void> type *)
    let r = Reason.Rno_return_async pos in
    let rty = r, Tclass ((pos, SN.Classes.cAwaitable), [r, Tprim Nast.Tvoid]) in
    Typing_suggest.save_return env ret rty;
    Type.sub_type pos Reason.URreturn env rty ret

and block env stl =
  List.map_env env stl stmt

and stmt env = function
  | Fallthrough ->
      env, T.Fallthrough
  | GotoLabel _
  | Goto _
  | Noop ->
      env, T.Noop
  | Expr e ->
      let env, te, ty = expr env e in
      (* NB: this check does belong here and not in expr, even though it only
       * applies to expressions -- we actually want to perform the check on
       * statements that are expressions, e.g., "foo();" we want to check, but
       * "return foo();" we do not even though the expression "foo()" is a
       * subexpression of the statement "return foo();". *)
       (match snd e with
         | Nast.Binop (Ast.Eq _, _, _) -> ()
         | _ -> Async.enforce_not_awaitable env (fst e) ty);
      env, T.Expr te
  | If (e, b1, b2)  ->
      let env, te, _ = expr env e in
      (* We stash away the locals environment because condition updates it
       * locally for checking b1. For example, we might have condition
       * $x === null, or $x instanceof C, which changes the type of $x in
       * lenv *)
      let parent_lenv = env.Env.lenv in
      let env   = condition env true e in
      let env, tb1 = block env b1 in
      let lenv1 = env.Env.lenv in
      let env   = { env with Env.lenv = parent_lenv } in
      let env   = condition env false e in
      let env, tb2 = block env b2 in
      let lenv2 = env.Env.lenv in
      let terminal1 = Nast_terminality.Terminal.block env b1 in
      let terminal2 = Nast_terminality.Terminal.block env b2 in
      let env =
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
        else LEnv.intersect env parent_lenv lenv1 lenv2 in
      (* TODO TAST: annotate with joined types *)
      env, T.If(te, tb1, tb2)
  | Return (p, None) ->
      let rty = match Env.get_fn_kind env with
        | Ast.FSync -> (Reason.Rwitness p, Tprim Tvoid)
        | Ast.FGenerator
        (* Return type checked against the "yield". *)
        | Ast.FAsyncGenerator -> (Reason.Rnone, Tany)
        | Ast.FAsync -> (Reason.Rwitness p,
            Tclass ((p, SN.Classes.cAwaitable),
            [(Reason.Rwitness p, Tprim Tvoid)])) in
      let expected_return = Env.get_return env in
      Typing_suggest.save_return env expected_return rty;
      let env = Type.sub_type p Reason.URreturn env rty expected_return in
      env, T.Return (p, None)
  | Return (p, Some e) ->
      let pos = fst e in
      let env, te, rty = expr env e in
      let rty = match Env.get_fn_kind env with
        | Ast.FSync -> rty
        | Ast.FGenerator
        (* Is an error, but caught in NastCheck. *)
        | Ast.FAsyncGenerator -> (Reason.Rnone, Terr)
        | Ast.FAsync ->
          (Reason.Rwitness p), Tclass ((p, SN.Classes.cAwaitable), [rty]) in
      let expected_return = Env.get_return env in
      (match snd (Env.expand_type env expected_return) with
      | r, Tprim Tvoid ->
          (* Yell about returning a value from a void function. This catches
           * more issues than just unifying with void would do -- in particular
           * just unifying allows you to return a Tany from a void function,
           * which is clearly wrong. Note this check is best-effort; if the
           * function returns a generic type which later ends up being Tvoid
           * then there's not much we can do here. *)
          Errors.return_in_void p (Reason.to_pos r);
          env, T.Return(p, Some te)
      | _, Tunresolved _ ->
          (* we allow return types to grow for anonymous functions *)
          let env, rty = TUtils.unresolved env rty in
          let env = Type.sub_type pos Reason.URreturn env rty expected_return in
          env, T.Return(p, Some te)
      | _, (Terr | Tany | Tmixed | Tarraykind _ | Toption _ | Tprim _
        | Tvar _ | Tfun _ | Tabstract (_, _) | Tclass (_, _) | Ttuple _
        | Tanon (_, _) | Tobject | Tshape _) ->
          Typing_suggest.save_return env expected_return rty;
          let env = Type.sub_type pos Reason.URreturn env rty expected_return in
          env, T.Return(p, Some te)
      )
  | Do (b, e) as st ->
      (* NOTE: leaks scope as currently implemented; this matches
         the behavior in naming (cf. `do_stmt` in naming/naming.ml).
       *)
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let env, _ = block env b in
      let env, te, _ = expr env e in
      let after_block = env.Env.lenv in
      let alias_depth =
        if env.Env.in_loop then 1 else Typing_alias.get_depth st in
      let env, tb = Env.in_loop env begin
          iter_n_acc alias_depth begin fun env ->
            let env = condition env true e in
            let env, tb = block env b in
            env, tb
          end end in
      let env =
        if Nast.Visitor.HasContinue.block b
        then LEnv.fully_integrate env parent_lenv
        else
          let env = LEnv.integrate env parent_lenv env.Env.lenv in
          let env = { env with Env.lenv = after_block } in
          env in
      let env = condition env false e in
      env, T.Do(tb, te)
  | While (e, b) as st ->
      let env, te, _ = expr env e in
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let alias_depth =
        if env.Env.in_loop then 1 else Typing_alias.get_depth st in
      let env, tb = Env.in_loop env begin
        iter_n_acc alias_depth begin fun env ->
          let env = condition env true e in
          (* TODO TAST: avoid repeated generation of block *)
          let env, tb = block env b in
          env, tb
        end
      end in
      let env = LEnv.fully_integrate env parent_lenv in
      let env = condition env false e in
      env, T.While (te, tb)
  | For (e1, e2, e3, b) as st ->
      (* For loops leak their initalizer, but nothing that's defined in the
         body
       *)
      let (env, te1, _) = expr env e1 in      (* initializer *)
      let (env, te2, _) = expr env e2 in
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let alias_depth =
        if env.Env.in_loop then 1 else Typing_alias.get_depth st in
      let env, (tb, te3) = Env.in_loop env begin
        iter_n_acc alias_depth begin fun env ->
          let env = condition env true e2 in (* iteration 0 *)
          let env, tb = block env b in
          let (env, te3, _) = expr env e3 in
          env, (tb, te3)
        end
      end in
      let env = LEnv.fully_integrate env parent_lenv in
      let env = condition env false e2 in
      env, T.For(te1, te2, te3, tb)
  | Switch (e, cl) ->
      let cl = List.map ~f:drop_dead_code_after_break cl in
      Nast_terminality.SafeCase.check (fst e) env cl;
      let env, te, ty = expr env e in
      Async.enforce_not_awaitable env (fst e) ty;
      let env = check_exhaustiveness env (fst e) ty cl in
      let parent_lenv = env.Env.lenv in
      let env, cl, tcl = case_list parent_lenv ty env cl in
      let env = LEnv.intersect_list env parent_lenv cl in
      env, T.Switch(te, tcl)
  | Foreach (e1, e2, b) as st ->
      let env, te1, ty1 = expr env e1 in
      let parent_lenv = env.Env.lenv in
      let env = Env.freeze_local_env env in
      let env, ty2 = as_expr env (fst e1) e2 in
      let env = Type.sub_type (fst e1) Reason.URforeach env ty1 ty2 in
      let alias_depth =
        if env.Env.in_loop then 1 else Typing_alias.get_depth st in
      let env, (te2, tb) = Env.in_loop env begin
        iter_n_acc alias_depth begin fun env ->
          let env, te2 = bind_as_expr env ty2 e2 in
          let env, tb = block env b in
          env, (te2, tb)
        end
      end in
      let env = LEnv.fully_integrate env parent_lenv in
      env, T.Foreach (te1, te2, tb)
  | Try (tb, cl, fb) ->
    let env, ttb, tcl = try_catch env tb cl in
    let env, tfb = block env fb in
    env, T.Try (ttb, tcl, tfb)
  | Static_var el ->
    let env = List.fold_left el ~f:begin fun env e ->
      match e with
        | _, Binop (Ast.Eq _, (_, Lvar (p, x)), _) ->
          Env.add_todo env (TGen.no_generic p x)
        | _ -> env
    end ~init:env in
    let env, tel, _ = exprs env el in
    env, T.Static_var tel
  | Global_var el ->
    let env = List.fold_left el ~f:begin fun env e ->
      match e with
        | _, Binop (Ast.Eq _, (_, Lvar (p, x)), _) ->
          Env.add_todo env (TGen.no_generic p x)
        | _ -> env
    end ~init:env in
    let env, tel, _ = exprs env el in
    env, T.Global_var tel
  | Throw (is_terminal, e) ->
    let p = fst e in
    let env, te, ty = expr env e in
    let env = exception_ty p env ty in
    env, T.Throw(is_terminal, te)
  | Continue p ->
    env, T.Continue p
  | Break p ->
    env, T.Break p

and check_exhaustiveness env pos ty caselist =
  check_exhaustiveness_ env pos ty caselist false

and check_exhaustiveness_ env pos ty caselist enum_coming_from_unresolved =
  (* Right now we only do exhaustiveness checking for enums. *)
  (* This function has a built in hack where if Tunresolved has an enum
     inside then it tells the enum exhaustiveness checker to
     not punish for extra default *)
  let env, (_, ty) = Env.expand_type env ty in
  match ty with
    | Tunresolved tyl ->
      let new_enum = enum_coming_from_unresolved ||
        (List.length tyl> 1 && List.exists tyl ~f:begin fun cur_ty ->
        let _, (_, cur_ty) = Env.expand_type env cur_ty in
        match cur_ty with
          | Tabstract (AKenum _, _) -> true
          | _ -> false
      end) in
      List.fold_left tyl ~init:env ~f:begin fun env ty ->
        check_exhaustiveness_ env pos ty caselist new_enum
      end
    | Tabstract (AKenum id, _) ->
      let tc = unsafe_opt @@ Env.get_enum env id in
      Typing_enum.check_enum_exhaustiveness pos tc
        caselist enum_coming_from_unresolved;
      env
    | Terr | Tany | Tmixed | Tarraykind _ | Tclass _ | Toption _ | Tprim _
    | Tvar _ | Tfun _ | Tabstract (_, _) | Ttuple _ | Tanon (_, _)
    | Tobject | Tshape _ -> env

and case_list parent_lenv ty env cl =
  let env = { env with Env.lenv = parent_lenv } in
  case_list_ parent_lenv ty env cl

and try_catch env tb cl =
  let parent_lenv = env.Env.lenv in
  let env = Env.freeze_local_env env in
  let env, ttb = block env tb in
  let after_try = env.Env.lenv in
  let env, term_lenv_tcb_l = List.map_env env cl
    begin fun env (_, _, b as catch_block) ->
      let env, lenv, tcb = catch parent_lenv after_try env catch_block in
      let term = Nast_terminality.Terminal.block env b in
      env, (term, lenv, tcb)
    end in
  let term_lenv_l = List.map term_lenv_tcb_l (fun (a, b, _) -> (a, b)) in
  let tcb_l = List.map term_lenv_tcb_l (fun (_, _, a) -> a) in
  let term_lenv_l =
    (Nast_terminality.Terminal.block env tb, after_try) :: term_lenv_l in
  let env = LEnv.intersect_list env parent_lenv term_lenv_l in
  env, ttb, tcb_l

and drop_dead_code_after_break_block = function
  | [] -> [], false
  | Break x :: _ -> [Break x], true
  | x :: rest ->
    let x', drop =
      match x with
      | If (_, [], []) as if_stmt -> if_stmt, false
      | If (cond, b1, b2) ->
        let b1, drop1 = if b1 = [] then [], true
                        else drop_dead_code_after_break_block b1 in
        let b2, drop2 = if b2 = [] then [], true
                        else drop_dead_code_after_break_block b2 in
        If (cond, b1, b2), drop1 && drop2
      | x -> x, false
    in
    if drop then ([x'], true) else begin
      let rest', drop = drop_dead_code_after_break_block rest in
      x'::rest', drop
    end

and drop_dead_code_after_break = function
  | Default b -> Default (fst (drop_dead_code_after_break_block b))
  | Case (e, b) -> Case (e, fst (drop_dead_code_after_break_block b))

and case_list_ parent_lenv ty env = function
  | [] -> env, [], []
  | Default b :: _ ->
      (* TODO this is wrong, should continue on to the other cases, but it
       * doesn't matter in practice since our parser won't parse default
       * anywhere but in the last position :) Should fix all of this as well
       * as totality detection for switch. *)
    let env, tb = block env b in
    env, [Nast_terminality.Terminal.case env (Default b), env.Env.lenv],
      [T.Default tb]
  | (Case (e, b)) as ce :: rl ->
    (* TODO - we should consider handling the comparisons the same
     * way as Binop Ast.EqEq, since case statements work using ==
     * comparison rules *)

    (* The way we handle terminal/nonterminal here is not quite right, you
     * can still break the type system with things like P3131824. *)
    let ty_num = (Reason.Rnone, Tprim Nast.Tnum) in
    let ty_arraykey = (Reason.Rnone, Tprim Nast.Tarraykey) in
    let both_are_sub_types env tprim ty1 ty2 =
      (SubType.is_sub_type env ty1 tprim) &&
      (SubType.is_sub_type env ty2 tprim) in
    let env, te, ty2 = expr env e in
    let env, _ = if (both_are_sub_types env ty_num ty ty2) ||
                    (both_are_sub_types env ty_arraykey ty ty2)
                 then env, ty
                 else Type.unify (fst e) Reason.URnone env ty ty2 in

    if Nast_terminality.Terminal.block env b then
      let env, tb = block env b in
      let lenv = env.Env.lenv in
      let env, rl, tcl = case_list parent_lenv ty env rl in
      env, (Nast_terminality.Terminal.case env ce, lenv) :: rl, T.Case (te, tb)::tcl
    else
      (* Since this block is not terminal we will end up falling through to the
       * next block. This means the lenv will include what our current
       * environment is, intersected (or integrated?) with the environment
       * after executing the block. Example:
       *
       *  $x = 0; // $x = int
       *  switch (0) {
       *    case 1:
       *      $x = ''; // $x = string
       *      // FALLTHROUGH
       *    case 2:
       *      $x; // $x = int & string
       *    ...
       *)
      let lenv1 = env.Env.lenv in
      let env, tb = block env b in
      (* PERF: If the case is empty or a Noop then we do not need to intersect
       * the lenv since they will be the same.
       *
       * This saves the cost of intersecting the lenv for the common pattern of
       *   case 1:
       *   case 2:
       *   case 3:
       *   ...
       *)
      let env = match b with
        | [] | [Noop] -> env
        | _ -> LEnv.intersect env parent_lenv lenv1 env.Env.lenv in
      let env, rl, tcl = case_list_ parent_lenv ty env rl in
      env, rl, T.Case (te, tb)::tcl

and catch parent_lenv after_try env (sid, exn, b) =
  let env = { env with Env.lenv = after_try } in
  let env = LEnv.fully_integrate env parent_lenv in
  let cid = CI (sid, []) in
  let ety_p = (fst sid) in
  TUtils.process_class_id cid;
  let env, _, _ = instantiable_cid ety_p env cid in
  let env, _te, ety = static_class_id ety_p env cid in
  let env = exception_ty ety_p env ety in
  let env = Env.set_local env (snd exn) ety in
  let env, tb = block env b in
  (* Only keep the local bindings if this catch is non-terminal *)
  env, env.Env.lenv, (sid, exn, tb)

and as_expr env pe =
let make_result ty = env, ty in
function
  | As_v _ ->
      let ty = Env.fresh_type() in
      let tvector = Tclass ((pe, SN.Collections.cTraversable), [ty]) in
      make_result (Reason.Rforeach pe, tvector)
  | As_kv _ ->
      let ty1 = Env.fresh_type() in
      let ty2 = Env.fresh_type() in
      let tmap = Tclass((pe, SN.Collections.cKeyedTraversable), [ty1; ty2]) in
      make_result (Reason.Rforeach pe, tmap)
  | Await_as_v _ ->
      let ty = Env.fresh_type() in
      let tvector = Tclass ((pe, SN.Classes.cAsyncIterator), [ty]) in
      make_result (Reason.Rasyncforeach pe, tvector)
  | Await_as_kv _ ->
      let ty1 = Env.fresh_type() in
      let ty2 = Env.fresh_type() in
      let tmap = Tclass ((pe, SN.Classes.cAsyncKeyedIterator), [ty1; ty2]) in
      make_result (Reason.Rasyncforeach pe, tmap)

and bind_as_expr env ty aexpr =
  let env, ety = Env.expand_type env ty in
  let p, ty1, ty2 =
    match ety with
    | _, Tclass ((p, _), [ty2]) -> (p, (Reason.Rnone, Tmixed), ty2)
    | _, Tclass ((p, _), [ty1; ty2]) -> (p, ty1, ty2)
    | _ -> assert false in
  match aexpr with
    | As_v ev ->
      let env, te, _ = assign p env ev ty2 in
      env, T.As_v te
    | Await_as_v (p, ev) ->
      let env, te, _ = assign p env ev ty2 in
      env, T.Await_as_v(p, te)
    | As_kv ((p, Lvar ((_, k) as id)), ev) ->
      let env, ty1' = set_valid_rvalue p env k ty1 in
      let env, te, _ = assign p env ev ty2 in
      env, T.As_kv(T.make_typed_expr p ty1' (T.Lvar id), te)
    | Await_as_kv (p, (p1, Lvar ((_, k) as id)), ev) ->
      let env, ty1' = set_valid_rvalue p env k ty1 in
      let env, te, _ = assign p env ev ty2 in
      env, T.Await_as_kv(p, T.make_typed_expr p1 ty1' (T.Lvar id), te)
    | _ -> (* TODO Probably impossible, should check that *)
      assert false

and expr env e =
  raw_expr ~in_cond:false env e

and raw_expr ~in_cond ?lhs_of_null_coalesce ?valkind:(valkind=`other) env e =
  debug_last_pos := fst e;
  let env, te, ty = expr_ ~in_cond ?lhs_of_null_coalesce ~valkind env e in
  let () = match !expr_hook with
    | Some f -> f e (Typing_expand.fully_expand env ty)
    | None -> () in
  Typing_hooks.dispatch_infer_ty_hook ty (fst e) env;
  env, te, ty

and lvalue env e =
  let valkind = `lvalue in
  expr_ ~in_cond:false ~valkind env e

and is_pseudo_function s =
  s = SN.PseudoFunctions.hh_show ||
  s = SN.PseudoFunctions.hh_show_env ||
  s = SN.PseudoFunctions.hh_log_level

(* $x ?? 0 is handled similarly to $x ?: 0, except that the latter will also
 * look for sketchy null checks in the condition. *)
(* TODO TAST: type refinement should be made explicit in the typed AST *)
and eif env ~coalesce ~in_cond p c e1 e2 =
  let condition = condition ~lhs_of_null_coalesce:coalesce in
  let env, tc, tyc = raw_expr ~in_cond ~lhs_of_null_coalesce:coalesce env c in
  let parent_lenv = env.Env.lenv in
  let c = if coalesce then (p, Binop (Ast.Diff2, c, (p, Null))) else c in
  let env = condition env true c in
  let env, te1, ty1 = match e1 with
    | None ->
        let env, ty = non_null env tyc in
        env, None, ty
    | Some e1 ->
        let env, te1, ty1 = expr env e1 in
        env, Some te1, ty1
    in
  let lenv1 = env.Env.lenv in
  let env = { env with Env.lenv = parent_lenv } in
  let env = condition env false c in
  let env, te2, ty2 = expr env e2 in
  let lenv2 = env.Env.lenv in
  let fake_members =
    LEnv.intersect_fake lenv1.Env.fake_members lenv2.Env.fake_members in
  (* we restore the locals to their parent state so as not to leak the
   * effects of the `condition` calls above *)
  let env = { env with Env.lenv =
              { parent_lenv with Env.fake_members = fake_members } } in
  (* This is a shortened form of what we do in Typing_lenv.intersect. The
   * latter takes local environments as arguments, but our types here
   * aren't assigned to local variables in an environment *)
  let env, ty1 = TUtils.unresolved env ty1 in
  let env, ty2 = TUtils.unresolved env ty2 in
  let env, ty = Unify.unify env ty1 ty2 in
  let te = if coalesce then T.NullCoalesce(tc, te2) else T.Eif(tc, te1, te2) in
  env, T.make_typed_expr p ty te, ty

and exprs env el =
  match el with
  | [] ->
    env, [], []

  | e::el ->
    let env, te, ty = expr env e in
    let env, tel, tyl = exprs env el in
    env, te::tel, ty::tyl

and expr_
  ~in_cond
  ?lhs_of_null_coalesce
  ~(valkind: [> `lvalue | `lvalue_subexpr | `other ])
  env (p, e) =
  let make_result env te ty =
    env, T.make_typed_expr p ty te, ty in

  (**
   * Given a list of types, computes their supertype. If any of the types are
   * unknown (e.g., comes from PHP), the supertype will be Tany.
   *)
  let compute_supertype env tys =
    let env, supertype = Env.fresh_unresolved_type env in
    let has_unknown = List.exists tys (fun (_, ty) -> ty = Tany) in
    let env, tys = List.rev_map_env env tys TUtils.unresolved in
    let subtype_value env ty =
      Type.sub_type p Reason.URarray_value env ty supertype in
    if has_unknown then
      (* If one of the values comes from PHP land, we have to be conservative
       * and consider that we don't know what the type of the values are. *)
      env, (Reason.Rnone, Tany)
    else
      let env = List.fold_left tys ~init:env ~f:subtype_value in
      env, supertype in

  (**
   * Given a 'a list and a method to extract an expr and its ty from a 'a, this
   * function extracts a list of exprs from the list, and computes the supertype
   * of all of the expressions' tys.
   *)
  let compute_exprs_and_supertype env l extract_expr_and_ty =
    let env, exprs_and_tys = List.map_env env l extract_expr_and_ty in
    let exprs, tys = List.unzip exprs_and_tys in
    let env, supertype = compute_supertype env tys in
    env, exprs, supertype in

  let shape_and_tuple_arrays_enabled =
    not @@
      TypecheckerOptions.experimental_feature_enabled
        (Env.get_options env)
        TypecheckerOptions.experimental_disable_shape_and_tuple_arrays in

  match e with
  | Any -> expr_error env (Reason.Rwitness p)
  | Array [] ->
    make_result env (T.Array []) (Reason.Rwitness p, Tarraykind AKempty)
  | Array l
    when Typing_arrays.is_shape_like_array env l &&
      shape_and_tuple_arrays_enabled ->
      let env, (tafl, fdm) = List.fold_left_env env l
        ~init:([], ShapeMap.empty)
        ~f:begin fun env (tafl,fdm) x ->
          let env, taf, (key, value) = akshape_field env x in
          env, (taf::tafl, Nast.ShapeMap.add key value fdm)
        end in
      make_result env (T.Array(List.rev tafl))
        (Reason.Rwitness p, Tarraykind (AKshape fdm))

  | Array (x :: rl as l) ->
      let fields_consistent = check_consistent_fields x rl in
      let is_vec = match x with
        | Nast.AFvalue _ -> true
        | Nast.AFkvalue _ -> false in
      if fields_consistent && is_vec then
        let env, tel, arraykind =
          if shape_and_tuple_arrays_enabled then
            let env, tel, fields =
              List.foldi l ~f:begin fun index (env, tel, acc) e ->
                let env, te, ty = aktuple_field env e in
                env, te::tel, IMap.add index ty acc
              end ~init:(env, [], IMap.empty) in
            env, tel, AKtuple fields
          else
            let env, tel, value_ty =
              compute_exprs_and_supertype env l array_field_value in
            env, tel, AKvec value_ty in
        make_result env
          (T.Array (List.map tel (fun e -> T.AFvalue e)))
          (Reason.Rwitness p, Tarraykind arraykind)
      else
      let env, value_exprs_and_tys = List.rev_map_env env l array_field_value in
      let tvl, value_tys = List.unzip value_exprs_and_tys in
      let env, value = compute_supertype env value_tys in
      (* TODO TAST: produce a typed expression here *)
      if is_vec then
        make_result env T.Any
          (Reason.Rwitness p, Tarraykind (AKvec value))
      else
        let env, key_exprs_and_tys = List.rev_map_env env l array_field_key in
        let tkl, key_tys = List.unzip key_exprs_and_tys in
        let env, key = compute_supertype env key_tys in
        make_result env
          (T.Array (List.map (List.rev (List.zip_exn tkl tvl))
            (fun (tek, tev) -> T.AFkvalue (tek, tev))))
          (Reason.Rwitness p, Tarraykind (AKmap (key, value)))

  | Darray l ->
      let keys, values = List.unzip l in

      let env, value_exprs, value_ty =
        compute_exprs_and_supertype env values array_value in
      let env, key_exprs, key_ty =
        compute_exprs_and_supertype env keys array_value in

      let field_exprs = List.zip_exn key_exprs value_exprs in
      make_result env
        (T.Darray field_exprs)
        (Reason.Rwitness p, Tarraykind (AKdarray (key_ty, value_ty)))

  | Varray values ->
      let env, value_exprs, value_ty =
        compute_exprs_and_supertype env values array_value in
      make_result env
        (T.Varray value_exprs)
        (Reason.Rwitness p, Tarraykind (AKvarray value_ty))

  | ValCollection (kind, el) ->
      let env, x = Env.fresh_unresolved_type env in
      let env, tel, tyl = exprs env el in
      let env, tyl = List.map_env env tyl Typing_env.unbind in
      let env, tyl = List.map_env env tyl TUtils.unresolved in
      let subtype_val env ty =
        Type.sub_type p Reason.URvector env ty x in
      let env =
        List.fold_left tyl ~init:env ~f:subtype_val in
      let tvector = Tclass ((p, vc_kind_to_name kind), [x]) in
      let ty = Reason.Rwitness p, tvector in
      make_result env (T.ValCollection (kind, tel)) ty
  | KeyValCollection (kind, l) ->
      let kl, vl = List.unzip l in
      let env, tkl, kl = exprs env kl in
      let env, kl = List.map_env env kl Typing_env.unbind in
      let env, tvl, vl = exprs env vl in
      let env, vl = List.map_env env vl Typing_env.unbind in
      let env, k = Env.fresh_unresolved_type env in
      let env, v = Env.fresh_unresolved_type env in
      let env, kl = List.map_env env kl TUtils.unresolved in
      let subtype_key env ty = Type.sub_type p Reason.URkey env ty k in
      let env =
        List.fold_left kl ~init:env ~f:subtype_key in
      let subtype_val env ty = Type.sub_type p Reason.URvalue env ty v in
      let env, vl = List.map_env env vl TUtils.unresolved in
      let env =
        List.fold_left vl ~init:env ~f:subtype_val in
      let ty = Tclass ((p, kvc_kind_to_name kind), [k; v])
      in
      make_result env (T.KeyValCollection (kind, List.zip_exn tkl tvl))
        (Reason.Rwitness p, ty)
  | Clone e ->
    let env, te, ty = expr env e in
    (* Clone only works on objects; anything else fatals at runtime *)
    let tobj = (Reason.Rwitness p, Tobject) in
    let env = Type.sub_type p Reason.URclone env ty tobj in
    make_result env (T.Clone te) ty
  | This when Env.is_static env ->
      Errors.this_in_static p;
      expr_error env (Reason.Rwitness p)
  | This when valkind = `lvalue ->
     Errors.this_lvalue p;
     expr_error env (Reason.Rwitness p)
  | This ->
      let r, _ = Env.get_self env in
      if r = Reason.Rnone
      then Errors.this_var_outside_class p;
      let env, (_, ty) = Env.get_local env this in
      let r = Reason.Rwitness p in
      let ty = (r, ty) in
      let ty = r, TUtils.this_of ty in
      (* '$this' always refers to the late bound static type *)
      let env, new_ty = ExprDepTy.make env CIstatic ty in
      make_result env T.This (new_ty)
  | Assert (AE_assert e) ->
      let env, te, _ = expr env e in
      let env = condition env true e in
      make_result env (T.Assert (T.AE_assert te)) (Reason.Rwitness p, Tprim Tvoid)
  | True ->
      make_result env T.True (Reason.Rwitness p, Tprim Tbool)
  | False ->
      make_result env T.False (Reason.Rwitness p, Tprim Tbool)
    (* TODO TAST: consider checking that the integer is in range. Right now
     * it's possible for HHVM to fail on well-typed Hack code
     *)
  | Int s ->
      make_result env (T.Int s) (Reason.Rwitness p, Tprim Tint)
  | Float s ->
      make_result env (T.Float s) (Reason.Rwitness p, Tprim Tfloat)
    (* TODO TAST: consider introducing a "null" type, and defining ?t to
     * be null | t
     *)
  | Null ->
      let ty = Env.fresh_type() in
      make_result env T.Null (Reason.Rwitness p, Toption ty)
  | String s ->
      make_result env (T.String s) (Reason.Rwitness p, Tprim Tstring)
  | String2 idl ->
      let env, tel = string2 env idl in
      make_result env (T.String2 tel) (Reason.Rwitness p, Tprim Tstring)
  | Fun_id x ->
      Typing_hooks.dispatch_id_hook x env;
      let env, fty = fun_type_of_id env x [] in
      begin match fty with
      | _, Tfun fty -> check_deprecated (fst x) fty;
      | _ -> ()
      end;
      make_result env (T.Fun_id x) fty
  | Id ((cst_pos, cst_name) as id) ->
      Typing_hooks.dispatch_id_hook id env;
      Typing_hooks.dispatch_global_const_hook id;
      (match Env.get_gconst env cst_name with
      | None when Env.is_strict env ->
          Errors.unbound_global cst_pos;
          expr_error env (Reason.Rwitness cst_pos)
      | None ->
          make_result env (T.Id id) (Reason.Rnone, Tany)
      | Some ty ->
        let env, ty =
          Phase.localize_with_self env ty in
        make_result env (T.Id id) ty
      )
  | Method_id (instance, meth) ->
    (* Method_id is used when creating a "method pointer" using the magic
     * inst_meth function.
     *
     * Typing this is pretty simple, we just need to check that instance->meth
     * is public+not static and then return its type.
     *)
    Typing_hooks.dispatch_fun_id_hook (p, "\\"^SN.SpecialFunctions.inst_meth);
    let env, te, ty1 = expr env instance in
    let env, result, vis =
      obj_get_with_visibility ~is_method:true ~nullsafe:None env ty1
                              (CIexpr instance) meth (fun x -> x) in
    let has_lost_info = Env.FakeMembers.is_invalid env instance (snd meth) in
    if has_lost_info
    then
      let name = "the method "^snd meth in
      let env, result = Env.lost_info name env result in
      make_result env (T.Method_id (te, meth)) result
    else
      begin
        (match result with
        | _, Tfun fty -> check_deprecated p fty
        | _ -> ());
        (match vis with
        | Some (method_pos, Vprivate _) ->
            Errors.private_inst_meth method_pos p
        | Some (method_pos, Vprotected _) ->
            Errors.protected_inst_meth method_pos p
        | _ -> ()
        );
        make_result env (T.Method_id (te, meth)) result
      end
  | Method_caller ((pos, class_name) as pos_cname, meth_name) ->
    (* meth_caller('X', 'foo') desugars to:
     * $x ==> $x->foo()
     *)
    Typing_hooks.dispatch_fun_id_hook (p, "\\"^SN.SpecialFunctions.meth_caller);
    let class_ = Env.get_class env class_name in
    (match class_ with
    | None -> unbound_name env pos_cname
    | Some class_ ->
       (* Create a class type for the given object instantiated with unresolved
        * types for its type parameters.
        *)
        let env, tvarl =
          List.map_env env class_.tc_tparams TUtils.unresolved_tparam in
        let params = List.map class_.tc_tparams begin fun (_, (p, n), _) ->
          Reason.Rwitness p, Tgeneric n
        end in
        let obj_type = Reason.Rwitness p, Tapply (pos_cname, params) in
        let ety_env = {
          (Phase.env_with_self env) with
          substs = Subst.make class_.tc_tparams tvarl;
        } in
        let env, local_obj_ty = Phase.localize ~ety_env env obj_type in
        let env, fty =
          obj_get ~is_method:true ~nullsafe:None env local_obj_ty
                 (CI ((pos, class_name), [])) meth_name (fun x -> x) in
        (match fty with
        | reason, Tfun fty ->
            check_deprecated p fty;
            (* We are creating a fake closure:
             * function(Class $x, arg_types_of(Class::meth_name))
                 : return_type_of(Class::meth_name)
             *)
            let ety_env = {
              ety_env with substs = Subst.make class_.tc_tparams tvarl
            } in
            let env =
              Phase.check_tparams_constraints ~ety_env env class_.tc_tparams in
            let env, local_obj_ty = Phase.localize ~ety_env env obj_type in
            let fty = { fty with
                        ft_params = (None, local_obj_ty) :: fty.ft_params } in
            let fun_arity = match fty.ft_arity with
              | Fstandard (min, max) -> Fstandard (min + 1, max + 1)
              | Fvariadic (min, x) -> Fvariadic (min + 1, x)
              | Fellipsis min -> Fellipsis (min + 1) in
            let caller = {
              ft_pos = pos;
              ft_deprecated = None;
              ft_abstract = false;
              ft_arity = fun_arity;
              ft_tparams = fty.ft_tparams;
              ft_where_constraints = fty.ft_where_constraints;
              ft_params = fty.ft_params;
              ft_ret = fty.ft_ret;
            } in
            make_result env (T.Method_caller(pos_cname, meth_name))
              (reason, Tfun caller)
        | _ ->
            (* This can happen if the method lives in PHP *)
            make_result env (T.Method_caller(pos_cname, meth_name))
              (Reason.Rwitness pos, Tany)
        )
    )
  | Smethod_id (c, meth) ->
    (* Smethod_id is used when creating a "method pointer" using the magic
     * class_meth function.
     *
     * Typing this is pretty simple, we just need to check that c::meth is
     * public+static and then return its type.
     *)
    Typing_hooks.dispatch_fun_id_hook (p, "\\"^SN.SpecialFunctions.class_meth);
    let class_ = Env.get_class env (snd c) in
    (match class_ with
    | None ->
      (* The class given as a static string was not found. *)
      unbound_name env c
    | Some class_ ->
      let smethod = Env.get_static_member true env class_ (snd meth) in
      (match smethod with
      | None -> (* The static method wasn't found. *)
        smember_not_found p ~is_const:false ~is_method:true class_ (snd meth);
        expr_error env Reason.Rnone
      | Some { ce_type = lazy ty; ce_visibility; _ } ->
        let cid = CI (c, []) in
        let env, _te, cid_ty = static_class_id (fst c) env cid in
        let ety_env = {
          type_expansions = [];
          substs = SMap.empty;
          this_ty = cid_ty;
          from_class = Some cid;
        } in
        let env, smethod_type = Phase.localize ~ety_env env ty in
        (match smethod_type with
        | _, Tfun fty -> check_deprecated p fty
        | _ -> ());
        (match smethod_type, ce_visibility with
        | (r, (Tfun _ as ty)), Vpublic ->
          make_result env (T.Smethod_id(c, meth)) (r, ty)
        | (r, Tfun _), Vprivate _ ->
          Errors.private_class_meth (Reason.to_pos r) p;
          expr_error env r
        | (r, Tfun _), Vprotected _ ->
          Errors.protected_class_meth (Reason.to_pos r) p;
          expr_error env r
        | (r, _), _ ->
          Errors.internal_error p "We have a method which isn't callable";
          expr_error env r
        )
      )
    )
  | Lplaceholder p ->
      let r = Reason.Rplaceholder p in
      let ty = r, Tprim Tvoid in
      make_result env (T.Lplaceholder p) ty
  | Dollardollar ((_, x) as id) ->
      let env, ty =
        Env.get_local env x in
      make_result env (T.Dollardollar id) ty
  | Lvar ((_, x) as id) ->
      Typing_hooks.dispatch_lvar_hook id env;
      let env, ty = Env.get_local env x in
      make_result env (T.Lvar id) ty
  | Lvarvar (i, id) ->
      Typing_hooks.dispatch_lvar_hook id env;
      (** Can't easily track any typing information for variable variable. *)
      make_result env (T.Lvarvar (i, id)) (Reason.Rnone, Tany)
  | List el ->
      let env, tel, tyl = exprs env el in
      (* TODO TAST: figure out role of unbind here *)
      let env, tyl = List.map_env env tyl Typing_env.unbind in
      let ty = Reason.Rwitness p, Ttuple tyl in
      make_result env (T.List tel) ty
  | Pair (e1, e2) ->
      let env, te1, ty1 = expr env e1 in
      let env, ty1 = Typing_env.unbind env ty1 in
      let env, te2, ty2 = expr env e2 in
      let env, ty2 = Typing_env.unbind env ty2 in
      let ty =
        Reason.Rwitness p, Tclass ((p, SN.Collections.cPair), [ty1; ty2]) in
      make_result env (T.Pair (te1, te2)) ty
  | Expr_list el ->
      let env, tel, tyl = exprs env el in
      let ty = Reason.Rwitness p, Ttuple tyl in
      make_result env (T.Expr_list tel) ty
  | Array_get (e, None) ->
      let env, te1, ty1 = update_array_type p env e None valkind in
      let env, ty = array_append p env ty1 in
      make_result env (T.Array_get(te1, None)) ty
  | Array_get (e1, Some e2) ->
      let env, te1, ty1 =
        update_array_type ?lhs_of_null_coalesce p env e1 (Some e2) valkind in
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, te2, ty2 = expr env e2 in
      let is_lvalue = (valkind == `lvalue) in
      let env, ty =
        array_get ?lhs_of_null_coalesce is_lvalue p env ty1 e2 ty2 in
      make_result env (T.Array_get(te1, Some te2)) ty
  | Call (Cnormal, (pos_id, Id ((_, s) as id)), hl, el, [])
      when is_pseudo_function s ->
      let env, tel, tys = exprs env el in
      if s = SN.PseudoFunctions.hh_show
      then List.iter tys (Typing_log.hh_show p env)
      else
      if s = SN.PseudoFunctions.hh_show_env
      then Typing_log.hh_show_env p env
      else
      if s = SN.PseudoFunctions.hh_log_level
      then match el with
        | [(_, Int (_, level_str))] ->
          Typing_log.hh_log_level (int_of_string level_str)
        | _ -> ()
      else ();
      make_result env
        (T.Call(
          Cnormal,
          T.make_implicitly_typed_expr pos_id (T.Id id),
          hl,
          tel,
          [])) (Env.fresh_type())
  | Call (call_type, e, hl, el, uel) ->
      let env, te, result = dispatch_call p env call_type e hl el uel in
      let env = Env.forget_members env p in
      env, te, result
    (* This case covers expressions like "e1 += e2". It is typed as if
     * "e1 = e1 + e2" was written, meaning that e1 is typechecked twice. The
     * returned typed expression correctly preserves the "e1 += e2" structure.
     *)
  | Binop (Ast.Eq (Some op), e1, e2) ->
      let e_fake = (p, Binop (Ast.Eq None, e1, (p, Binop (op, e1, e2)))) in
      let env, te_fake, ty = raw_expr in_cond env e_fake in
      begin match snd te_fake with
        | T.Binop (_, te1, (_, T.Binop (_, _, te2))) ->
          let te = T.Binop (Ast.Eq (Some op), te1, te2) in
          make_result env te ty
        | _ -> assert false
      end
  | Binop (Ast.Eq None, e1, e2) ->
      let env, te2, ty2 = raw_expr in_cond env e2 in
      let env, te1, ty = assign p env e1 ty2 in
      Typing_hooks.dispatch_assign_hook p ty2 env;
      (* If we are assigning a local variable to another local variable then
       * the expression ID associated with e2 is transferred to e1
       *)
      (match e1, e2 with
      | (_, Lvar (_, x1)), (_, Lvar (_, x2)) ->
          let eid2 = Env.get_local_expr_id env x2 in
          let env =
            Option.value_map
              eid2 ~default:env
              ~f:(Env.set_local_expr_id env x1) in
          make_result env (T.Binop(Ast.Eq None, te1, te2)) ty
      | _ ->
        make_result env (T.Binop(Ast.Eq None, te1, te2)) ty
      )
  | Binop ((Ast.AMpamp | Ast.BArbar as bop), e1, e2) ->
      let c = bop = Ast.AMpamp in
      let lenv = env.Env.lenv in
      let env, te1, ty1 = expr env e1 in
      let env = condition env c e1 in
      let env, te2, ty2 = raw_expr in_cond env e2 in
      let env = { env with Env.lenv = lenv } in
      Typing_hooks.dispatch_binop_hook p bop ty1 ty2;
      make_result env (T.Binop(bop, te1, te2))
        (Reason.Rlogic_ret p, Tprim Tbool)
  | Binop (bop, e1, e2) when Env.is_strict env
                        && (snd e1 = Nast.Null || snd e2 = Nast.Null)
                        && (bop = Ast.EQeqeq || bop = Ast.Diff2) ->
      let e, ne = if snd e2 = Nast.Null then e1, e2 else e2, e1 in
      let _, te, ty = raw_expr in_cond env e in
      if not in_cond
      then Typing_equality_check.assert_nullable p bop env ty;
      let tne = T.make_typed_expr (fst ne) ty T.Null in
      let te1, te2 = if snd e2 = Nast.Null then te, tne else tne, te in
      make_result env (T.Binop(bop, te1, te2))
        (Reason.Rcomp p, Tprim Tbool)
  | Binop (bop, e1, e2) ->
      let env, te1, ty1 = raw_expr in_cond env e1 in
      let env, te2, ty2 = raw_expr in_cond env e2 in
      let env, te3, ty =
        binop in_cond p env bop (fst e1) te1 ty1 (fst e2) te2 ty2 in
      Typing_hooks.dispatch_binop_hook p bop ty1 ty2;
      env, te3, ty
  | Pipe ((_, id) as e0, e1, e2) ->
      let env, te1, ty = expr env e1 in
      (** id is the ID of the $$ that is implicitly declared by the pipe.
       * Set the local type for the $$ in the RHS. *)
      let env = Env.set_local env id ty in
      let env, te2, ty2 = expr env e2 in
      (**
       * Return ty2 since the type of the pipe expression is the type of the
       * RHS.
       *
       * Note: env does have the type of this Pipe's $$, but it doesn't
       * override the outer one since they have different ID's.
       *
       * For example:
       *   a() |> ( inner1($$) |> inner2($$) ) + $$
       *
       *   The rightmost $$ refers to the result of a()
       *)
      make_result env (T.Pipe(e0, te1, te2)) ty2
  | Unop (uop, e) ->
      let env, te, ty = raw_expr in_cond env e in
      unop p env uop te ty
  | Eif (c, e1, e2) -> eif env ~coalesce:false ~in_cond p c e1 e2
  | NullCoalesce (e1, e2) -> eif env ~coalesce:true ~in_cond p e1 None e2
  | Typename sid ->
      begin match Env.get_typedef env (snd sid) with
        | Some {td_tparams = tparaml; _} ->
            (* Typedef type parameters cannot have constraints *)
            let params = List.map ~f:begin fun (_, (p, x), _) ->
              Reason.Rwitness p, Tgeneric x
            end tparaml in
            let tdef = Reason.Rwitness (fst sid), Tapply (sid, params) in
            let typename =
              Reason.Rwitness p, Tapply((p, SN.Classes.cTypename), [tdef]) in
            let env, tparams = List.map_env env tparaml begin fun env _ ->
              Env.fresh_unresolved_type env
            end in
            let ety_env = { (Phase.env_with_self env) with
                            substs = Subst.make tparaml tparams } in
            let env = Phase.check_tparams_constraints ~ety_env env tparaml in
            let env, ty = Phase.localize ~ety_env env typename in
            make_result env (T.Typename sid) ty
        | None ->
            (* Should never hit this case since we only construct this AST node
             * if in the expression Foo::class, Foo is a type def.
             *)
            expr_error env (Reason.Rwitness p)
      end
  | Class_const (cid, mid) -> class_const env p (cid, mid)
  | Class_get (x, (py, y))
      when Env.FakeMembers.get_static env x y <> None ->
        let env, local = Env.FakeMembers.make_static p env x y in
        let local = p, Lvar (p, local) in
        let env, _, ty = expr env local in
        let cid, env = begin match x with
          | CIparent -> T.CIparent, env
          | CIself -> T.CIself, env
          | CIstatic -> T.CIstatic, env
          | CIexpr ((_, This) as e)
          | CIexpr ((_, Lvar (_, _)) as e) ->
            let env, te, _ = expr env e in
            T.CIexpr te, env
          | CI x -> T.CI x, env
          | CIexpr _ -> assert false
        end in
        make_result env (T.Class_get (cid, (py, y))) ty
  | Class_get (cid, mid) ->
      TUtils.process_static_find_ref cid mid;
      let env, te, cty = static_class_id p env cid in
      let env, ty, _ =
        class_get ~is_method:false ~is_const:false env cty mid cid in
      if Env.FakeMembers.is_static_invalid env cid (snd mid)
      then
        let fake_name = Env.FakeMembers.make_static_id cid (snd mid) in
        let env, ty = Env.lost_info fake_name env ty in
        make_result env (T.Class_get (te, mid)) ty
      else
        make_result env (T.Class_get (te, mid)) ty
    (* Fake member property access. For example:
     *   if ($x->f !== null) { ...$x->f... }
     *)
  | Obj_get (e, (pid, Id (py, y)), nf)
      when Env.FakeMembers.get env e y <> None ->
        let env, local = Env.FakeMembers.make p env e y in
        let local = p, Lvar (p, local) in
        let env, _, ty = expr env local in
        let env, t_lhs, _ = expr env e in
        let t_rhs = T.make_typed_expr pid ty (T.Id (py, y)) in
        make_result env (T.Obj_get (t_lhs, t_rhs, nf)) ty
    (* Statically-known instance property access e.g. $x->f *)
  | Obj_get (e1, (pm, Id m), nullflavor) ->
      let nullsafe =
        (match nullflavor with
          | OG_nullthrows -> None
          | OG_nullsafe -> Some p
        ) in
      let env, te1, ty1 = expr env e1 in
      let env, result =
        obj_get ~is_method:false ~nullsafe env ty1 (CIexpr e1) m (fun x -> x) in
      let has_lost_info = Env.FakeMembers.is_invalid env e1 (snd m) in
      let env, result =
        if has_lost_info
        then
          let name = "the member " ^ snd m in
          Env.lost_info name env result
        else
          env, result
      in
      make_result env (T.Obj_get(te1,
        T.make_implicitly_typed_expr pm (T.Id m), nullflavor)) result
    (* Dynamic instance property access e.g. $x->$f *)
  | Obj_get (e1, e2, nullflavor) ->
    let env, te1, _ = expr env e1 in
    let env, te2, _ = expr env e2 in
    let ty = (Reason.Rwitness p, Tany) in
    make_result env (T.Obj_get(te1, te2, nullflavor)) ty
  | Yield_break ->
      make_result env T.Yield_break (Reason.Rwitness p, Tany)
  | Yield af ->
      let env, (taf, opt_key, value) = array_field env af in
      let send = Env.fresh_type () in
      let env, key = match af, opt_key with
        | Nast.AFvalue (p, _), None ->
          let result_ty =
            match Env.get_fn_kind env with
              | Ast.FSync
              | Ast.FAsync ->
                  Errors.internal_error p "yield found in non-generator";
                  Reason.Rnone, Tany
              | Ast.FGenerator ->
                  (Reason.Rwitness p, Tprim Tint)
              | Ast.FAsyncGenerator ->
                  (Reason.Ryield_asyncnull p,
                    Toption (Env.fresh_type ()))
            in
            env, result_ty
        | _, Some x ->
            env, x
        | _, _ -> assert false in
      let rty = match Env.get_fn_kind env with
        | Ast.FGenerator ->
            Reason.Ryield_gen p,
            Tclass ((p, SN.Classes.cGenerator), [key; value; send])
        | Ast.FAsyncGenerator ->
            Reason.Ryield_asyncgen p,
            Tclass ((p, SN.Classes.cAsyncGenerator), [key; value; send])
        | Ast.FSync | Ast.FAsync ->
            failwith "Parsing should never allow this" in
      let env =
        Type.sub_type p (Reason.URyield) env rty (Env.get_return env) in
      let env = Env.forget_members env p in
      make_result env (T.Yield taf) (Reason.Ryield_send p, Toption send)
  | Await e ->
      let env, te, rty = expr env e in
      let env, ty = Async.overload_extract_from_awaitable env p rty in
      make_result env (T.Await te) ty
  | Special_func func -> special_func env p func
  | New (c, el, uel) ->
      Typing_hooks.dispatch_new_id_hook c env p;
      TUtils.process_static_find_ref c (p, SN.Members.__construct);
      let check_not_abstract = true in
      let env, tc, tel, tuel, ty =
        new_object ~check_not_abstract p env c el uel in
      let env = Env.forget_members env p in
      let env, new_ty = (ExprDepTy.make env c ty) in
      make_result env (T.New(tc, tel, tuel)) new_ty
  | Cast ((_, Harray (None, None)), _)
    when Env.is_strict env
    || TCO.migration_flag_enabled (Env.get_tcopt env) "array_cast" ->
      Errors.array_cast p;
      expr_error env (Reason.Rwitness p)
  | Cast (hint, e) ->
      let env, te, ty2 = expr env e in
      Async.enforce_not_awaitable env (fst e) ty2;
      let env, ty = Phase.hint_locl env hint in
      make_result env (T.Cast (hint, te)) ty
  | InstanceOf (e, cid) ->
      let env, te, _ = expr env e in
      TUtils.process_class_id cid;
      let env, te2, _class = instantiable_cid p env cid in
      make_result env (T.InstanceOf (te, te2)) (Reason.Rwitness p, Tprim Tbool)
  | Efun (f, idl) ->
      let ft = Decl.fun_decl_in_env env.Env.decl_env f in
      (* When creating a closure, the 'this' type will mean the late bound type
       * of the current enclosing class
       *)
      let ety_env =
        { (Phase.env_with_self env) with from_class = Some CIstatic } in
      let env, ft = Phase.localize_ft ~ety_env env ft in
      (* check for recursive function calls *)
      let anon = anon_make env p f idl in
      let env, anon_id = Env.add_anonymous env anon in
      let env, tefun, _ = Errors.try_with_error
        (fun () ->
          let _, tefun, ty = anon env ft.ft_params
          in env, tefun, ty)
        (fun () ->
          (* If the anonymous function declaration has errors itself, silence
             them in any subsequent usages. *)
          let anon env fun_params =
            Errors.ignore_ (fun () -> (anon env fun_params)) in
          let _, tefun, ty = anon env ft.ft_params in
          let env = Env.set_anonymous env anon_id anon in
          env, tefun, ty) in
      env, tefun, (Reason.Rwitness p, Tanon (ft.ft_arity, anon_id))
  | Xml (sid, attrl, el) ->
      let cid = CI (sid, []) in
      let env, _te, obj = expr env (fst sid, New (cid, [], [])) in
      let env, attr_ptyl = List.map_env env attrl begin fun env attr ->
        (* Typecheck the expressions - this just checks that the expressions are
         * valid, not that they match the declared type for the attribute *)
        let namepstr, valexpr = attr in
        let valp, _ = valexpr in
        let env, te, valty = expr env valexpr in
        env, (namepstr, (valp, valty), te)
      end in
      let tal = List.map attr_ptyl (fun (a, _, c) -> (a, c)) in
      let env, tel, _body = exprs env el in
      let txml = T.Xml (sid, tal, tel) in
      let env, _te, classes = class_id_for_new p env cid in
      (match classes with
       | [] -> make_result env txml (Reason.Runknown_class p, Tobject)
         (* OK to ignore rest of list; class_info only used for errors, and
          * cid = CI sid cannot produce a union of classes anyhow *)
       | (_, class_info, _)::_ ->
        let env = List.fold_left attr_ptyl ~f:begin fun env attr ->
          let namepstr, valpty, _ = attr in
          let valp, valty = valpty in
          (* We pretend that XHP attributes are stored as member variables,
           * prefixed with a colon.
           *
           * This converts the member name to an attribute name. *)
          let name = ":" ^ (snd namepstr) in
          let env, declty =
            obj_get ~is_method:false ~nullsafe:None env obj cid
              (fst namepstr, name) (fun x -> x) in
          let ureason = Reason.URxhp (class_info.tc_name, snd namepstr) in
          Type.sub_type valp ureason env valty declty
        end ~init:env in
        make_result env txml obj
      )
    (* TODO TAST: change AST so that order of shape expressions is preserved.
     * At present, evaluation order is unspecified in TAST *)
  | Shape fdm ->
      (* allow_inter adds a type-variable *)
      let env, tfdm =
        ShapeMap.map_env
          (fun env e -> let env, te, ty = expr env e in env, (te,ty))
          env fdm in
      let env, fdm =
        let convert_expr_and_type_to_shape_field_type env (_, ty) =
          let env, sft_ty = TUtils.unresolved env ty in
          (* An expression evaluation always corresponds to a shape_field_type
             with sft_optional = false. *)
          env, { sft_optional = false; sft_ty } in
        ShapeMap.map_env convert_expr_and_type_to_shape_field_type env tfdm in
      let env = check_shape_keys_validity env p (ShapeMap.keys fdm) in
      (* Fields are fully known, because this shape is constructed
       * using shape keyword and we know exactly what fields are set. *)
      make_result env (T.Shape (ShapeMap.map (fun (te,_) -> te) tfdm))
        (Reason.Rwitness p, Tshape (FieldsFullyKnown, fdm))

and class_const ?(incl_tc=false) env p (cid, mid) =
  TUtils.process_static_find_ref cid mid;
  let env, ce, cty = static_class_id p env cid in
  let env, const_ty, cc_abstract_info =
    class_get ~is_method:false ~is_const:true ~incl_tc env cty mid cid in
  match cc_abstract_info with
    | Some (cc_pos, cc_name) ->
      let () = match cid with
        | CIstatic | CIexpr _ -> ();
        | _ -> Errors.abstract_const_usage p cc_pos cc_name; ()
      in env, T.make_typed_expr p const_ty (T.Class_const (ce, mid)), const_ty
    | None ->
      env, T.make_typed_expr p const_ty (T.Class_const (ce, mid)), const_ty

and anon_sub_type pos ur env ty_sub ty_super =
  Errors.try_add_err pos (Reason.string_of_ureason ur)
    (fun () -> SubType.sub_type env ty_sub ty_super)
    (fun () -> env)

(*****************************************************************************)
(* Anonymous functions. *)
(*****************************************************************************)
and anon_bind_param params (env, t_params) ty : Env.env * Tast.fun_param list =
  match !params with
  | [] ->
      (* This code cannot be executed normally, because the arity is wrong
       * and it will error later. Bind as many parameters as we can and carry
       * on. *)
      env, t_params
  | param :: paraml ->
      params := paraml;
      match param.param_hint with
      | Some h ->

        let h = Decl_hint.hint env.Env.decl_env h in
        (* When creating a closure, the 'this' type will mean the
         * late bound type of the current enclosing class
         *)
        let ety_env =
          { (Phase.env_with_self env) with from_class = Some CIstatic } in
        let env, h = Phase.localize ~ety_env env h in
        let pos = Reason.to_pos (fst ty) in
        (* Don't use Type.sub_type as it resets env.Env.pos unnecessarily *)
        let env = anon_sub_type pos Reason.URparam env ty h in
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
        let env, t_param = bind_param env (h, param) in
        env, t_params @ [t_param]
      | None ->
        let env, t_param = bind_param env (ty, param) in
        env, t_params @ [t_param]

and anon_bind_opt_param env param : Env.env =
  match param.param_expr with
  | None ->
      let ty = Reason.Rnone, Tany in
      let env, _ = bind_param env (ty, param) in
      env
  | Some default ->
      let env, _te, ty = expr env default in
      Typing_sequencing.sequence_check_expr default;
      let env, _ = bind_param env (ty, param) in
      env

and anon_check_param env param =
  match param.param_hint with
  | None -> env
  | Some hty ->
      let env, hty = Phase.hint_locl env hty in
      let env, paramty = Env.get_local env (Local_id.get param.param_name) in
      let hint_pos = Reason.to_pos (fst hty) in
      let env = Type.sub_type hint_pos Reason.URhint env paramty hty in
      env

and anon_make tenv p f idl =
  let anon_lenv = tenv.Env.lenv in
  let is_typing_self = ref false in
  let nb = Nast.assert_named_body f.f_body in
  fun env (supplied_params: (string option * _) list) ->
    if !is_typing_self
    then begin
      Errors.anonymous_recursive p;
      expr_error env (Reason.Rwitness p)
    end
    else begin
      is_typing_self := true;
      Env.anon anon_lenv env begin fun env ->
        let params = ref f.f_params in
        let env, t_params = List.fold_left ~f:(anon_bind_param params) ~init:(env, [])
          (List.map supplied_params snd) in
        let env = List.fold_left ~f:anon_bind_opt_param ~init:env !params in
        let env = List.fold_left ~f:anon_check_param ~init:env f.f_params in
        let env, hret =
          match f.f_ret with
          | None -> Env.fresh_unresolved_type env
          | Some x ->
            let ret = TI.instantiable_hint env x in
            (* If a 'this' type appears it needs to be compatible with the
             * late static type
             *)
            let ety_env =
              { (Phase.env_with_self env) with
                from_class = Some CIstatic } in
            Phase.localize ~ety_env env ret in
        let env = Env.set_return env hret in
        let env = Env.set_fn_kind env f.f_fun_kind in
        let env, tb = block env nb.fnb_nast in
        let env =
          if Nast_terminality.Terminal.block tenv nb.fnb_nast
            || nb.fnb_unsafe || !auto_complete
          then env
          else fun_implicit_return env p hret nb.fnb_nast f.f_fun_kind
        in
        is_typing_self := false;
        let tfun_ = {
          T.f_mode = f.f_mode;
          T.f_ret = f.f_ret;
          T.f_name = f.f_name;
          T.f_tparams = f.f_tparams;
          T.f_where_constraints = f.f_where_constraints;
          T.f_fun_kind = f.f_fun_kind;
          T.f_user_attributes = List.map f.f_user_attributes (user_attribute env);
          T.f_body = T.NamedBody {
            T.fnb_nast = tb;
            T.fnb_unsafe = nb.fnb_unsafe;
          };
          T.f_params = t_params;

          T.f_variadic = T.FVnonVariadic; (* TODO TAST: Variadic efuns *)
        } in
        let te = T.make_typed_expr p hret (T.Efun (tfun_, idl)) in
        env, te, hret
      end
    end

(*****************************************************************************)
(* End of anonymous functions. *)
(*****************************************************************************)

and special_func env p func =
  let env, tfunc, ty = (match func with
  | Gena e ->
      let env, te, ety = expr env e in
      let env, ty = Async.gena env p ety in
      env, T.Gena te, ty
  | Genva el ->
      let env, tel, etyl = exprs env el in
      let env, ty = Async.genva env p etyl in
      env, T.Genva tel, ty
  | Gen_array_rec e ->
      let env, te, ety = expr env e in
      let env, ty = Async.gen_array_rec env p ety in
      env, T.Gen_array_rec te, ty
  ) in
  let result_ty =
    (Reason.Rwitness p, Tclass ((p, SN.Classes.cAwaitable), [ty])) in
  env, T.make_typed_expr p result_ty (T.Special_func tfunc), result_ty


and requires_consistent_construct = function
  | CIstatic -> true
  | CIexpr _ -> true
  | CIparent -> false
  | CIself -> false
  | CI _ -> false

and new_object ~check_not_abstract p env cid el uel =
  (* Obtain class info from the cid expression. We get multiple
   * results with a CIexpr that has a union type *)
  let env, tcid, classes = instantiable_cid p env cid in
  let rec gather env tel tuel res classes =
    match classes with
    | [] ->
      begin
        match res with
        | [] ->
          let _ = exprs env el in
          env, tcid, tel, tuel, (Reason.Runknown_class p, Tobject)
        | [ty] -> env, tcid, tel, tuel, ty
        | tyl -> env, tcid, tel, tuel, (Reason.Rwitness p, Tunresolved tyl)
      end

    | (cname, class_info, c_ty)::classes ->
      if check_not_abstract && class_info.tc_abstract
        && not (requires_consistent_construct cid) then
        uninstantiable_error p cid class_info.tc_pos class_info.tc_name p c_ty;
      let env, obj_ty_, params =
        match cid, snd c_ty with
        | CI (_, _::_), Tclass(_, tyl) -> env, (snd c_ty), tyl
        | _, _ ->
          let env, params = List.map_env env class_info.tc_tparams
            (fun env _ -> Env.fresh_unresolved_type env) in
          env, (Tclass (cname, params)), params in
      let r_witness = Reason.Rwitness p in
      let obj_ty = (r_witness, obj_ty_) in
      let env, _tcid, tel, tuel =
        call_construct p env class_info params el uel cid in
      if not (snd class_info.tc_construct) then
        (match cid with
          | CIstatic -> Errors.new_inconsistent_construct p cname `static
          | CIexpr _ -> Errors.new_inconsistent_construct p cname `classname
          | _ -> ());
      match cid with
        | CIstatic ->
          gather env tel tuel
            ((r_witness, TUtils.this_of obj_ty)::res) classes
        | CIparent ->
          (match (fst class_info.tc_construct) with
            | Some {ce_type = lazy ty; _ } ->
              let ety_env = {
                type_expansions = [];
                substs = SMap.empty;
                this_ty = obj_ty;
                from_class = None;
              } in
              let _, ce_type = Phase.localize ~ety_env env ty in
              ignore (check_abstract_parent_meth SN.Members.__construct p ce_type)
            | None -> ());
          gather env tel tuel (obj_ty::res) classes
        | CI _ | CIself -> gather env tel tuel (obj_ty::res) classes
        | CIexpr _ ->
          let c_ty = r_witness, snd c_ty in
          (* When constructing from a (classname) variable, the variable
           * dictates what the constructed object is going to be. This allows
           * for generic and dependent types to be correctly carried
           * through the 'new $foo()' iff the constructed obj_ty is a
           * supertype of the variable-dictated c_ty *)
          let env = SubType.sub_type env c_ty obj_ty in
          gather env tel tuel (c_ty::res) classes
  in
  gather env [] [] [] classes

(* FIXME: we need to separate our instantiability into two parts. Currently,
 * all this function is doing is checking if a given type is inhabited --
 * that is, whether there are runtime values of type T. However,
 * instantiability should be the stricter notion that T has a runtime
 * constructor; that is, `new T()` should be valid. In particular, interfaces
 * are inhabited, but not instantiable.
 * To make this work with classname, we likely need to add something like
 * concrete_classname<T>, where T cannot be an interface.
 * *)
and instantiable_cid p env cid =
  let env, te, classes = class_id_for_new p env cid in
  begin
    List.iter classes begin fun ((pos, name), class_info, c_ty) ->
      if class_info.tc_kind = Ast.Ctrait || class_info.tc_kind = Ast.Cenum
      then
         match cid with
          | CIexpr _ | CI _ ->
            uninstantiable_error p cid class_info.tc_pos name pos c_ty
          | CIstatic | CIparent | CIself -> ()
      else if class_info.tc_kind = Ast.Cabstract && class_info.tc_final
      then
        uninstantiable_error p cid class_info.tc_pos name pos c_ty
      else () end;
    env, te, classes
  end

and uninstantiable_error reason_pos cid c_tc_pos c_name c_usage_pos c_ty =
  let reason_msgl = match cid with
    | CIexpr _ ->
      let ty_str = "This would be "^Typing_print.error (snd c_ty) in
      [(reason_pos, ty_str)]
    | _ -> [] in
  Errors.uninstantiable_class c_usage_pos c_tc_pos c_name reason_msgl

and exception_ty pos env ty =
  let exn_ty = Reason.Rthrow pos, Tclass ((pos, SN.Classes.cException), []) in
  Type.sub_type pos (Reason.URthrow) env ty exn_ty

and shape_field_pos = function
  | Ast.SFlit (p, _) -> p
  | Ast.SFclass_const ((cls_pos, _), (mem_pos, _)) -> Pos.btw cls_pos mem_pos

and check_shape_keys_validity env pos keys =
    (* If the key is a class constant, get its class name and type. *)
    let get_field_info env key =
      let key_pos = shape_field_pos key in
      (* Empty strings or literals that start with numbers are not
         permitted as shape field names. *)
      (match key with
        | Ast.SFlit (_, key_name) ->
           if (String.length key_name = 0) then
             (Errors.invalid_shape_field_name_empty key_pos)
           else if (key_name.[0] >= '0' && key_name.[0] <='9') then
             (Errors.invalid_shape_field_name_number key_pos);
           env, key_pos, None
        | Ast.SFclass_const (_, cls as x, y) ->
          let env, _te, ty = class_const env pos (CI (x, []), y) in
          let env = Typing_enum.check_valid_array_key_type
            Errors.invalid_shape_field_type ~allow_any:false
            env key_pos ty in
          env, key_pos, Some (cls, ty))
    in

    let check_field witness_pos witness_info env key =
      let env, key_pos, key_info = get_field_info env key in
      (match witness_info, key_info with
        | Some _, None ->
          Errors.invalid_shape_field_literal key_pos witness_pos; env
        | None, Some _ ->
          Errors.invalid_shape_field_const key_pos witness_pos; env
        | None, None -> env
        | Some (cls1, ty1), Some (cls2, ty2) ->
          if cls1 <> cls2 then
            Errors.shape_field_class_mismatch
              key_pos witness_pos (strip_ns cls2) (strip_ns cls1);
          (* We want to use our own error message here instead of the normal
           * unification one. *)
          Errors.try_
            (fun () -> Unify.iunify env ty1 ty2)
            (fun _ ->
              Errors.shape_field_type_mismatch
                key_pos witness_pos
                (Typing_print.error (snd ty2)) (Typing_print.error (snd ty1));
              env))
    in

    (* Sort the keys by their positions since the error messages will make
     * more sense if we take the one that appears first as canonical and if
     * they are processed in source order. *)
    let cmp_keys x y = Pos.compare (shape_field_pos x) (shape_field_pos y) in
    let keys = List.sort cmp_keys keys in

    match keys with
      | [] -> env
      | witness :: rest_keys ->
        let env, pos, info = get_field_info env witness in
        List.fold_left ~f:(check_field pos info) ~init:env rest_keys

and check_valid_rvalue p env ty =
    let rec iter_over_types env tyl =
      match tyl with
      | [] ->
        env, ty

      | ty::tyl ->
        let env, ety = Env.expand_type env ty in
        match ety with
        | r, Tprim Tnoreturn ->
          Errors.noreturn_usage p
            (Reason.to_string "A noreturn function always throws or exits" r);
          env, (r, Terr)

        | r, Tprim Tvoid ->
          Errors.void_usage p
            (Reason.to_string "A void function doesn't return a value" r);
          env, (r, Terr)

        | _, Tunresolved tyl2 ->
          iter_over_types env (tyl2 @ tyl)

        | _, _ ->
          iter_over_types env tyl in
    iter_over_types env [ty]

and set_valid_rvalue p env x ty =
  let env, ty = check_valid_rvalue p env ty in
  let env = Env.set_local env x ty in
  (* We are assigning a new value to the local variable, so we need to
   * generate a new expression id
   *)
  let env = Env.set_local_expr_id env x (Ident.tmp()) in
  env, ty

(* Deal with assignment of a value of type ty2 to lvalue e1 *)
and assign p env e1 ty2 : _ * T.expr * T.ty =
let make_result env te1 ty1 = (env, T.make_typed_expr p ty1 te1, ty1) in
  match e1 with
  | (_, Lvar ((_, x) as id)) ->
    let env, ty1 = set_valid_rvalue p env x ty2 in
    make_result env (T.Lvar id) ty1
  | (_, Lplaceholder id) ->
    let placeholder_ty = Reason.Rplaceholder p, (Tprim Tvoid) in
    make_result env (T.Lplaceholder id) placeholder_ty
  | (_, List el) ->
    let env, folded_ty2 = TUtils.fold_unresolved env ty2 in
    let resl =
      try_over_concrete_supertypes env folded_ty2
        begin fun env ty2 ->
          match ty2 with
          (* Vector<t> or ImmVector<t> or ConstVector<t> or vec<T> *)
          | (_, Tclass ((_, x), [elt_type]))
          when x = SN.Collections.cVector
            || x = SN.Collections.cImmVector
            || x = SN.Collections.cVec
            || x = SN.Collections.cConstVector ->
            let env, tel = List.map_env env el begin fun env e ->
              let env, te, _ = assign (fst e) env e elt_type in
              env, te
            end in
            make_result env (T.List tel) ty2
          (* array<t> or varray<t> *)
          | (_, Tarraykind (AKvec elt_type))
          | (_, Tarraykind (AKvarray elt_type)) ->
            let env, tel = List.map_env env el begin fun env e ->
              let env, te, _ = assign (fst e) env e elt_type in
              env, te
            end in
            make_result env (T.List tel) ty2
          (* array or empty array or Tany *)
          | (r, (Tarraykind (AKany | AKempty) | Tany)) ->
            let env, tel = List.map_env env el begin fun env e ->
              let env, te, _ = assign (fst e) env e (r, Tany) in
              env, te
            end in
            make_result env (T.List tel) ty2
          (* Pair<t1,t2> *)
          | ((r, Tclass ((_, coll), [ty1; ty2])) as folded_ety2)
            when coll = SN.Collections.cPair ->
              (match el with
            | [x1; x2] ->
                let env, te1, _ = assign p env x1 ty1 in
                let env, te2, _ = assign p env x2 ty2 in
                make_result env (T.List [te1; te2]) folded_ety2
            | _ ->
                Errors.pair_arity p;
                make_result env T.Any (r, Terr))
          (* tuple-like array *)
          | (r, Tarraykind (AKtuple fields)) ->
            let p1 = fst e1 in
            let p2 = Reason.to_pos r in
            let tyl = List.rev (IMap.values fields) in
            let size1 = List.length el in
            let size2 = List.length tyl in
            if size1 <> size2
            then begin
              Errors.tuple_arity p2 size2 p1 size1;
              make_result env T.Any (r, Terr)
            end
            else
              let env, reversed_tel =
                List.fold2_exn el tyl ~f:begin fun (env,tel) lvalue ty2 ->
                let env, te, _ = assign p env lvalue ty2 in
                env, te::tel
              end ~init:(env,[]) in
              make_result env (T.List (List.rev reversed_tel)) ty2
        (* Other, including tuples. Create a tuple type for the left hand
         * side and attempt subtype against it. In particular this deals with
         * types such as (string,int) | (int,bool) *)
        | _ ->
          let env, tyl =
            List.map_env env el (fun env _ -> Env.fresh_unresolved_type env) in
          let env = Type.sub_type p Reason.URassign env folded_ty2
              (Reason.Rwitness (fst e1), Ttuple tyl) in
          let env, reversed_tel =
            List.fold2_exn el tyl ~init:(env,[]) ~f:(fun (env,tel) lvalue ty2 ->
            let env, te, _ = assign p env lvalue ty2 in
            env, te::tel) in
          make_result env (T.List (List.rev reversed_tel)) ty2
        end in
    begin match resl with
      | [res] -> res
      | _ -> assign_simple p env e1 ty2
    end
  | _, Class_get _
  | _, Obj_get _ ->
      let lenv = env.Env.lenv in
      let no_fakes = LEnv.env_with_empty_fakes env in
      (* In this section, we check that the assignment is compatible with
       * the real type of a member. Remember that members can change
       * type (cf fake_members). But when we assign a value to $this->x,
       * we want to make sure that the type assign to $this->x is compatible
       * with the actual type hint. In this portion of the code, type-check
       * the assignment in an environment without fakes, and therefore
       * check that the assignment is compatible with the type of
       * the member.
       *)
      let env, te1, real_type = lvalue no_fakes e1 in
      let env, exp_real_type = Env.expand_type env real_type in
      let env = { env with Env.lenv = lenv } in
      let env, ety2 = Env.expand_type env ty2 in
      let real_type_list =
        match exp_real_type with
        | _, Tunresolved tyl -> tyl
        | ty -> [ty]
      in
      let env = List.fold_left real_type_list ~f:begin fun env real_type ->
        Type.sub_type p (Reason.URassign) env ety2 real_type
      end ~init:env in
      (match e1 with
      | _, Obj_get ((_, This | _, Lvar _ as obj),
                    (_, Id (_, member_name)),
                    _) ->
          let env, local = Env.FakeMembers.make p env obj member_name in
          let () = (match obj with
            | _, This ->
              Typing_suggest.save_member member_name env exp_real_type ty2
            | _ -> ()
          ) in
          let env, ty = set_valid_rvalue p env local ty2 in
          env, te1, ty
      | _, Class_get (x, (_, y)) ->
          let env, local = Env.FakeMembers.make_static p env x y in
          let env, ty3 = set_valid_rvalue p env local ty2 in
          (match x with
          | CIself
          | CIstatic ->
              Typing_suggest.save_member y env exp_real_type ty2;
          | _ -> ());
          env, te1, ty3
      | _ -> env, te1, ty2
      )
  | _, Array_get ((_, Lvar (_, lvar)) as shape, ((Some _) as e2)) ->
    let access_type = Typing_arrays.static_array_access env e2 in
      (* In the case of an assignment of the form $x['new_field'] = ...;
      * $x could be a shape where the field 'new_field' is not yet defined.
      * When that is the case we want to add the field to its type.
      *)
    let env, _te, shape_ty = expr env shape in
    let env, shape_ty = Typing_arrays.update_array_type_on_lvar_assignment
      p access_type env shape_ty in
    let env, _ = set_valid_rvalue p env lvar shape_ty in
    (* We still need to call assign_simple in order to bind the freshly
    * created variable in added shape field. Moreover, it's needed because
    * shape_ty could be more than just a shape. It could be an unresolved
    * type where some elements are shapes and some others are not.
    *)
    assign_simple p env e1 ty2
  | _, This ->
     Errors.this_lvalue p;
     make_result env T.Any (Reason.Rwitness p, Terr)
  | pref, Unop (Ast.Uref, e1') ->
    (* references can be "lvalues" in foreach bindings *)
    if Env.is_strict env then
      Errors.reference_expr pref;
    let env, texpr, ty = assign p env e1' ty2 in
    make_result env (T.Unop (Ast.Uref, texpr)) ty
  | _ ->
      assign_simple p env e1 ty2

and assign_simple pos env e1 ty2 =
  let env, te1, ty1 = lvalue env e1 in
  let env, ty2 = check_valid_rvalue pos env ty2 in
  let env, ty2 = TUtils.unresolved env ty2 in
  let env = Type.sub_type pos (Reason.URassign) env ty2 ty1 in
  env, te1, ty2

and array_field env = function
  | Nast.AFvalue ve ->
    let env, tve, tv = expr env ve in
    let env, tv = Typing_env.unbind env tv in
    env, (T.AFvalue tve, None, tv)
  | Nast.AFkvalue (ke, ve) ->
      let env, tke, tk = expr env ke in
      let env, tve, tv = expr env ve in
      let env, tv = Typing_env.unbind env tv in
      env, (T.AFkvalue (tke, tve), Some tk, tv)

and array_value env x =
  let env, te, ty = expr env x in
  let env, ty = Typing_env.unbind env ty in
  env, (te, ty)

and array_field_value env = function
  | Nast.AFvalue x
  | Nast.AFkvalue (_, x) ->
      array_value env x

and array_field_key env = function
  (* This shouldn't happen *)
  | Nast.AFvalue (p, _) ->
      env, (T.make_implicitly_typed_expr p T.Any,
        (Reason.Rwitness p, Tprim Tint))
  | Nast.AFkvalue (x, _) ->
      array_value env x

and akshape_field env = function
  | Nast.AFkvalue (k, v) ->
      let env, tek, tk = expr env k in
      let env, tk = Typing_env.unbind env tk in
      let env, tk = TUtils.unresolved env tk in
      let env, tev, tv = expr env v in
      let env, tv = Typing_env.unbind env tv in
      let env, tv = TUtils.unresolved env tv in
      let field_name =
        match TUtils.shape_field_name env Pos.none (snd k) with
        | Some field_name -> field_name
        | None -> assert false in  (* Typing_arrays.is_shape_like_array
                                    * should have prevented this *)
      env, T.AFkvalue (tek, tev), (field_name, (tk, tv))
  | Nast.AFvalue _ -> assert false (* Typing_arrays.is_shape_like_array
                                    * should have prevented this *)

and aktuple_afvalue env v =
  let env, tev, tv = expr env v in
  let env, tv = Typing_env.unbind env tv in
  let env, ty = TUtils.unresolved env tv in
  env, tev, ty

and aktuple_field env = function
  | Nast.AFvalue v -> aktuple_afvalue env v
  | Nast.AFkvalue _ -> assert false (* check_consistent_fields
                                     * should have prevented this *)
and check_parent_construct pos env el uel env_parent =
  let check_not_abstract = false in
  let env, env_parent = Phase.localize_with_self env env_parent in
  let env, _tcid, tel, tuel, parent =
    new_object ~check_not_abstract pos env CIparent el uel in
  let env, _ = Type.unify pos (Reason.URnone) env env_parent parent in
  env, tel, tuel, (Reason.Rwitness pos, Tprim Tvoid)

and call_parent_construct pos env el uel =
  let parent = Env.get_parent env in
  match parent with
    | _, Tapply _ ->
      check_parent_construct pos env el uel parent
    | _,
      (
        Tany
        | Tmixed
        | Tarray (_, _)
        | Tdarray (_, _)
        | Tvarray _
        | Tvarray_or_darray _
        | Tgeneric _
        | Toption _
        | Tprim _
        | Terr
        | Tfun _
        | Ttuple _
        | Tshape _
        | Taccess (_, _)
        | Tthis
      ) -> (* continue here *)
      let default = env, [], [], (Reason.Rnone, Tany) in
      match Env.get_self env with
        | _, Tclass ((_, self), _) ->
          (match Env.get_class env self with
            | Some ({tc_kind = Ast.Ctrait; _}
                       as trait) ->
              (match trait_most_concrete_req_class trait env with
                | None -> Errors.parent_in_trait pos; default
                | Some (_, parent_ty) ->
                  check_parent_construct pos env el uel parent_ty
              )
            | Some self_tc ->
              if not self_tc.tc_members_fully_known
              then () (* Don't know the hierarchy, assume it's correct *)
              else Errors.undefined_parent pos;
              default
            | None -> assert false)
        | _, (Terr | Tany | Tmixed | Tarraykind _ | Toption _
              | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tvar _
              | Tabstract (_, _) | Tanon (_, _) | Tunresolved _ | Tobject
             ) ->
           Errors.parent_outside_class pos;
           env, [], [], (Reason.Rnone, Terr)

(* parent::method() in a class definition invokes the specific parent
 * version of the method ... it better be callable *)
and check_abstract_parent_meth mname pos fty =
  if is_abstract_ft fty
  then Errors.parent_abstract_call mname pos (Reason.to_pos (fst fty));
  fty

and is_abstract_ft fty = match fty with
  | _r, Tfun { ft_abstract = true; _ } -> true
  | _r, (Terr | Tany | Tmixed | Tarraykind _ | Toption _ | Tprim _
            | Tvar _ | Tfun _ | Tclass (_, _) | Tabstract (_, _) | Ttuple _
            | Tanon _ | Tunresolved _ | Tobject | Tshape _
        )
    -> false

(* Depending on the kind of expression we are dealing with
 * The typing of call is different.
 *)
and dispatch_call p env call_type (fpos, fun_expr as e) hl el uel =
  let make_call env te thl tel tuel ty =
    env, T.make_typed_expr p ty (T.Call (call_type, te, thl, tel, tuel)), ty in
  let make_call_special env id tel ty =
    make_call env (T.make_implicitly_typed_expr fpos (T.Id id)) [] tel [] ty in
  match fun_expr with
  (* Special function `echo` *)
  | Id ((_, pseudo_func) as id) when pseudo_func = SN.SpecialFunctions.echo ->
      let env, tel, _ = exprs env el in
      make_call_special env id tel (Reason.Rwitness p, Tprim Tvoid)
  (* Special function `empty` *)
  | Id ((_, pseudo_func) as id) when pseudo_func = SN.PseudoFunctions.empty ->
    let env, tel, _ = exprs env el in
    if uel <> [] then
      Errors.unpacking_disallowed_builtin_function p pseudo_func;
    if Env.is_strict env then
      Errors.empty_in_strict p;
    make_call_special env id tel (Reason.Rwitness p, Tprim Tbool)
  (* Special function `isset` *)
  | Id ((_, pseudo_func) as id) when pseudo_func = SN.PseudoFunctions.isset ->
    let env, tel, _ = exprs env el in
    if uel <> [] then
      Errors.unpacking_disallowed_builtin_function p pseudo_func;
    if Env.is_strict env then
      Errors.isset_in_strict p;
    make_call_special env id tel (Reason.Rwitness p, Tprim Tbool)
  (* Special function `unset` *)
  | Id ((_, pseudo_func) as id) when pseudo_func = SN.PseudoFunctions.unset ->
     let env, tel, _ = exprs env el in
     if uel <> [] then
       Errors.unpacking_disallowed_builtin_function p pseudo_func;
     let env = if Env.is_strict env then
       (match el, uel with
         | [(_, Array_get ((_, Class_const _), Some _))], [] ->
           Errors.const_mutation p Pos.none "";
           env
         | [(_, Array_get (ea, Some _))], [] ->
           let env, _te, ty = expr env ea in
           if List.exists ~f:(fun super -> SubType.is_sub_type env ty super) [
             (Reason.Rnone, (Tclass ((Pos.none, SN.Collections.cDict),
               [(Reason.Rnone, Tany); (Reason.Rnone, Tany)])));
             (Reason.Rnone, (Tclass ((Pos.none, SN.Collections.cKeyset),
               [(Reason.Rnone, Tany)])));
             (Reason.Rnone, Tarraykind AKany)
           ] then env
           else begin
             let env, (r, ety) = Env.expand_type env ty in
             Errors.unset_nonidx_in_strict
               p
               (Reason.to_string ("This is " ^ Typing_print.error ety) r);
             env
           end
         | _ -> Errors.unset_nonidx_in_strict p []; env)
       else env in
      (match el with
        | [(p, Obj_get (_, _, OG_nullsafe))] ->
          begin
            Errors.nullsafe_property_write_context p;
            make_call_special env id tel (Reason.Rwitness p, Terr)
          end;
        | _ ->
          make_call_special env id tel (Reason.Rwitness p, Tprim Tvoid))
  (* Pseudo-function `get_called_class` *)
  | Id (cp, get_called_class) when
      get_called_class = SN.StdlibFunctions.get_called_class
      && el = [] && uel = [] ->
    (* get_called_class fetches the late-bound class *)
    if Env.is_outside_class env then Errors.static_outside_class p;
    class_const env p (CIstatic, (cp, SN.Members.mClass))
  (* Special function `array_filter` *)
  | Id ((_, array_filter) as id)
      when array_filter = SN.StdlibFunctions.array_filter && el <> [] && uel = [] ->
      (* dispatch the call to typecheck the arguments *)
      let env, fty = fun_type_of_id env id hl in
      let env, tel, tuel, res = call p env fty el uel in
      (* but ignore the result and overwrite it with custom return type *)
      let x = List.hd_exn el in
      let env, _tx, ty = expr env x in
      let explain_array_filter (r, t) =
        (Reason.Rarray_filter (p, r), t) in
      let get_value_type env tv =
        let env, tv =
          if List.length el > 1
          then env, tv
          else non_null env tv in
        env, explain_array_filter tv in
      let rec get_array_filter_return_type env ty =
        let env, ety = Env.expand_type env ty in
        (match ety with
        | (_, Tarraykind (AKany | AKempty)) as array_type ->
            env, array_type
        | (_, Tarraykind (AKtuple _)) ->
            let env, ty = Typing_arrays.downcast_aktypes env ty in
            get_array_filter_return_type env ty
        | (r, Tarraykind (AKvec tv | AKvarray tv)) ->
            let env, tv = get_value_type env tv in
            env, (r, Tarraykind (AKvec tv))
        | (r, Tunresolved x) ->
            let env, x = List.map_env env x get_array_filter_return_type in
            env, (r, Tunresolved x)
        | (r, Tany) ->
            env, (r, Tany)
        | (r, Terr) ->
            env, (r, Terr)
        | (r, _) ->
            let tk, tv = Env.fresh_type (), Env.fresh_type () in
            Errors.try_
              (fun () ->
                let keyed_container = (
                  Reason.Rnone,
                  Tclass (
                    (Pos.none, SN.Collections.cKeyedContainer), [tk; tv]
                  )
                ) in
                let env = SubType.sub_type env ety keyed_container in
                let env, tv = get_value_type env tv in
                env, (r, Tarraykind (AKmap (
                  (explain_array_filter tk),
                  tv)
                )))
              (fun _ -> Errors.try_
                (fun () ->
                  let container = (
                    Reason.Rnone,
                    Tclass (
                      (Pos.none, SN.Collections.cContainer), [tv]
                    )
                  ) in
                  let env = SubType.sub_type env ety container in
                  let env, tv = get_value_type env tv in
                  env, (r, Tarraykind (AKmap (
                    (explain_array_filter (r, Tprim Tarraykey)),
                    tv))))
                (fun _ -> env, res)))
      in let env, rty = get_array_filter_return_type env ty in
      make_call env (T.make_implicitly_typed_expr fpos (T.Id id)) hl tel tuel rty
  (* Special function `type_structure` *)
  | Id (p, type_structure)
      when type_structure = SN.StdlibFunctions.type_structure
           && (List.length el = 2) && uel = [] ->
    (match el with
     | [e1; e2] ->
       (match e2 with
        | p, Nast.String cst ->
          (* find the class constant implicitly defined by the typeconst *)
          let cid = (match e1 with
            | _, Class_const (cid, (_, x))
            | _, Class_get (cid, (_, x)) when x = SN.Members.mClass -> cid
            | _ -> Nast.CIexpr e1) in
          class_const ~incl_tc:true env p (cid, cst)
        | _ ->
          Errors.illegal_type_structure p "second argument is not a string";
          expr_error env Reason.Rnone)
     | _ -> assert false)
  (* Special function `array_map` *)
  | Id ((_, array_map) as x)
      when array_map = SN.StdlibFunctions.array_map && el <> [] && uel = [] ->
      let env, fty = fun_type_of_id env x [] in
      let env, fty = Env.expand_type env fty in
      let env, fty = match fty, el with
        | ((r_fty, Tfun fty), _::args) when args <> [] ->
          let arity = List.length args in
          (*
            Builds a function with signature:

            function<T1, ..., Tn, Tr>(
              (function(T1, ..., Tn):Tr),
              Container<T1>,
              ...,
              Container<Tn>,
            ): R

            where R is constructed by build_output_container applied to Tr
          *)
          let build_function env build_output_container =
            let env, vars, tr =
              (* If T1, ... Tn, Tr are provided explicitly, instantiate the function parameters with
               * those directly. *)
              if List.length hl = 0
              then env, List.map args (fun _ -> Env.fresh_type()), Env.fresh_type ()
              else if List.length hl <> List.length args + 1 then begin
                Errors.expected_tparam fty.ft_pos (1 + (List.length args));
                env, List.map args (fun _ -> Env.fresh_type()), Env.fresh_type () end
              else
              let env, vars_and_tr = List.map_env env hl Phase.hint_locl in
              let vars, trl = List.split_n vars_and_tr (List.length vars_and_tr - 1) in
              (* Since we split the arguments and return type at the last index and the length is
                 non-zero this is safe. *)
              let tr = List.hd_exn trl in
              env, vars, tr
            in
            let f = (None, (
              r_fty,
              Tfun {
                ft_pos = fty.ft_pos;
                ft_deprecated = None;
                ft_abstract = false;
                ft_arity = Fstandard (arity, arity);
                ft_tparams = [];
                ft_where_constraints = [];
                ft_params = List.map vars (fun x -> (None, x));
                ft_ret = tr;
              }
            )) in
            let containers = List.map vars (fun var ->
              (None,
                (r_fty,
                  Tclass ((fty.ft_pos, SN.Collections.cContainer), [var])
                )
              )
            ) in
            env, (r_fty, Tfun {fty with
                               ft_arity = Fstandard (arity+1, arity+1);
                               ft_params = f::containers;
                               ft_ret =  build_output_container tr;
                              })
          in

          (*
            Takes a Container type and returns a function that can "pack" a type
            into an array of appropriate shape, preserving the key type, i.e.:
            array                 -> f, where f R = array
            array<X>              -> f, where f R = array<R>
            array<X, Y>           -> f, where f R = array<X, R>
            Vector<X>             -> f  where f R = array<R>
            KeyedContainer<X, Y>  -> f, where f R = array<X, R>
            Container<X>          -> f, where f R = array<arraykey, R>
            X                     -> f, where f R = Y
          *)
          let rec build_output_container
            (env:Env.env) (x:locl ty) : (Env.env * (locl ty -> locl ty)) =
            let env, x = Env.expand_type env x in (match x with
              | (_, Tarraykind (AKany | AKempty)) as array_type ->
                env, (fun _ -> array_type)
              | (_, Tarraykind (AKtuple _ )) ->
                let env, x = Typing_arrays.downcast_aktypes env x in
                build_output_container env x
              | (r, Tarraykind (AKvec _ | AKvarray _)) ->
                env, (fun tr -> (r, Tarraykind (AKvec(tr))) )
              | ((_, Tany) as any) ->
                env, (fun _ -> any)
              | ((_, Terr) as err) ->
                env, (fun _ -> err)
              | (r, Tunresolved x) ->
                let env, x = List.map_env env x build_output_container in
                env, (fun tr -> (r, Tunresolved (List.map x (fun f -> f tr))))
              | (r, _) ->
                let tk, tv = Env.fresh_type(), Env.fresh_type() in
                let try_vector env =
                  let vector = (
                    r_fty,
                    Tclass (
                      (fty.ft_pos, SN.Collections.cConstVector), [tv]
                    )
                  ) in
                  let env = SubType.sub_type env x vector in
                  env, (fun tr -> (r, Tarraykind (
                    AKvec(tr)
                  ))) in
                let try_keyed_container env =
                  let keyed_container = (
                    r_fty,
                    Tclass (
                      (fty.ft_pos, SN.Collections.cKeyedContainer), [tk; tv]
                    )
                  ) in
                  let env = SubType.sub_type env x keyed_container in
                  env, (fun tr -> (r, Tarraykind (AKmap (
                    tk,
                    tr
                  )))) in
                let try_container env =
                  let container = (
                    r_fty,
                    Tclass (
                      (fty.ft_pos, SN.Collections.cContainer), [tv]
                    )
                  ) in
                  let env = SubType.sub_type env x container in
                  env, (fun tr -> (r, Tarraykind (AKmap (
                    (r, Tprim Tarraykey),
                    tr)))) in
                Errors.try_
                  (fun () ->
                    try_vector  env)
                  (fun _ -> Errors.try_
                    (fun () ->
                      try_keyed_container env)
                    (fun _ -> Errors.try_
                      (fun () ->
                        try_container env)
                      (fun _ -> env, (fun _ -> (Reason.Rwitness p, Tany)))))) in
          (*
            Single argument calls preserve the key type, multi argument
            calls always return an array<Tr>
          *)
          (match args with
            | [x] ->
              let env, _tx, x = expr env x in
              let env, output_container = build_output_container env x in
              build_function env output_container
            | _ ->
              build_function env (fun tr ->
                (r_fty, Tarraykind (AKvec(tr)))))
        | _ -> env, fty in
      let env, tel, _tuel, ty = call p env fty el [] in
      make_call_special env x tel ty
  (* Special function `idx` *)
  | Id ((_, idx) as id) when idx = SN.FB.idx ->
      (* Directly call get_fun so that we can muck with the type before
       * instantiation -- much easier to work in terms of Tgeneric Tk/Tv than
       * trying to figure out which Tvar is which. *)
      Typing_hooks.dispatch_fun_id_hook (p, SN.FB.idx);
      (match Env.get_fun env (snd id) with
      | Some fty ->
        let param1, (name2, (r2, _)), (name3, (r3, _)) =
          match fty.ft_params with
            | [param1; param2; param3] -> param1, param2, param3
            | _ -> assert false in
        let params, ret = match List.length el with
          | 2 ->
            let param2 = (name2, (r2, Toption (r2, Tgeneric "Tk"))) in
            let rret = fst fty.ft_ret in
            let ret = (rret, Toption (rret, Tgeneric "Tv")) in
            [param1; param2], ret
          | 3 ->
            let param2 = (name2, (r2, Tgeneric "Tk")) in
            let param3 = (name3, (r3, Tgeneric "Tv")) in
            let ret = (fst fty.ft_ret, Tgeneric "Tv") in
            [param1; param2; param3], ret
          | _ -> fty.ft_params, fty.ft_ret in
        let fty = { fty with ft_params = params; ft_ret = ret } in
        let ety_env = Phase.env_with_self env in
        let env, fty = Phase.localize_ft ~ety_env env fty in
        let tfun = Reason.Rwitness fty.ft_pos, Tfun fty in
        let env, tel, _tuel, ty = call p env tfun el [] in
        make_call_special env id tel ty
      | None -> unbound_name env id)

  (* Special function `Shapes::idx` *)
  | Class_const (CI((_, shapes), _) as class_id, ((_, idx) as method_id))
      when shapes = SN.Shapes.cShapes && idx = SN.Shapes.idx ->
      overload_function p env class_id method_id el uel
      begin fun env fty res el -> match el with
        | [shape; field] ->
          let env, _ts, shape_ty = expr env shape in
          Typing_shapes.idx env fty shape_ty field None
        | [shape; field; default] ->
            let env, _ts, shape_ty = expr env shape in
            let env, _td, default_ty = expr env default in
            Typing_shapes.idx env fty shape_ty field
              (Some ((fst default), default_ty))
        | _ -> env, res
      end
   (* Special function `Shapes::keyExists` *)
   | Class_const (CI((_, shapes), _) as class_id, ((_, key_exists) as method_id))
      when shapes = SN.Shapes.cShapes && key_exists = SN.Shapes.keyExists ->
      overload_function p env class_id method_id el uel
      begin fun env fty res el -> match el with
        | [shape; field] ->
          let env, _te, shape_ty = expr env shape in
          (* try accessing the field, to verify existence, but ignore
           * the returned type and keep the one coming from function
           * return type hint *)
          let env, _ = Typing_shapes.idx env fty shape_ty field None in
          env, res
        | _  -> env, res
      end
   (* Special function `Shapes::removeKey` *)
   | Class_const (CI((_, shapes), _) as class_id, ((_, remove_key) as method_id))
      when shapes = SN.Shapes.cShapes && remove_key = SN.Shapes.removeKey ->
      overload_function p env class_id method_id el uel
      begin fun env _ res el -> match el with
        | [shape; field] -> begin match shape with
            | (_, Lvar (_, lvar)) ->
              let env, _te, shape_ty = expr env shape in
              let env, shape_ty =
                Typing_shapes.remove_key p env shape_ty field in
              let env, _ = set_valid_rvalue p env lvar shape_ty in
              env, res
            | _ ->
              Errors.invalid_shape_remove_key (fst shape);
              env, res
          end
        | _  -> env, res
      end
  (* Special function `Shapes::toArray` *)
  | Class_const (CI((_, shapes), _) as class_id, ((_, to_array) as method_id))
    when shapes = SN.Shapes.cShapes && to_array = SN.Shapes.toArray ->
    overload_function p env class_id method_id el uel
    begin fun env _ res el -> match el with
      | [shape] ->
         let env, _te, shape_ty = expr env shape in
         Typing_shapes.to_array env shape_ty res
      | _  -> env, res
    end

  (* Special function `parent::__construct` *)
  | Class_const (CIparent, ((callee_pos, construct) as id))
    when construct = SN.Members.__construct ->
      Typing_hooks.dispatch_parent_construct_hook env callee_pos;
      let env, tel, tuel, ty = call_parent_construct p env el uel in
      make_call env (T.make_implicitly_typed_expr fpos
        (T.Class_const (T.CIparent, id))) hl tel tuel ty

  (* Calling parent method *)
  | Class_const (CIparent, m) ->
      let env, _te, ty1 = static_class_id p env CIparent in
      if Env.is_static env
      then begin
        (* in static context, you can only call parent::foo() on static
         * methods *)
        let env, fty, _ =
          class_get ~is_method:true ~is_const:false ~explicit_tparams:hl env ty1 m CIparent in
        let fty = check_abstract_parent_meth (snd m) p fty in
        let env, tel, tuel, ty = call p env fty el uel in
        make_call env (T.make_typed_expr fpos fty
          (T.Class_const (T.CIparent, m))) hl tel tuel ty
      end
      else begin
        (* in instance context, you can call parent:foo() on static
         * methods as well as instance methods *)
        if not(class_contains_smethod env ty1 m)
        then
            (* parent::nonStaticFunc() is really weird. It's calling a method
             * defined on the parent class, but $this is still the child class.
             * We can deal with this by hijacking the continuation that
             * calculates the SN.Typehints.this type *)
            let env, this_ty = ExprDepTy.make env CIstatic
              (Reason.Rwitness fpos, TUtils.this_of (Env.get_self env)) in
            let k_lhs _ = this_ty in
            let env, method_, _ =
              obj_get_ ~is_method:true ~nullsafe:None env ty1 CIparent m
              begin fun (env, fty, _) ->
                let fty = check_abstract_parent_meth (snd m) p fty in
                let env, _tel, _tuel, method_ = call p env fty el uel in
                env, method_, None
              end
              k_lhs
            in
            make_call env (T.make_typed_expr fpos this_ty
              (T.Class_const (T.CIparent, m))) hl [] [] method_
        else
            let env, fty, _ =
              class_get ~is_method:true ~is_const:false ~explicit_tparams:hl env ty1 m CIparent in
            let fty = check_abstract_parent_meth (snd m) p fty in
            let env, tel, tuel, ty = call p env fty el uel in
            make_call env (T.make_typed_expr fpos fty
              (T.Class_const (T.CIparent, m))) hl tel tuel ty
      end
  (* Call class method *)
  | Class_const(e1, m) ->
      TUtils.process_static_find_ref e1 m;
      let env, te1, ty1 = static_class_id p env e1 in
      let env, fty, _ =
        class_get ~is_method:true ~is_const:false ~explicit_tparams:hl env ty1 m e1 in
      let () = match e1 with
        | CIself when is_abstract_ft fty ->
          (match Env.get_self env with
            | _, Tclass ((_, self), _) ->
              (* at runtime, self:: in a trait is a call to whatever
               * self:: is in the context of the non-trait "use"-ing
               * the trait's code *)
              (match Env.get_class env self with
                | Some { tc_kind = Ast.Ctrait; _ } -> ()
                | _ -> Errors.self_abstract_call (snd m) p (Reason.to_pos (fst fty))
              )
            | _ -> ())
        | CI (c, _) when is_abstract_ft fty ->
          Errors.classname_abstract_call (snd c) (snd m) p (Reason.to_pos (fst fty))
        | _ -> () in
      let env, tel, tuel, ty = call p env fty el uel in
      make_call env (T.make_typed_expr fpos fty
        (T.Class_const(te1, m))) hl tel tuel ty

  (* Call instance method *)
  | Obj_get(e1, (pos_id, Id m), nullflavor) ->
      let is_method = call_type = Cnormal in
      let env, te1, ty1 = expr env e1 in
      let nullsafe =
        (match nullflavor with
          | OG_nullthrows -> None
          | OG_nullsafe -> Some p
        ) in
      let fn = (fun (env, fty, _) ->
        let env, _tel, _tuel, method_ = call p env fty el uel in
        env, method_, None) in
      let env, ty = obj_get ~is_method ~nullsafe ~explicit_tparams:hl env ty1 (CIexpr e1) m fn in
      make_call env (T.make_implicitly_typed_expr fpos (T.Obj_get(te1,
        T.make_implicitly_typed_expr pos_id (T.Id m), nullflavor))) hl [] [] ty

  (* Function invocation *)
  | Fun_id x ->
      Typing_hooks.dispatch_id_hook x env;
      let env, fty = fun_type_of_id env x hl in
      let env, tel, tuel, ty = call p env fty el uel in
      make_call env (T.make_typed_expr fpos fty (T.Fun_id x)) hl tel tuel ty
  | Id x ->
      Typing_hooks.dispatch_id_hook x env;
      let env, fty = fun_type_of_id env x hl in
      let env, tel, tuel, ty = call p env fty el uel in
      make_call env (T.make_typed_expr fpos fty (T.Id x)) hl tel tuel ty
  | _ ->
      let env, te, fty = expr env e in
      let env, tel, tuel, ty = call p env fty el uel in
      make_call env te hl tel tuel ty

and fun_type_of_id env x hl =
  Typing_hooks.dispatch_fun_id_hook x;
  let env, fty =
    match Env.get_fun env (snd x) with
    | None -> let env, _, ty = unbound_name env x in env, ty
    | Some fty ->
        let ety_env = Phase.env_with_self env in
        let env, fty = Phase.localize_ft ~explicit_tparams:hl ~ety_env env fty in
        env, (Reason.Rwitness fty.ft_pos, Tfun fty)
  in
  env, fty

(*****************************************************************************)
(* Function type-checking expressions accessing an array (example: $x[...]).
 * The parameter is_lvalue is true when the expression is on the left hand
 * side of an assignment (example: $x[...] = 0).
 *)
(*****************************************************************************)
and array_get ?(lhs_of_null_coalesce=false) is_lvalue p env ty1 e2 ty2 =
  (* This is a little weird -- we enforce the right arity when you use certain
   * collections, even in partial mode (where normally completely omitting the
   * type parameter list is admitted). Basically the "omit type parameter"
   * hole was for compatibility with certain interfaces like ArrayAccess, not
   * for collections! But it's hard to go back on now, so since we've always
   * errored (with an inscrutable error message) when you try to actually use
   * a collection with omitted type parameters, we can continue to error and
   * give a more useful error message. *)
  let env, ety1 = Env.expand_type env ty1 in
  let arity_error (_, name) =
    Errors.array_get_arity p name (Reason.to_pos (fst ety1)) in
  match snd ety1 with
  | Tunresolved tyl ->
      let env, tyl = List.map_env env tyl begin fun env ty1 ->
        array_get ~lhs_of_null_coalesce is_lvalue p env ty1 e2 ty2
      end in
      env, (fst ety1, Tunresolved tyl)
  | Tarraykind (AKvarray ty | AKvec ty) ->
      let ty1 = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
      let env = Type.sub_type p Reason.index_array env ty2 ty1 in
      env, ty
  | Tarraykind (AKvarray_or_darray ty) ->
      let ty1 = Reason.Rvarray_or_darray_key p, Tprim Tarraykey in
      let env = Type.sub_type p Reason.index_array env ty2 ty1 in
      env, ty
  | Tclass ((_, cn) as id, argl)
    when cn = SN.Collections.cVector
    || cn = SN.Collections.cVec ->
      let ty = match argl with
        | [ty] -> ty
        | _ -> arity_error id; err_witness p in
      let ty1 = Reason.Ridx_vector (fst e2), Tprim Tint in
      let env = Type.sub_type p (Reason.index_class cn) env ty2 ty1 in
      env, ty
  | Tclass ((_, cn) as id, argl)
    when cn = SN.Collections.cMap
    || cn = SN.Collections.cStableMap
    || cn = SN.Collections.cDict
    || cn = SN.Collections.cKeyset ->
      if cn = SN.Collections.cKeyset && is_lvalue then begin
        Errors.keyset_set p (Reason.to_pos (fst ety1));
        env, (Reason.Rwitness p, Terr)
      end else
        let (k, v) = match argl with
          | [t] when cn = SN.Collections.cKeyset -> (t, t)
          | [k; v] when cn <> SN.Collections.cKeyset -> (k, v)
          | _ ->
              arity_error id;
              let any = err_witness p in
              any, any
        in
        let env, ty2 = TUtils.unresolved env ty2 in
        let env = Type.sub_type p (Reason.index_class cn) env ty2 k in
        env, v
  (* Certain container/collection types are intended to be immutable/const,
   * thus they should never appear as a lvalue when indexing i.e.
   *
   *   $x[0] = 100; // ERROR
   *   $x[0]; // OK
   *)
  | Tclass ((_, cn) as id, argl)
      when cn = SN.Collections.cConstMap
        || cn = SN.Collections.cImmMap
        || cn = SN.Collections.cIndexish
        || cn = SN.Collections.cKeyedContainer ->
    if is_lvalue then
      error_const_mutation env p ety1
    else
      let (k, v) = match argl with
        | [k; v] -> (k, v)
        | _ ->
            arity_error id;
            let any = err_witness p in
            any, any
      in
      let env = Type.sub_type p (Reason.index_class cn) env ty2 k in
      env, v
  | Tclass ((_, cn) as id, argl)
      when not is_lvalue &&
        (cn = SN.Collections.cConstVector || cn = SN.Collections.cImmVector) ->
      let ty = match argl with
        | [ty] -> ty
        | _ -> arity_error id; err_witness p in
      let ty1 = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
      let env = Type.sub_type p (Reason.index_class cn) env ty2 ty1 in
      env, ty
  | Tclass ((_, cn), _)
      when is_lvalue &&
        (cn = SN.Collections.cConstVector || cn = SN.Collections.cImmVector) ->
    error_const_mutation env p ety1
  | Tarraykind (AKdarray (k, v) | AKmap (k, v)) ->
      let env, ty2 = TUtils.unresolved env ty2 in
      let env = Type.sub_type p Reason.index_array env ty2 k in
      env, v
  | Tarraykind ((AKshape  _ |  AKtuple _) as akind) ->
      let key = Typing_arrays.static_array_access env (Some e2) in
      let env, result = match key, akind with
        | Typing_arrays.AKtuple_index index, AKtuple fields ->
            begin match IMap.get index fields with
              | Some ty ->
                  let ty1 = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
                  let env = Type.sub_type p Reason.index_array env ty2 ty1 in
                  env, Some ty
              | None -> env, None
            end
        | Typing_arrays.AKshape_key field_name, AKshape fdm ->
            begin match Nast.ShapeMap.get field_name fdm with
              | Some (k, v) ->
                  let env, ty2 = TUtils.unresolved env ty2 in
                  let env = Type.sub_type p Reason.index_array env ty2 k in
                  env, Some v
              | None -> env, None
            end
        | _ -> env, None in
      begin match result with
        | Some ty -> env, ty
        | None ->
          (* Key is dynamic, or static and not in the array - treat it as
            regular map or vec like array *)
          let env, ty1 = Typing_arrays.downcast_aktypes env ety1 in
          array_get is_lvalue p env ty1 e2 ty2
      end
  | Terr -> env, (Reason.Rnone, Terr)
  | Tany | Tarraykind (AKany | AKempty) ->
      env, (Reason.Rnone, Tany)
  | Tprim Tstring ->
      let ty = Reason.Rwitness p, Tprim Tstring in
      let int = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
      let env = Type.sub_type p Reason.index_array env ty2 int in
      env, ty
  | Ttuple tyl ->
      (match e2 with
      | p, Int n ->
          (try
            let idx = int_of_string (snd n) in
            let nth = List.nth_exn tyl idx in
            env, nth
          with _ ->
            Errors.typing_error p (Reason.string_of_ureason Reason.index_tuple);
            env, (Reason.Rwitness p, Terr)
          )
      | p, _ ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URtuple_access);
          env, (Reason.Rwitness p, Terr)
      )
  | Tclass ((_, cn) as id, argl) when cn = SN.Collections.cPair ->
      let (ty1, ty2) = match argl with
        | [ty1; ty2] -> (ty1, ty2)
        | _ ->
            arity_error id;
            let any = err_witness p in
            any, any
      in
      (match e2 with
      | p, Int n ->
          (try
            let idx = int_of_string (snd n) in
            let nth = List.nth_exn [ty1; ty2] idx in
            env, nth
          with _ ->
            Errors.typing_error p @@
            Reason.string_of_ureason (Reason.index_class cn);
            env, (Reason.Rwitness p, Terr)
          )
      | p, _ ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URpair_access);
          env, (Reason.Rwitness p, Terr)
      )
  | Tshape (_, fdm) ->
    let p, e2' = e2 in
    (match TUtils.shape_field_name env p e2' with
      | None ->
          (* there was already an error in shape_field name,
             don't report another one for a missing field *)
          env, (Reason.Rwitness p, Terr)
      | Some field -> (match ShapeMap.get field fdm with
        | None ->
          Errors.undefined_field
            p (TUtils.get_printable_shape_field_name field);
          env, (Reason.Rwitness p, Terr)
        | Some { sft_optional = true; _ }
          when not is_lvalue && not lhs_of_null_coalesce ->
          let declared_field =
              List.find_exn
                ~f:(fun x -> Ast.ShapeField.compare field x = 0)
                (ShapeMap.keys fdm) in
          let declaration_pos = match declared_field with
            | Ast.SFlit (p, _) | Ast.SFclass_const ((p, _), _) -> p in
          Errors.array_get_with_optional_field
            p
            declaration_pos
            (TUtils.get_printable_shape_field_name field);
          env, (Reason.Rwitness p, Terr)
        | Some { sft_optional = _; sft_ty } -> env, sft_ty)
    )
  | Toption tyl when lhs_of_null_coalesce ->
      (* Normally, we would not allow indexing into a nullable container,
         however, because the pattern shows up so frequently, we are allowing
         indexing into a nullable container as long as it is on the lhs of a
         null coalesce *)
      array_get ~lhs_of_null_coalesce is_lvalue p env tyl e2 ty2
  | Toption _ ->
      Errors.null_container p
        (Reason.to_string
          "This is what makes me believe it can be null"
          (fst ety1)
        );
      env, (Reason.Rwitness p, Terr)
  | Tobject ->
      if Env.is_strict env
      then error_array env p ety1
      else env, (Reason.Rnone, Tany)
  | Tabstract (AKnewtype (ts, [ty]), Some (r, Tshape (fk, fields)))
        when ts = SN.FB.cTypeStructure ->
      let env, fields = TS.transform_shapemap env ty fields in
      let ty = r, Tshape (fk, fields) in
      array_get is_lvalue p env ty e2 ty2
  | Tabstract _ ->
    let resl =
    try_over_concrete_supertypes env ety1
      begin fun env ty ->
        array_get is_lvalue p env ty e2 ty2
      end in
    begin match resl with
    | [res] -> res
    | res::rest
      when List.for_all rest ~f:(fun x -> ty_equal (snd x) (snd res)) -> res
    | _ -> error_array env p ety1
    end
  | Tmixed | Tprim _ | Tvar _ | Tfun _
  | Tclass (_, _) | Tanon (_, _) ->
      error_array env p ety1

and array_append p env ty1 =
  let env, ty1 = TUtils.fold_unresolved env ty1 in
  let resl = try_over_concrete_supertypes env ty1
    begin fun env ty ->
      match ty with
      | (_, ty_) ->
        match ty_ with
        | Tany | Tarraykind (AKany | AKempty) ->
          env, (Reason.Rnone, Tany)

        | Terr ->
          env, (Reason.Rnone, Terr)

        | Tclass ((_, n), [ty])
            when n = SN.Collections.cVector
            || n = SN.Collections.cSet
            || n = SN.Collections.cVec
            || n = SN.Collections.cKeyset ->
            env, ty
        | Tclass ((_, n), [])
            when n = SN.Collections.cVector || n = SN.Collections.cSet ->
            (* Handle the case where "Vector" or "Set" was used as a typehint
               without type parameters *)
            env, (Reason.Rnone, Tany)
        | Tclass ((_, n), [tkey; tvalue]) when n = SN.Collections.cMap ->
            (* You can append a pair to a map *)
          env, (Reason.Rmap_append p, Tclass ((p, SN.Collections.cPair),
              [tkey; tvalue]))
        | Tclass ((_, n), []) when n = SN.Collections.cMap ->
            (* Handle the case where "Map" was used as a typehint without
               type parameters *)
            env, (Reason.Rmap_append p,
              Tclass ((p, SN.Collections.cPair), []))
        | Tarraykind (AKvec ty | AKvarray ty) ->
            env, ty
        | Tobject ->
            if Env.is_strict env
            then error_array_append env p ty1
            else env, (Reason.Rnone, Tany)
        | Tmixed | Tarraykind _ | Toption _ | Tprim _
        | Tvar _ | Tfun _ | Tclass (_, _) | Ttuple _
        | Tanon (_, _) | Tunresolved _ | Tshape _ | Tabstract _ ->
          error_array_append env p ty1
    end in
  match resl with
  | [res] -> res
  | _ -> error_array_append env p ty1


and error_array env p (r, ty) =
  Errors.array_access p (Reason.to_pos r) (Typing_print.error ty);
  env, err_witness p

and error_array_append env p (r, ty) =
  Errors.array_append p (Reason.to_pos r) (Typing_print.error ty);
  env, err_witness p

and error_const_mutation env p (r, ty) =
  Errors.const_mutation p (Reason.to_pos r) (Typing_print.error ty);
  env, err_witness p

(**
 * Checks if a class (given by cty) contains a given static method.
 *
 * We could refactor this + class_get
 *)
and class_contains_smethod env cty (_pos, mid) =
  let lookup_member ty =
    match ty with
    | _, Tclass ((_, c), _) ->
      (match Env.get_class env c with
      | None -> false
      | Some class_ ->
        Option.is_some @@ Env.get_static_member true env class_ mid
      )
    | _ -> false in
  let _env, tyl = TUtils.get_concrete_supertypes env cty in
  List.exists tyl ~f:lookup_member

and class_get ~is_method ~is_const ?(explicit_tparams=[]) ?(incl_tc=false) env cty (p, mid) cid =
  let env, this_ty =
    if is_method then
      this_for_method env cid cty
    else
      env, cty in
  let ety_env = {
    type_expansions = [];
    this_ty = this_ty;
    substs = SMap.empty;
    from_class = Some cid;
  } in
  class_get_ ~is_method ~is_const ~ety_env ~explicit_tparams ~incl_tc env cid cty (p, mid)

and class_get_ ~is_method ~is_const ~ety_env ?(explicit_tparams=[]) ?(incl_tc=false) env cid cty
(p, mid) =
  let env, cty = Env.expand_type env cty in
  match cty with
  | _, Tany -> env, (Reason.Rnone, Tany), None
  | _, Terr -> env, err_none, None
  | _, Tunresolved tyl ->
      let env, tyl = List.map_env env tyl begin fun env ty ->
      let env, ty, _ =
        class_get_ ~is_method ~is_const ~ety_env ~explicit_tparams ~incl_tc env cid ty (p, mid)
          in env, ty
      end in
      let env, method_ = TUtils.in_var env (fst cty, Tunresolved tyl) in
      env, method_, None

  | _, Tabstract _ ->
      begin match TUtils.get_concrete_supertypes env cty with
      | env, [cty] ->
        class_get_ ~is_method ~is_const ~ety_env ~explicit_tparams ~incl_tc env cid cty (p, mid)
      | env, _ ->
        env, (Reason.Rnone, Tany), None
      end
  | _, Tclass ((_, c), paraml) ->
      let class_ = Env.get_class env c in
      (match class_ with
      | None -> env, (Reason.Rnone, Tany), None
      | Some class_ ->
        Typing_hooks.dispatch_smethod_hook class_ paraml (p, mid) env
          ety_env.from_class ~is_method ~is_const;
        (* We need to instantiate generic parameters in the method signature *)
        let ety_env =
          { ety_env with
            substs = Subst.make class_.tc_tparams paraml } in
        if is_const then begin
          let const =
            if incl_tc then Env.get_const env class_ mid else
            match Env.get_typeconst env class_ mid with
            | Some _ ->
              Errors.illegal_typeconst_direct_access p;
              None
            | None ->
              Env.get_const env class_ mid
          in
          match const with
          | None ->
            smember_not_found p ~is_const ~is_method class_ mid;
            env, (Reason.Rnone, Terr), None
          | Some { cc_type; cc_abstract; cc_pos; _ } ->
            let env, cc_type = Phase.localize ~ety_env env cc_type in
            env, cc_type,
            (if cc_abstract
             then Some (cc_pos, class_.tc_name ^ "::" ^ mid)
             else None)
        end else begin
          let smethod = Env.get_static_member is_method env class_ mid in
          match smethod with
          | None ->
            (match Env.get_static_member is_method env class_
              SN.Members.__callStatic with
              | None ->
                smember_not_found p ~is_const ~is_method class_ mid;
                env, (Reason.Rnone, Terr), None
              | Some {ce_visibility = vis; ce_type = lazy (r, Tfun ft); _} ->
                let p_vis = Reason.to_pos r in
                TVis.check_class_access p env (p_vis, vis) cid class_;
                let env, ft =
                  Phase.localize_ft ~ety_env ~explicit_tparams:explicit_tparams env ft in
                let ft = { ft with
                  ft_arity = Fellipsis 0;
                  ft_tparams = []; ft_params = [];
                } in
                env, (r, Tfun ft), None
              | _ -> assert false)
          | Some { ce_visibility = vis; ce_type = lazy method_; _ } ->
            let p_vis = Reason.to_pos (fst method_) in
            TVis.check_class_access p env (p_vis, vis) cid class_;
            let env, method_ =
              begin match method_ with
                (* We special case Tfun here to allow passing in explicit tparams to localize_ft. *)
                | r, Tfun ft ->
                  let env, ft =
                    Phase.localize_ft ~ety_env ~explicit_tparams:explicit_tparams env ft
                  in env, (r, Tfun ft)
                | _ -> Phase.localize ~ety_env env method_
              end in
            env, method_, None
        end
      )
  | _, (Tmixed | Tarraykind _ | Toption _
        | Tprim _ | Tvar _ | Tfun _ | Ttuple _ | Tanon (_, _) | Tobject
       | Tshape _) ->
      (* should never happen; static_class_id takes care of these *)
      env, (Reason.Rnone, Tany), None

and smember_not_found pos ~is_const ~is_method class_ member_name =
  let kind =
    if is_const then `class_constant
    else if is_method then `static_method
    else `class_variable in
  let error hint =
    let cid = (class_.tc_pos, class_.tc_name) in
    Errors.smember_not_found kind pos cid member_name hint
  in
  match Env.suggest_static_member is_method class_ member_name with
  | None ->
      (match Env.suggest_member is_method class_ member_name with
      | None when not class_.tc_members_fully_known ->
          (* no error in this case ... the member might be present
           * in one of the parents of class_ that the typing cannot see *)
          ()
      | None ->
          error `no_hint
      | Some (pos2, v) ->
          error (`closest (pos2, v))
      );
  | Some (pos2, v) ->
      error (`did_you_mean (pos2, v))

and member_not_found pos ~is_method class_ member_name r =
  let kind = if is_method then `method_ else `member in
  let cid = class_.tc_pos, class_.tc_name in
  let reason = Reason.to_string
    ("This is why I think it is an object of type "^strip_ns class_.tc_name) r
  in
  let error hint =
    Errors.member_not_found kind pos cid member_name hint reason in
  match Env.suggest_member is_method class_ member_name with
    | None ->
      (match Env.suggest_static_member is_method class_ member_name with
        | None when not class_.tc_members_fully_known ->
          (* no error in this case ... the member might be present
           * in one of the parents of class_ that the typing cannot see *)
          ()
        | None ->
          error `no_hint
        | Some (def_pos, v) ->
          error (`closest (def_pos, v))
      )
    | Some (def_pos, v) ->
        error (`did_you_mean (def_pos, v))

(* The type of the object member is passed into the continuation k. This is
 * useful for typing nullsafed method calls. Consider `$x?->f()`: obj_get will
 * pass `k` the type of f, and `k` will typecheck the method call and return
 * the method's return type. obj_get then wraps that type in a Toption. *)
and obj_get ~is_method ~nullsafe ?(explicit_tparams=[]) env ty1 cid id k =
  let env =
    match nullsafe with
    | Some p when not (type_could_be_null env ty1) ->
      let env, (r, _) = Env.expand_type env ty1 in
        Errors.nullsafe_not_needed p
          (Reason.to_string
           "This is what makes me believe it cannot be null" r);
        env
    | _ -> env in
  let env, method_, _ =
    obj_get_with_visibility ~is_method ~nullsafe ~explicit_tparams env ty1 cid id k in
  env, method_

and obj_get_with_visibility ~is_method ~nullsafe ?(explicit_tparams=[]) env ty1
                            cid id k =
  obj_get_ ~is_method ~nullsafe ~explicit_tparams env ty1 cid id k (fun ty -> ty)

(* We know that the receiver is a concrete class: not a generic with
 * bounds, or a Tunresolved. *)
and obj_get_concrete_ty ~is_method ?(explicit_tparams=[]) env concrete_ty class_id
    (id_pos, id_str as id) k_lhs =
  let default () = env, (Reason.Rnone, Tany), None in
  match concrete_ty with
  | (r, Tclass(x, paraml)) ->
    begin
      match Env.get_class env (snd x) with
      | None ->
        default ()

      | Some class_info when not is_method
          && not (Env.is_strict env)
          && class_info.tc_name = SN.Classes.cStdClass ->
          default ()

      | Some class_info ->
        let paraml =
          if List.length paraml = 0
          then List.map class_info.tc_tparams
              (fun _ -> Reason.Rwitness id_pos, Tany)
          else paraml in
        let member_info = Env.get_member is_method env class_info id_str in
        Typing_hooks.dispatch_cmethod_hook class_info paraml id env
          (Some class_id) ~is_method;

        match member_info with
        | None when not is_method ->
          if not (SN.Members.is_special_xhp_attribute id_str)
          then member_not_found id_pos ~is_method class_info id_str r;
          default ()

        | None ->
          begin
          match Env.get_member is_method env class_info SN.Members.__call with
          | None ->
            member_not_found id_pos ~is_method class_info id_str r;
            default ()

          | Some {ce_visibility = vis; ce_type = lazy (r, Tfun ft); _}  ->
            let mem_pos = Reason.to_pos r in
            TVis.check_obj_access id_pos env (mem_pos, vis);

            (* the return type of __call can depend on the
             * class params or be this *)
            let this_ty = k_lhs (r, (Tclass(x, paraml))) in
            let ety_env = {
              type_expansions = [];
              this_ty = this_ty;
              substs = Subst.make class_info.tc_tparams paraml;
              from_class = Some class_id;
            } in
            let env, ft = Phase.localize_ft ~ety_env env ft in

            (* we change the params of the underlying
             * declaration to act as a variadic function
             * ... this transform cannot be done when
             * processing the declaration of call because
             * direct calls to $inst->__call are also
             * valid.  *)
            let ft = {ft with
              ft_arity = Fellipsis 0; ft_tparams = []; ft_params = []; } in

            let member_ty = (r, Tfun ft) in
            env, member_ty, Some (mem_pos, vis)

          | _ -> assert false
          end

    | Some ({ce_visibility = vis; ce_type = lazy member_; _ } as member_ce) ->
      let mem_pos = Reason.to_pos (fst member_) in
      TVis.check_obj_access id_pos env (mem_pos, vis);
      let member_ty = Typing_enum.member_type env member_ce in
      let this_ty = k_lhs (r, (Tclass(x, paraml))) in
      let ety_env = {
        type_expansions = [];
        this_ty = this_ty;
        substs = Subst.make class_info.tc_tparams paraml;
        from_class = Some class_id;
      } in
      let env, member_ty =
        begin match member_ty with
          | (r, Tfun ft) ->
            (* We special case function types here to be able to pass explicit type parameters. *)
            let (env, ft) = Phase.localize_ft ~explicit_tparams ~ety_env env ft in
            (env, (r, Tfun ft))
          | _ -> Phase.localize ~ety_env env member_ty
        end in
      env, member_ty, Some (mem_pos, vis)
    end

  | _, Tobject
  | _, Tany
  | _, Terr ->
    default ()

  | _ ->
    Errors.non_object_member
      id_str id_pos (Typing_print.error (snd concrete_ty))
      (Reason.to_pos (fst concrete_ty));
    default ()


(* k_lhs takes the type of the object receiver *)
and obj_get_ ~is_method ~nullsafe ?(explicit_tparams=[]) env ty1 cid (id_pos, id_str as id)
    k k_lhs =
  let env, ety1 = Env.expand_type env ty1 in
  match ety1 with
  | _, Tunresolved tyl ->
      let (env, vis), tyl = List.map_env (env, None) tyl
        begin fun (env, vis) ty ->
          let env, ty, vis' =
            obj_get_ ~is_method ~nullsafe ~explicit_tparams env ty cid id k k_lhs in
          (* There is one special case where we need to expose the
           * visibility outside of obj_get (checkout inst_meth special
           * function).
           * We keep a witness of the "most restrictive" visibility
           * we encountered (position + visibility), to be able to
           * special case inst_meth.
           *)
          let vis = TVis.min_vis_opt vis vis' in
          (env, vis), ty
        end in
      let env, method_ = TUtils.in_var env (fst ety1, Tunresolved (tyl)) in
      env, method_, vis

  | p', (Tabstract(ak, Some ty)) ->
    let k_lhs' ty = match ak with
    | AKnewtype (_, _) -> k_lhs ty
    | _ -> k_lhs (p', Tabstract (ak, Some ty)) in
    obj_get_ ~is_method ~nullsafe ~explicit_tparams env ty cid id k k_lhs'

  | p', (Tabstract(ak,_)) ->
    let resl =
    try_over_concrete_supertypes env ety1
      (fun env ty ->
      (* We probably don't want to rewrap new types for the 'this' closure *)
      (* TODO AKENN: we shouldn't refine constraints by changing
       * the type like this *)
         let k_lhs' ty = match ak with
         | AKnewtype (_, _) -> k_lhs ty
         | _ -> k_lhs (p', Tabstract (ak, Some ty)) in
         obj_get_concrete_ty ~is_method ~explicit_tparams env ty cid id k_lhs'
      ) in
    begin match resl with
      | [] -> begin
          Errors.non_object_member
            id_str id_pos (Typing_print.error (snd ety1))
            (Reason.to_pos (fst ety1));
          k (env, err_none, None)
        end
      | ((_env, (_, ty), _vis) as res)::rest ->
        if List.exists rest (fun (_, (_,ty'), _) -> ty' <> ty)
        then
        begin
          Errors.ambiguous_member
            id_str id_pos (Typing_print.error (snd ety1))
            (Reason.to_pos (fst ety1));
          k (env, err_none, None)
        end
        else k res
    end

  | _, Toption ty -> begin match nullsafe with
    | Some p1 ->
        let k' (env, fty, x) = begin
          let env, method_, x = k (env, fty, x) in
          let env, method_ = non_null env method_ in
          env, (Reason.Rnullsafe_op p1, Toption method_), x
        end in
        obj_get_ ~is_method ~nullsafe ~explicit_tparams env ty cid id k' k_lhs
    | None ->
        Errors.null_member id_str id_pos
          (Reason.to_string
             "This is what makes me believe it can be null"
             (fst ety1)
          );
        k (env, (fst ety1, Terr), None)
      end
  | _, _ ->
    k (obj_get_concrete_ty ~is_method ~explicit_tparams env ety1 cid id k_lhs)


(* Return true if the type ty1 contains the null value *)
and type_could_be_null env ty1 =
  let _, tyl = TUtils.get_concrete_supertypes env ty1 in
  List.exists tyl
    (fun ety ->
      match snd ety with
        Toption _ | Tunresolved _ | Tmixed | Tany | Terr -> true
      | Tarraykind _ | Tprim _ | Tvar _ | Tfun _ | Tabstract _
      | Tclass (_, _) | Ttuple _ | Tanon (_, _) | Tobject
      | Tshape _ -> false)

and class_id_for_new p env cid =
  let env, te, ty = static_class_id p env cid in
  (* Need to deal with union case *)
  let rec get_info res tyl =
    match tyl with
    | [] -> env, te, res
    | ty::tyl ->
      match snd ty with
      | Tunresolved tyl' ->
        get_info res (tyl' @ tyl)
      | _ ->
        (* Instantiation on an abstract class (e.g. from classname<T>) is
         * via the base type (to check constructor args), but the actual
         * type `ty` must be preserved. *)
        match TUtils.get_base_type env ty with
        | _, Tclass (sid, _) ->
          begin
            let class_ = Env.get_class env (snd sid) in
            match class_ with
            | None -> get_info res tyl
            | Some class_info -> get_info ((sid, class_info, ty)::res) tyl
          end
        | _, (Tany | Terr | Tmixed | Tarraykind _ | Toption _ | Tprim _
        | Tvar _ | Tfun _ | Tabstract (_, _) | Ttuple _ | Tanon (_, _)
             | Tunresolved _ | Tobject | Tshape _) -> get_info res tyl in
  get_info [] [ty]

(* To be a valid trait declaration, all of its 'require extends' must
 * match; since there's no multiple inheritance, it follows that all of
 * the 'require extends' must belong to the same inheritance hierarchy
 * and one of them should be the child of all the others *)
and trait_most_concrete_req_class trait env =
  List.fold_left trait.tc_req_ancestors ~f:begin fun acc (_p, ty) ->
    let _r, (_p, name), _paraml = TUtils.unwrap_class_type ty in
    let keep = match acc with
      | Some (c, _ty) -> SMap.mem name c.tc_ancestors
      | None -> false
    in
    if keep then acc
    else
      let class_ = Env.get_class env name in
      (match class_ with
        | None
        | Some { tc_kind = Ast.Cinterface; _ } -> acc
        | Some { tc_kind = Ast.Ctrait; _ } ->
          (* this is an error case for which the nastCheck spits out
           * an error, but does *not* currently remove the offending
           * 'require extends' or 'require implements' *)
          acc
        | Some c -> Some (c, ty)
      )
  end ~init:None

(* For explicit type arguments we support a wildcard syntax `_` for which
 * Hack will generate a fresh type variable
 *)
and type_argument env hint =
  match hint with
  | (_, Happly((_, "_"), [])) ->
    Env.fresh_unresolved_type env
  | _ ->
    Phase.hint_locl env hint

(* If there are no explicit type arguments then generate fresh type variables
 * for all of them. Otherwise, check the arity, and use the explicit types *)
and type_arguments env p class_name tparams hintl =
  let default () = List.map_env env tparams begin fun env _ ->
    Env.fresh_unresolved_type env end in
  if hintl = []
  then default ()
  else if List.length hintl != List.length tparams
  then begin
    Errors.type_arity p class_name (string_of_int (List.length tparams));
    default ()
  end
  else
    List.map_env env hintl type_argument

(* When invoking a method the class_id is used to determine what class we
 * lookup the method in, but the type of 'this' will be the late bound type.
 * For example:
 *
 *  class C {
 *    public static function get(): this { return new static(); }
 *
 *    public static function alias(): this { return self::get(); }
 *  }
 *
 *  In C::alias, when we invoke self::get(), 'self' is resolved to the class
 *  in the lexical scope (C), so call C::get. However the method is executed in
 *  the current context, so static inside C::get will be resolved to the late
 *  bound type (get_called_class() within C::alias).
 *
 *  This means when determining the type of this, CIparent and CIself should be
 *  changed to CIstatic. For the other cases of C::get() or $c::get(), we only
 *  look at the left hand side of the '::' and use the type type associated
 *  with it.
 *
 *  Thus C::get() will return a type C, while $c::get() will return the same
 *  type as $c.
 *)
and this_for_method env cid default_ty = match cid with
  | CIparent | CIself | CIstatic ->
      let p = Reason.to_pos (fst default_ty) in
      let env, _te, ty = static_class_id p env CIstatic in
      ExprDepTy.make env CIstatic ty
  | _ ->
      env, default_ty

and static_class_id p env =
  let make_result env te ty =
    env, te, ty in
  function
  | CIparent ->
    (match Env.get_self env with
      | _, Tclass ((_, self), _) ->
        (match Env.get_class env self with
          | Some (
            {tc_kind = Ast.Ctrait; _}
              as trait) ->
            (match trait_most_concrete_req_class trait env with
              | None ->
                Errors.parent_in_trait p;
                make_result env T.CIparent (Reason.Rwitness p, Terr)
              | Some (_, parent_ty) ->
                (* inside a trait, parent is SN.Typehints.this, but with the
                 * type of the most concrete class that the trait has
                 * "require extend"-ed *)
                let r = Reason.Rwitness p in
                let env, parent_ty = Phase.localize_with_self env parent_ty in
                make_result env T.CIparent (r, TUtils.this_of parent_ty)
            )
          | _ ->
            let parent = Env.get_parent env in
            let parent_defined = snd parent <> Tany in
            if not parent_defined
            then Errors.parent_undefined p;
            let r = Reason.Rwitness p in
            let env, parent = Phase.localize_with_self env parent in
            (* parent is still technically the same object. *)
            make_result env T.CIparent (r, TUtils.this_of (r, snd parent))
          )
      | _, (Terr | Tany | Tmixed | Tarraykind _ | Toption _ | Tprim _
            | Tfun _ | Ttuple _ | Tshape _ | Tvar _
            | Tanon (_, _) | Tunresolved _ | Tabstract (_, _) | Tobject
           ) ->
        let parent = Env.get_parent env in
        let parent_defined = snd parent <> Tany in
        if not parent_defined
        then Errors.parent_undefined p;
        let r = Reason.Rwitness p in
        let env, parent = Phase.localize_with_self env parent in
        (* parent is still technically the same object. *)
        make_result env T.CIparent (r, TUtils.this_of (r, snd parent))
    )
  | CIstatic ->
    make_result env T.CIstatic
      (Reason.Rwitness p, TUtils.this_of (Env.get_self env))
  | CIself ->
    make_result env T.CIself
      (Reason.Rwitness p, snd (Env.get_self env))
  | CI (c, hl) ->
    let class_ = Env.get_class env (snd c) in
    (match class_ with
      | None ->
        make_result env (T.CI (c, hl)) (Reason.Rnone, Tany)
      | Some class_ ->
        let env, tyl = type_arguments env p (snd c) class_.tc_tparams hl in
        make_result env (T.CI (c, hl))
          (Reason.Rwitness (fst c), Tclass (c, tyl))
    )
  | CIexpr (p, _ as e) ->
      let env, te, ty = expr env e in
      let rec resolve_ety ty =
        let env, ty = TUtils.fold_unresolved env ty in
        let _, ty = Env.expand_type env ty in
        match TUtils.get_base_type env ty with
        | _, Tabstract (AKnewtype (classname, [the_cls]), _) when
            classname = SN.Classes.cClassname -> resolve_ety the_cls
        | _, Tabstract (AKgeneric _, _)
        | _, Tclass _ -> ty
        | r, Tunresolved tyl -> r, Tunresolved (List.map tyl resolve_ety)
        | _, Tvar _ as ty -> resolve_ety ty
        | _, (Tany | Tprim Tstring | Tabstract (_, None) | Tmixed | Tobject)
              when not (Env.is_strict env) ->
          Reason.Rnone, Tany
        | _, (Terr | Tany | Tmixed | Tarraykind _ | Toption _
                 | Tprim _ | Tfun _ | Ttuple _
                 | Tabstract ((AKenum _ | AKdependent _ | AKnewtype _), _)
                 | Tanon (_, _) | Tobject | Tshape _ as ty
        ) ->
          Errors.expected_class ~suffix:(", but got "^Typing_print.error ty) p;
          Reason.Rnone, Terr in
      let result_ty = resolve_ety ty in
      make_result env (T.CIexpr te) result_ty

and call_construct p env class_ params el uel cid =
  let cid = if cid = CIparent then CIstatic else cid in
  let env, tcid, cid_ty = static_class_id p env cid in
  let ety_env = {
    type_expansions = [];
    this_ty = cid_ty;
    substs = Subst.make class_.tc_tparams params;
    from_class = Some cid;
  } in
  let env = Phase.check_tparams_constraints ~ety_env env class_.tc_tparams in
  if SSet.mem "XHP" class_.tc_extends then env, tcid, [], [] else
  let cstr = Env.get_construct env class_ in
  let mode = Env.get_mode env in
  Typing_hooks.dispatch_constructor_hook class_ params env p;
  match (fst cstr) with
    | None ->
      if el <> [] &&
        (mode = FileInfo.Mstrict || mode = FileInfo.Mpartial) &&
        class_.tc_members_fully_known
      then Errors.constructor_no_args p;
      let env, tel, _tyl = exprs env el in
      env, tcid, tel, []
    | Some { ce_visibility = vis; ce_type = lazy m; _ } ->
      TVis.check_obj_access p env (Reason.to_pos (fst m), vis);
      let env, m = Phase.localize ~ety_env env m in
      let env, tel, tuel, _ty = call p env m el uel in
      env, tcid, tel, tuel

and check_arity ?(check_min=true) pos pos_def (arity:int) exp_arity =
  let exp_min = (Typing_defs.arity_min exp_arity) in
  if check_min && arity < exp_min then
    Errors.typing_too_few_args pos pos_def;
  match exp_arity with
    | Fstandard (_, exp_max) ->
      if (arity > exp_max)
      then Errors.typing_too_many_args pos pos_def;
    | Fvariadic _ | Fellipsis _ -> ()

and check_deprecated p { ft_pos; ft_deprecated; _ } =
  match ft_deprecated with
  | Some s -> Errors.deprecated_use p ft_pos s
  | None -> ()

(* The variadic capture argument is an array listing the passed
 * variable arguments for the purposes of the function body; callsites
 * should not unify with it *)
and variadic_param env ft =
  match ft.ft_arity with
    | Fvariadic (_, p_ty) -> env, Some p_ty
    | Fellipsis _ | Fstandard _ -> env, None

and call pos env fty el uel =
  let env, tel, tuel, ty = call_ pos env fty el uel in
  (* We need to solve the constraints after every single function call.
   * The type-checker is control-flow sensitive, the same value could
   * have different type depending on the branch that we are in.
   * When this is the case, a call could violate one of the constraints
   * in a branch. *)
  let env = Env.check_todo env in
  env, tel, tuel, ty

(* Enforces that e is unpackable. If e is a tuple, appends its unpacked types
 * into the e_tyl returned.
 *)
and unpack_expr env e_tyl e =
  let env, _te, ety = expr env e in
  (match ety with
  | _, Ttuple tyl ->
      (* Todo: Check that tuples are allowed - that is, disallow a tuple
       * unpacking after an array unpacking.
       *)
      let unpacked_e_tyl = List.map tyl (fun ty -> e, ty) in
      env, e_tyl @ unpacked_e_tyl, true
  | _ ->
    let pos = fst e in
    let unpack_r = Reason.Runpack_param pos in
    let container_ty = (unpack_r, Tclass ((pos, SN.Collections.cContainer),
                                          [unpack_r, Tany])) in
    let env = Type.sub_type pos Reason.URparam env ety container_ty in
    env, e_tyl, false
  )

(* Unpacks uel. If tuples are found, unpacked types are appended to the
 * e_tyl returned.
 *)
and unpack_exprl env e_tyl uel =
  List.fold_left uel ~init:(env, e_tyl, false)
    ~f: begin fun (env, e_tyl, unpacked_tuple) e ->
      let env, e_tyl, is_tuple = unpack_expr env e_tyl e in
      (env, e_tyl, is_tuple || unpacked_tuple)
    end

and call_ pos env fty el uel =
  let env, efty = Env.expand_type env fty in
  (match efty with
  | _, (Terr | Tany | Tunresolved []) ->
    let el = el @ uel in
    let env, tel = List.map_env env el begin fun env elt ->
      let env, te, arg_ty = expr env elt in
      let env, _arg_ty = check_valid_rvalue pos env arg_ty in
      env, te
    end in
    Typing_hooks.dispatch_fun_call_hooks [] (List.map (el @ uel) fst) env;
    env, tel, [], (Reason.Rnone, Tany)
  | r, Tunresolved tyl ->
    let env, retl = List.map_env env tyl begin fun env ty ->
      let env, _, _, ty = call pos env ty el uel in env, ty
    end in
    let env, ty = TUtils.in_var env (r, Tunresolved retl) in
    env, [], [], ty
  | r2, Tfun ft ->
    (* Typing of format string functions. It is dependent on the arguments (el)
     * so it cannot be done earlier.
     *)
    let env, ft = Typing_exts.retype_magic_func env ft el in
    check_deprecated pos ft;
    let pos_def = Reason.to_pos r2 in
    let env, var_param = variadic_param env ft in
    let env, e_te_tyl = List.map_env env el begin fun env e ->
      let env, te, ty = expr env e in
      env, (e, te, ty)
    end in
    let tel = List.map e_te_tyl (fun (_, b, _) -> b) in
    let e_tyl = List.map e_te_tyl (fun (a, _, c) -> a, c) in
    let env, e_tyl, unpacked_tuple = unpack_exprl env e_tyl uel in
    let arity = if unpacked_tuple
      then List.length e_tyl
      (* Each array unpacked corresponds with at least 1 param. *)
      else List.length el + List.length uel in
    (* If we unpacked an array, we don't check arity exactly. Since each
     * unpacked array consumes 1 or many parameters, it is nonsensical to say
     * that not enough args were passed in (so we don't do the min check).
     *)
    let () = check_arity ~check_min:(uel = [] || unpacked_tuple)
      pos pos_def arity ft.ft_arity in
    let todos = ref [] in
    let env = wfold_left_default (call_param todos) (env, var_param)
      ft.ft_params e_tyl in
    let env = fold_fun_list env !todos in
    Typing_hooks.dispatch_fun_call_hooks
      ft.ft_params (List.map (el @ uel) fst) env;
    env, tel, [], ft.ft_ret
  | r2, Tanon (arity, id) when uel = [] ->
    let env, tel, tyl = exprs env el in
    let anon = Env.get_anonymous env id in
    let fpos = Reason.to_pos r2 in
    (match anon with
      | None ->
        Errors.anonymous_recursive_call pos;
        env, tel, [], err_none
      | Some anon ->
        let () = check_arity pos fpos (List.length tyl) arity in
        let tyl = List.map tyl (fun ty -> None, ty) in
        let env, _, ty = anon env tyl in
        env, tel, [], ty)
  | _, Tarraykind _ when not (Env.is_strict env) ->
    (* Relaxing call_user_func to work with an array in partial mode *)
    env, [], [], (Reason.Rnone, Tany)
  | _, ty ->
    bad_call pos ty;
    env, [], [], err_none
  )

and call_param todos env (name, x) ((pos, _ as e), arg_ty) =
  (match name with
  | None -> ()
  | Some name -> Typing_suggest.save_param name env x arg_ty
  );
  let env, arg_ty = check_valid_rvalue pos env arg_ty in

  (* When checking params the type 'x' may be expression dependent. Since
   * we store the expression id in the local env for Lvar, we want to apply
   * it in this case.
   *)
  let env, dep_ty = match snd e with
    | Lvar _ -> ExprDepTy.make env (CIexpr e) arg_ty
    | _ -> env, arg_ty in
  (* We solve for Tanon types after all the other params because we want to
   * typecheck the lambda bodies with as much type information as possible. For
   * example, in array_map(fn, x), we might be able to use the type of x to
   * infer the type of fn, but if we call sub_type on fn first, we end up
   * typechecking its body without the benefit of knowing its full type. If
   * fn is typehinted but not x, we could use fn to infer the type of x, but
   * in practice the reverse situation is more likely. This rearrangement is
   * particularly useful since higher-order functions usually put fn before x.
  *)
  match arg_ty with
  | _, Tanon _ ->
      todos := (fun env ->
                Type.sub_type pos Reason.URparam env arg_ty x) :: !todos;
      env
  | _, (Terr | Tany | Tmixed | Tarraykind _ | Toption _ | Tprim _
    | Tvar _ | Tfun _ | Tabstract (_, _) | Tclass (_, _) | Ttuple _
    | Tunresolved _ | Tobject | Tshape _) ->
    Type.sub_type pos Reason.URparam env dep_ty x

and bad_call p ty =
  Errors.bad_call p (Typing_print.error ty)

and unop p env uop te ty =
  let make_result env te result_ty =
    env, T.make_typed_expr p result_ty (T.Unop(uop, te)), result_ty in
  match uop with
  | Ast.Unot ->
      Async.enforce_nullable_or_not_awaitable env p ty;
      (* !$x (logical not) works with any type, so we just return Tbool *)
      make_result env te (Reason.Rlogic_ret p, Tprim Tbool)
  | Ast.Utild ->
      (* ~$x (bitwise not) only works with int *)
      let env = Type.sub_type p Reason.URnone env ty
        (Reason.Rarith p, Tprim Tint) in
      make_result env te (Reason.Rarith p, Tprim Tint)
  | Ast.Uincr
  | Ast.Upincr
  | Ast.Updecr
  | Ast.Udecr
  | Ast.Uplus
  | Ast.Uminus ->
      (* math operators work with int or floats, so we call sub_type *)
      let env = Type.sub_type p Reason.URnone env ty
        (Reason.Rarith p, Tprim Tnum) in
      make_result env te ty
  | Ast.Uref ->
      (* We basically just ignore references in non-strict files *)
      if Env.is_strict env then
        Errors.reference_expr p;
      make_result env te ty
  | Ast.Usplat ->
      (* TODO(13988978) Splat operator not supported at use sites yet. *)
      make_result env te ty
  | Ast.Usilence ->
      (* Silencing does not change the type *)
      make_result env te ty

and binop in_cond p env bop p1 te1 ty1 p2 te2 ty2 =
  let rec is_any ty =
    match Env.expand_type env ty with
    | (_, (_, (Tany | Terr))) -> true
    | (_, (_, Tunresolved tyl)) -> List.for_all tyl is_any
    | _ -> false in
  (* Test if `ty` is *not* the any type (or a variant thereof) and
   * is a subtype of the primitive type `prim`. *)
  let is_sub_prim env ty prim =
    let ty_prim = (Reason.Rarith p, Tprim prim) in
    if not (is_any ty) && SubType.is_sub_type env ty ty_prim
    then Some (fst ty) else None in
  (* Test if `ty` is *not* the any type (or a variant thereof) and
   * is a subtype of `num` but is not a subtype of `int` *)
  let is_sub_num_not_sub_int env ty =
    let ty_num = (Reason.Rarith p, Tprim Tnum) in
    let ty_int = (Reason.Rarith p, Tprim Tint) in
    if not (is_any ty) && SubType.is_sub_type env ty ty_num
       && not (SubType.is_sub_type env ty ty_int)
    then Some (fst ty) else None in
  (* Force ty1 to be a subtype of ty2 (unless it is any) *)
  let enforce_sub_ty env ty1 ty2 =
    let env = Type.sub_type p Reason.URnone env ty1 ty2 in
    Env.expand_type env ty1 in
  let make_result env te1 te2 ty =
    env, T.make_typed_expr p ty (T.Binop(bop, te1, te2)), ty in
  match bop with
  | Ast.Plus ->
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, ty2 = TUtils.fold_unresolved env ty2 in
      let env, ety1 = Env.expand_type env ty1 in
      let env, ety2 = Env.expand_type env ty2 in
      (match ety1, ety2 with
      (* For array<V1>+array<V2> and array<K1,V1>+array<K2,V2>, allow
       * the addition to produce a supertype. (We could also handle
       * when they have mismatching annotations, but we get better error
       * messages if we just let those get unified in the next case. *)
      (* The general types are:
       *   function<Tk,Tv>(array<Tk,Tv>, array<Tk,Tv>): array<Tk,Tv>
       *   function<T>(array<T>, array<T>): array<T>
       * and subtyping on the arguments deals with everything
       *)
      | (_, Tarraykind (AKmap _ as ak)), (_, Tarraykind (AKmap _))
      | (_, Tarraykind (AKvec _ as ak)), (_, Tarraykind (AKvec _)) ->
          let env, a_sup = Env.fresh_unresolved_type env in
          let env, b_sup = Env.fresh_unresolved_type env in
          let res_ty = Reason.Rarray_plus_ret p, Tarraykind (
            match ak with
              | AKvec _ -> AKvec a_sup
              | AKmap _ -> AKmap (a_sup, b_sup)
              | _ -> assert false
          ) in
          let env = Type.sub_type p1 Reason.URnone env ety1 res_ty in
          let env = Type.sub_type p2 Reason.URnone env ety2 res_ty in
          make_result env te1 te2 res_ty
      | (_, Tarraykind _), (_, Tarraykind (AKshape _)) ->
        let env, ty2 = Typing_arrays.downcast_aktypes env ty2 in
        binop in_cond p env bop p1 te1 ty1 p2 te2 ty2
      | (_, Tarraykind (AKshape _)), (_, Tarraykind _) ->
        let env, ty1 = Typing_arrays.downcast_aktypes env ty1 in
        binop in_cond p env bop p1 te1 ty1 p2 te2 ty2
      | (_, Tarraykind _), (_, Tarraykind _)
      | (_, (Tany | Terr)), (_, Tarraykind _)
      | (_, Tarraykind _), (_, Tany) ->
          let env, ty = Type.unify p Reason.URnone env ty1 ty2 in
          make_result env te1 te2 ty
      | (_, (Tany | Terr | Tmixed | Tarraykind _ | Toption _
        | Tprim _ | Tvar _ | Tfun _ | Tabstract (_, _) | Tclass (_, _)
        | Ttuple _ | Tanon (_, _) | Tunresolved _ | Tobject | Tshape _
            )
        ), _ ->
        let env, texpr, ty =
          binop in_cond p env Ast.Minus p1 te1 ty1 p2 te2 ty2 in
        match snd texpr with
          | T.Binop (_, te1, te2) -> make_result env te1 te2 ty
          | _ -> assert false
      )
  | Ast.Minus | Ast.Star ->
    begin
      let env, ty1 = enforce_sub_ty env ty1 (Reason.Rarith p1, Tprim Tnum) in
      let env, ty2 = enforce_sub_ty env ty2 (Reason.Rarith p2, Tprim Tnum) in
      (* If either side is a float then float: 1.0 - 1 -> float *)
      (* These have types
       *   function(float, num): float
       *   function(num, float): float
       *)
      match is_sub_prim env ty1 Tfloat, is_sub_prim env ty2 Tfloat with
      | (Some r, _) | (_, Some r) ->
        make_result env te1 te2 (r, Tprim Tfloat)
      | _, _ ->
      (* Both sides are integers, then integer: 1 - 1 -> int *)
      (* This has type
       *   function(int, int): int
       *)
        match is_sub_prim env ty1 Tint, is_sub_prim env ty2 Tint with
        | (Some _, Some _) ->
          make_result env te1 te2 (Reason.Rarith_ret p, Tprim Tint)
        | _, _ ->
          (* Either side is a non-int num then num *)
          (* This has type
           *   function(num, num): num
           *)
          match is_sub_num_not_sub_int env ty1,
                is_sub_num_not_sub_int env ty2 with
          | (Some r, _) | (_, Some r) ->
            make_result env te1 te2 (r, Tprim Tnum)
          (* Otherwise? *)
          | _, _ -> make_result env te1 te2 ty1
    end
  | Ast.Slash | Ast.Starstar ->
    begin
      let env, ty1 = enforce_sub_ty env ty1 (Reason.Rarith p1, Tprim Tnum) in
      let env, ty2 = enforce_sub_ty env ty2 (Reason.Rarith p2, Tprim Tnum) in
      (* If either side is a float then float *)
      (* These have types
       *   function(float, num) : float
       *   function(num, float) : float
       * [Actually, for division result can be false if second arg is zero]
       *)
      match is_sub_prim env ty1 Tfloat, is_sub_prim env ty2 Tfloat with
      | (Some r, _) | (_, Some r) ->
        make_result env te1 te2 (r, Tprim Tfloat)
      (* Otherwise it has type
       *   function(num, num) : num
       * [Actually, for division result can be false if second arg is zero]
       *)
      | _, _ ->
      let r = match bop with
        | Ast.Slash -> Reason.Rret_div p
        | _ -> Reason.Rarith_ret p in
      make_result env te1 te2 (r, Tprim Tnum)
    end
  | Ast.Percent ->
     (* Integer remainder function has type
      *   function(int, int) : int
      * [Actually, result can be false if second arg is zero]
      *)
      let env, _ = enforce_sub_ty env ty1 (Reason.Rarith p1, Tprim Tint) in
      let env, _ = enforce_sub_ty env ty2 (Reason.Rarith p1, Tprim Tint) in
      make_result env te1 te2 (Reason.Rarith_ret p, Tprim Tint)
  | Ast.Xor ->
    begin
      match is_sub_prim env ty1 Tbool, is_sub_prim env ty2 Tbool with
      | (Some _, _) | (_, Some _) ->
        (* Logical xor:
         *   function(bool, bool) : bool
         *)
        let env, _ =
          enforce_sub_ty env ty1 (Reason.Rlogic_ret p1, Tprim Tbool) in
        let env, _ =
          enforce_sub_ty env ty2 (Reason.Rlogic_ret p1, Tprim Tbool) in
        make_result env te1 te2 (Reason.Rlogic_ret p, Tprim Tbool)
      | _, _ ->
        (* Arithmetic xor:
         *   function(int, int) : int
         *)
        let env, _ = enforce_sub_ty env ty1 (Reason.Rarith p1, Tprim Tint) in
        let env, _ = enforce_sub_ty env ty2 (Reason.Rarith p1, Tprim Tint) in
        make_result env te1 te2 (Reason.Rarith_ret p, Tprim Tint)
    end
  (* Equality and disequality:
   *   function<T>(T, T): bool
   *)
  | Ast.Eqeq  | Ast.Diff  ->
      make_result env te1 te2 (Reason.Rcomp p, Tprim Tbool)
  | Ast.EQeqeq | Ast.Diff2 ->
      if not in_cond
      then Typing_equality_check.assert_nontrivial p bop env ty1 ty2;
      make_result env te1 te2 (Reason.Rcomp p, Tprim Tbool)
  | Ast.Lt | Ast.Lte | Ast.Gt | Ast.Gte | Ast.Cmp ->
      let ty_result = match bop with Ast.Cmp -> Tprim Tint | _ -> Tprim Tbool in
      let ty_num = (Reason.Rcomp p, Tprim Nast.Tnum) in
      let ty_string = (Reason.Rcomp p, Tprim Nast.Tstring) in
      let ty_datetime =
        (Reason.Rcomp p, Tclass ((p, SN.Classes.cDateTime), [])) in
      let both_sub ty =
        SubType.is_sub_type env ty1 ty && SubType.is_sub_type env ty2 ty in
      (* So we have three different types here:
       *   function(num, num): bool
       *   function(string, string): bool
       *   function(DateTime, DateTime): bool
       *)
      if both_sub ty_num || both_sub ty_string || both_sub ty_datetime
      then make_result env te1 te2 (Reason.Rcomp p, ty_result)
      else
        (* TODO this is questionable; PHP's semantics for conversions with "<"
         * are pretty crazy and we may want to just disallow this? *)
        (* This is universal:
         *   function<T>(T, T): bool
         *)
        let env, _ = Type.unify p Reason.URnone env ty1 ty2 in
        make_result env te1 te2 (Reason.Rcomp p, ty_result)
  | Ast.Dot ->
    (* A bit weird, this one:
     *   function(Stringish | string, Stringish | string) : string)
     *)
      let env = SubType.sub_string p1 env ty1 in
      let env = SubType.sub_string p2 env ty2 in
      make_result env te1 te2 (Reason.Rconcat_ret p, Tprim Tstring)
  | Ast.LogXor
  | Ast.AMpamp
  | Ast.BArbar ->
      make_result env te1 te2 (Reason.Rlogic_ret p, Tprim Tbool)
  | Ast.Amp | Ast.Bar | Ast.Ltlt | Ast.Gtgt ->
      let env, _ = enforce_sub_ty env ty1 (Reason.Rbitwise p1, Tprim Tint) in
      let env, _ = enforce_sub_ty env ty2 (Reason.Rbitwise p2, Tprim Tint) in
      make_result env te1 te2 (Reason.Rbitwise_ret p, Tprim Tint)
  | Ast.Eq _ ->
      assert false

and non_null env ty =
  let env, ty = Env.expand_type env ty in
  match ty with
  | _, Toption ty ->
      (* When "??T" appears in the typing environment due to implicit
       * typing, the recursion here ensures that it's treated as
       * isomorphic to "?T"; that is, all nulls are created equal.
       *)
      non_null env ty
  | r, Tunresolved tyl ->
      let env, tyl = List.map_env env tyl
        (fun env e -> non_null env e) in
      (* We need to flatten the unresolved types, otherwise we could
       * end up with "Tunresolved[Tunresolved _]" which is not supposed
       * to happen.
      *)
      let tyl = List.fold_right tyl ~f:begin fun ty tyl ->
        match ty with
        | _, Tunresolved l -> l @ tyl
        | x -> x :: tyl
      end ~init:[] in
      env, (r, Tunresolved tyl)

  | r, Tabstract (ak, _) ->
    begin match TUtils.get_concrete_supertypes env ty with
      | env, [ty] -> let env, ty = non_null env ty in
        env, (r, Tabstract (ak, Some ty))
      | env, _ -> env, ty
    end
  | _, (Terr | Tany | Tmixed | Tarraykind _ | Tprim _ | Tvar _
    | Tclass (_, _) | Ttuple _ | Tanon (_, _) | Tfun _
    | Tobject | Tshape _) ->
      env, ty

and condition_var_non_null env = function
  | _, Lvar (_, x)
  | _, Dollardollar (_, x) ->
      let env, x_ty = Env.get_local env x in
      let env, x_ty = non_null env x_ty in
      Env.set_local env x x_ty
  | p, Class_get (cname, (_, member_name)) as e ->
      let env, _te, ty = expr env e in
      let env, local = Env.FakeMembers.make_static p env cname member_name in
      let env = Env.set_local env local ty in
      let local = p, Lvar (p, local) in
      condition_var_non_null env local
    (* TODO TAST: generate an assignment to the fake local in the TAST *)
  | p, Obj_get ((_, This | _, Lvar _ as obj),
                (_, Id (_, member_name)),
                _) as e ->
      let env, _te, ty = expr env e in
      let env, local = Env.FakeMembers.make p env obj member_name in
      let env = Env.set_local env local ty in
      let local = p, Lvar (p, local) in
      condition_var_non_null env local
  | _ -> env

and condition_isset env = function
  | _, Array_get (x, _) -> condition_isset env x
  | v -> condition_var_non_null env v

(**
 * Build an environment for the true or false branch of
 * conditional statements.
 *)
and condition ?lhs_of_null_coalesce env tparamet =
  let expr env x =
    let env, _te, ty = raw_expr ?lhs_of_null_coalesce ~in_cond:true env x in
    Async.enforce_nullable_or_not_awaitable env (fst x) ty;
    env, ty
  in let condition = condition ?lhs_of_null_coalesce
  in function
  | _, Expr_list [] -> env
  | _, Expr_list [x] ->
      let env, _ = expr env x in
      condition env tparamet x
  | r, Expr_list (x::xs) ->
      let env, _ = expr env x in
      condition env tparamet (r, Expr_list xs)
  | _, Call (Cnormal, (_, Id (_, func)), _, [param], [])
    when SN.PseudoFunctions.isset = func && tparamet &&
    not (Env.is_strict env) ->
      condition_isset env param
  | _, Call (Cnormal, (_, Id (_, func)), _, [e], [])
    when not tparamet && SN.StdlibFunctions.is_null = func ->
      condition_var_non_null env e
  | r, Binop ((Ast.Eqeq | Ast.EQeqeq as bop),
              (_, Null), e)
  | r, Binop ((Ast.Eqeq | Ast.EQeqeq as bop),
              e, (_, Null)) when not tparamet ->
      let env, x_ty = expr env e in
      let env =
        if bop == Ast.Eqeq then check_null_wtf env r x_ty else env in
      condition_var_non_null env e
  | (p, (Lvar _ | Obj_get _ | Class_get _) as e) ->
      let env, ty = expr env e in
      let env, ety = Env.expand_type env ty in
      (match ety with
      | _, Tarraykind (AKany | AKempty)
      | _, Tprim Tbool -> env
      | _, (Terr | Tany | Tmixed | Tarraykind _ | Toption _
        | Tprim _ | Tvar _ | Tfun _ | Tabstract (_, _) | Tclass (_, _)
        | Ttuple _ | Tanon (_, _) | Tunresolved _ | Tobject | Tshape _
        ) ->
          condition env (not tparamet) (p, Binop (Ast.Eqeq, e, (p, Null))))
  | r, Binop (Ast.Eq None, var, e) when tparamet ->
      let env, e_ty = expr env e in
      let env = check_null_wtf env r e_ty in
      condition_var_non_null env var
  | p1, Binop (Ast.Eq None, (_, (Lvar _ | Obj_get _) as lv), (p2, _)) ->
      let env, _ = expr env (p1, Binop (Ast.Eq None, lv, (p2, Null))) in
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
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_array ->
      is_array env `PHPArray p f lv
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_vec ->
      is_array env `HackVec p f lv
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_dict ->
      is_array env `HackDict p f lv
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_keyset ->
      is_array env `HackKeyset p f lv
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_int ->
      is_type env lv Tint (Reason.Rpredicated (p, f))
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_bool ->
      is_type env lv Tbool (Reason.Rpredicated (p, f))
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_float ->
      is_type env lv Tfloat (Reason.Rpredicated (p, f))
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_string ->
      is_type env lv Tstring (Reason.Rpredicated (p, f))
  | _, Call (Cnormal, (p, Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_resource ->
      is_type env lv Tresource (Reason.Rpredicated (p, f))
  | _, Unop (Ast.Unot, e) ->
      condition env (not tparamet) e
  | p, InstanceOf (ivar, cid) when tparamet && is_instance_var ivar ->
      (* Check the expession and determine its static type *)
      let env, _te, x_ty = raw_expr ~in_cond:false env ivar in

      (* What is the local variable bound to the expression? *)
      let env, (ivar_pos, x) = get_instance_var env ivar in

      (* The position p here is not really correct... it's the position
       * of the instanceof expression, not the class id. But we don't store
       * position data for the latter. *)
      let env, _te, obj_ty = static_class_id p env cid in

      (* New implementation of instanceof that is statically safe *)
      let safe_instanceof env obj_ty _c class_info =
        (* Generate fresh names consisting of formal type parameter name
         * with unique suffix *)
        let env, tparams_with_new_names =
          List.map_env env class_info.tc_tparams
            (fun env ((_,(_,name),_) as tp) ->
              let env, name = Env.add_fresh_generic_parameter env name in
              env, (tp, name)) in
        let s =
            snd _c ^ "<" ^
            String.concat "," (List.map tparams_with_new_names ~f:snd)
            ^ ">" in
        let reason = Reason.Rinstanceof (ivar_pos, s) in
        let tyl_fresh = List.map
            ~f:(fun (_,newname) -> (reason, Tabstract(AKgeneric newname, None)))
            tparams_with_new_names in

        (* Type of variable in block will be class name
         * with fresh type parameters *)
        let obj_ty = (fst obj_ty, Tclass(_c, tyl_fresh)) in

        (* Add in constraints as assumptions on those type parameters *)
        let ety_env = {
          type_expansions = [];
          substs = Subst.make class_info.tc_tparams tyl_fresh;
          this_ty = obj_ty; (* In case `this` appears in constraints *)
          from_class = None;
        } in
        let add_bounds env ((_, _, cstr_list), ty_fresh) =
            List.fold_left cstr_list ~init:env ~f:begin fun env (ck, ty) ->
              (* Substitute fresh type parameters for
               * original formals in constraint *)
            let env, ty = Phase.localize ~ety_env env ty in
            SubType.add_constraint p env ck ty_fresh ty end in
        let env =
          List.fold_left (List.zip_exn class_info.tc_tparams tyl_fresh)
            ~f:add_bounds ~init:env in

        (* Finally, if we have a class-test on something with static class type,
         * then we can chase the hierarchy and decompose the types to deduce
         * further assumptions on type parameters. For example, we might have
         *   class B<Tb> { ... }
         *   class C extends B<int>
         * and have obj_ty = C and x_ty = B<T> for a generic parameter T.
         * Then SubType.add_constraint will deduce that T=int and add int as
         * both lower and upper bound on T in env.lenv.tpenv
         *)
         let env = SubType.add_constraint p env Ast.Constraint_as obj_ty x_ty in
        env, obj_ty in

        if SubType.is_sub_type env obj_ty (
          Reason.none, Tclass ((Pos.none, SN.Classes.cAwaitable), [Reason.none, Tany])
        ) then () else Async.enforce_nullable_or_not_awaitable env (fst ivar) x_ty;

      let safe_instanceof_enabled =
        TypecheckerOptions.experimental_feature_enabled
          (Env.get_options env) TypecheckerOptions.experimental_instanceof in
      let rec resolve_obj env obj_ty =
        (* Expand so that we don't modify x *)
        let env, obj_ty = Env.expand_type env obj_ty in
        match obj_ty with
        (* If it's a generic that's expression dependent, we need to
          look at all of its upper bounds and create an unresolved type to
          represent all of the possible types.
        *)
        | r, Tabstract (AKgeneric s, _) when AbstractKind.is_generic_dep_ty s ->
          let upper_bounds = Env.get_upper_bounds env s in
          let env, tyl = List.map_env env upper_bounds resolve_obj in
          env, (r, Tunresolved tyl)
        | _, Tabstract (AKgeneric name, _) ->
          if safe_instanceof_enabled
          then Errors.instanceof_generic_classname p name;
          env, obj_ty
        | _, Tabstract (AKdependent (`this, []), Some (_, Tclass _)) ->
          let env, obj_ty =
            (* Technically instanceof static is not strong enough to prove
             * that a type is exactly the same as the late bound type.
             * For now we allow this lie to exist. To solve
             * this we either need to create a new type that means
             * subtype of static or provide a way of specifying exactly
             * the late bound type i.e. $x::class === static::class
             *)
            if cid = CIstatic then
              ExprDepTy.make env CIstatic obj_ty
            else
              env, obj_ty in
          env, obj_ty
        | _, Tabstract ((AKdependent _ | AKnewtype _), Some ty) ->
          resolve_obj env ty
        | _, Tclass ((_, cid as _c), tyl) ->
          begin match Env.get_class env cid with
            (* Why would this happen? *)
            | None ->
              env, (Reason.Rwitness ivar_pos, Tobject)

            | Some class_info ->
              if SubType.is_sub_type env x_ty obj_ty
              then
                (* If the right side of the `instanceof` object is
                 * a super type of what we already knew. In this case,
                 * since we already have a more specialized object, we
                 * don't touch the original object. Check out the unit
                 * test srecko.php if this is unclear.
                 *
                 * Note that if x_ty is Tany, no amount of subtype
                 * checking will be able to specify it
                 * further. This is arguably desirable to maintain
                 * the invariant that removing annotations gets rid
                 * of typing errors in partial mode (See also
                 * t3216948).  *)
                env, x_ty
              else
                (* We only implement the safe instanceof in strict mode *)
                (* Also: for generic types we implememt it only with
                 * experimental feature enabled *)
              if Env.is_strict env && (tyl = [] || safe_instanceof_enabled)
              then safe_instanceof env obj_ty _c class_info
              else env, obj_ty
          end
        | r, Tunresolved tyl ->
          let env, tyl = List.map_env env tyl resolve_obj in
          env, (r, Tunresolved tyl)
        | _, (Terr | Tany | Tmixed | Tarraykind _ | Tprim _ | Tvar _ | Tfun _
            | Tabstract ((AKenum _ | AKnewtype _ | AKdependent _), _)
            | Ttuple _ | Tanon (_, _) | Toption _ | Tobject | Tshape _) ->
          env, (Reason.Rwitness ivar_pos, Tobject)
      in
      let env, x_ty = resolve_obj env obj_ty in
      Env.set_local env x x_ty
  | _, Binop ((Ast.Eqeq | Ast.EQeqeq), e, (_, Null))
  | _, Binop ((Ast.Eqeq | Ast.EQeqeq), (_, Null), e) ->
      let env, _ = expr env e in
      env
  | e ->
      let env, _ = expr env e in
      env

and is_instance_var = function
  | _, (Lvar _ | This) -> true
  | _, Obj_get ((_, This), (_, Id _), _) -> true
  | _, Obj_get ((_, Lvar _), (_, Id _), _) -> true
  | _, Class_get (_, _) -> true
  | _ -> false

and get_instance_var env = function
  | p, Class_get (cname, (_, member_name)) ->
    let env, local = Env.FakeMembers.make_static p env cname member_name in
    env, (p, local)
  | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name)), _) ->
    let env, local = Env.FakeMembers.make p env obj member_name in
    env, (p, local)
  | _, Lvar (p, x) -> env, (p, x)
  | p, This -> env, (p, this)
  | _ -> failwith "Should only be called when is_instance_var is true"

and check_null_wtf env p ty =
  if not (Env.is_strict env) then env else
    let env, ty = TUtils.fold_unresolved env ty in
    let env, ety = Env.expand_type env ty in
    match ety with
      | _, Toption ty ->
        (* Find sketchy nulls hidden under singleton Tunresolved *)
        let env, ty = TUtils.fold_unresolved env ty in
        (match ty with
          | _, Tmixed ->
            Errors.sketchy_null_check p
          | _, Tprim _ ->
            Errors.sketchy_null_check_primitive p
          | _, (Terr | Tany | Tarraykind _ | Toption _ | Tvar _ | Tfun _
          | Tabstract (_, _) | Tclass (_, _) | Ttuple _ | Tanon (_, _)
          | Tunresolved _ | Tobject | Tshape _ ) -> ());
        env
      | _, (Terr | Tany | Tmixed | Tarraykind _ | Tprim _ | Tvar _
        | Tfun _ | Tabstract (_, _) | Tclass (_, _) | Ttuple _ | Tanon (_, _)
        | Tunresolved _ | Tobject | Tshape _ ) -> env

and is_type env e tprim r =
  match e with
    | p, Class_get (cname, (_, member_name)) ->
      let env, local = Env.FakeMembers.make_static p env cname member_name in
      Env.set_local env local (r, Tprim tprim)
    | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name)), _) ->
      let env, local = Env.FakeMembers.make p env obj member_name in
      Env.set_local env local (r, Tprim tprim)
    | _, Lvar (_px, x) ->
      Env.set_local env x (r, Tprim tprim)
    | _ -> env

(* Refine type for is_array, is_vec, is_keyset and is_dict tests
 * `pred_name` is the function name itself (e.g. 'is_vec')
 * `p` is position of the function name in the source
 * `arg_expr` is the argument to the function
 *)
and is_array env ty p pred_name arg_expr =
  let env, _te, arg_ty = expr env arg_expr in
  let r = Reason.Rpredicated (p, pred_name) in
  let env, tarrkey_name = Env.add_fresh_generic_parameter env "Tk" in
  let tarrkey = (r, Tabstract (AKgeneric tarrkey_name, None)) in
  let env = SubType.add_constraint p env Ast.Constraint_as
      tarrkey (r, Tprim Tarraykey) in
  let env, tfresh_name = Env.add_fresh_generic_parameter env "T" in
  let tfresh = (r, Tabstract (AKgeneric tfresh_name, None)) in
  (* This is the refined type of e inside the branch *)
  let refined_ty =
    (r, (match ty with
    | `HackDict ->
      Tclass ((Pos.none, SN.Collections.cDict), [tarrkey; tfresh])
    | `HackVec ->
      Tclass ((Pos.none, SN.Collections.cVec), [tfresh])
    | `HackKeyset ->
      Tclass ((Pos.none, SN.Collections.cKeyset), [tarrkey])
    | `PHPArray ->
      Tarraykind AKany)) in
  (* Add constraints on generic parameters that must
   * hold for refined_ty <:arg_ty. For example, if arg_ty is Traversable<T>
   * and refined_ty is keyset<T#1> then we know T#1 <: T *)
  let env = SubType.add_constraint p env Ast.Constraint_as refined_ty arg_ty in
  match arg_expr with
  | (_, Class_get (cname, (_, member_name))) ->
      let env, local = Env.FakeMembers.make_static p env cname member_name in
      Env.set_local env local refined_ty
  | (_, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name)), _)) ->
      let env, local = Env.FakeMembers.make p env obj member_name in
      Env.set_local env local refined_ty
  | (_, Lvar (_, x)) ->
      Env.set_local env x refined_ty
  | _ -> env

and string2 env idl =
  let env, tel =
    List.fold_left idl ~init:(env,[]) ~f:begin fun (env,tel) x ->
      let env, te, ty = expr env x in
      let p = fst x in
      let env = SubType.sub_string p env ty in
      env, te::tel
    end in
  env, List.rev tel

(* If the current class inherits from classes that take type arguments, we need
 * to check that the arguments provided are consistent with the constraints on
 * the type parameters. *)
and check_implements_tparaml (env: Env.env) ht =
  let _r, (p, c), paraml = TUtils.unwrap_class_type ht in
  let class_ = Decl_env.get_class_dep env.Env.decl_env c in
  match class_ with
  | None ->
      (* The class lives in PHP land *)
      ()
  | Some class_ ->
      let size1 = List.length class_.dc_tparams in
      let size2 = List.length paraml in
      if size1 <> size2 then Errors.class_arity p class_.dc_pos c size1;
      let subst = Inst.make_subst class_.dc_tparams paraml in
      iter2_shortest begin fun (_, (p, _), cstrl) ty ->
        List.iter cstrl begin fun (ck, cstr) ->
          (* Constraint might contain uses of generic type parameters *)
          let cstr = Inst.instantiate subst cstr in
          match ck with
          | Ast.Constraint_as ->
            Type.sub_type_decl p Reason.URnone env ty cstr
          | Ast.Constraint_eq ->
            (* This code could well be unreachable, because we don't allow
             * equality constraints on class generics. *)
            Type.sub_type_decl p Reason.URnone env ty cstr;
            Type.sub_type_decl p Reason.URnone env cstr ty
          | Ast.Constraint_super ->
            Type.sub_type_decl p Reason.URnone env cstr ty
        end
      end class_.dc_tparams paraml

(* In order to type-check a class, we need to know what "parent"
 * refers to. Sometimes people write "parent::", when that happens,
 * we need to know the type of parent.
 *)
and class_def_parent env class_def class_type =
  match class_def.c_extends with
  | (_, Happly ((_, x), _) as parent_ty) :: _ ->
      let parent_type = Decl_env.get_class_dep env.Env.decl_env x in
      (match parent_type with
      | Some parent_type -> check_parent class_def class_type parent_type
      | None -> ());
      let parent_ty = Decl_hint.hint env.Env.decl_env parent_ty in
      env, Some x, parent_ty
  (* The only case where we have more than one parent class is when
   * dealing with interfaces and interfaces cannot use parent.
   *)
  | _ :: _
  | _ -> env, None, (Reason.Rnone, Tany)

and check_parent class_def class_type parent_type =
  let position = fst class_def.c_name in
  (* Are all the parents in Hack? Do we know all their methods?
   * If so, let's check that the abstract methods have been implemented.
   *)
  if class_type.tc_members_fully_known
  then check_parent_abstract position parent_type class_type;
  if parent_type.dc_final
  then Errors.extend_final position parent_type.dc_pos parent_type.dc_name
  else ()

and check_parent_abstract position parent_type class_type =
  let is_final = class_type.tc_final in
  if parent_type.dc_kind = Ast.Cabstract &&
    (class_type.tc_kind <> Ast.Cabstract || is_final)
  then begin
    check_extend_abstract_meth ~is_final position class_type.tc_methods;
    check_extend_abstract_meth ~is_final position class_type.tc_smethods;
    check_extend_abstract_const ~is_final position class_type.tc_consts;
    check_extend_abstract_typeconst
      ~is_final position class_type.tc_typeconsts;
  end else ()

and class_def tcopt c =
  let filename = Pos.filename (fst c.Nast.c_name) in
  let dep = Dep.Class (snd c.c_name) in
  let env = Env.empty tcopt filename (Some dep) in
  (* Set up self identifier and type *)
  let env = Env.set_self_id env (snd c.c_name) in
  let self = get_self_from_c c in
  (* For enums, localize makes self:: into an abstract type, which we don't
   * want *)
  let env, self = match c.c_kind with
    | Ast.Cenum -> env, (fst self, Tclass (c.c_name, []))
    | Ast.Cinterface | Ast.Cabstract | Ast.Ctrait
    | Ast.Cnormal -> Phase.localize_with_self env self in
  let env = Env.set_self env self in
  let c = TNBody.class_meth_bodies tcopt c in
  if not !auto_complete then begin
    NastCheck.class_ env c;
    NastInitCheck.class_ env c;
  end;
  let tc = Env.get_class env (snd c.c_name) in
  match tc with
  | None ->
      (* This can happen if there was an error during the declaration
       * of the class. *)
      None
  | Some tc ->
    Typing_requirements.check_class env tc;
    Some (class_def_ env c tc)

(* Given a class definition construct a type consisting of the
 * class instantiated at its generic parameters.
*)
and get_self_from_c c =
  let tparams = List.map (fst c.c_tparams) begin fun (_, (p, s), _) ->
    Reason.Rwitness p, Tgeneric s
  end in
  Reason.Rwitness (fst c.c_name), Tapply (c.c_name, tparams)

and class_def_ env c tc =
  Typing_hooks.dispatch_enter_class_def_hook c tc;
  let env = Env.set_mode env c.c_mode in
  let pc, _ = c.c_name in
  let impl = List.map
    (c.c_extends @ c.c_implements @ c.c_uses)
    (Decl_hint.hint env.Env.decl_env) in
  TI.check_tparams_instantiable env (fst c.c_tparams);
  let env, constraints =
    Phase.localize_generic_parameters_with_bounds env (fst c.c_tparams)
      ~ety_env:(Phase.env_with_self env) in
  let env = add_constraints (fst c.c_name) env constraints in
  Typing_variance.class_ (Env.get_options env) (snd c.c_name) tc impl;
  List.iter impl (check_implements_tparaml env);

  let env, parent_id, parent = class_def_parent env c tc in
  let is_final = tc.tc_final in
  if (tc.tc_kind = Ast.Cnormal || is_final) && tc.tc_members_fully_known
  then begin
    check_extend_abstract_meth ~is_final pc tc.tc_methods;
    check_extend_abstract_meth ~is_final pc tc.tc_smethods;
    check_extend_abstract_const ~is_final pc tc.tc_consts;
    check_extend_abstract_typeconst ~is_final pc tc.tc_typeconsts;
  end;
  let env = Env.set_parent env parent in
  let env = match parent_id with
    | None -> env
    | Some parent_id -> Env.set_parent_id env parent_id in
  if tc.tc_final then begin
    match c.c_kind with
    | Ast.Cinterface -> Errors.interface_final (fst c.c_name)
    | Ast.Cabstract -> ()
    | Ast.Ctrait -> Errors.trait_final (fst c.c_name)
    | Ast.Cenum ->
      Errors.internal_error pc "The parser should not parse final on enums"
    | Ast.Cnormal -> ()
  end;
  SMap.iter (check_static_method tc.tc_methods) tc.tc_smethods;
  List.iter impl (class_implements_type env c);
  let typed_vars = List.map c.c_vars (class_var_def env ~is_static:false c) in
  let typed_methods = List.map c.c_methods (method_def env) in
  let typed_typeconsts = List.map c.c_typeconsts (typeconst_def env) in
  let typed_consts, const_types =
    List.unzip (List.map c.c_consts (class_const_def env)) in
  let env = Typing_enum.enum_class_check env tc c.c_consts const_types in
  let typed_constructor = class_constr_def env c in
  let env = Env.set_static env in
  let typed_static_vars =
    List.map c.c_static_vars (class_var_def env ~is_static:true c) in
  let typed_static_methods = List.map c.c_static_methods (method_def env) in
  Typing_hooks.dispatch_exit_class_def_hook c tc;
  {
    T.c_mode = c.c_mode;
    T.c_final = c.c_final;
    T.c_is_xhp = c.c_is_xhp;
    T.c_kind = c.c_kind;
    T.c_name = c.c_name;
    T.c_tparams = c.c_tparams;
    T.c_extends = c.c_extends;
    T.c_uses = c.c_uses;
    T.c_xhp_attr_uses = c.c_xhp_attr_uses;
    T.c_xhp_category = c.c_xhp_category;
    T.c_req_extends = c.c_req_extends;
    T.c_req_implements = c.c_req_implements;
    T.c_implements = c.c_implements;
    T.c_consts = typed_consts;
    T.c_typeconsts = typed_typeconsts;
    T.c_static_vars = typed_static_vars;
    T.c_vars = typed_vars;
    T.c_constructor = typed_constructor;
    T.c_static_methods = typed_static_methods;
    T.c_methods = typed_methods;
    T.c_user_attributes = List.map c.c_user_attributes (user_attribute env);
    T.c_enum = c.c_enum;
  }, env

and check_static_method obj method_name static_method =
  if SMap.mem method_name obj
  then begin
    let lazy (static_method_reason, _) = static_method.ce_type in
    let dyn_method = SMap.find_unsafe method_name obj in
    let lazy (dyn_method_reason, _) = dyn_method.ce_type in
    Errors.static_dynamic
      (Reason.to_pos static_method_reason)
      (Reason.to_pos dyn_method_reason)
      method_name
  end
  else ()

and check_extend_abstract_meth ~is_final p smap =
  SMap.iter begin fun x ce ->
    match ce.ce_type with
    | lazy (r, Tfun { ft_abstract = true; _ }) ->
        Errors.implement_abstract ~is_final p (Reason.to_pos r) "method" x
    | _ -> ()
  end smap

(* Type constants must be bound to a concrete type for non-abstract classes.
 *)
and check_extend_abstract_typeconst ~is_final p smap =
  SMap.iter begin fun x tc ->
    if tc.ttc_type = None then
      Errors.implement_abstract ~is_final p (fst tc.ttc_name) "type constant" x
  end smap

and check_extend_abstract_const ~is_final p smap =
  SMap.iter begin fun x cc ->
    match cc.cc_type with
    | r, _ when cc.cc_abstract && not cc.cc_synthesized ->
      Errors.implement_abstract ~is_final p (Reason.to_pos r) "constant" x
    | _,
      (
        Terr
        | Tany
        | Tmixed
        | Tarray (_, _)
        | Tdarray (_, _)
        | Tvarray _
        | Tvarray_or_darray _
        | Toption _
        | Tprim _
        | Tfun _
        | Tapply (_, _)
        | Ttuple _
        | Tshape _
        | Taccess (_, _)
        | Tthis
        | Tgeneric _
      ) -> ()
  end smap

and typeconst_def env {
  c_tconst_name = (pos, _) as id;
  c_tconst_constraint;
  c_tconst_type;
} =
  let env, cstr = opt Phase.hint_locl env c_tconst_constraint in
  let env, ty = opt Phase.hint_locl env c_tconst_type in
  ignore (
    Option.map2 ty cstr ~f:(Type.sub_type pos Reason.URtypeconst_cstr env)
  );
  {
    T.c_tconst_name = id;
    T.c_tconst_constraint = c_tconst_constraint;
    T.c_tconst_type = c_tconst_type;
  }

and class_const_def env (h, id, e) =
  let env, ty =
    match h with
    | None -> env, Env.fresh_type()
    | Some h ->
       Phase.hint_locl env h
  in
  match e with
    | Some e ->
      let env, te, ty' = expr env e in
      ignore (Type.sub_type (fst id) Reason.URhint env ty' ty);
      (h, id, Some te), ty'
    | None ->
      (h, id, None), ty

and class_constr_def env c =
  Option.map c.c_constructor (method_def env)

and class_implements_type env c1 ctype2 =
  let params =
    List.map (fst c1.c_tparams) begin fun (_, (p, s), _) ->
      (Reason.Rwitness p, Tgeneric s)
    end in
  let r = Reason.Rwitness (fst c1.c_name) in
  let ctype1 = r, Tapply (c1.c_name, params) in
  Typing_extends.check_implements env ctype2 ctype1;
  ()

and class_var_def env ~is_static c cv =
  let env, typed_cv_expr, ty =
    match cv.cv_expr with
    | None -> env, None, Env.fresh_type()
    | Some e -> let env, te, ty = expr env e in env, Some te, ty in
  (match cv.cv_type with
  | None when Env.is_strict env ->
      Errors.add_a_typehint (fst cv.cv_id)
  | None ->
      let pos, name = cv.cv_id in
      let name = if is_static then "$"^name else name in
      let var_type = Reason.Rwitness pos, Tany in
      (match cv.cv_expr with
      | None ->
          Typing_suggest.uninitialized_member (snd c.c_name) name env var_type ty;
          ()
      | Some _ ->
          Typing_suggest.save_member name env var_type ty;
          ()
      )
  | Some (p, _ as cty) ->
      let env =
        (* If this is an XHP attribute and we're in strict mode,
           relax to partial mode to allow the use of the "array"
           annotation without specifying type parameters. Until
           recently HHVM did not allow "array" with type parameters
           in XHP attribute declarations, so this is a temporary
           hack to support existing code for now. *)
        (* Task #5815945: Get rid of this Hack *)
        if cv.cv_is_xhp && (Env.is_strict env)
          then Env.set_mode env FileInfo.Mpartial
          else env in
      let cty = TI.instantiable_hint env cty in
      let env, cty = Phase.localize_with_self env cty in
      let _ = Type.sub_type p Reason.URhint env ty cty in
      ());
  {
    T.cv_final = cv.cv_final;
    T.cv_is_xhp = cv.cv_is_xhp;
    T.cv_visibility = cv.cv_visibility;
    T.cv_type = cv.cv_type;
    T.cv_id = cv.cv_id;
    T.cv_expr = typed_cv_expr;
  }

and localize_where_constraints
    ~ety_env (env:Env.env) (where_constraints:Nast.where_constraint list) =
  let add_constraint env (h1, ck, h2) =
    let env, ty1 =
      Phase.localize env (Decl_hint.hint env.Env.decl_env h1) ~ety_env in
    let env, ty2 =
      Phase.localize env (Decl_hint.hint env.Env.decl_env h2) ~ety_env in
    SubType.add_constraint (fst h1) env ck ty1 ty2
  in
  List.fold_left where_constraints ~f:add_constraint ~init:env

and add_constraints p env constraints =
  let add_constraint env (ty1, ck, ty2) =
    SubType.add_constraint p env ck ty1 ty2 in
  List.fold_left constraints ~f:add_constraint ~init: env

and user_attribute env ua =
  let typed_ua_params =
    List.map ua.ua_params (fun e -> let _env, te, _ty = expr env e in te) in
  {
    T.ua_name = ua.ua_name;
    T.ua_params = typed_ua_params;
  }

and make_default_return name
  = if snd name = SN.Members.__destruct
    || snd name = SN.Members.__construct
    then (Reason.Rwitness (fst name), Tprim Tvoid)
    else (Reason.Rwitness (fst name), Tany)

and method_def env m =
  (* reset the expression dependent display ids for each method body *)
  Reason.expr_display_id_map := IMap.empty;
  Typing_hooks.dispatch_enter_method_def_hook m;
  let env =
    Env.env_with_locals env Typing_continuations.Map.empty Local_id.Map.empty
  in
  let ety_env =
    { (Phase.env_with_self env) with from_class = Some CIstatic; } in
  let env, constraints =
    Phase.localize_generic_parameters_with_bounds env m.m_tparams
    ~ety_env:ety_env in
  TI.check_tparams_instantiable env m.m_tparams;
  let env = add_constraints (fst m.m_name) env constraints in
  let env =
    localize_where_constraints ~ety_env env m.m_where_constraints in
  let env = Env.set_local env this (Env.get_self env) in
  let env, ret = match m.m_ret with
    | None -> env, make_default_return m.m_name
    | Some ret ->
      let ret = TI.instantiable_hint env ret in
      (* If a 'this' type appears it needs to be compatiable with the
       * late static type
       *)
      let ety_env =
        { (Phase.env_with_self env) with
          from_class = Some CIstatic } in
      Phase.localize ~ety_env env ret in
  TI.check_params_instantiable env m.m_params;
  let env, param_tys = List.map_env env m.m_params make_param_local_ty in
  if Env.is_strict env then begin
    List.iter2_exn ~f:(check_param env) m.m_params param_tys;
  end;
  if Attributes.mem SN.UserAttributes.uaMemoize m.m_user_attributes then
    List.iter2_exn ~f:(check_memoizable env) m.m_params param_tys;
  let env, typed_params =
    List.map_env env (List.zip_exn param_tys m.m_params) bind_param in
  let env, t_variadic = match m.m_variadic with
    | FVvariadicArg vparam ->
      TI.check_param_instantiable env vparam;
      let env, ty = make_param_local_ty env vparam in
      if Env.is_strict env then
        check_param env vparam ty;
      if Attributes.mem SN.UserAttributes.uaMemoize m.m_user_attributes then
        check_memoizable env vparam ty;
      let env, t_variadic = bind_param env (ty, vparam) in
      env, (T.FVvariadicArg t_variadic)
    | FVellipsis -> env, T.FVellipsis
    | FVnonVariadic -> env, T.FVnonVariadic in
  let nb = Nast.assert_named_body m.m_body in
  let env, tb =
    fun_ ~abstract:m.m_abstract env ret (fst m.m_name) nb m.m_fun_kind in
  let env =
    Env.check_todo env in
  let m_ret =
    match m.m_ret with
    | None when
         snd m.m_name = SN.Members.__destruct
      || snd m.m_name = SN.Members.__construct ->
      Some (fst m.m_name, Happly((fst m.m_name, "void"), []))
    | None when Env.is_strict env ->
      suggest_return env (fst m.m_name) ret; None
    | None -> let (pos, id) = m.m_name in
              let id = (Env.get_self_id env) ^ "::" ^ id in
              Typing_suggest.save_fun_or_method (pos, id);
              m.m_ret
    | Some _ -> m.m_ret in
  let m = { m with m_ret = m_ret; } in
  Typing_hooks.dispatch_exit_method_def_hook m;
  {
    T.m_final = m.m_final;
    T.m_abstract = m.m_abstract;
    T.m_visibility = m.m_visibility;
    T.m_name = m.m_name;
    T.m_tparams = m.m_tparams;
    T.m_where_constraints = m.m_where_constraints;
    T.m_variadic = t_variadic;
    T.m_params = typed_params;
    T.m_fun_kind = m.m_fun_kind;
    T.m_user_attributes = List.map m.m_user_attributes (user_attribute env);
    T.m_ret = m.m_ret;
    T.m_body = T.NamedBody {
      T.fnb_nast = tb;
      T.fnb_unsafe = nb.fnb_unsafe;
    };
  }

and typedef_def tcopt typedef  =
  let tid = (snd typedef.t_name) in
  let filename = Pos.filename (fst typedef.t_kind) in
  let dep = Typing_deps.Dep.Class tid in
  let env =
    Typing_env.empty tcopt filename (Some dep) in
  (* Mode for typedefs themselves doesn't really matter right now, but
   * they can expand hints, so make it loose so that the typedef doesn't
   * fail. (The hint will get re-checked with the proper mode anyways.)
   * Ideally the typedef would carry the right mode with it, but it's a
   * slightly larger change than I want to deal with right now. *)
  let env = Typing_env.set_mode env FileInfo.Mdecl in
  let env, constraints =
    Phase.localize_generic_parameters_with_bounds env typedef.t_tparams
              ~ety_env:(Phase.env_with_self env) in
  let env = add_constraints (fst typedef.t_name) env constraints in
  NastCheck.typedef env typedef;
  let {
    t_name = t_pos, _;
    t_tparams = _;
    t_constraint = tcstr;
    t_kind = hint;
    t_user_attributes = _;
    t_vis = _;
    t_mode = _;
  } = typedef in
  let ty = TI.instantiable_hint env hint in
  let env, ty = Phase.localize_with_self env ty in
  let env = begin match tcstr with
    | Some tcstr ->
      let cstr = TI.instantiable_hint env tcstr in
      let env, cstr = Phase.localize_with_self env cstr in
      Typing_ops.sub_type t_pos Reason.URnewtype_cstr env ty cstr
    | _ -> env
  end in
  let env = begin match hint with
    | pos, Hshape { nsi_allows_unknown_fields=_; nsi_field_map } ->
      check_shape_keys_validity env pos (ShapeMap.keys nsi_field_map)
    | _ -> env
  end in
  {
    T.t_name = typedef.t_name;
    T.t_mode = typedef.t_mode;
    T.t_vis = typedef.t_vis;
    T.t_user_attributes = List.map typedef.t_user_attributes (user_attribute env);
    T.t_constraint = typedef.t_constraint;
    T.t_kind = typedef.t_kind;
    T.t_tparams = typedef.t_tparams;
  }, env

and gconst_def cst tcopt =
  Typing_hooks.dispatch_global_const_hook cst.cst_name;
  let filename = Pos.filename (fst cst.cst_name) in
  let dep = Typing_deps.Dep.GConst (snd cst.cst_name) in
  let env = Typing_env.empty tcopt filename (Some dep) in
  let env = Typing_env.set_mode env cst.cst_mode in
  let typed_cst_value, tenv =
    match cst.cst_value with
    | None -> None, env
    | Some value ->
      let env, te, value_type = expr env value in
      match cst.cst_type with
      | Some hint ->
        let ty = TI.instantiable_hint env hint in
        let env, dty = Phase.localize_with_self env ty in
        let env = Typing_utils.sub_type env value_type dty in
        Some te, env
      | None -> Some te, env
  in
  { T.cst_mode = cst.cst_mode;
    T.cst_name = cst.cst_name;
    T.cst_type = cst.cst_type;
    T.cst_value = typed_cst_value
  }, tenv

(* Calls the method of a class, but allows the f callback to override the
 * return value type *)
and overload_function p env class_id method_id el uel f =
  let env, _ce, ty = static_class_id p env class_id in
  let env, _tel, _ = exprs env el in
  let env, fty, _ =
    class_get ~is_method:true ~is_const:false env ty method_id class_id in
  (* call the function as declared to validate arity and input types,
     but ignore the result and overwrite with custom one *)
  let (env, res), has_error = Errors.try_with_error
      (* TODO: Should we be passing hints here *)
     (fun () -> (let env, _, _, ty = call p env fty el uel in env, ty), false)
     (fun () -> (env, (Reason.Rwitness p, Tany)), true) in
   (* if there are errors already stop here - going forward would
    * report them twice *)
   if has_error then env, T.make_typed_expr p res T.Any, res
   else let env, ty = f env fty res el in
   (* TODO TAST: do this right *)
   env, T.make_typed_expr p ty T.Any, ty

and update_array_type ?lhs_of_null_coalesce p env e1 e2 valkind  =
  let access_type = Typing_arrays.static_array_access env e2 in
  let type_mapper =
    Typing_arrays.update_array_type p access_type in
  match valkind with
    | `lvalue | `lvalue_subexpr ->
      let env, te1, ty1 =
        raw_expr ~valkind:`lvalue_subexpr ~in_cond:false env e1 in
      let env, ty1 = type_mapper env ty1 in
      begin match e1 with
        | (_, Lvar (_, x)) ->
          (* type_mapper has updated the type in ty1 typevars, but we
             need to update the local variable type too *)
          let env, ty1 = set_valid_rvalue p env x ty1 in
          env, te1, ty1
        | _ -> env, te1, ty1
      end
    | _ ->
      raw_expr ~in_cond:false ?lhs_of_null_coalesce env e1
