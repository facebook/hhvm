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

let has_key_constraint entity ((_, _, key) : T.expr) (ty : Typing_defs.locl_ty)
    : constraint_ list =
  match entity with
  | Some entity ->
    begin
      match key with
      | A.String _ -> [Has_static_key (entity, key, ty)]
      | _ -> [Has_dynamic_key entity]
    end
  | None -> []

let rec expr ((ty, pos, e) : T.expr) : entity * constraint_ list =
  match e with
  | A.Int _
  | A.Float _
  | A.String _
  | A.True
  | A.False ->
    (None, [])
  | A.Darray (_, key_value_pairs)
  | A.KeyValCollection (A.Dict, _, key_value_pairs) ->
    let entity_ = Literal pos in
    let entity = Some entity_ in
    let has_key_constraint (key, ((ty, _, _) as value)) =
      let (_key_entity, key_constraints) = expr key in
      let (_val_entity, val_constraints) = expr value in
      has_key_constraint entity key ty @ key_constraints @ val_constraints
    in
    let constraints =
      Exists entity_ :: List.concat_map key_value_pairs ~f:has_key_constraint
    in
    (entity, constraints)
  | A.Array_get (exp, Some ix) ->
    let (entity_exp, constraints_exp) = expr exp in
    let (_entity_ix, constraints_ix) = expr ix in
    let new_constraints = has_key_constraint entity_exp ix ty in
    let constraints = new_constraints @ constraints_exp @ constraints_ix in
    (None, constraints)
  | _ -> failwith "An expression is not yet handled"

let expr (e : T.expr) : constraint_ list = expr e |> snd

let stmt ((_, stmt) : T.stmt) : constraint_ list =
  match stmt with
  | A.Expr e -> expr e
  | A.Return (Some e) -> expr e
  | _ -> failwith "An expression is not yet handled"

let block : T.block -> constraint_ list = List.concat_map ~f:stmt

let callable body : constraint_ list = block body.A.fb_ast

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
    let constraints = walk_tast tast in
    List.iter
      ~f:(fun c -> Format.printf "%s\n" (show_constraint_ empty_typing_env c))
      constraints
  | _ -> ()
