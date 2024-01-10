(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type collected_type = Tast_env.env * Typing_defs.phase_ty

val collect_types :
  Provider_context.t ->
  Tast.def list ->
  collected_type list Pos.AbsolutePosMap.t

val get_from_pos_map :
  Pos.absolute ->
  collected_type list Pos.AbsolutePosMap.t ->
  collected_type list option
