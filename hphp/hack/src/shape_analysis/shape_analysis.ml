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

(* A program analysis to find shape like dicts and the static keys used in
   these dicts. *)

let log_pos = Format.printf "%a\n" Pos.pp

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
      | A.String _ -> Has_static_key (entity, key, ty)
      | _ -> Has_dynamic_key entity
    in
    Env.add_constraint env constraint_
  | None -> env

let assign (env : env) ((_, _, lval) : T.expr) (rhs : entity) : env =
  match lval with
  | A.Lvar (_, lid) -> Env.set_local lid rhs env
  | _ -> failwith "An lvalue is not yet supported"

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
  | _ -> failwith "An expression is not yet handled"

let expr (env : env) (e : T.expr) : env = expr env e |> fst

let stmt (env : env) ((_, stmt) : T.stmt) : env =
  match stmt with
  | A.Expr e
  | A.Return (Some e) ->
    expr env e
  | _ -> failwith "An expression is not yet handled"

let block (env : env) : T.block -> env = List.fold ~init:env ~f:stmt

let callable body : constraint_ list =
  let env = Env.init in
  let env = block env body.A.fb_ast in
  env.constraints

let walk_tast : Tast.program -> constraint_ list =
  let def : T.def -> constraint_ list = function
    | A.Fun fd ->
      let A.{ f_body; _ } = fd.A.fd_fun in
      callable f_body
    | A.Class A.{ c_methods; _ } ->
      let handle_method A.{ m_body; _ } = callable m_body in
      List.concat_map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def

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
    walk_tast tast
    |> List.map ~f:(show_constraint_ empty_typing_env)
    |> List.sort ~compare:String.compare
    |> List.iter ~f:(Format.printf "%s\n")
  | _ -> ()
