(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


(* This module performs checks after the naming has been done.
   Basically any check that doesn't fall in the typing category. *)
(* Check of type application arity *)
(* Check no `new AbstractClass` (or trait, or interface) *)
(* Check no top-level break / continue *)

(* NOTE: since the typing environment does not generally contain
   information about non-Hack code, some of these checks can
   only be done in strict (i.e. "everything is Hack") mode. Use
   `if Env.is_strict env.tenv` style checks accordingly.
*)


open Core_kernel
open Nast
open Typing_defs
open Utils

module Env = Typing_env
module Inst = Decl_instantiate
module Phase = Typing_phase
module SN = Naming_special_names
module TGenConstraint = Typing_generic_constraint
module Subst = Decl_subst
module TUtils = Typing_utils
module Cls = Typing_classes_heap


type control_context =
  | Toplevel
  | LoopContext
  | SwitchContext

type env = {
  function_name: string option;
  class_name: string option;
  class_kind: Ast.class_kind option;
  typedef_tparams : Nast.tparam list;
  is_array_append_allowed: bool;
  is_reactive: bool; (* The enclosing function is reactive *)
  tenv: Env.env;
}

module CheckFunctionBody = struct
  let rec stmt f_type env st = match f_type, (snd st) with
    | Ast.FSync, Return None
    | Ast.FAsync, Return None -> ()
    | Ast.FSync, Return (Some e)
    | Ast.FAsync, Return (Some e) ->
        expr_allow_await f_type env e;
        ()
    | (Ast.FGenerator | Ast.FAsyncGenerator), Return _ -> ()
    | Ast.FCoroutine, Return e ->
        Option.iter e ~f:(expr f_type env)
    | _, Throw (_, e) ->
        expr f_type env e
    | _, Expr e ->
        expr_allow_await_or_rx_move f_type env e;
        ()
    | _, ( Noop | Fallthrough | GotoLabel _ | Goto _ | Break | Continue
         | Unsafe_block _ ) -> ()
    | _, Awaitall (el, b) ->
        List.iter el (fun (_, y) ->
          expr f_type env y;
        );
        block f_type env b;
        ()
    | _, If (cond, b1, b2) ->
        expr f_type env cond;
        block f_type env b1;
        block f_type env b2;
        ()
    | _, Do (b, e) ->
        block f_type env b;
        expr f_type env e;
        ()
    | _, While (e, b) ->
        expr f_type env e;
        block f_type env b;
        ()
    | _, Using { us_has_await = has_await; us_expr = e; us_block = b; _ } ->
        if has_await then found_await f_type (fst e);
        expr_allow_await_list f_type env e;
        block f_type env b;
        ()
    | _, For (init, cond, incr, b) ->
        expr f_type env init;
        expr f_type env cond;
        expr f_type env incr;
        block f_type env b;
        ()
    | _, Switch (e, cl) ->
        expr f_type env e;
        List.iter cl (case f_type env);
        ()
    | _, Foreach (_, (Await_as_v (p, _) | Await_as_kv (p, _, _)), _) ->
        found_await f_type p

    | _, Foreach (v, _, b) ->
        expr f_type env v;
        block f_type env b;
        ()
    | _, Try (b, cl, fb) ->
        block f_type env b;
        List.iter cl (catch f_type env);
        block f_type env fb;
        ()
    | _, Def_inline _ -> ()
    | _, Let ((p, x), _, e) ->
        (* We treat let statement as an assignment expression *)
        let fake_expr = (p, Binop (Ast.Eq None, (p, Lvar (p, x)), e)) in
        expr_allow_await f_type env fake_expr;
        ()
    | _, Block b -> block f_type env b;
    | _, Markup (_, eopt) -> (match eopt with Some e -> expr f_type env e | None -> ())
    | _, Declare (_, e, b) ->
      expr f_type env e;
      block f_type env b;

  and found_await ftype p =
    match ftype with
    | Ast.FCoroutine -> ()
    | Ast.FSync | Ast.FGenerator -> Errors.await_in_sync_function p
    | _ -> ()

  and block f_type env stl =
    List.iter stl (stmt f_type env)

  and start f_type env stl =
    block f_type env stl

  and case f_type env = function
    | Default b -> block f_type env b
    | Case (cond, b) ->
        expr f_type env cond;
        block f_type env b;
        ()

  and catch f_type env (_, _, b) = block f_type env b

  and afield f_type env = function
    | AFvalue e -> expr f_type env e
    | AFkvalue (e1, e2) -> expr2 f_type env (e1, e2)

  and expr f_type env (p, e) =
    expr_ p f_type env e

  and expr_allow_await_list f_type env ((_, exp) as e) =
    match exp with
    | Expr_list el ->
      List.iter el (expr_allow_await f_type env)
    | _ ->
      expr_allow_await f_type env e

  and expr_allow_await ?(is_rhs=false) f_type env (p, exp) = match f_type, exp with
    | Ast.FAsync, Await e
    | Ast.FAsyncGenerator, Await e -> expr f_type env e; ()
    | Ast.FAsync, Binop (Ast.Eq None, e1, (_, Await e))
    | Ast.FAsyncGenerator, Binop (Ast.Eq None, e1, (_, Await e)) when not is_rhs ->
      expr f_type env e1;
      expr f_type env e;
      ()
    | _ -> expr_ p f_type env exp; ()

  and expr_allow_rx_move orelse f_type env  exp  =
    match exp with
    | _, Call (_, e, _, el, uel) when is_rx_move e ->
      expr f_type env e;
      List.iter el ~f:(expr f_type env);
      List.iter uel ~f:(expr f_type env);
      ()
    | _ ->
      orelse f_type env exp

  and expr_allow_await_or_rx_move f_type env exp =
    match exp with
    | _, Binop (Ast.Eq None, e1, rhs) ->
      expr f_type env e1;
      expr_allow_rx_move (expr_allow_await ~is_rhs:true) f_type env rhs
    | _ ->
      expr_allow_await f_type env exp

  and is_rx_move e =
    match e with
    | _, Id (_, v) -> v = SN.Rx.move
    | _ -> false

  and expr2 f_type env (e1, e2) =
    expr f_type env e1;
    expr f_type env e2;
    ()

  and expr_ p f_type env exp = match f_type, exp with
    | _, Collection _
    | _, Import _
    | _, Omitted
    | _, BracedExpr _
    | _, ParenthesizedExpr _ -> failwith "AST should not contain these nodes after naming"
    | _, Any -> ()
    | _, Class_const _
    | _, Fun_id _
    | _, Method_id _
    | _, Smethod_id _
    | _, Method_caller _
    | _, This
    | _, Class_get _
    | _, Typename _
    | _, Lplaceholder _
    | _, Dollardollar _ -> ()
    | _, Id _ -> ()

    | _, ImmutableVar _
    | _, Lvar _ ->
        ()
    | _, Pipe (_, l, r) ->
        expr f_type env l;
        expr f_type env r;
    | _, Array afl ->
        List.iter afl (afield f_type env);
        ()
    | _, Darray (_, afl) ->
        List.iter afl (expr2 f_type env);
        ()
    | _, Varray (_, afl) ->
        List.iter afl (expr f_type env);
        ()
    | _, ValCollection (_, _, el) ->
        List.iter el (expr f_type env);
        ()
    | _, KeyValCollection (_, _, fdl) ->
        List.iter fdl (expr2 f_type env);
        ()
    | _, Clone e -> expr f_type env e; ()
    | _, Obj_get (e, (_, Id _s), _) ->
        expr f_type env e;
        ()
    | _, Obj_get (e1, e2, _) ->
        expr2 f_type env (e1, e2);
        ()
    | _, Array_get (e, eopt) ->
        expr f_type env e;
        maybe (expr f_type) env eopt;
        ()
    | _, Call (_, e, _, _, _) when is_rx_move e ->
        Errors.rx_move_invalid_location (fst e);
        ()
    | _, Call (_, e, _, el, uel) ->
        expr f_type env e;
        List.iter el (expr_allow_rx_move expr f_type env);
        List.iter uel (expr_allow_rx_move expr f_type env);
        ()
    | _, True | _, False | _, Int _
    | _, Float _ | _, Null | _, String _ -> ()
    | _, PrefixedString (_, e) ->
        expr f_type env e;
        ()
    | _, String2 el ->
        List.iter el (expr f_type env);
        ()
    | _, List el ->
        List.iter el (expr f_type env);
        ()
    | _, Pair (e1, e2) ->
        expr2 f_type env (e1, e2);
        ()
    | _, Expr_list el ->
        List.iter el (expr f_type env);
        ()
    | _, Unop (_, e) -> expr f_type env e
    | _, Binop (_, e1, e2) ->
        expr2 f_type env (e1, e2);
        ()
    | _, Eif (e1, None, e3) ->
        expr2 f_type env (e1, e3);
        ()
    | _, Eif (e1, Some e2, e3) ->
        List.iter [e1; e2; e3] (expr f_type env);
        ()
    | _, New (_, _, el, uel, _) ->
      List.iter el (expr f_type env);
      List.iter uel (expr f_type env);
      ()
    | _, Record (_, afl) ->
      List.iter afl (expr2 f_type env);
      ()
    | _, InstanceOf (e, _)
    | _, Is (e, _)
    | _, As (e, _, _)
    | _, Cast (_, e) ->
        expr f_type env e;
        ()
    | _, Efun _ -> ()
    | _, Lfun _ -> ()

    | _, PU_atom _ -> ()
    | _, PU_identifier _ -> ()

    | Ast.FGenerator, Yield_break
    | Ast.FAsyncGenerator, Yield_break -> ()
    | Ast.FGenerator, Yield af
    | Ast.FAsyncGenerator, Yield af -> afield f_type env af; ()
    | Ast.FGenerator, Yield_from e
    | Ast.FAsyncGenerator, Yield_from e -> expr f_type env e; ()
    (* Should never happen -- presence of yield should make us FGenerator or
     * FAsyncGenerator. *)
    | (Ast.FSync | Ast.FAsync), (Yield _ | Yield_from _ | Yield_break) -> assert false

    | (Ast.FGenerator | Ast.FSync | Ast.FCoroutine), Await _ ->
      found_await f_type p

    | Ast.FAsync, Await _
    | Ast.FAsyncGenerator, Await _ -> Errors.await_not_allowed p

    | Ast.FCoroutine, (Yield _ | Yield_break | Yield_from _ | Suspend _)
    | (Ast.FSync | Ast.FAsync | Ast.FGenerator | Ast.FAsyncGenerator), Suspend _ -> ()
    | _, Special_func func ->
        (match func with
          | Gena e
          | Gen_array_rec e -> expr f_type env e
          | Genva el -> List.iter el (expr f_type env));
        ()
    | _, Xml (_, attrl, el) ->
        List.iter attrl (fun attr -> expr f_type env (get_xhp_attr_expr attr));
        List.iter el (expr f_type env);
        ()
    | _, Unsafe_expr _ -> ()
    | _, Callconv (_, e) ->
        expr f_type env e;
        ()
    | _, Assert (AE_assert e) ->
        expr f_type env e;
        ()
    | _, Shape fdm ->
        List.iter ~f:(fun (_, v) -> expr f_type env v) fdm;
        ()

end

let is_some_reactivity_attribute { ua_name = (_, name); _ } =
  name = SN.UserAttributes.uaReactive ||
  name = SN.UserAttributes.uaLocalReactive ||
  name = SN.UserAttributes.uaShallowReactive

(* During NastCheck, all reactivity kinds are the same *)
let fun_is_reactive user_attributes =
  List.exists user_attributes ~f:is_some_reactivity_attribute

let rec fun_ tenv f named_body =
  let env = { is_array_append_allowed = false;
              class_name = None; class_kind = None;
              typedef_tparams = [];
              tenv = tenv;
              function_name = None;
              is_reactive = fun_is_reactive f.f_user_attributes;
              } in
  func env f named_body

and func env f named_body =
  let p, _ = f.f_name in
  (* Add type parameters to typing environment and localize the bounds
     and where constraints *)
  let ety_env = Phase.env_with_self env.tenv in
  let tenv, constraints =
    Phase.localize_generic_parameters_with_bounds env.tenv f.f_tparams
      ~ety_env in
  let tenv = add_constraints p tenv constraints in
  let tenv =
    Phase.localize_where_constraints ~ety_env tenv f.f_where_constraints in
  let env = { env with
    tenv = Env.set_mode tenv f.f_mode;
    is_reactive = env.is_reactive || fun_is_reactive f.f_user_attributes;
  } in
  maybe hint env f.f_ret;

  List.iter f.f_tparams (tparam env);
  List.iter f.f_params (fun_param env);
  block env named_body.fb_ast;
  CheckFunctionBody.start
    f.f_fun_kind
    env
    named_body.fb_ast

and tparam env t =
  List.iter t.tp_constraints (fun (_, h) -> hint env h)

and hint env (p, h) =
  hint_ env p h

and hint_ env p = function
  | Hany  | Hmixed | Hnonnull | Hprim _  | Hthis | Haccess _ | Habstr _  |
    Hdynamic | Hnothing ->
     ()
  | Harray (ty1, ty2) ->
      maybe hint env ty1;
      maybe hint env ty2
  | Hdarray (ty1, ty2) ->
      hint env ty1;
      hint env ty2
  | Hvarray_or_darray ty
  | Hvarray ty ->
      hint env ty
  | Htuple hl ->
      List.iter hl (hint env)
  | Hoption h
  | Hsoft h
  | Hlike h ->
      hint env h
  | Hfun (_, _, hl, _, _, variadic_hint, h, _) ->
      List.iter hl (hint env);
      hint env h;
      begin match variadic_hint with
      | Hvariadic (Some h) -> hint env h;
      | _ -> ()
      end
  | Happly ((_, x), hl) as h when Env.is_typedef x ->
    begin match Typing_lazy_heap.get_typedef x with
      | Some _ ->
        check_happly env.typedef_tparams env.tenv (p, h);
        List.iter hl (hint env)
      | None -> ()
    end
  | Happly ((_, x), hl) as h ->
      (match Env.get_class env.tenv x with
      | None -> ()
      | Some _ ->
          check_happly env.typedef_tparams env.tenv (p, h);
          List.iter hl (hint env)
      );
      ()
  | Hshape { nsi_allows_unknown_fields=_; nsi_field_map } ->
      let compute_hint_for_shape_field_info { sfi_hint; _; } =
        hint env sfi_hint in
      List.iter ~f:compute_hint_for_shape_field_info nsi_field_map

and check_happly unchecked_tparams env h =
  let env = { env with Env.pos = (fst h) } in
  let decl_ty = Decl_hint.hint env.Env.decl_env h in
  let unchecked_tparams =
    List.map unchecked_tparams begin fun t ->
      let cstrl = List.map t.tp_constraints (fun (ck, cstr) ->
        let cstr = Decl_hint.hint env.Env.decl_env cstr in
        (ck, cstr)) in
      {
        Typing_defs.tp_variance = t.tp_variance;
        tp_name = t.tp_name;
        tp_constraints = cstrl;
        tp_reified = t.tp_reified;
        tp_user_attributes = t.tp_user_attributes;
      }
    end in
  let tyl = List.map unchecked_tparams (fun t -> Reason.Rwitness (fst t.tp_name), Tany) in
  let subst = Inst.make_subst unchecked_tparams tyl in
  let decl_ty = Inst.instantiate subst decl_ty in
  match decl_ty with
  | _, Tapply (_, tyl) when tyl <> [] ->
      let env, locl_ty = Phase.localize_with_self env decl_ty in
      begin match TUtils.get_base_type env locl_ty with
        | _, Tclass (cls, _, tyl) ->
          (match Env.get_class env (snd cls) with
            | Some cls ->
                let tc_tparams = Cls.tparams cls in
                (* We want to instantiate the class type parameters with the
                 * type list of the class we are localizing. We do not want to
                 * add any more constraints when we localize the constraints
                 * stored in the class_type since it may lead to infinite
                 * recursion
                 *)
                let ety_env =
                  { (Phase.env_with_self env) with
                    substs = Subst.make tc_tparams tyl;
                  } in
                iter2_shortest begin fun { tp_name = (p, x); tp_constraints = cstrl; _ } ty ->
                  List.iter cstrl (fun (ck, cstr_ty) ->
                      let r = Reason.Rwitness p in
                      let env, cstr_ty = Phase.localize ~ety_env env cstr_ty in
                      ignore @@ Errors.try_
                        (fun () ->
                           TGenConstraint.check_constraint env ck ty ~cstr_ty
                        )
                        (fun l ->
                          Reason.explain_generic_constraint env.Env.pos r x l;
                          env
                        ))
                end tc_tparams tyl
            | _ -> ()
            )
        | _ -> ()
      end
  | _ -> ()

and class_ tenv c =
  let cname = Some (snd c.c_name) in
  let env = { is_array_append_allowed = false;
              class_name = cname;
              class_kind = Some c.c_kind;
              typedef_tparams = [];
              is_reactive = false;
              function_name = None;
              tenv = tenv;
            } in
  (* Add type parameters to typing environment and localize the bounds *)
  let tenv, constraints = Phase.localize_generic_parameters_with_bounds
               tenv c.c_tparams.c_tparam_list
               ~ety_env:(Phase.env_with_self tenv) in
  let tenv = add_constraints (fst c.c_name) tenv constraints in
  let env = { env with tenv = Env.set_mode tenv c.c_mode } in

  if not (c.c_kind = Ast.Cinterface) then begin
    maybe method_ env c.c_constructor;
  end;
  List.iter c.c_tparams.c_tparam_list (tparam env);
  List.iter c.c_extends (hint env);
  List.iter c.c_implements (hint env);
  List.iter c.c_consts (class_const env);
  List.iter c.c_typeconsts (typeconst (env, c.c_tparams.c_tparam_list));
  List.iter c.c_static_vars (class_var env);
  List.iter c.c_vars (class_var env);
  List.iter c.c_static_methods (method_ env);
  List.iter c.c_methods (method_ env);

and class_const env (h, _, e) =
  maybe hint env h;
  maybe expr env e;
  ()

and typeconst (env, _) tconst =
  maybe hint env tconst.c_tconst_type;
  maybe hint env tconst.c_tconst_constraint;
and class_var env cv =
  let hint_env =
    (* If this is an XHP attribute and we're in strict mode,
       relax to partial mode to allow the use of generic
       classes without specifying type parameters. This is
       a temporary hack to support existing code for now. *)
    (* Task #5815945: Get rid of this Hack *)
    if cv.cv_is_xhp && (Typing_env.is_strict env.tenv)
      then { env with tenv = Typing_env.set_mode env.tenv FileInfo.Mpartial }
      else env in
  maybe hint hint_env cv.cv_type;
  maybe expr env cv.cv_expr;
  ()

and add_constraint pos tenv (ty1, ck, ty2) =
  Typing_subtype.add_constraint pos tenv ck ty1 ty2

and add_constraints pos tenv (cstrs: locl where_constraint list) =
  List.fold_left cstrs ~init:tenv ~f:(add_constraint pos)

and method_ env m =
  let env =
    { env with is_reactive = fun_is_reactive m.m_user_attributes } in
  let named_body = assert_named_body m.m_body in
  let env = { env with function_name = Some (snd m.m_name) } in
  (* Add method type parameters to environment and localize the bounds
     and where constraints *)
  let ety_env = Phase.env_with_self env.tenv in
  let tenv, constraints =
    Phase.localize_generic_parameters_with_bounds env.tenv m.m_tparams
      ~ety_env in
  let tenv = add_constraints (fst m.m_name) tenv constraints in
  let tenv =
    Phase.localize_where_constraints ~ety_env tenv m.m_where_constraints in
  let env = { env with tenv = tenv } in

  List.iter m.m_params (fun_param env);
  List.iter m.m_tparams (tparam env);
  block env named_body.fb_ast;
  maybe hint env m.m_ret;
  CheckFunctionBody.start
    m.m_fun_kind
    env
    named_body.fb_ast;

and fun_param env param =
  maybe hint env param.param_hint;
  maybe expr env param.param_expr;

and stmt env (_, s) = stmt_ env s

and stmt_ env = function
  | Return None
  | GotoLabel _
  | Goto _
  | Noop
  | Unsafe_block _
  | Fallthrough
  | Break
  | Continue -> ()
  | Return (Some e)
  | Expr e | Throw (_, e) ->
    expr env e
  | Awaitall (el, b) ->
      List.iter el (fun (_, y) ->
        expr env y;
      );
      block env b;
      ()
  | If (e, b1, b2) ->
    expr env e;
    block env b1;
    block env b2;
    ()
  | Do (b, e) ->
    block env b;
    expr env e;
    ()
  | While (e, b) ->
      expr env e;
      block env b;
      ()
  | Using { us_expr = e; us_block = b; _ } ->
      expr env e;
      block env b;
      ()
  | For (e1, e2, e3, b) ->
      expr env e1;
      expr env e2;
      expr env e3;
      block env b;
      ()
  | Switch (e, cl) ->
      expr env e;
      List.iter cl (case env);
      ()
  | Foreach (e1, ae, b) ->
      expr env e1;
      as_expr env ae;
      block env b;
      ()
  | Try (b, cl, fb) ->
      block env b;
      List.iter cl (catch env);
      block env fb;
      ()
  | Def_inline _ -> ()
  | Let ((p, x), _, e) ->
      (* We treat let statement as assignment expresssions *)
      let fake_expr = (p, Binop (Ast.Eq None, (p, Lvar (p, x)), e)) in
      expr env fake_expr;
      ()
  | Block b -> block env b;
  | Markup (_, eopt) -> (match eopt with Some e -> expr env e | None -> ())
  | Declare (_, e, b) ->
    expr env e;
    block env b;

and as_expr env = function
  | As_v e
  | Await_as_v (_, e) -> expr env e
  | As_kv (e1, e2)
  | Await_as_kv (_, e1, e2) ->
      expr env e1;
      expr env e2;
      ()

and afield env = function
  | AFvalue e -> expr env e
  | AFkvalue (e1, e2) -> expr env e1; expr env e2;

and block env stl =
  List.iter stl (stmt env)

and expr env (p, e) =
  expr_ env p e

and expr_ env _p = function
  | Collection _
  | Import _
  | Omitted
  | BracedExpr _
  | ParenthesizedExpr _ -> failwith "AST should not contain these nodes after naming"
  | Any
  | Fun_id _
  | Method_id _
  | Smethod_id _
  | Method_caller _
  | This
  | Typename _
  | Lvar _
  | ImmutableVar _
  | Lplaceholder _
  | Dollardollar _
  | PU_identifier _
  | PU_atom _
  | Unsafe_expr _
  | Class_const _ ->
    ()
  | Pipe (_, e1, e2) ->
      expr env e1;
      expr env e2
  | Class_get _  ->
    ()
  | Id _ -> ()
  | Array afl ->
      List.iter afl (afield env);
      ()
  | Darray (_, fdl) ->
      List.iter fdl (field env);
      ()
  | Varray (_, el) ->
      List.iter el (expr env);
      ()
  | ValCollection (_, _, el) ->
      List.iter el (expr env);
      ()
  | KeyValCollection (_, _, fdl) ->
      List.iter fdl (field env);
      ()
  | Clone e -> expr env e; ()
  | Obj_get (e1, e2, _) ->
      let env' = {env with is_array_append_allowed = false} in
      expr env' e1;
      expr env' e2;
      ()
  | Array_get ((p, _), None) when not env.is_array_append_allowed ->
    Errors.reading_from_append p;
    ()
  | Array_get (e, eopt) ->
      let env' = {env with is_array_append_allowed = false} in
      expr env' e;
      maybe expr env' eopt;
      ()
  | Call (_, e, _, el, uel) ->
      expr env e;
      List.iter el (expr env);
      List.iter uel (expr env);
      ()
  | True | False | Int _
  | Float _ | Null | String _ | PrefixedString _ -> ()
  | String2 el ->
      List.iter el (expr env);
      ()
  | Unop (Ast.Uref, e) ->
    expr env e;
  | Unop (_, e) -> expr env e;
  | Yield_break -> ()
  | Special_func func ->
      (match func with
        | Gena e
        | Gen_array_rec e ->
          expr env e
        | Genva el ->
          List.iter el (expr env));
      ()
  | Yield e ->
      afield env e;
      ()
  | Yield_from e ->
      expr env e;
      ()
  | Await e ->
      expr env e;
      ()
  | Suspend e ->
      expr env e;
      ()
  | List el ->
      List.iter el (expr env);
      ()
  | Pair (e1, e2) ->
    expr env e1;
    expr env e2;
    ()
  | Expr_list el ->
      List.iter el (expr env);
      ()
  | Cast (h, e) ->
      hint env h;
      expr env e;
      ()
  | Binop (op, e1, e2) ->
      let lvalue_env = match op with
      | Ast.Eq _ -> { env with is_array_append_allowed = true }
      | _ -> env in
      expr lvalue_env e1;
      expr env e2;
      ()
  | Eif (e1, None, e3) ->
      expr env e1;
      expr env e3;
      ()
  | Eif (e1, Some e2, e3) ->
      expr env e1;
      expr env e2;
      expr env e3;
      ()
  | Assert (AE_assert e) ->
      expr env e;
      ()
  | InstanceOf (e, _) ->
      expr env e;
      ()
  | Is (e, h)
  | As (e, h, _)->
      expr env e;
      hint env h;
      ()
  | New (_, _, el, uel, _) ->
      List.iter el (expr env);
      List.iter uel (expr env);
      ()
  | Record (_, fdl) ->
      List.iter fdl (field env);
      ()
  | Efun (f, _)
  | Lfun (f, _) ->
      let body = Nast.assert_named_body f.f_body in
      func env f body; ()
  | Xml (_, attrl, el) ->
      List.iter attrl (fun attr -> expr env (get_xhp_attr_expr attr));
      List.iter el (expr env);
      ()
  | Callconv (_, e) ->
      expr env e;
      let rec aux = function
        | _, Lvar _ -> true
        | _, Array_get (e1, Some _) -> aux e1
        | _ -> false
      in
      if not (aux e) then Errors.inout_argument_bad_expr (fst e);
      ()
  | Shape fdm ->
      List.iter ~f:(fun (_, v) -> expr env v) fdm

and case env = function
  | Default b -> block env b
  | Case (e, b) ->
      expr env e;
      block env b;
      ()

and catch env (_, _, b) = block env b
and field env (e1, e2) =
  expr env e1;
  expr env e2;
  ()

let typedef tenv t =
  let env = { is_array_append_allowed = false;
              class_name = None; class_kind = None;
              function_name = None;
              (* Since typedefs cannot have constraints we shouldn't check
               * if its type params satisfy the constraints of any tapply it
               * references.
               *)
              typedef_tparams = t.t_tparams;
              tenv = tenv;
              is_reactive = false;
              } in
  maybe hint env t.t_constraint;
  hint env t.t_kind
