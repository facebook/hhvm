(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module Map = Local_id.Map
module Set = Local_id.Set

type typed_local_env = {
  locals: Pos.t Map.t;
  declared_ids: Set.t;
  assigned_ids: Set.t;
}

let empty =
  { locals = Map.empty; declared_ids = Set.empty; assigned_ids = Set.empty }

let add_local env id pos = { env with locals = Map.add id pos env.locals }

let add_declared_id env id =
  { env with declared_ids = Set.add id env.declared_ids }

let add_assigned_id env id =
  { env with assigned_ids = Set.add id env.assigned_ids }

let get_local env id = Map.find_opt id env.locals

let get_local_pos env id =
  match get_local env id with
  | Some pos -> pos
  | None -> Pos.none

let restrict_env env uses =
  List.fold_left uses ~init:empty ~f:(fun new_env (_, (_, id)) ->
      let new_env =
        match get_local env id with
        | None -> new_env
        | Some pos -> add_local new_env id pos
      in
      let new_env =
        if Set.mem id env.declared_ids then
          add_declared_id new_env id
        else
          new_env
      in
      if Set.mem id env.assigned_ids then
        add_assigned_id new_env id
      else
        new_env)

let extend_error_map error_map env1 env2 clashes =
  Set.fold
    (fun id error_map ->
      Map.add id (get_local_pos env1 id, get_local_pos env2 id) error_map)
    clashes
    error_map

let join2 error_map env1 env2 =
  let two_declare = Set.inter env1.assigned_ids env2.declared_ids in
  let error_map = extend_error_map error_map env2 env1 two_declare in
  let one_declare = Set.inter env1.declared_ids env2.assigned_ids in
  let error_map = extend_error_map error_map env1 env2 one_declare in
  let both_declare = Set.inter env1.declared_ids env2.declared_ids in
  let error_map = extend_error_map error_map env1 env2 both_declare in
  let locals =
    Set.fold
      (fun id locals -> Map.add id (Map.find id env1.locals) locals)
      (Set.union
         env1.declared_ids
         (Set.diff env1.assigned_ids env2.declared_ids))
      env2.locals
  in
  let declared_ids = Set.union env1.declared_ids env2.declared_ids in
  let assigned_ids =
    Set.diff
      (Set.union env1.assigned_ids env2.assigned_ids)
      (Set.union one_declare two_declare)
  in
  ({ locals; assigned_ids; declared_ids }, error_map)

let join envs before_env custom_err_config =
  let (env, error_map) =
    List.fold_right
      envs
      ~init:({ empty with locals = before_env.locals }, Map.empty)
      ~f:(fun env1 (env2, error_map) -> join2 error_map env1 env2)
  in
  Map.iter
    (fun id (id_pos, def_pos) ->
      Diagnostics.add_diagnostic
        Naming_error_utils.(
          to_user_diagnostic
            (Naming_error.Illegal_typed_local
               { join = true; id_pos; id_name = Local_id.to_string id; def_pos })
            custom_err_config))
    error_map;
  {
    env with
    declared_ids = Set.union env.declared_ids before_env.declared_ids;
    assigned_ids = Set.union env.assigned_ids before_env.assigned_ids;
  }

let rec check_assign_lval on_expr env (((), pos, expr_) as expr) =
  on_expr env expr;
  match expr_ with
  | Lvar lid ->
    let name = snd lid in
    (match get_local env name with
    | None ->
      let env = add_local env name pos in
      add_assigned_id env name
    | Some _ -> env)
  | List exprs
  | Tuple exprs ->
    List.fold_left exprs ~init:env ~f:(check_assign_lval on_expr)
  | Shape fields ->
    List.fold_left fields ~init:env ~f:(fun env (_, expr) ->
        check_assign_lval on_expr env expr)
  | _ -> env

let check_assign_expr on_expr env (((), _pos, expr_) as expr) =
  on_expr env expr;
  match expr_ with
  | Assign (lhs, _, _) -> check_assign_lval on_expr env lhs
  | _ -> env

let rec check_stmt on_expr env (id_pos, stmt_) custom_err_config =
  let check_block = check_block on_expr in
  match stmt_ with
  | Declare_local (id, _hint, init) ->
    Option.iter ~f:(on_expr env) init;
    let name = snd id in
    (match get_local env name with
    | None ->
      let env = add_local env name id_pos in
      let env = add_declared_id env name in
      env
    | Some def_pos ->
      Diagnostics.add_diagnostic
        Naming_error_utils.(
          to_user_diagnostic
            Naming_error.(
              Illegal_typed_local
                {
                  join = false;
                  id_pos;
                  id_name = Local_id.to_string name;
                  def_pos;
                })
            custom_err_config);
      env)
  | Expr expr -> check_assign_expr on_expr env expr
  | If (cond, then_block, else_block) ->
    on_expr env cond;
    let new_env = { empty with locals = env.locals } in
    let then_env = check_block new_env then_block custom_err_config in
    let else_env = check_block new_env else_block custom_err_config in
    join [then_env; else_env] env custom_err_config
  | For (init_exprs, cond, (_, _, update_exprs), body) ->
    let env =
      List.fold_left init_exprs ~init:env ~f:(check_assign_expr on_expr)
    in
    Option.iter ~f:(fun (_, _, e) -> on_expr env e) cond;
    let env = check_block env body custom_err_config in
    List.fold_left update_exprs ~init:env ~f:(fun x ->
        check_assign_expr on_expr x)
  | Switch (expr, cases, default) ->
    on_expr env expr;
    let new_env = { empty with locals = env.locals } in
    let envs =
      List.map
        ~f:(fun x -> check_case on_expr new_env x custom_err_config)
        cases
    in
    let default_env =
      match default with
      | None -> empty
      | Some (_, block) -> check_block new_env block custom_err_config
    in
    join (envs @ [default_env]) env custom_err_config
  | Match { sm_expr; sm_arms } ->
    on_expr env sm_expr;
    let new_env = { empty with locals = env.locals } in
    let envs =
      List.map
        ~f:(fun x -> check_stmt_match_arm on_expr new_env x custom_err_config)
        sm_arms
    in
    join envs env custom_err_config
  | Do (block, (_, _, cond)) ->
    let env = check_block env block custom_err_config in
    on_expr env cond;
    env
  | While ((_, _, cond), block) ->
    on_expr env cond;
    check_block env block custom_err_config
  | Awaitall (inits, block) ->
    List.iter ~f:(fun (_, e) -> on_expr env e) inits;
    check_block env block custom_err_config
  | Block (_, block) -> check_block env block custom_err_config
  | Concurrent block -> check_block env block custom_err_config
  | Return (Some expr)
  | Throw expr ->
    on_expr env expr;
    env
  | Foreach (expr, as_expr, block) ->
    on_expr env expr;
    let env =
      match as_expr with
      | As_v e
      | Await_as_v (_, e) ->
        check_assign_lval on_expr env e
      | As_kv (e1, e2)
      | Await_as_kv (_, e1, e2) ->
        let env = check_assign_lval on_expr env e1 in
        check_assign_lval on_expr env e2
    in
    check_block env block custom_err_config
  | Try (try_block, catches, finally_block) ->
    let env = check_block env try_block custom_err_config in
    let env = check_catches on_expr env catches custom_err_config in
    let env = check_block env finally_block custom_err_config in
    env
  | Using
      {
        us_is_block_scoped = _;
        us_has_await = _;
        us_exprs = (_, exprs);
        us_block = block;
      } ->
    let env = List.fold_left exprs ~init:env ~f:(check_assign_expr on_expr) in
    check_block env block custom_err_config
  | Return None
  | Fallthrough
  | Break
  | Continue
  | Yield_break
  | Noop
  | Markup _ ->
    env

and check_block on_expr env block custom_err_config =
  match block with
  | [] -> env
  | stmt :: stmts ->
    let env = check_stmt on_expr env stmt custom_err_config in
    let env = check_block on_expr env stmts custom_err_config in
    env

and check_catch on_expr env (_cn, (pos, name), block) =
  let env =
    match get_local env name with
    | None ->
      let env = add_local env name pos in
      add_assigned_id env name
    | Some _ -> env
  in
  check_block on_expr env block

and check_catches on_expr env catches custom_err_config =
  let new_env = { empty with locals = env.locals } in
  let envs =
    List.map
      ~f:(fun x -> check_catch on_expr new_env x custom_err_config)
      catches
  in
  join envs env custom_err_config

and check_case on_expr env (expr, block) custom_err_config =
  on_expr env expr;
  check_block on_expr env block custom_err_config

and check_stmt_match_arm on_expr env { sma_pat = _; sma_body } custom_err_config
    =
  check_block on_expr env sma_body custom_err_config

let ignorefn f x y = ignore (f x y)

let visitor custom_error_config =
  object (s)
    inherit [_] Aast.endo as super

    method on_'ex (_ : typed_local_env) () = ()

    method on_'en (_ : typed_local_env) () = ()

    method! on_stmt env stmt =
      let _env =
        check_stmt (ignorefn super#on_expr) env stmt custom_error_config
      in
      stmt

    method! on_block env block =
      let _env =
        check_block (ignorefn super#on_expr) env block custom_error_config
      in
      block

    method! on_fun_ env fun_ =
      let env =
        List.fold_left fun_.f_params ~init:env ~f:(fun env param ->
            let local_id = Local_id.make_unscoped param.param_name in
            let env = add_local env local_id param.param_pos in
            add_assigned_id env local_id)
      in
      let _env =
        check_block
          (ignorefn super#on_expr)
          env
          fun_.f_body.fb_ast
          custom_error_config
      in
      fun_

    method! on_efun env efun =
      let env = restrict_env env efun.ef_use in
      let _fun_ = s#on_fun_ env efun.ef_fun in
      efun

    method! on_fun_def _env fun_def =
      let _fun_ = s#on_fun_ empty fun_def.fd_fun in
      fun_def

    method! on_method_ _env method_ =
      let env =
        List.fold_left method_.m_params ~init:empty ~f:(fun env param ->
            let local_id = Local_id.make_unscoped param.param_name in
            let env = add_local env local_id param.param_pos in
            add_assigned_id env local_id)
      in
      let _env =
        check_block
          (ignorefn super#on_expr)
          env
          method_.m_body.fb_ast
          custom_error_config
      in
      method_
  end

let elab_fun_def elem ~custom_err_config =
  let visitor = visitor custom_err_config in
  visitor#on_fun_def empty elem

let elab_class elem ~custom_err_config =
  let visitor = visitor custom_err_config in
  visitor#on_class_ empty elem

let elab_program elem ~custom_err_config =
  let visitor = visitor custom_err_config in
  visitor#on_program empty elem

let elab_stmt elem ~custom_err_config =
  let visitor = visitor custom_err_config in
  visitor#on_stmt empty elem
