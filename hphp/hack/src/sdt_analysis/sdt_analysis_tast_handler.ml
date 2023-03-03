(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Sdt_analysis_types
module IntraWalker = Sdt_analysis_intra_walker
module InterWalker = Sdt_analysis_inter_walker

let should_visit = Fn.non Tast_env.is_hhi

let strip_constraints { decorated_data; _ } = decorated_data

let create_handler ctx writer =
  let write_ids_and_inters id (cs : H.inter_constraint_ decorated list) =
    H.Write.add_id writer id;
    cs
    |> List.map ~f:strip_constraints
    |> List.iter ~f:(H.Write.add_inter writer id)
  in

  let write_intras id decorated_intras =
    decorated_intras
    |> List.map ~f:strip_constraints
    |> List.iter ~f:(H.Write.add_intra writer id)
  in

  let at_defs defs =
    IdMap.iter write_ids_and_inters @@ InterWalker.program ctx defs;
    IdMap.iter write_intras @@ IntraWalker.program ctx defs
  in

  object
    inherit Tast_visitor.handler_base

    method! at_class_ env def =
      if should_visit env then at_defs [Aast_defs.Class def]

    method! at_fun_def env def =
      if should_visit env then at_defs [Aast_defs.Fun def]
  end
