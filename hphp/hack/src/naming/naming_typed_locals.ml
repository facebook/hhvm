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

let find_clashes env other_env clashes =
  Set.iter
    (fun id ->
      let id_pos = get_local_pos other_env id in
      let def_pos = get_local_pos env id in
      Errors.add_error
        Naming_error.(
          to_user_error
            (Illegal_typed_local
               { join = true; id_pos; id_name = Local_id.to_string id; def_pos })))
    clashes

let join env other_env declared_above assigned_above =
  let clashes = Set.inter env.declared_ids other_env.declared_ids in
  find_clashes env other_env clashes;
  let clashes = Set.inter env.declared_ids other_env.assigned_ids in
  find_clashes other_env env clashes;
  let clashes = Set.inter env.assigned_ids other_env.declared_ids in
  find_clashes env other_env clashes;
  let other_ids = Set.union other_env.declared_ids other_env.assigned_ids in
  let locals =
    Set.fold
      (fun id locals ->
        if Map.mem id env.locals then
          locals
        else
          match get_local other_env id with
          | None -> locals
          | Some pos -> Map.add id pos locals)
      other_ids
      env.locals
  in
  let declared_ids =
    Set.union env.declared_ids (Set.union other_env.declared_ids declared_above)
  in
  let assigned_ids =
    Set.union env.assigned_ids (Set.union other_env.assigned_ids assigned_above)
  in
  { locals; declared_ids; assigned_ids }

let check_assign_expr env ((), _pos, expr_) =
  match expr_ with
  | Binop { bop = Ast_defs.Eq _; lhs = ((), pos, Lvar lid); rhs = _ } ->
    let name = snd lid in
    let env = add_local env name pos in
    add_assigned_id env name
  | _ -> env

let rec check_stmt env (id_pos, stmt_) =
  match stmt_ with
  | Declare_local (id, _hint, _init) ->
    let name = snd id in
    (match get_local env name with
    | None ->
      let env = add_local env name id_pos in
      let env = add_declared_id env name in
      env
    | Some def_pos ->
      Errors.add_error
        Naming_error.(
          to_user_error
            (Illegal_typed_local
               {
                 join = false;
                 id_pos;
                 id_name = Local_id.to_string name;
                 def_pos;
               }));
      env)
  | Expr expr -> check_assign_expr env expr
  (* TODO: make sure _cond is visited in case it has lambdas in it *)
  | If (_cond, then_block, else_block) ->
    let new_env = { empty with locals = env.locals } in
    let then_env = check_block new_env then_block in
    let else_env = check_block new_env else_block in
    join then_env else_env env.declared_ids env.assigned_ids
  (* TODO: make sure _cond is visited in case it has lambdas in it *)
  | For (init_exprs, _cond, update_exprs, body) ->
    let env = List.fold_left init_exprs ~init:env ~f:check_assign_expr in
    let env = check_block env body in
    List.fold_left update_exprs ~init:env ~f:check_assign_expr
  (* TODO: make sure _cond is visited in case it has lambdas in it *)
  | While (_cond, block) -> check_block env block
  | _ -> env

and check_block env block =
  match block with
  | [] -> env
  | stmt :: stmts ->
    let env = check_stmt env stmt in
    let env = check_block env stmts in
    env

let visitor =
  object
    inherit [_] Aast.endo as _super

    method on_'ex (_ : typed_local_env) () = ()

    method on_'en (_ : typed_local_env) () = ()

    method! on_stmt env stmt =
      let _env = check_stmt env stmt in
      stmt

    method! on_block env block =
      let _env = check_block env block in
      block

    method! on_fun_def _env fun_def =
      let env =
        List.fold_left fun_def.fd_fun.f_params ~init:empty ~f:(fun env param ->
            let local_id = Local_id.make_unscoped param.param_name in
            let env = add_local env local_id param.param_pos in
            add_assigned_id env local_id)
      in
      let _env = check_block env fun_def.fd_fun.f_body.fb_ast in
      fun_def

    method! on_method_ _env method_ =
      let env =
        List.fold_left method_.m_params ~init:empty ~f:(fun env param ->
            let local_id = Local_id.make_unscoped param.param_name in
            let env = add_local env local_id param.param_pos in
            add_assigned_id env local_id)
      in
      let _env = check_block env method_.m_body.fb_ast in
      method_
  end

let elab_fun_def elem = visitor#on_fun_def empty elem

let elab_class elem = visitor#on_class_ empty elem

let elab_program elem = visitor#on_program empty elem
