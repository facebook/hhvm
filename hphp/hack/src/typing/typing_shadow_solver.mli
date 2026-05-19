(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type solution = {
  local_name: string;
  local_pos: Pos.t;
  shadow_tvid: Tvid.t;
  inferred_ty: Typing_defs.locl_ty option;
}

val solve :
  as_data:bool ->
  Typing_env_types.env ->
  Typing_inference_env.t ->
  Tast.dynamic_local_info list ->
  solution list

val solution_to_json : Tast_env.env -> solution -> Yojson.Safe.t
