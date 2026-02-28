(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val should_log : int -> Tany_logger_types.log_mask -> bool

val string_of_pos : Relative_path.t Pos.pos -> string

val string_of_context_id : Tany_logger_types.context_id -> string
