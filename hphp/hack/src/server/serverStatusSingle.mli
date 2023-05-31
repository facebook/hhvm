(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  ServerCommandTypes.file_input list ->
  Provider_context.t ->
  Errors.finalized_error list * Tast.program Relative_path.Map.t
