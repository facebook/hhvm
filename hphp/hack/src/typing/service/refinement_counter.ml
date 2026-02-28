(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* silence "unused open! Ppx_yojson_conv_lib.Yojson_conv.Primitives" *)
[@@@warning "-66"]

open Hh_prelude
open Ppx_yojson_conv_lib.Yojson_conv.Primitives

type summary = {
  count: int;  (** Number of occurrences of a legacy refinement kind *)
  positions: SSet.t;
      (** The set of positions (stored as a string), where the refinement occurs.
          This field value is depended on the log level:
            refinement_counter = 3: Include all positions
            refinement_counter = 2: Include all positions, except for class refinements
            otherwise: Do not include positions (will be SSet.empty) *)
}
[@@deriving yojson_of]

type t = {
  legacy_refinements: summary SMap.t;
  new_refinements: int;
  total_refinements: int;
}
[@@deriving yojson_of]

module Summary = struct
  let zero =
    {
      legacy_refinements = SMap.empty;
      new_refinements = 0;
      total_refinements = 0;
    }

  let plus
      {
        legacy_refinements = legacy1;
        new_refinements = new1;
        total_refinements = total1;
      }
      {
        legacy_refinements = legacy2;
        new_refinements = new2;
        total_refinements = total2;
      } =
    {
      legacy_refinements =
        SMap.merge
          (fun _ c_opt c_opt' ->
            match (c_opt, c_opt') with
            | (Some c, Some c') ->
              Some
                {
                  count = c.count + c'.count;
                  positions = SSet.union c.positions c'.positions;
                }
            | (None, Some c)
            | (Some c, None) ->
              Some c
            | (None, None) -> None)
          legacy1
          legacy2;
      new_refinements = new1 + new2;
      total_refinements = total1 + total2;
    }

  let legacy_refinement err pos_opt =
    {
      legacy_refinements =
        SMap.singleton
          err
          {
            count = 1;
            positions =
              (match pos_opt with
              | None -> SSet.empty
              | Some pos ->
                SSet.singleton Pos.(to_relative_string pos |> string));
          };
      new_refinements = 0;
      total_refinements = 1;
    }

  let new_refinement =
    {
      legacy_refinements = SMap.empty;
      new_refinements = 1;
      total_refinements = 1;
    }
end

let count ctx program =
  let reducer =
    object (self)
      inherit [t] Tast_visitor.reduce as super

      method zero = Summary.zero

      method plus = Summary.plus

      method! on_Is env e hint =
        let result =
          match Tast_env.supports_new_refinement env hint with
          | Result.Ok () -> Summary.new_refinement
          | Result.Error err ->
            let pos_opt =
              let log_level =
                TypecheckerOptions.log_levels (Tast_env.get_tcopt env)
                |> SMap.find_opt "refinement_counter"
              in
              match log_level with
              | Some 3 -> Some (fst hint)
              | Some 2 when not @@ String.is_substring err ~substring:"class" ->
                Some (fst hint)
              | _ -> None
            in
            Summary.legacy_refinement err pos_opt
        in
        self#plus (super#on_Is env e hint) result
    end
  in
  reducer#go ctx program

let is_enabled tcopt =
  TypecheckerOptions.log_levels tcopt
  |> SMap.find_opt "refinement_counter"
  |> Option.map ~f:(fun level -> level >= 1)
  |> Option.value ~default:false

let map
    (ctx : Provider_context.t)
    (_path : Relative_path.t)
    (tasts : Tast.by_names)
    _errors : t =
  Tast.tasts_as_list tasts
  |> List.map ~f:(fun t -> t.Tast_with_dynamic.under_normal_assumptions)
  |> count ctx

let reduce = Summary.plus

let finalize ~progress:_ ~init_id:_ ~recheck_id:_ _counts = ()
