(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Refactor_sd_types
module Cont = Typing_continuations
module A = Aast
module T = Tast
module Env = Refactor_sd_env
module Utils = Aast_names_utils

let failwithpos pos msg =
  raise @@ Refactor_sd_exn (Format.asprintf "%a: %s" Pos.pp pos msg)

let redirect (env : env) (entity_ : entity_) : env * entity_ =
  let var = Env.fresh_var () in
  let env = Env.add_constraint env (Subset (entity_, var)) in
  (env, var)

let assign (env : env) ((_, pos, lval) : T.expr) (rhs : entity) : env =
  match lval with
  | A.Lvar (_, lid) -> Env.set_local env lid rhs
  | A.Array_get ((_vc_type, _, A.Lvar (_, lid)), _) ->
    let entity = Env.get_local env lid in
    begin
      match entity with
      | Some entity_ ->
        let (env, var) = redirect env entity_ in
        let env =
          match rhs with
          | Some rhs -> Env.add_constraint env (Subset (rhs, var))
          | _ -> env
        in
        Env.set_local env lid (Some var)
      | None ->
        (* We might end up here as a result of deadcode, such as a dictionary
           assignment after an unconditional break in a loop. In this
           situation, it is not meaningful to report a candidate. *)
        env
    end
  | _ -> failwithpos pos ("Unsupported lvalue: " ^ Utils.expr_name lval)

let join (env : env) (then_entity : entity) (else_entity : entity) =
  (* Create a join point entity. It is pretty much Option.marge except that
     that function doesn't allow threading state (`env`) through *)
  match (then_entity, else_entity) with
  | (Some then_entity_, Some else_entity_) ->
    let var = Env.fresh_var () in
    let env = Env.add_constraint env @@ Subset (then_entity_, var) in
    let env = Env.add_constraint env @@ Subset (else_entity_, var) in
    (env, Some var)
  | (None, Some _) -> (env, else_entity)
  | (_, _) -> (env, then_entity)

let rec expr_ (upcasted_id : string) (env : env) ((_ty, pos, e) : T.expr) :
    env * entity =
  match e with
  | A.Int _
  | A.Float _
  | A.String _
  | A.Null
  | A.True
  | A.False ->
    (env, None)
  | A.Lvar (_, lid) ->
    let entity = Env.get_local env lid in
    (env, entity)
  | A.Unop
      ( ( Ast_defs.Utild | Ast_defs.Unot | Ast_defs.Uplus | Ast_defs.Uminus
        | Ast_defs.Uincr | Ast_defs.Udecr | Ast_defs.Upincr | Ast_defs.Updecr
        | Ast_defs.Usilence ),
        e ) ->
    (* unary operations won't return function pointrs, so we discard the entity. *)
    let (env, _) = expr_ upcasted_id env e in
    (env, None)
  | A.Binop (Ast_defs.Eq None, e1, e2) ->
    let (env, entity_rhs) = expr_ upcasted_id env e2 in
    let env = assign env e1 entity_rhs in
    (env, None)
  | A.Binop (Ast_defs.QuestionQuestion, e1, e2) ->
    let (env, entity1) = expr_ upcasted_id env e1 in
    let (env, entity2) = expr_ upcasted_id env e2 in
    join env entity1 entity2
  | A.Binop (Ast_defs.Eq (Some Ast_defs.QuestionQuestion), e1, e2) ->
    let (env, entity1) = expr_ upcasted_id env e1 in
    let (env, entity2) = expr_ upcasted_id env e2 in
    let (env, entity_rhs) = join env entity1 entity2 in
    let env = assign env e1 entity_rhs in
    (env, None)
  | A.Binop
      ( ( Ast_defs.Plus | Ast_defs.Minus | Ast_defs.Star | Ast_defs.Slash
        | Ast_defs.Eqeq | Ast_defs.Eqeqeq | Ast_defs.Starstar | Ast_defs.Diff
        | Ast_defs.Diff2 | Ast_defs.Ampamp | Ast_defs.Barbar | Ast_defs.Lt
        | Ast_defs.Lte | Ast_defs.Gt | Ast_defs.Gte | Ast_defs.Dot
        | Ast_defs.Amp | Ast_defs.Bar | Ast_defs.Ltlt | Ast_defs.Gtgt
        | Ast_defs.Percent | Ast_defs.Xor | Ast_defs.Cmp ),
        e1,
        e2 ) ->
    (* most binary operations won't return function pointers, so we discard the entity. *)
    let (env, _) = expr_ upcasted_id env e1 in
    let (env, _) = expr_ upcasted_id env e2 in
    (env, None)
  | A.ValCollection (vc_kind, _, expr_list) ->
    begin
      match vc_kind with
      | A.Vec ->
        let var = Env.fresh_var () in
        let handle_init (env : env) (e_inner : T.expr) =
          let (env, entity_rhs) = expr_ upcasted_id env e_inner in
          match entity_rhs with
          | Some entity_rhs_ ->
            Env.add_constraint env (Subset (entity_rhs_, var))
          | _ -> env
        in
        let env = List.fold ~init:env ~f:handle_init expr_list in
        (env, Some var)
      | _ -> failwithpos pos ("Unsupported expression: " ^ Utils.expr_name e)
    end
  | A.Array_get (((_, _, A.Lvar (_, _lid)) as base), Some ix) ->
    let (env, entity_exp) = expr_ upcasted_id env base in
    let (env, _entity_ix) = expr_ upcasted_id env ix in
    (env, entity_exp)
  | A.Upcast (e, _) ->
    let (env, entity) = expr_ upcasted_id env e in
    let env =
      match entity with
      | Some entity -> Env.add_constraint env (Upcast (entity, pos))
      | None -> env
    in
    (env, None)
  | Aast.FunctionPointer (Aast.FP_id (_, id), _) ->
    if String.equal upcasted_id id then
      let entity_ = Literal pos in
      let env = Env.add_constraint env (Introduction pos) in
      (* Handle copy-on-write by creating a variable indirection *)
      let (env, var) = redirect env entity_ in
      (env, Some var)
    else
      (env, None)
  | A.Eif (cond, Some then_expr, else_expr) ->
    let (parent_env, _cond_entity) = expr_ upcasted_id env cond in
    let base_env = Env.reset_constraints parent_env in
    let (then_env, then_entity) = expr_ upcasted_id base_env then_expr in
    let (else_env, else_entity) = expr_ upcasted_id base_env else_expr in
    let env = Env.union parent_env then_env else_env in
    join env then_entity else_entity
  | A.Eif (cond, None, else_expr) ->
    let (env, cond_entity) = expr_ upcasted_id env cond in
    let (env, else_entity) = expr_ upcasted_id env else_expr in
    join env cond_entity else_entity
  | A.Await e -> expr_ upcasted_id env e
  | A.As (e, _ty, _) -> expr_ upcasted_id env e
  | A.Is (e, _ty) ->
    (* `is` expressions always evaluate to bools, so we discard the entity. *)
    let (env, _) = expr_ upcasted_id env e in
    (env, None)
  | _ -> failwithpos pos ("Unsupported expression: " ^ Utils.expr_name e)

let expr (upcasted_id : string) (env : env) (e : T.expr) : env =
  expr_ upcasted_id env e |> fst

let rec switch
    (upcasted_id : string)
    (parent_locals : lenv)
    (env : env)
    (cases : ('ex, 'en) A.case list)
    (dfl : ('ex, 'en) A.default_case option) : env =
  let initialize_next_cont env =
    let env = Env.restore_conts_from env ~from:parent_locals [Cont.Next] in
    let env = Env.update_next_from_conts env [Cont.Next; Cont.Fallthrough] in
    Env.drop_cont env Cont.Fallthrough
  in
  let handle_case env (e, b) =
    let env = initialize_next_cont env in
    let env = expr upcasted_id env e in
    block upcasted_id env b
  in
  let handle_default_case env dfl =
    dfl
    |> Option.fold ~init:env ~f:(fun env (_, b) ->
           let env = initialize_next_cont env in
           block upcasted_id env b)
  in
  let env = List.fold ~init:env ~f:handle_case cases in
  let env = handle_default_case env dfl in
  env

and stmt (upcasted_id : string) (env : env) ((pos, stmt) : T.stmt) : env =
  match stmt with
  | A.Expr e -> expr upcasted_id env e
  | A.If (cond, then_bl, else_bl) ->
    let parent_env = expr upcasted_id env cond in
    let base_env = Env.reset_constraints parent_env in
    let then_env = block upcasted_id base_env then_bl in
    let else_env = block upcasted_id base_env else_bl in
    Env.union parent_env then_env else_env
  | A.Switch (cond, cases, dfl) ->
    let env = expr upcasted_id env cond in
    (* NB: A 'continue' inside a 'switch' block is equivalent to a 'break'.
     * See the note in
     * http://php.net/manual/en/control-structures.continue.php *)
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let parent_locals = env.lenv in
    let env = switch upcasted_id parent_locals env cases dfl in
    Env.update_next_from_conts env [Cont.Continue; Cont.Break; Cont.Next]
  | A.Fallthrough -> Env.move_and_merge_next_in_cont env Cont.Fallthrough
  | A.Continue -> Env.move_and_merge_next_in_cont env Cont.Continue
  | A.Break -> Env.move_and_merge_next_in_cont env Cont.Break
  | A.While (cond, bl) ->
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let env = Env.save_and_merge_next_in_cont env Cont.Continue in
    let env_before_iteration = env in
    let env_after_iteration =
      let env = expr upcasted_id env cond in
      let env = block upcasted_id env bl in
      env
    in
    let env =
      Env.loop_continuation Cont.Next ~env_before_iteration ~env_after_iteration
    in
    let env = Env.update_next_from_conts env [Cont.Continue; Cont.Next] in
    let env = expr upcasted_id env cond in
    let env = Env.update_next_from_conts env [Cont.Break; Cont.Next] in
    env
  | A.Noop
  | A.AssertEnv _
  | A.Markup _ ->
    env
  | _ -> failwithpos pos ("Unsupported statement: " ^ Utils.stmt_name stmt)

and block (upcasted_id : string) (env : env) : T.block -> env =
  List.fold ~init:env ~f:(stmt upcasted_id)

let init_params _tast_env (params : T.fun_param list) :
    constraint_ list * entity LMap.t =
  let add_param (constraints, lmap) = function
    | _ -> (constraints, lmap)
  in
  List.fold ~f:add_param ~init:([], LMap.empty) params

let callable function_id tast_env params body : constraint_ list =
  let (param_constraints, param_env) = init_params tast_env params in
  let env = Env.init tast_env param_constraints param_env in
  let env = block function_id env body.A.fb_ast in
  env.constraints

let program
    (function_id : string) (ctx : Provider_context.t) (tast : Tast.program) :
    constraint_ list SMap.t =
  let def (def : T.def) : (string * constraint_ list) list =
    let tast_env = Tast_env.def_env ctx def in
    match def with
    | A.Fun fd ->
      let A.{ f_body; f_name = (_, id); f_params; _ } = fd.A.fd_fun in
      [(id, callable function_id tast_env f_params f_body)]
    | A.Class A.{ c_methods; c_name = (_, class_name); _ } ->
      let handle_method A.{ m_body; m_name = (_, method_name); m_params; _ } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable function_id tast_env m_params m_body)
      in
      List.map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list
