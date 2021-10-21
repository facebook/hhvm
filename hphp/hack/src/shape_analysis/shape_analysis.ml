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
module A = Aast
module T = Tast
module SN = Naming_special_names
module Env = Shape_analysis_env
module Solver = Shape_analysis_solver

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

let add_key_constraint
    (env : env) entity ((_, _, key) : T.expr) (ty : Typing_defs.locl_ty) : env =
  match entity with
  | Some entity ->
    let constraint_ =
      match key with
      | A.String str ->
        let ty = fully_expand_type env ty in
        Has_static_key (entity, SK_string str, ty)
      | _ -> Has_dynamic_key entity
    in
    Env.add_constraint env constraint_
  | None -> env

let assign (env : env) ((_, pos, lval) : T.expr) (rhs : entity) : env =
  match lval with
  | A.Lvar (_, lid) -> Env.set_local lid rhs env
  | _ -> failwithpos pos "An lvalue is not yet supported"

let rec expr (env : env) ((ty, pos, e) : T.expr) : env * entity =
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
    let env = Env.add_constraint env (Exists entity_) in
    let add_key_constraint env (key, ((ty, _, _) as value)) : env =
      let (env, _key_entity) = expr env key in
      let (env, _val_entity) = expr env value in
      add_key_constraint env entity key ty
    in
    let env = List.fold ~init:env ~f:add_key_constraint key_value_pairs in
    (env, entity)
  | A.Array_get (exp, Some ix) ->
    let (env, entity_exp) = expr env exp in
    let (env, _entity_ix) = expr env ix in
    let env = add_key_constraint env entity_exp ix ty in
    (env, None)
  | A.Lvar (_, lid) ->
    let entity = Env.get_local lid env in
    (env, entity)
  | A.Binop (Ast_defs.Eq None, e1, e2) ->
    let (env, entity_rhs) = expr env e2 in
    let env = assign env e1 entity_rhs in
    (env, None)
  | _ -> failwithpos pos "An expression is not yet handled"

let expr (env : env) (e : T.expr) : env = expr env e |> fst

let stmt (env : env) ((pos, stmt) : T.stmt) : env =
  match stmt with
  | A.Expr e
  | A.Return (Some e) ->
    expr env e
  | _ -> failwithpos pos "A statement is not yet handled"

let block (env : env) : T.block -> env = List.fold ~init:env ~f:stmt

let callable ~saved_env body : constraint_ list =
  let env = Env.init saved_env in
  let env = block env body.A.fb_ast in
  env.constraints

let walk_tast (tast : Tast.program) : constraint_ list SMap.t =
  let def : T.def -> (string * constraint_ list) list = function
    | A.Fun fd ->
      let A.{ f_body; f_name = (_, id); f_annotation = saved_env; _ } =
        fd.A.fd_fun
      in
      [(id, callable ~saved_env f_body)]
    | A.Class A.{ c_methods; c_name = (_, class_name); _ } ->
      let handle_method
          A.{ m_body; m_name = (_, method_name); m_annotation = saved_env; _ } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable ~saved_env m_body)
      in
      List.map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list

let analyse (options : options) (ctx : Provider_context.t) (tast : T.program) =
  let empty_typing_env =
    Typing_env.empty ~droot:None ctx Relative_path.default
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
