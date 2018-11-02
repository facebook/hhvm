(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  ServerEnv.env ->
  (ServerCommandTypes.file_input * int * int) ->
  basic_only:bool ->
  Lsp.SignatureHelp.result
(** Returns signature help for the given location. *)
