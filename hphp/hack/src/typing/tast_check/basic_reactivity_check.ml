open Core_kernel
open Tast
open Typing_defs

module Env = Tast_env
module SN = Naming_special_names
module LMap = Local_id.Map
module TM = Typing_mutability
module TME = Typing_mutability_env

include TM.Shared(Env)

let expr_is_valid_owned_arg (e : expr) : bool =
  match snd e with
  | Call(_, (_, Id (_, id)), _, _, _) -> id = SN.Rx.mutable_ || id = SN.Rx.move
  | _ -> false
(* true if expression is valid argument for __Mutable parameter:
  - Rx\move(owned-local)
  - Rx\mutable(call or new)
  - owned-or-borrowed-local *)
let expr_is_valid_borrowed_arg env (e: expr): bool =
  expr_is_valid_owned_arg e || begin
    match snd e with
    | Callconv (Ast.Pinout, (_, Lvar (_, id)))
    | Lvar (_, id) -> Env.local_is_mutable ~include_borrowed:true env id
    | This when Env.function_is_mutable env -> true
    | _ ->
      false
  end

(* basic reactivity checks:
  X no superglobals in reactive context
  X no static property accesses
  X no append calls *)
let rec is_byval_collection_or_string_or_any_type env ty =
  let check t =
    match t with
    | _, Tclass ((_, x), _, _) ->
      x = SN.Collections.cVec ||
      x = SN.Collections.cDict ||
      x = SN.Collections.cKeyset
    | _, (Tarraykind _ | Ttuple _ | Tshape _)
      -> true
    | _, Tprim Nast.Tstring
    | _, Tany -> true
    | _, Tunresolved tl -> List.for_all tl ~f:(is_byval_collection_or_string_or_any_type env)
    | _ -> false in
  let _, tl = Tast_env.get_concrete_supertypes env ty in
  List.for_all tl ~f:check

let rec is_valid_mutable_subscript_expression_target env v =
  match v with
  | (_, ty), Lvar _ ->
    is_byval_collection_or_string_or_any_type env ty
  | (_, ty), Array_get (e, _) ->
    is_byval_collection_or_string_or_any_type env ty &&
    is_valid_mutable_subscript_expression_target env e
  | (_, ty), Obj_get (e, _, _) ->
    is_byval_collection_or_string_or_any_type env ty &&
    (is_valid_mutable_subscript_expression_target env e || expr_is_valid_borrowed_arg env e)
  | _ -> false

let is_valid_append_target ty =
  match ty with
  | _, Tclass ((_, n), _, []) ->
    n <> SN.Collections.cVector &&
    n <> SN.Collections.cSet &&
    n <> SN.Collections.cMap
  | _, Tclass ((_, n), _, [_]) ->
    n <> SN.Collections.cVector &&
    n <> SN.Collections.cSet
  | _, Tclass ((_, n), _, [_; _]) ->
    n <> SN.Collections.cMap
  | _ -> true

let check_assignment_or_unset_target
  ~is_assignment
  ?append_pos_opt
  (env : Tast_env.env) (te1 : expr)  =
  (* Check for modifying immutable objects *)
  let p = get_position te1 in
  match snd te1 with
   (* Setting mutable locals is okay *)
  | Obj_get (e1, _, _) when expr_is_valid_borrowed_arg env e1 -> ()
  | Array_get (e1, i) when
    is_assignment &&
    not (is_valid_append_target (get_type e1)) ->
    Errors.nonreactive_indexing (i = None) (Option.value append_pos_opt ~default:p);
  | Array_get (e1, _)
    when expr_is_valid_borrowed_arg env e1 ||
         is_valid_mutable_subscript_expression_target env e1 -> ()
  | Class_get _ ->
    (* we already report errors about statics in rx context, no need to do it twice *)
    ()
  | Obj_get _
  | Array_get _ ->
    if is_assignment
    then Errors.obj_set_reactive p
    else Errors.invalid_unset_target_rx p
  | _ -> ()

let check_non_rx = object
  inherit Tast_visitor.iter as super
  method! on_expr env expr =
    match snd expr with
    | Id (p, n) when n = SN.Rx.is_enabled ->
      Errors.rx_enabled_in_non_rx_context p
    | _ -> super#on_expr env expr
end

let check_escaping_mutable env (pos, x) =
  let mut_env = Env.get_env_mutability env in
  let is_mutable =
    (x = this && Env.function_is_mutable env) ||
    begin match Local_id.Map.get x mut_env with
    | Some (_, TME.Immutable) | None -> false
    | _ -> true
    end in
  if is_mutable then Errors.escaping_mutable_object pos

type borrowable_args = Arg_this | Arg_local of Local_id.S.t

module Borrowable_args = Caml.Map.Make(struct
  type t = borrowable_args
  let compare (a: t) (b: t) = compare a b
end)

type args_mut_map = (Pos.t * param_mutability option) Borrowable_args.t

let with_mutable_value env e ~default ~f =
  match snd e with
  (* invoke f only for mutable values *)
  | This when Env.function_is_mutable env ->
    f Arg_this
  | Callconv (Ast.Pinout, (_, Lvar (_, id)))
  | Lvar (_, id) when Env.local_is_mutable ~include_borrowed:true env id ->
    f (Arg_local id)
  | _ -> default

let check_borrowing
  (env : Tast_env.env)
  (p: 'a fun_param)
  (mut_args : args_mut_map)
  (e: expr): args_mut_map =
  let mut_to_string m =
    match m with
    | None -> "immutable"
    | Some Param_owned_mutable -> "owned mutable"
    | Some Param_borrowed_mutable -> "mutable"
    | Some Param_maybe_mutable -> "maybe mutable" in
  let check key =
    (* only check mutable expressions *)
    match Borrowable_args.find_opt key mut_args, p.fp_mutability with
    (* first time we see the parameter - just record it *)
    | None, _ -> Borrowable_args.add key (get_position e, p.fp_mutability) mut_args
    (* error case 1, expression was already passed as mutable parameter *)
    (* error case 2, expression was passed a maybe mutable parameter before and
       now is passed again as mutable *)
    | Some (pos, (Some Param_owned_mutable as mut)), _
    | Some (pos, (Some Param_borrowed_mutable as mut)), _
    | Some (pos, (Some Param_maybe_mutable as mut)), Some Param_borrowed_mutable ->
      Errors.mutable_expression_as_multiple_mutable_arguments
        (get_position e)(mut_to_string p.fp_mutability) pos (mut_to_string mut);
      mut_args
    | _ -> mut_args in

  with_mutable_value env e ~default:mut_args ~f:check

let is_owned_local env e =
  match snd e with
  | Lvar (_, id) -> Env.local_is_mutable ~include_borrowed:false env id
  | _ -> false

let expr_is_maybe_mutable
  (env: Env.env)
  (e: expr): bool =
  match e with
  | _, Lvar (_, id) ->
    let mut_env = Env.get_env_mutability env in
    begin match LMap.get id mut_env with
    | Some (_, TME.MaybeMutable) -> true
    | _ -> false
    end
  | _ -> false

(* Checks that each parameter that is marked mutable is mutable *)
(* There's no List.iter2_shortest so I'm stuck with this *)
(* Return the remaining expressions to check against the variadic argument *)
let rec check_param_mutability (env : Env.env)
  (mut_args : args_mut_map)
  (params : 'a fun_params ) (el : expr list): args_mut_map * expr list  =
  match params, el with
  | [], _
  | _, [] -> mut_args, el
  | param::ps, e::es ->
    (* maybe mutable parameters allow anything *)
    if param.fp_mutability <> Some Param_maybe_mutable
    then begin match param.fp_mutability with
      (* maybe-mutable argument value *)
      | _ when expr_is_maybe_mutable env e ->
        Errors.maybe_mutable_argument_mismatch
          (param.fp_pos)
          (get_position e)
      | Some Param_owned_mutable when
        not (expr_is_valid_owned_arg e) ->
        (*  __OwnedMutable requires argument to be
            - Rx\mutable for all expressions except variable expressions
            - Rx\move for variable expression *)
        let arg_is_owned_local = is_owned_local env e in
        Errors.mutably_owned_argument_mismatch
          ~arg_is_owned_local
          (param.fp_pos)
          (get_position e)
      | Some Param_borrowed_mutable when
        not (expr_is_valid_borrowed_arg env e) ->
      (* mutable parameter, immutable argument *)
        Errors.mutable_argument_mismatch
          (param.fp_pos)
          (get_position e)
      | None when expr_is_valid_borrowed_arg env e ->
      (* immutable parameter, mutable argument *)
        Errors.immutable_argument_mismatch
          (param.fp_pos)
          (get_position e)
      | _ -> ()
    end;
    let mut_args = check_borrowing env param mut_args e in
    (* Check the rest *)
    check_param_mutability env mut_args ps es

let check_mutability_fun_params env mut_args fty el =
  (* exit early if when calling non-reactive function *)
  if fty.ft_reactive = Nonreactive then ()
  else
  let params = fty.ft_params in
  let mut_args, remaining_exprs = check_param_mutability env mut_args params el in
  let rec error_on_first_mismatched_argument ~req_mut param es =
    match es with
    | [] -> ()
    | e::es ->
      if expr_is_maybe_mutable env e then
        Errors.maybe_mutable_argument_mismatch (param.fp_pos) (get_position e)
      else begin
        match req_mut with
        (* non mutable parameter - disallow anythin mutable *)
        | None when expr_is_valid_borrowed_arg env e ->
          Errors.immutable_argument_mismatch (param.fp_pos) (get_position e)
        | Some Param_borrowed_mutable when not (expr_is_valid_borrowed_arg env e) ->
        (* mutably borrowed parameter - complain on immutable or mutably owned parameters.
          mutably owned are not allowed because Rx\move will unset the original local *)
          Errors.mutable_argument_mismatch (param.fp_pos) (get_position e)
        | Some Param_owned_mutable when not (expr_is_valid_owned_arg e) ->
        (* mutably owned parameter - all arguments need to be passed with Rx\move *)
          Errors.mutably_owned_argument_mismatch
            ~arg_is_owned_local:(is_owned_local env e)
            (param.fp_pos)
            (get_position e)
        | _ ->
          error_on_first_mismatched_argument ~req_mut param es
      end
  in
  begin match fty.ft_arity with
  (* maybe mutable variadic parameter *)
  | Fvariadic (_, ({ fp_mutability = Some Param_maybe_mutable; _ })) ->
    ()
  | Fvariadic (_, ({ fp_mutability = req_mut; _ } as param)) ->
    error_on_first_mismatched_argument ~req_mut param remaining_exprs
  | _ -> ()
  end;
  begin match fty.ft_arity with
  | Fvariadic (_, p) ->
    List.fold_left ~init:mut_args ~f:(check_borrowing env p) remaining_exprs
    |> ignore
  | _ -> ()
  end

let enforce_mutable_constructor_call env ctor_fty el =
  match ctor_fty with
  | _, Tfun fty ->
    check_mutability_fun_params env Borrowable_args.empty fty el
  | _ -> ()

(* Returns true if the expression is valid argument for Rx\mutable *)
let is_valid_rx_mutable_arg env e =
  match snd e with
  | New _
  | KeyValCollection ((`Map | `ImmMap), _)
  | ValCollection ((`Vector | `ImmVector | `Set | `ImmSet), _)
  | Pair _
  | Xml _ ->
    true
  | _ -> is_fun_call_returning_mutable env e

(* checks arguments to Rx\mutable function - should be owned mutable value
  excluding locals  *)
let check_rx_mutable_arguments
  (p : Pos.t) (env : Env.env) (tel : expr list) =
  match tel with
  | [e] when is_valid_rx_mutable_arg env e -> ()
  | _ ->
    (* HH\Rx\mutable function expects single fresh mutably owned value *)
    Errors.invalid_argument_of_rx_mutable_function p
let enforce_mutable_call (env : Env.env) (te : expr) =
  match snd te with
  | Call (_, (_, Id (_, s as id)), _, el, _)
  | Call (_, (_, Fun_id (_, s as id)), _, el, _)
    when s <> SN.Rx.move && s <> SN.Rx.freeze ->
    begin match Env.get_fun env (snd id) with
    | Some fty ->
      check_mutability_fun_params env Borrowable_args.empty fty el
    | None -> ()
    end
  (* static methods/lambdas *)
  | Call (_, ((_, (_, Tfun fty)), Class_const _), _, el, _)
  | Call (_, ((_, (_, Tfun fty)), Lvar _), _, el, _) ->
    check_mutability_fun_params env Borrowable_args.empty fty el
  (* $x->method() where method is mutable *)
  | Call (_, ((pos, (r, Tfun fty)), Obj_get (expr, _, _)), _, el, _) ->
    (* do not check receiver mutability when calling non-reactive function *)
    if fty.ft_reactive <> Nonreactive
    then begin
    let fpos = Reason.to_pos r in
    (* OwnedMutable annotation is not allowed on methods so
       we ignore it here since it already syntax error *)
    begin match fty.ft_mutability with
    (* mutable-or-immutable function - ok *)
    | Some Param_maybe_mutable -> ()
    (* mutable call on mutable-or-immutable value - error *)
    | Some Param_borrowed_mutable when expr_is_maybe_mutable env expr ->
      Errors.invalid_call_on_maybe_mutable ~fun_is_mutable:true pos fpos
    (* non-mutable call on mutable-or-immutable value - error *)
    | None when expr_is_maybe_mutable env expr ->
      Errors.invalid_call_on_maybe_mutable ~fun_is_mutable:false pos fpos
    (* mutable call on immutable value - error *)
    | Some Param_borrowed_mutable when not (expr_is_valid_borrowed_arg env expr) ->
      let rx_mutable_hint_pos =
        if is_valid_rx_mutable_arg env expr
        then Some (get_position expr)
        else None in
      Errors.mutable_call_on_immutable fpos pos rx_mutable_hint_pos
    (* immutable call on mutable value - error *)
    | None when expr_is_valid_borrowed_arg env expr ->
      Errors.immutable_call_on_mutable fpos pos
    (* anything else - ok *)
    | _ -> ()
    end;
    (* record mutability for the receiver *)
    let mut_args =
      with_mutable_value env expr ~default:Borrowable_args.empty
      ~f:(fun k -> Borrowable_args.singleton k (get_position expr, fty.ft_mutability)) in
    check_mutability_fun_params env mut_args fty el
    end
  (* TAny, T.Calls that don't have types, etc *)
  | _ -> ()

let check_conditional_operator
  (when_true : expr)
  (when_false : expr) =
  match TM.is_move_or_mutable_call (snd when_true),
        TM.is_move_or_mutable_call (snd when_false) with
  | true, true | false, false -> ()
  | true, _ ->
    Errors.inconsistent_mutability_for_conditional
      (get_position when_true)
      (get_position when_false)
  | false, _ ->
    Errors.inconsistent_mutability_for_conditional
      (get_position when_false)
      (get_position when_true)

let disallow_static_or_global ~is_static el =
  let rec get_name = function
  (* name *)
  | _, Lvar (_, id) -> Local_id.to_string id
  (* name = initializer *)
  | _, Binop (_, lhs, _) -> get_name lhs
  | _ -> "_" in
  (List.hd el) |> Option.iter ~f:(fun n ->
    let p = get_position n in
    let name = get_name n in
    if is_static then Errors.static_in_reactive_context p name
    else Errors.global_in_reactive_context p name)

type ctx = {
  reactivity: reactivity;
  allow_awaitable: bool;
  disallow_this: bool;
  is_expr_statement: bool;
  is_locallable_pass: bool;
}

let new_ctx reactivity = {
  reactivity; allow_awaitable = false; disallow_this = false;
  is_expr_statement=false; is_locallable_pass = false;
}

let new_ctx_for_is_locallable_pass reactivity =
  { (new_ctx reactivity) with is_locallable_pass = true }

let allow_awaitable ctx =
  if ctx.allow_awaitable then ctx
  else { ctx with allow_awaitable = true }

let disallow_awaitable ctx =
  if not ctx.allow_awaitable then ctx
  else { ctx with allow_awaitable = false }

let disallow_this ctx =
  if ctx.disallow_this then ctx
  else { ctx with disallow_this = true }

let get_reactivity_from_user_attributes user_attributes =
  let module UA = SN.UserAttributes in
  let rec go attrs =
    match attrs with
    | [] -> None
    | { ua_name = (_, n); _ } :: tl ->
      if n = UA.uaReactive then Some (Reactive None)
      else if n = UA.uaShallowReactive then Some (Shallow None)
      else if n = UA.uaLocalReactive then Some (Local None)
      else if n = UA.uaNonRx then Some Nonreactive
      else go tl in
  go user_attributes

let set_expr_statement ctx =
  if ctx.is_expr_statement then ctx
  else { ctx with is_expr_statement = true }

let set_nested_expr ctx =
  if not ctx.is_expr_statement then ctx
  else { ctx with is_expr_statement = false }

let set_reactivity ctx reactivity =
  { ctx with reactivity }
let check = object(self)
  inherit [ctx] Tast_visitor.iter_with_state as super

  method handle_body env ctx b =
    if ctx.reactivity = Nonreactive
    then begin match b with
    | NamedBody { fnb_nast; _ } -> List.iter fnb_nast (check_non_rx#on_stmt env)
    | _ -> ()
    end
    else begin
      match b with
      | NamedBody {
          fnb_nast = [If ((_, Id (_, c)), then_stmt, else_stmt ) ]; _
        } when c = SN.Rx.is_enabled ->
        List.iter then_stmt (self#on_stmt (env, ctx));
        List.iter else_stmt ~f:(check_non_rx#on_stmt env)
      | NamedBody b ->
        List.iter b.fnb_nast (self#on_stmt (env, ctx))
      | _ -> ();
    end

  method! on_Expr (env, ctx) e =
    self#on_expr (env, set_expr_statement ctx) e

  method! on_Global_var s el =
    disallow_static_or_global ~is_static:false el;
    super#on_Global_var s el

  method! on_Static_var s el =
    disallow_static_or_global ~is_static:true el;
    super#on_Static_var s el

  method! on_Awaitall ((env, ctx) as s) _ els =
    let allow_awaitable_s = (env, allow_awaitable ctx) in
    List.iter els ~f:(fun (lhs, rhs) ->
      Option.iter lhs ~f:(super#on_expr s);
      super#on_expr allow_awaitable_s rhs
    )

  method! on_expr (env, ctx) expr =
    let check_reactivity =
      ctx.reactivity <> Nonreactive &&
      not (TypecheckerOptions.unsafe_rx (Env.get_tcopt env)) in
    if check_reactivity
    then begin
    let is_expr_statement = ctx.is_expr_statement in
    let ctx = set_nested_expr ctx in
    if not ctx.allow_awaitable
    then begin match get_type expr, expr with
    | (_, Tclass ((_, cls), _, _)), (_, (Call _ | Special_func _ | Pipe _))
      when cls = SN.Classes.cAwaitable ->
      Errors.non_awaited_awaitable_in_rx (get_position expr);
    | _ -> ()
    end;
    match expr with
    | _, Await e ->
      super#on_expr (env, allow_awaitable ctx) e
    | _, Pipe (_, l, r) ->
      super#on_expr (env, ctx) l;
      super#on_expr (env, allow_awaitable ctx) r;
    | _, Special_func (Genva args) ->
      let ctx = allow_awaitable ctx in
      List.iter args ~f:(super#on_expr (env, ctx));
    | _, Eif (cond, e1, e2) ->
      super#on_expr (env, disallow_awaitable ctx) cond;
      let ctx = allow_awaitable ctx in
      Option.iter e1 ~f:(fun e1 ->
        check_conditional_operator e1 e2;
        super#on_expr (env, ctx) e1
      );
      super#on_expr (env, ctx) e2
    | _, Binop (Ast.QuestionQuestion, e1, e2) ->
      let ctx = allow_awaitable ctx in
      check_conditional_operator e1 e2;
      super#on_expr (env, ctx) e1;
      super#on_expr (env, ctx) e2
    | e ->
      let ctx = disallow_awaitable ctx in
      begin match e with
      | _, Lvar (p, id) ->
        let local_id = Local_id.to_string id in
        if SN.Superglobals.is_superglobal local_id
        then Errors.superglobal_in_reactive_context p local_id;
      | _, Class_get _ ->
        Errors.static_property_in_reactive_context (get_position expr);
        (* dive into subnodes *)
        super#on_expr (env, ctx) expr
      | _, This when ctx.disallow_this ->
        Errors.escaping_mutable_object (get_position e)
      | (_, (_, Tfun fty)), Efun (f, idl) ->
        if fty.ft_reactive <> Nonreactive
        then List.iter idl (check_escaping_mutable env);

        let ctx =
          if ctx.disallow_this || Env.function_is_mutable env
          then disallow_this ctx
          else ctx in

        let env = Tast_env.restore_fun_env env f in
        let env, ctx =
          if ctx.is_locallable_pass
          then begin
            match get_reactivity_from_user_attributes f.f_user_attributes with
            | Some rx -> (Env.set_env_reactive env rx), (set_reactivity ctx rx)
            | None -> env, ctx
          end
          else env, set_reactivity ctx (Env.env_reactivity env)
        in
        self#handle_body env ctx f.f_body;

      | _, Binop (Ast.Eq _, te1, _) ->
        check_assignment_or_unset_target
          ~is_assignment:true
          ~append_pos_opt:(get_position expr)
          env te1;
        (* dive into subnodes *)
        super#on_expr (env, ctx) expr
      | _, Call (_, (_, Id (_, f)), _, el, [])
        when f = SN.PseudoFunctions.unset->
        List.iter el
          ~f:(check_assignment_or_unset_target ~is_assignment:false env);
        (* dive into subnodes *)
        super#on_expr (env, ctx) expr
      | _, Call (_, (_, Id (_, f)), _, el, [])
        when f = SN.Rx.mutable_ ->
        check_rx_mutable_arguments (get_position expr) env el;
        super#on_expr (env, ctx) expr
      | _, Call (_, (_, Id (p, f)), _, _, [])
        when f = SN.SpecialFunctions.echo ->
        Errors.echo_in_reactive_context p;
        super#on_expr (env, ctx) expr
      | _, Call (_, f, _, _, _) ->
        enforce_mutable_call env expr;
        if not is_expr_statement
        then begin match get_type f with
        | _, Tfun fty when fty.ft_returns_void_to_rx ->
          Errors.returns_void_to_rx_function_as_non_expression_statement
            (get_position expr)
            fty.ft_pos
        | _ -> ()
        end;
        super#on_expr (env, ctx) expr
      | _, New (_, el, _, (_, ctor_fty)) ->
        enforce_mutable_constructor_call env ctor_fty el;
        super#on_expr (env, ctx) expr
      | _ ->
        (* dive into subnodes *)
        super#on_expr (env, ctx) expr
      end
    end
    else super#on_expr (env, ctx) expr
end

let check_redundant_rx_condition env pos r =
  match r with
  | Reactive (Some cond_ty) | Local (Some cond_ty) | Shallow (Some cond_ty) ->
    let env, cond_ty = Tast_env.localize_with_self env cond_ty in
    let _, is_subtype =
      Tast_env.subtype env (Tast_env.get_self_exn env) cond_ty in
    if is_subtype
    then Errors.redundant_rx_condition pos
  | _ -> ()

let handler = object
  inherit Tast_visitor.handler_base
  method! at_fun_def env f =
    let env = Tast_env.restore_fun_env env f in
    let ctx = new_ctx (Env.env_reactivity env) in
    check#handle_body env ctx f.f_body

  method! at_method_ env m =
    let env = Tast_env.restore_method_env env m in
    check_redundant_rx_condition env (fst m.m_name) (Env.env_reactivity env);
    let ctx = new_ctx (Env.env_reactivity env) in
    check#handle_body env ctx m.m_body
end
