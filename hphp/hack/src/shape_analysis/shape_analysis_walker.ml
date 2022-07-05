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
module Utils = Shape_analysis_utils

let join env left right =
  let join = Env.fresh_var () in
  let constraint_ = Join { left; right; join } in
  let env = Env.add_constraint env @@ constraint_ in
  (env, join)

let when_tast_check tast_env ~default f =
  let tcopt = Tast_env.get_tcopt tast_env |> TypecheckerOptions.log_levels in
  match SMap.find_opt "shape_analysis" tcopt with
  | Some level when level > 0 -> f ()
  | _ -> default

let failwithpos pos msg =
  raise @@ Shape_analysis_exn (Format.asprintf "%a: %s" Pos.pp pos msg)

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
let rec is_suitable_target_ty tast_env ty =
  let ty = Tast_env.fully_expand tast_env ty in
  match Typing_defs.get_node ty with
  | Typing_defs.Tclass ((_, id), _, [ty])
    when String.equal id SN.Classes.cAwaitable ->
    is_suitable_target_ty tast_env ty
  | Typing_defs.Tclass ((_, id), _, [key_ty; _])
    when String.equal id SN.Collections.cDict ->
    Tast_env.can_subtype
      tast_env
      key_ty
      (Typing_make_type.arraykey Typing_reason.Rnone)
    || Typing_utils.is_nothing (Tast_env.tast_env_as_typing_env tast_env) key_ty
  | _ -> false

let add_key_constraint
    (env : env) entity (((_, _, key), ty) : T.expr * Typing_defs.locl_ty) : env
    =
  match entity with
  | Some entity ->
    let constraint_ =
      match key with
      | A.String str ->
        let ty = Tast_env.fully_expand env.tast_env ty in
        Has_static_key (entity, SK_string str, ty)
      | _ -> Has_dynamic_key entity
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
          add_key_constraint env (Some current_assignment) (ix, ty_rhs)
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
  | _ -> failwithpos pos ("Unsupported lvalue: " ^ Utils.expr_name lval)

and expr_ (env : env) ((ty, pos, e) : T.expr) : env * entity =
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
    let add_key_constraint env (key, ((ty, _, _) as value)) : env =
      let (env, _key_entity) = expr_ env key in
      let (env, _val_entity) = expr_ env value in
      let env = add_key_constraint env entity (key, ty) in
      env
    in
    let env = List.fold ~init:env ~f:add_key_constraint key_value_pairs in

    (* Handle copy-on-write by creating a variable indirection *)
    let (env, var) = redirect env entity_ in
    (env, Some var)
  | A.Array_get (base, Some ix) ->
    let (env, entity_exp) = expr_ env base in
    let (env, _entity_ix) = expr_ env ix in
    let env = add_key_constraint env entity_exp (ix, ty) in
    (env, None)
  | A.Lvar (_, lid) ->
    let entity = Env.get_local env lid in
    (env, entity)
  | A.Binop (Ast_defs.Eq None, e1, ((ty_rhs, _, _) as e2)) ->
    let (env, entity_rhs) = expr_ env e2 in
    let env = assign pos env e1 entity_rhs ty_rhs in
    (env, None)
  | A.Call (_base, _targs, args, _unpacked) ->
    (* TODO: This is obviously incomplete. It just adds a constraint to each
       dict argument so that we know what shape type that reaches to the
       given position is. *)
    let expr_arg env (_param_kind, ((_ty, pos, _exp) as arg)) =
      let (env, arg_entity) = expr_ env arg in
      match arg_entity with
      | Some arg_entity_ ->
        let env = Env.add_constraint env (Exists (Argument, pos)) in
        let new_entity_ = Literal pos in
        let env =
          when_tast_check env.tast_env ~default:env @@ fun () ->
          Env.add_constraint env @@ Has_dynamic_key new_entity_
        in
        let env = Env.add_constraint env (Subset (arg_entity_, new_entity_)) in
        env
      | None -> env
    in
    let env = List.fold ~f:expr_arg ~init:env args in
    (env, None)
  | A.Eif (cond, Some then_expr, else_expr) ->
    let (parent_env, _cond_entity) = expr_ env cond in
    let base_env = Env.reset_constraints parent_env in
    let (then_env, then_entity) = expr_ base_env then_expr in
    let (else_env, else_entity) = expr_ base_env else_expr in
    let env = Env.union parent_env then_env else_env in
    (* Create a join point entity. It is pretty much Option.marge except that
       that function doesn't allow threading state (`env`) through *)
    let (env, entity) =
      match (then_entity, else_entity) with
      | (Some then_entity_, Some else_entity_) ->
        let (env, join) = join env then_entity_ else_entity_ in
        (env, Some join)
      | (None, Some _) -> (env, else_entity)
      | (_, _) -> (env, then_entity)
    in
    (env, entity)
  | A.Eif (cond, None, else_expr) ->
    let (env, cond_entity) = expr_ env cond in
    let (env, else_entity) = expr_ env else_expr in
    (* Create a join point entity. It is pretty much Option.marge except that
       that function doesn't allow threading state (`env`) through *)
    let (env, entity) =
      match (cond_entity, else_entity) with
      | (Some then_entity_, Some else_entity_) ->
        let (env, join) = join env then_entity_ else_entity_ in
        (env, Some join)
      | (None, Some _) -> (env, else_entity)
      | (_, _) -> (env, cond_entity)
    in
    (env, entity)
  | A.Await e -> expr_ env e
  | A.As (e, _ty, _) -> expr_ env e
  | A.Is (e, _ty) ->
    (* `is` expressions always evaluate to bools, so we discard the entity. *)
    let (env, _) = expr_ env e in
    (env, None)
  | _ -> failwithpos pos ("Unsupported expression: " ^ Utils.expr_name e)

let expr (env : env) (e : T.expr) : env = expr_ env e |> fst

let rec switch
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
    let env = expr env e in
    block env b
  in
  let handle_default_case env dfl =
    dfl
    |> Option.fold ~init:env ~f:(fun env (_, b) ->
           let env = initialize_next_cont env in
           block env b)
  in
  let env = List.fold ~init:env ~f:handle_case cases in
  let env = handle_default_case env dfl in
  env

and stmt (env : env) ((pos, stmt) : T.stmt) : env =
  match stmt with
  | A.Expr e -> expr env e
  | A.Return (Some e) ->
    let (env, entity) = expr_ env e in
    let add_dynamic_if_tast_check entity_ =
      when_tast_check env.tast_env ~default:env @@ fun () ->
      Env.add_constraint env @@ Has_dynamic_key entity_
    in
    Option.value_map entity ~default:env ~f:add_dynamic_if_tast_check
  | A.If (cond, then_bl, else_bl) ->
    let parent_env = expr env cond in
    let base_env = Env.reset_constraints parent_env in
    let then_env = block base_env then_bl in
    let else_env = block base_env else_bl in
    Env.union parent_env then_env else_env
  | A.Switch (cond, cases, dfl) ->
    let env = expr env cond in
    (* NB: A 'continue' inside a 'switch' block is equivalent to a 'break'.
     * See the note in
     * http://php.net/manual/en/control-structures.continue.php *)
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let parent_locals = env.lenv in
    let env = switch parent_locals env cases dfl in
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
  | A.Block statements -> block env statements
  | A.Noop
  | A.AssertEnv _
  | A.Markup _ ->
    env
  | _ -> failwithpos pos ("Unsupported statement: " ^ Utils.stmt_name stmt)

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
        let constraints =
          when_tast_check tast_env ~default:constraints @@ fun () ->
          Has_dynamic_key entity_ :: constraints
        in
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
