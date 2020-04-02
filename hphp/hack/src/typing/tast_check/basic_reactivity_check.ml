open Hh_prelude
open Aast
open Tast
open Typing_defs
module Env = Tast_env
module SN = Naming_special_names
module LMap = Local_id.Map
module TM = Typing_mutability
module TME = Typing_mutability_env
include TM.Shared (Env)

let expr_is_valid_owned_arg (e : expr) : bool =
  match snd e with
  | Call (_, (_, Id (_, id)), _, _, _) ->
    String.equal id SN.Rx.mutable_ || String.equal id SN.Rx.move
  | _ -> false

(* true if expression is valid argument for __Mutable parameter:
  - Rx\move(owned-local)
  - Rx\mutable(call or new)
  - owned-or-borrowed-local *)
let expr_is_valid_borrowed_arg env (e : expr) : bool =
  expr_is_valid_owned_arg e
  ||
  match snd e with
  | Callconv (Ast_defs.Pinout, (_, Lvar (_, id)))
  | Lvar (_, id) ->
    Env.local_is_mutable ~include_borrowed:true env id
  | This
    when Option.equal
           equal_param_mutability
           (Env.function_is_mutable env)
           (Some Param_borrowed_mutable) ->
    true
  | _ -> false

(* basic reactivity checks:
  X no superglobals in reactive context
  X no static property accesses
  X no append calls *)
let rec is_byval_collection_or_string_or_any_type env ty =
  let check ty =
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Toption inner -> is_byval_collection_or_string_or_any_type env inner
    | Tclass ((_, x), _, _) ->
      String.equal x SN.Collections.cVec
      || String.equal x SN.Collections.cDict
      || String.equal x SN.Collections.cKeyset
    | Tarraykind _
    | Ttuple _
    | Tshape _ ->
      true
    | Tprim Tstring
    | Tdynamic
    | Tany _ ->
      true
    | Tunion tyl ->
      List.for_all tyl ~f:(is_byval_collection_or_string_or_any_type env)
    | Tintersection tyl ->
      List.exists tyl ~f:(is_byval_collection_or_string_or_any_type env)
    | Tgeneric _
    | Tnewtype _
    | Tdependent _ ->
      (* FIXME we should probably look at the upper bounds here. *)
      false
    | Terr
    | Tnonnull
    | Tprim _
    | Tobject
    | Tfun _
    | Tvar _
    | Tpu _
    | Tpu_type_access _ ->
      false
  in
  let (_, tl) = Tast_env.get_concrete_supertypes env ty in
  List.for_all tl ~f:check

let rec is_valid_mutable_subscript_expression_target env v =
  match v with
  | ((_, ty), Lvar _) -> is_byval_collection_or_string_or_any_type env ty
  | ((_, ty), Array_get (e, _)) ->
    is_byval_collection_or_string_or_any_type env ty
    && is_valid_mutable_subscript_expression_target env e
  | ((_, ty), Obj_get (e, _, _)) ->
    is_byval_collection_or_string_or_any_type env ty
    && ( is_valid_mutable_subscript_expression_target env e
       || expr_is_valid_borrowed_arg env e )
  | _ -> false

let is_valid_append_target env ty =
  let (_env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tclass ((_, n), _, []) ->
    String.( <> ) n SN.Collections.cVector
    && String.( <> ) n SN.Collections.cSet
    && String.( <> ) n SN.Collections.cMap
  | Tclass ((_, n), _, [_]) ->
    String.( <> ) n SN.Collections.cVector
    && String.( <> ) n SN.Collections.cSet
  | Tclass ((_, n), _, [_; _]) -> String.( <> ) n SN.Collections.cMap
  | _ -> true

let check_assignment_or_unset_target
    ~is_assignment ?append_pos_opt (env : Tast_env.env) (te1 : expr) =
  (* Check for modifying immutable objects *)
  let p = get_position te1 in
  match snd te1 with
  (* Setting mutable locals is okay *)
  | Obj_get (e1, _, _) when expr_is_valid_borrowed_arg env e1 -> ()
  | Array_get (e1, i)
    when is_assignment && not (is_valid_append_target env (get_type e1)) ->
    Errors.nonreactive_indexing
      (Option.is_none i)
      (Option.value append_pos_opt ~default:p)
  | Array_get (e1, _)
    when expr_is_valid_borrowed_arg env e1
         || is_valid_mutable_subscript_expression_target env e1 ->
    ()
  | Class_get _ ->
    (* we already report errors about statics in rx context, no need to do it twice *)
    ()
  | Obj_get _
  | Array_get _ ->
    if is_assignment then
      Errors.obj_set_reactive p
    else
      Errors.invalid_unset_target_rx p
  | _ -> ()

let check_non_rx =
  object
    inherit Tast_visitor.iter as super

    method! on_expr env expr =
      match snd expr with
      | Id (p, n) when SN.Rx.is_enabled n ->
        Errors.rx_enabled_in_non_rx_context p
      | _ -> super#on_expr env expr
  end

let check_escaping_mutable env (pos, x) =
  let mut_env = Env.get_env_mutability env in
  let is_mutable =
    (Local_id.equal x this && Option.is_some (Env.function_is_mutable env))
    ||
    match Local_id.Map.find_opt x mut_env with
    | Some (_, TME.Immutable)
    | None ->
      false
    | _ -> true
  in
  if is_mutable then Errors.escaping_mutable_object pos

type borrowable_args =
  | Arg_this
  | Arg_local of Local_id.S.t
[@@deriving ord]

module Borrowable_args = Caml.Map.Make (struct
  type t = borrowable_args [@@deriving ord]
end)

type args_mut_map = (Pos.t * param_mutability option) Borrowable_args.t

let with_mutable_value env e ~default ~f =
  match snd e with
  (* invoke f only for mutable values *)
  | This when Option.is_some (Env.function_is_mutable env) -> f Arg_this
  | Callconv (Ast_defs.Pinout, (_, Lvar (_, id)))
  | Lvar (_, id)
    when Env.local_is_mutable ~include_borrowed:true env id ->
    f (Arg_local id)
  | _ -> default

let check_borrowing
    (env : Tast_env.env) (p : 'a fun_param) (mut_args : args_mut_map) (e : expr)
    : args_mut_map =
  let mut_to_string m =
    match m with
    | None -> "immutable"
    | Some Param_owned_mutable -> "owned mutable"
    | Some Param_borrowed_mutable -> "mutable"
    | Some Param_maybe_mutable -> "maybe mutable"
  in
  let check key =
    (* only check mutable expressions *)
    match (Borrowable_args.find_opt key mut_args, p.fp_mutability) with
    (* first time we see the parameter - just record it *)
    | (None, _) ->
      Borrowable_args.add key (get_position e, p.fp_mutability) mut_args
    (* error case 1, expression was already passed as mutable parameter *)
    (* error case 2, expression was passed a maybe mutable parameter before and
       now is passed again as mutable *)
    | (Some (pos, (Some Param_owned_mutable as mut)), _)
    | (Some (pos, (Some Param_borrowed_mutable as mut)), _)
    | ( Some (pos, (Some Param_maybe_mutable as mut)),
        Some Param_borrowed_mutable ) ->
      Errors.mutable_expression_as_multiple_mutable_arguments
        (get_position e)
        (mut_to_string p.fp_mutability)
        pos
        (mut_to_string mut);
      mut_args
    | _ -> mut_args
  in
  with_mutable_value env e ~default:mut_args ~f:check

let is_owned_local env e =
  match snd e with
  | Lvar (_, id) -> Env.local_is_mutable ~include_borrowed:false env id
  | _ -> false

let expr_is_maybe_mutable (env : Env.env) (e : expr) : bool =
  match e with
  | (_, Lvar (_, id)) ->
    let mut_env = Env.get_env_mutability env in
    begin
      match LMap.find_opt id mut_env with
      | Some (_, TME.MaybeMutable) -> true
      | _ -> false
    end
  | _ -> false

(* Checks that each parameter that is marked mutable is mutable *)
(* There's no List.iter2_shortest so I'm stuck with this *)
(* Return the remaining expressions to check against the variadic argument *)
let rec check_param_mutability
    (env : Env.env)
    (mut_args : args_mut_map)
    (params : 'a fun_params)
    (el : expr list) : args_mut_map * expr list =
  match (params, el) with
  | ([], _)
  | (_, []) ->
    (mut_args, el)
  | (param :: ps, e :: es) ->
    (* maybe mutable parameters allow anything *)
    ( if
      not
        (Option.equal
           equal_param_mutability
           param.fp_mutability
           (Some Param_maybe_mutable))
    then
      match param.fp_mutability with
      (* maybe-mutable argument value *)
      | _ when expr_is_maybe_mutable env e ->
        Errors.maybe_mutable_argument_mismatch param.fp_pos (get_position e)
      | Some Param_owned_mutable when not (expr_is_valid_owned_arg e) ->
        (*  __OwnedMutable requires argument to be
            - Rx\mutable for all expressions except variable expressions
            - Rx\move for variable expression *)
        let arg_is_owned_local = is_owned_local env e in
        Errors.mutably_owned_argument_mismatch
          ~arg_is_owned_local
          param.fp_pos
          (get_position e)
      | Some Param_borrowed_mutable when not (expr_is_valid_borrowed_arg env e)
        ->
        (* mutable parameter, immutable argument *)
        Errors.mutable_argument_mismatch param.fp_pos (get_position e)
      | None when expr_is_valid_borrowed_arg env e ->
        (* immutable parameter, mutable argument *)
        Errors.immutable_argument_mismatch param.fp_pos (get_position e)
      | _ -> () );
    let mut_args = check_borrowing env param mut_args e in
    (* Check the rest *)
    check_param_mutability env mut_args ps es

let check_mutability_fun_params env mut_args call_ty el =
  match get_node call_ty with
  | Tfun fty ->
    (* exit early if when calling non-reactive function *)
    if equal_reactivity fty.ft_reactive Nonreactive then
      ()
    else
      let params = fty.ft_params in
      let (mut_args, remaining_exprs) =
        check_param_mutability env mut_args params el
      in
      let rec error_on_first_mismatched_argument ~req_mut param es =
        match es with
        | [] -> ()
        | e :: es ->
          if expr_is_maybe_mutable env e then
            Errors.maybe_mutable_argument_mismatch param.fp_pos (get_position e)
          else (
            match req_mut with
            (* non mutable parameter - disallow anythin mutable *)
            | None when expr_is_valid_borrowed_arg env e ->
              Errors.immutable_argument_mismatch param.fp_pos (get_position e)
            | Some Param_borrowed_mutable
              when not (expr_is_valid_borrowed_arg env e) ->
              (* mutably borrowed parameter - complain on immutable or mutably owned parameters.
            mutably owned are not allowed because Rx\move will unset the original local *)
              Errors.mutable_argument_mismatch param.fp_pos (get_position e)
            | Some Param_owned_mutable when not (expr_is_valid_owned_arg e) ->
              (* mutably owned parameter - all arguments need to be passed with Rx\move *)
              Errors.mutably_owned_argument_mismatch
                ~arg_is_owned_local:(is_owned_local env e)
                param.fp_pos
                (get_position e)
            | _ -> error_on_first_mismatched_argument ~req_mut param es
          )
      in
      begin
        match fty.ft_arity with
        (* maybe mutable variadic parameter *)
        | Fvariadic (_, { fp_mutability = Some Param_maybe_mutable; _ }) -> ()
        | Fvariadic (_, ({ fp_mutability = req_mut; _ } as param)) ->
          error_on_first_mismatched_argument ~req_mut param remaining_exprs
        | _ -> ()
      end;
      begin
        match fty.ft_arity with
        | Fvariadic (_, p) ->
          List.fold_left
            ~init:mut_args
            ~f:(check_borrowing env p)
            remaining_exprs
          |> ignore
        | _ -> ()
      end
  | _ -> ()

let enforce_mutable_constructor_call env ctor_fty el =
  let (env, ctor_fty) = Env.expand_type env ctor_fty in
  check_mutability_fun_params env Borrowable_args.empty ctor_fty el

(* Returns true if the expression is valid argument for Rx\mutable *)
let is_valid_rx_mutable_arg env e =
  match snd e with
  | New _
  | KeyValCollection ((Map | ImmMap), _, _)
  | ValCollection ((Vector | ImmVector | Set | ImmSet), _, _)
  | Pair _
  | Clone _
  | Xml _ ->
    true
  | _ -> is_fun_call_returning_mutable env e

(* checks arguments to Rx\mutable function - should be owned mutable value
  excluding locals  *)
let check_rx_mutable_arguments (p : Pos.t) (env : Env.env) (tel : expr list) =
  match tel with
  | [e] when is_valid_rx_mutable_arg env e -> ()
  | _ ->
    (* HH\Rx\mutable function expects single fresh mutably owned value *)
    Errors.invalid_argument_of_rx_mutable_function p

let enforce_mutable_call (env : Env.env) (te : expr) =
  match snd te with
  | Call (_, (_, Id ((_, s) as id)), _, el, _)
  | Call (_, (_, Fun_id ((_, s) as id)), _, el, _)
    when String.( <> ) s SN.Rx.move && String.( <> ) s SN.Rx.freeze ->
    begin
      match Env.get_fun env (snd id) with
      | Some { fe_type; _ } ->
        check_mutability_fun_params env Borrowable_args.empty fe_type el
      | _ -> ()
    end
  (* static methods/lambdas *)
  | Call (_, ((_, fun_ty), Class_const _), _, el, _)
  | Call (_, ((_, fun_ty), Lvar _), _, el, _) ->
    check_mutability_fun_params env Borrowable_args.empty fun_ty el
  (* $x->method() where method is mutable *)
  | Call (_, ((pos, fun_ty), Obj_get (expr, _, _)), _, el, _) ->
    begin
      match get_node fun_ty with
      | Tfun fty ->
        (* do not check receiver mutability when calling non-reactive function *)
        if not (equal_reactivity fty.ft_reactive Nonreactive) then (
          let fpos = get_pos fun_ty in
          (* OwnedMutable annotation is not allowed on methods so
       we ignore it here since it already syntax error *)
          begin
            match get_ft_param_mutable fty with
            (* mutable-or-immutable function - ok *)
            | Some Param_maybe_mutable -> ()
            (* mutable call on mutable-or-immutable value - error *)
            | Some Param_borrowed_mutable when expr_is_maybe_mutable env expr ->
              Errors.invalid_call_on_maybe_mutable ~fun_is_mutable:true pos fpos
            (* non-mutable call on mutable-or-immutable value - error *)
            | None when expr_is_maybe_mutable env expr ->
              Errors.invalid_call_on_maybe_mutable
                ~fun_is_mutable:false
                pos
                fpos
            (* mutable call on immutable value - error *)
            | Some Param_borrowed_mutable
              when not (expr_is_valid_borrowed_arg env expr) ->
              let rx_mutable_hint_pos =
                if is_valid_rx_mutable_arg env expr then
                  Some (get_position expr)
                else
                  None
              in
              Errors.mutable_call_on_immutable fpos pos rx_mutable_hint_pos
            (* immutable call on mutable value - error *)
            | None when expr_is_valid_borrowed_arg env expr ->
              Errors.immutable_call_on_mutable fpos pos
            (* anything else - ok *)
            | _ -> ()
          end;

          (* record mutability for the receiver *)
          let mut_args =
            with_mutable_value
              env
              expr
              ~default:Borrowable_args.empty
              ~f:(fun k ->
                Borrowable_args.singleton
                  k
                  (get_position expr, get_ft_param_mutable fty))
          in
          check_mutability_fun_params env mut_args fun_ty el
        )
      | _ -> ()
    end
  (* TAny, T.Calls that don't have types, etc *)
  | _ -> ()

let check_conditional_operator (when_true : expr) (when_false : expr) =
  match
    ( TM.is_move_or_mutable_call (snd when_true),
      TM.is_move_or_mutable_call (snd when_false) )
  with
  | (true, true)
  | (false, false) ->
    ()
  | (true, _) ->
    Errors.inconsistent_mutability_for_conditional
      (get_position when_true)
      (get_position when_false)
  | (false, _) ->
    Errors.inconsistent_mutability_for_conditional
      (get_position when_false)
      (get_position when_true)

type ctx = {
  reactivity: reactivity;
  allow_awaitable: bool;
  disallow_this: bool;
  is_expr_statement: bool;
  is_locallable_pass: bool;
  allow_mutable_locals: bool;
}

let new_ctx reactivity =
  {
    reactivity;
    allow_awaitable = false;
    disallow_this = false;
    is_expr_statement = false;
    is_locallable_pass = false;
    allow_mutable_locals = true;
  }

let new_ctx_for_is_locallable_pass reactivity =
  { (new_ctx reactivity) with is_locallable_pass = true }

let allow_awaitable ctx =
  if ctx.allow_awaitable then
    ctx
  else
    { ctx with allow_awaitable = true }

let disallow_awaitable ctx =
  if not ctx.allow_awaitable then
    ctx
  else
    { ctx with allow_awaitable = false }

let allow_mutable_locals ctx =
  if ctx.allow_mutable_locals then
    ctx
  else
    { ctx with allow_mutable_locals = true }

let disallow_mutable_locals ctx =
  if not ctx.allow_mutable_locals then
    ctx
  else
    { ctx with allow_mutable_locals = false }

let disallow_this ctx =
  if ctx.disallow_this then
    ctx
  else
    { ctx with disallow_this = true }

let get_reactivity_from_user_attributes user_attributes =
  let module UA = SN.UserAttributes in
  let rec go attrs =
    match attrs with
    | [] -> None
    | { ua_name = (_, n); _ } :: tl ->
      if String.equal n UA.uaPure then
        Some (Pure None)
      else if String.equal n UA.uaReactive then
        Some (Reactive None)
      else if String.equal n UA.uaShallowReactive then
        Some (Shallow None)
      else if String.equal n UA.uaLocalReactive then
        Some (Local None)
      else if String.equal n UA.uaNonRx then
        Some Nonreactive
      else
        go tl
  in
  go user_attributes

let set_expr_statement ctx =
  if ctx.is_expr_statement then
    ctx
  else
    { ctx with is_expr_statement = true }

let set_nested_expr ctx =
  if not ctx.is_expr_statement then
    ctx
  else
    { ctx with is_expr_statement = false }

let set_reactivity ctx reactivity = { ctx with reactivity }

let check =
  object (self)
    inherit [ctx] Tast_visitor.iter_with_state as super

    method handle_body env ctx b =
      if equal_reactivity ctx.reactivity Nonreactive then
        List.iter b.fb_ast (check_non_rx#on_stmt env)
      else
        match b.fb_ast with
        | [(_, If ((_, Id (_, c)), then_stmt, else_stmt))]
          when SN.Rx.is_enabled c ->
          List.iter then_stmt (self#on_stmt (env, ctx));
          List.iter else_stmt ~f:(check_non_rx#on_stmt env)
        | _ -> List.iter b.fb_ast (self#on_stmt (env, ctx))

    method! on_Expr (env, ctx) e = self#on_expr (env, set_expr_statement ctx) e

    method! on_Awaitall (env, ctx) els b =
      let allow_awaitable_s = (env, allow_awaitable ctx) in
      List.iter els ~f:(fun (_, rhs) -> super#on_expr allow_awaitable_s rhs);
      self#on_block (env, ctx) b

    method! on_expr (env, ctx) expr =
      let check_reactivity =
        (not (equal_reactivity ctx.reactivity Nonreactive))
        && not (TypecheckerOptions.unsafe_rx (Env.get_tcopt env))
      in
      if check_reactivity then (
        let is_expr_statement = ctx.is_expr_statement in
        let ctx = set_nested_expr ctx in
        ( if not ctx.allow_awaitable then
          let (_env, ty) = Env.expand_type env (get_type expr) in
          match (get_node ty, expr) with
          | (Tclass ((_, cls), _, _), (_, (Call _ | Pipe _)))
            when String.equal cls SN.Classes.cAwaitable ->
            Errors.non_awaited_awaitable_in_rx (get_position expr)
          | _ -> () );
        let ctx =
          if ctx.allow_mutable_locals then
            ctx
          else (
            begin
              match expr with
              | (_, Lvar (p, id)) ->
                let mut_env = Env.get_env_mutability env in
                begin
                  match LMap.find_opt id mut_env with
                  | Some (_, TME.Immutable)
                  | None ->
                    ()
                  | Some (_, TME.MaybeMutable) ->
                    Errors.reassign_maybe_mutable_var ~in_collection:true p
                  | Some (_, (TME.Borrowed | TME.Mutable)) ->
                    Errors.reassign_mutable_var ~in_collection:true p
                end
              | (_, This) when Option.is_some (Env.function_is_mutable env) ->
                let is_maybe_mutable =
                  Option.equal
                    equal_param_mutability
                    (Env.function_is_mutable env)
                    (Some Param_maybe_mutable)
                in
                Errors.reassign_mutable_this
                  ~in_collection:true
                  ~is_maybe_mutable
                  (get_position expr)
              | _ -> ()
            end;
            allow_mutable_locals ctx
          )
        in
        match expr with
        | (_, Varray (_, els))
        | (_, ValCollection (_, _, els)) ->
          let ctx = disallow_mutable_locals ctx in
          List.iter els ~f:(self#on_expr (env, ctx))
        | (_, Array fs) ->
          let ctx = disallow_mutable_locals ctx in
          List.iter fs ~f:(function
              | AFvalue e -> self#on_expr (env, ctx) e
              | AFkvalue (k, v) ->
                self#on_expr (env, ctx) k;
                self#on_expr (env, ctx) v)
        | (_, Darray (_, els))
        | (_, KeyValCollection (_, _, els)) ->
          let ctx = disallow_mutable_locals ctx in
          List.iter els ~f:(fun (k, v) ->
              self#on_expr (env, ctx) k;
              self#on_expr (env, ctx) v)
        | (_, Shape els) ->
          let ctx = disallow_mutable_locals ctx in
          List.iter els (fun (_, v) -> self#on_expr (env, ctx) v)
        | (_, Await e) -> self#on_expr (env, allow_awaitable ctx) e
        | (_, Pipe (_, l, r)) ->
          self#on_expr (env, ctx) l;
          self#on_expr (env, allow_awaitable ctx) r
        | (_, Eif (cond, e1, e2)) ->
          self#on_expr (env, disallow_awaitable ctx) cond;
          let ctx = allow_awaitable ctx in
          Option.iter e1 ~f:(fun e1 ->
              check_conditional_operator e1 e2;
              self#on_expr (env, ctx) e1);
          self#on_expr (env, ctx) e2
        | (_, Binop (Ast_defs.QuestionQuestion, e1, e2)) ->
          let ctx = allow_awaitable ctx in
          check_conditional_operator e1 e2;
          self#on_expr (env, ctx) e1;
          self#on_expr (env, ctx) e2
        | e ->
          let ctx = disallow_awaitable ctx in
          begin
            match e with
            | (_, Lvar (p, id)) ->
              let local_id = Local_id.to_string id in
              if
                SN.Superglobals.is_superglobal local_id
                || String.equal local_id SN.Superglobals.globals
              then
                Errors.superglobal_in_reactive_context p local_id
            | (_, Class_get _) ->
              Errors.static_property_in_reactive_context (get_position expr);

              (* dive into subnodes *)
              super#on_expr (env, ctx) expr
            | (_, This) when ctx.disallow_this ->
              Errors.escaping_mutable_object (get_position e)
            | ((_, fun_ty), Efun (f, idl))
            | ((_, fun_ty), Lfun (f, idl))
              when is_fun fun_ty ->
              List.iter idl (check_escaping_mutable env);

              let ctx =
                if
                  ctx.disallow_this
                  || Option.is_some (Env.function_is_mutable env)
                then
                  disallow_this ctx
                else
                  ctx
              in
              let env = Tast_env.restore_fun_env env f in
              let (env, ctx) =
                if ctx.is_locallable_pass then
                  match
                    get_reactivity_from_user_attributes f.f_user_attributes
                  with
                  | Some rx ->
                    (Env.set_env_reactive env rx, set_reactivity ctx rx)
                  | None -> (env, ctx)
                else
                  (env, set_reactivity ctx (Env.env_reactivity env))
              in
              self#handle_body env ctx f.f_body
            | (_, Binop (Ast_defs.Eq _, te1, _)) ->
              check_assignment_or_unset_target
                ~is_assignment:true
                ~append_pos_opt:(get_position expr)
                env
                te1;

              (* dive into subnodes *)
              super#on_expr (env, ctx) expr
            | (_, Call (_, (_, Id (_, f)), _, el, None))
              when String.equal f SN.PseudoFunctions.unset ->
              List.iter
                el
                ~f:(check_assignment_or_unset_target ~is_assignment:false env);

              (* dive into subnodes *)
              super#on_expr (env, ctx) expr
            | (_, Call (_, (_, Id (_, f)), _, el, None))
              when String.equal f SN.Rx.mutable_ ->
              check_rx_mutable_arguments (get_position expr) env el;
              super#on_expr (env, ctx) expr
            | (_, Call (_, (_, Id (p, f)), _, _, None))
              when String.equal f SN.SpecialFunctions.echo ->
              Errors.echo_in_reactive_context p;
              super#on_expr (env, ctx) expr
            | (_, Call (_, f, _, _, _)) ->
              enforce_mutable_call env expr;
              ( if not is_expr_statement then
                let (_env, fun_ty) = Env.expand_type env (get_type f) in
                match get_node fun_ty with
                | Tfun fty when get_ft_returns_void_to_rx fty ->
                  Errors.returns_void_to_rx_function_as_non_expression_statement
                    (get_position expr)
                    (get_pos fun_ty)
                | _ -> () );
              super#on_expr (env, ctx) expr
            | (_, New (_, _, el, _, (_, ctor_fty))) ->
              enforce_mutable_constructor_call env ctor_fty el;
              super#on_expr (env, ctx) expr
            | _ ->
              (* dive into subnodes *)
              super#on_expr (env, ctx) expr
          end
      ) else
        super#on_expr (env, ctx) expr
  end

let check_redundant_rx_condition env pos r =
  match r with
  | Pure (Some cond_ty)
  | Reactive (Some cond_ty)
  | Local (Some cond_ty)
  | Shallow (Some cond_ty) ->
    let (env, cond_ty) = Tast_env.localize_with_self env cond_ty in
    if Tast_env.can_subtype env (Tast_env.get_self_ty_exn env) cond_ty then
      Errors.redundant_rx_condition pos
  | _ -> ()

let error_on_attr env attrs attr f =
  let find x xs =
    List.find xs (fun { ua_name; _ } -> String.equal x (snd ua_name))
  in
  if not (TypecheckerOptions.unsafe_rx (Env.get_tcopt env)) then
    match find attr attrs with
    | Some { ua_name = (p, _); _ } -> f p
    | _ -> ()

let check_has_at_most_rx_as_rxfunc_attribute env attrs =
  error_on_attr
    env
    attrs
    SN.UserAttributes.uaAtMostRxAsFunc
    Errors.atmost_rx_as_rxfunc_invalid_location

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def env f =
      let env = Tast_env.restore_fun_env env f in
      let ctx = new_ctx (Env.env_reactivity env) in
      check#handle_body env ctx f.f_body

    method! at_fun_ env f =
      check_has_at_most_rx_as_rxfunc_attribute env f.f_user_attributes

    method! at_method_ env m =
      check_has_at_most_rx_as_rxfunc_attribute env m.m_user_attributes;
      let env = Tast_env.restore_method_env env m in
      check_redundant_rx_condition env (fst m.m_name) (Env.env_reactivity env);
      let ctx = new_ctx (Env.env_reactivity env) in
      check#handle_body env ctx m.m_body

    method! at_class_ env c =
      check_has_at_most_rx_as_rxfunc_attribute env c.c_user_attributes
  end
