(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
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

let analyse (options : options) (ctx : Provider_context.t) (tast : T.program) =
  match options.mode with
  | FlagTargets ->
    let { expressions_to_modify; hints_to_modify } =
      collect_analysis_targets#go ctx tast
    in
    Format.printf "~Expressions~\n";
    List.iter expressions_to_modify ~f:log_pos;
    Format.printf "~Hints~\n";
    List.iter hints_to_modify ~f:log_pos
  | _ -> ()
