(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
open Shape_analysis_pretty_printer
module Cont = Typing_continuations
module A = Aast
module T = Tast
module SN = Naming_special_names
module Env = Shape_analysis_env
module Solver = Shape_analysis_solver
module Logic = Shape_analysis_logic

(* A program analysis to find shape like dicts and the static keys used in
   these dicts. *)

let failwithpos pos msg = failwith (Format.asprintf "%a: %s" Pos.pp pos msg)

let log_pos = Format.printf "%a\n" Pos.pp

let fully_expand_type env ty =
  let (_env, ty) =
    Typing_inference_env.fully_expand_type env.saved_env.Tast.inference_env ty
  in
  ty

type target_accumulator = {
  expressions_to_modify: Pos.t list;
  hints_to_modify: Pos.t list;
}

let collect_analysis_targets =
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

(* Is the type a suitable dict that can be coverted into shape. For the moment,
   that's only the case if the key is a string. TODO: support int and arraykey
   once we support constant keys. *)
let is_suitable_target_ty ty =
  match Typing_defs.get_node ty with
  | Typing_defs.Tclass ((_, id), _, [key_ty; _])
    when String.equal id SN.Collections.cDict ->
    (match Typing_defs.get_node key_ty with
    | Typing_defs.Tprim A.Tstring -> true
    | _ -> false)
  | _ -> false

let add_key_constraint
    (env : env) entity ((_, _, key) : T.expr) (ty : Typing_defs.locl_ty) : env =
  match entity with
  | Some entity ->
    let constraint_ =
      match key with
      | A.String str ->
        let ty = fully_expand_type env ty in
        Has_static_keys (entity, Logic.singleton (SK_string str) ty)
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
        let env = Env.add_constraint env (Exists (Extension, assignment_pos)) in
        let env = add_key_constraint env (Some current_assignment) ix ty_rhs in

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
    let add_key_constraint env (key, ((ty, _, _) as value)) : env =
      let (env, _key_entity) = expr env key in
      let (env, _val_entity) = expr env value in
      add_key_constraint env entity key ty
    in
    let env = List.fold ~init:env ~f:add_key_constraint key_value_pairs in

    (* Handle copy-on-write by creating a variable indirection *)
    let (env, var) = redirect env entity_ in
    (env, Some var)
  | A.Array_get (base, Some ix) ->
    let (env, entity_exp) = expr env base in
    let (env, _entity_ix) = expr env ix in
    let env = add_key_constraint env entity_exp ix ty in
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
      if is_suitable_target_ty ty then
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

let init_params (params : T.fun_param list) : constraint_ list * entity LMap.t =
  let add_param (constraints, lmap) = function
    | A.
        {
          param_pos;
          param_name;
          param_type_hint = (ty, _);
          param_is_variadic = false;
          _;
        } ->
      if is_suitable_target_ty ty then
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

let callable ~saved_env params body : constraint_ list =
  let (param_constraints, param_env) = init_params params in
  let env = Env.init saved_env param_constraints param_env in
  let env = block env body.A.fb_ast in
  env.constraints

let walk_tast (tast : Tast.program) : constraint_ list SMap.t =
  let def : T.def -> (string * constraint_ list) list = function
    | A.Fun fd ->
      let A.{ f_body; f_name = (_, id); f_annotation = saved_env; f_params; _ }
          =
        fd.A.fd_fun
      in
      [(id, callable ~saved_env f_params f_body)]
    | A.Class A.{ c_methods; c_name = (_, class_name); _ } ->
      let handle_method
          A.
            {
              m_body;
              m_name = (_, method_name);
              m_annotation = saved_env;
              m_params;
              _;
            } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable ~saved_env m_params m_body)
      in
      List.map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list

let analyse (options : options) (ctx : Provider_context.t) (tast : T.program) =
  let empty_typing_env =
    Typing_env_types.empty ~droot:None ctx Relative_path.default
  in
  match options.mode with
  | FlagTargets ->
    let { expressions_to_modify; hints_to_modify } =
      collect_analysis_targets#go ctx tast
    in
    Format.printf "~Expressions~\n";
    List.iter expressions_to_modify ~f:log_pos;
    Format.printf "~Hints~\n";
    List.iter hints_to_modify ~f:log_pos
  | DumpConstraints ->
    let print_function_constraints
        (id : string) (constraints : constraint_ list) : unit =
      Format.printf "Constraints for %s:\n" id;
      constraints
      |> List.map ~f:(show_constraint_ empty_typing_env)
      |> List.sort ~compare:String.compare
      |> List.iter ~f:(Format.printf "%s\n");
      Format.printf "\n"
    in
    walk_tast tast |> SMap.iter print_function_constraints
  | SimplifyConstraints ->
    let print_callable_summary (id : string) (results : shape_result list) :
        unit =
      Format.printf "Summary for %s:\n" id;
      List.iter results ~f:(fun result ->
          Format.printf "%s\n" (show_shape_result empty_typing_env result))
    in
    let process_callable id constraints =
      Solver.simplify empty_typing_env constraints |> print_callable_summary id
    in
    walk_tast tast |> SMap.iter process_callable
  | SolveConstraints -> ()
