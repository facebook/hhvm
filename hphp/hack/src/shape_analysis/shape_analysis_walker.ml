(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module Cont = Typing_continuations
module A = Aast
module T = Tast
module SN = Naming_special_names
module Env = Shape_analysis_env
module Logic = Shape_analysis_logic

let failwithpos pos msg = failwith (Format.asprintf "%a: %s" Pos.pp pos msg)

let collect_analysis_targets :
    Provider_context.t -> Tast.program -> potential_targets =
  let reducer =
    object (this)
      inherit [_] Tast_visitor.reduce as super

      method zero = { expressions_to_modify = []; hints_to_modify = [] }

      method plus
          { expressions_to_modify = es1; hints_to_modify = hs1 }
          { expressions_to_modify = es2; hints_to_modify = hs2 } =
        { expressions_to_modify = es1 @ es2; hints_to_modify = hs1 @ hs2 }

      method! on_expr env ((_, pos, exp_proper) as exp) =
        let expressions_to_modify =
          match exp_proper with
          | A.Darray _ -> [pos]
          | A.KeyValCollection (A.Dict, _, _) -> [pos]
          | _ -> []
        in
        let accumulator = { expressions_to_modify; hints_to_modify = [] } in
        this#plus accumulator (super#on_expr env exp)

      method! on_hint env ((pos, hint_proper) as hint) =
        let hints_to_modify =
          match hint_proper with
          | A.Happly ((_, id), _) when String.equal id SN.Collections.cDict ->
            [pos]
          | _ -> []
        in
        let accumulator = { expressions_to_modify = []; hints_to_modify } in
        this#plus accumulator (super#on_hint env hint)
    end
  in
  reducer#go

(* Is the type a suitable dict that can be coverted into shape. For the moment,
   that's only the case if the key is a string. *)
let is_suitable_target_ty tast_env ty =
  let ty = Tast_env.fully_expand tast_env ty in
  match Typing_defs.get_node ty with
  | Typing_defs.Tclass ((_, id), _, [key_ty; _])
    when String.equal id SN.Collections.cDict ->
    Tast_env.can_subtype
      tast_env
      key_ty
      (Typing_make_type.arraykey Typing_reason.Rnone)
    || Typing_utils.is_nothing (Tast_env.tast_env_as_typing_env tast_env) key_ty
  | _ -> false

type proto_constraint =
  | Static_keys of Typing_defs.locl_ty ShapeKeyMap.t
  | Dynamic_key

let add_key_constraints
    (result_id : ResultID.t)
    (env : env)
    entity
    (keys_and_tys : (T.expr * Typing_defs.locl_ty) list) : env =
  let prep_key_and_ty acc ((_, _, key), ty) =
    match (key, acc) with
    | (A.String str, Static_keys static_keys) ->
      let ty = Tast_env.fully_expand env.tast_env ty in
      Static_keys (ShapeKeyMap.add (SK_string str) ty static_keys)
    | _ -> Dynamic_key
  in
  match entity with
  | Some entity ->
    let proto_constraint =
      List.fold
        ~f:prep_key_and_ty
        ~init:(Static_keys ShapeKeyMap.empty)
        keys_and_tys
    in
    let constraint_ =
      match proto_constraint with
      | Static_keys shape_keys ->
        Has_static_keys (entity, (result_id, shape_keys))
      | Dynamic_key -> Has_dynamic_key entity
    in
    Env.add_constraint env constraint_
  | None -> env

let redirect (env : env) (entity_ : entity_) : env * entity_ =
  let var = Env.fresh_var () in
  let env = Env.add_constraint env (Subset (entity_, var)) in
  (env, var)

let rec assign
    (assignment_pos : Pos.t)
    (env : env)
    ((_, pos, lval) : T.expr)
    (rhs : entity)
    (ty_rhs : Typing_defs.locl_ty) : env =
  match lval with
  | A.Lvar (_, lid) -> Env.set_local env lid rhs
  | A.Array_get ((_, _, A.Lvar (_, lid)), Some ix) ->
    let entity = Env.get_local env lid in
    begin
      match entity with
      | Some entity_ ->
        let current_assignment = Literal assignment_pos in
        let env =
          Env.add_constraint env (Subset (entity_, current_assignment))
        in
        let env =
          add_key_constraints
            ResultID.empty
            env
            (Some current_assignment)
            [(ix, ty_rhs)]
        in

        (* Handle copy-on-write by creating a variable indirection *)
        let (env, var) = redirect env current_assignment in
        Env.set_local env lid (Some var)
      | None ->
        (* We might end up here as a result of deadcode, such as a dictionary
           assignment after an unconditional break in a loop. In this
           situation, it is not meaningful to report a candidate. *)
        env
    end
  | _ -> failwithpos pos "An lvalue is not yet supported"

and expr (env : env) ((ty, pos, e) : T.expr) : env * entity =
  match e with
  | A.Int _
  | A.Float _
  | A.String _
  | A.True
  | A.False ->
    (env, None)
  | A.Darray (_, key_value_pairs)
  | A.KeyValCollection (A.Dict, _, key_value_pairs) ->
    let entity_ = Literal pos in
    let entity = Some entity_ in
    let env = Env.add_constraint env (Exists (Allocation, pos)) in
    let result_id = Logic.fresh_result_id () in
    let collect_key_constraint (env, keys_and_tys) (key, ((ty, _, _) as value))
        : env * (T.expr * Typing_defs.locl_ty) list =
      let (env, _key_entity) = expr env key in
      let (env, _val_entity) = expr env value in
      (env, (key, ty) :: keys_and_tys)
    in
    let (env, keys_and_tys) =
      List.fold ~init:(env, []) ~f:collect_key_constraint key_value_pairs
    in
    let env = add_key_constraints result_id env entity keys_and_tys in

    (* Handle copy-on-write by creating a variable indirection *)
    let (env, var) = redirect env entity_ in
    (env, Some var)
  | A.Array_get (base, Some ix) ->
    let (env, entity_exp) = expr env base in
    let (env, _entity_ix) = expr env ix in
    let env = add_key_constraints ResultID.empty env entity_exp [(ix, ty)] in
    (env, None)
  | A.Lvar (_, lid) ->
    let entity = Env.get_local env lid in
    (env, entity)
  | A.Binop (Ast_defs.Eq None, e1, ((ty_rhs, _, _) as e2)) ->
    let (env, entity_rhs) = expr env e2 in
    let env = assign pos env e1 entity_rhs ty_rhs in
    (env, None)
  | A.Call (_base, _targs, args, _unpacked) ->
    (* TODO: This is obviously incomplete. It just adds a constraint to each
       dict argument so that we know what shape type that reaches to the
       given position is. *)
    let expr_arg env (_param_kind, ((ty, pos, _exp) as arg)) =
      let (env, arg_entity) = expr env arg in
      if is_suitable_target_ty env.tast_env ty then
        let env = Env.add_constraint env (Exists (Argument, pos)) in
        match arg_entity with
        | Some arg_entity_ ->
          let new_entity_ = Literal pos in
          Env.add_constraint env (Subset (arg_entity_, new_entity_))
        | None -> env
      else
        env
    in
    let env = List.fold ~f:expr_arg ~init:env args in
    (env, None)
  | _ -> failwithpos pos "An expression is not yet handled"

let expr (env : env) (e : T.expr) : env = expr env e |> fst

let rec case_list
    (parent_locals : lenv) (env : env) (cases : ('ex, 'en) A.case list) : env =
  let initialize_next_cont env =
    let env = Env.restore_conts_from env ~from:parent_locals [Cont.Next] in
    let env = Env.update_next_from_conts env [Cont.Next; Cont.Fallthrough] in
    Env.drop_cont env Cont.Fallthrough
  in
  let handle_case env = function
    | A.Default (_, b) ->
      let env = initialize_next_cont env in
      block env b
    | A.Case (e, b) ->
      let env = initialize_next_cont env in
      let env = expr env e in
      block env b
  in
  List.fold ~init:env ~f:handle_case cases

and stmt (env : env) ((pos, stmt) : T.stmt) : env =
  match stmt with
  | A.Expr e
  | A.Return (Some e) ->
    expr env e
  | A.If (cond, then_bl, else_bl) ->
    let parent_env = expr env cond in
    let base_env = Env.reset_constraints parent_env in
    let then_env = block base_env then_bl in
    let else_env = block base_env else_bl in
    Env.union parent_env then_env else_env
  | A.Switch (cond, cases) ->
    let env = expr env cond in
    (* NB: A 'continue' inside a 'switch' block is equivalent to a 'break'.
     * See the note in
     * http://php.net/manual/en/control-structures.continue.php *)
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let parent_locals = env.lenv in
    let env = case_list parent_locals env cases in
    Env.update_next_from_conts env [Cont.Continue; Cont.Break; Cont.Next]
  | A.Fallthrough -> Env.move_and_merge_next_in_cont env Cont.Fallthrough
  | A.Continue -> Env.move_and_merge_next_in_cont env Cont.Continue
  | A.Break -> Env.move_and_merge_next_in_cont env Cont.Break
  | A.While (cond, bl) ->
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let env = Env.save_and_merge_next_in_cont env Cont.Continue in
    let env_before_iteration = env in
    let env_after_iteration =
      let env = expr env cond in
      let env = block env bl in
      env
    in
    let env =
      Env.loop_continuation Cont.Next ~env_before_iteration ~env_after_iteration
    in
    let env = Env.update_next_from_conts env [Cont.Continue; Cont.Next] in
    let env = expr env cond in
    let env = Env.update_next_from_conts env [Cont.Break; Cont.Next] in
    env
  | A.Noop
  | A.AssertEnv _
  | A.Markup _ ->
    env
  | _ -> failwithpos pos "A statement is not yet handled"

and block (env : env) : T.block -> env = List.fold ~init:env ~f:stmt

let init_params tast_env (params : T.fun_param list) :
    constraint_ list * entity LMap.t =
  let add_param (constraints, lmap) = function
    | A.
        {
          param_pos;
          param_name;
          param_type_hint = (ty, _);
          param_is_variadic = false;
          _;
        } ->
      if is_suitable_target_ty tast_env ty then
        let param_lid = Local_id.make_unscoped param_name in
        let entity_ = Literal param_pos in
        let lmap = LMap.add param_lid (Some entity_) lmap in
        let constraints = Exists (Parameter, param_pos) :: constraints in
        (constraints, lmap)
      else
        (constraints, lmap)
    | _ -> (constraints, lmap)
  in
  List.fold ~f:add_param ~init:([], LMap.empty) params

let callable tast_env params body : constraint_ list =
  let (param_constraints, param_env) = init_params tast_env params in
  let env = Env.init tast_env param_constraints param_env in
  let env = block env body.A.fb_ast in
  env.constraints

let program (ctx : Provider_context.t) (tast : Tast.program) :
    constraint_ list SMap.t =
  let def (def : T.def) : (string * constraint_ list) list =
    let tast_env = Tast_env.def_env ctx def in
    match def with
    | A.Fun fd ->
      let A.{ f_body; f_name = (_, id); f_params; _ } = fd.A.fd_fun in
      [(id, callable tast_env f_params f_body)]
    | A.Class A.{ c_methods; c_name = (_, class_name); _ } ->
      let handle_method A.{ m_body; m_name = (_, method_name); m_params; _ } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable tast_env m_params m_body)
      in
      List.map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list
