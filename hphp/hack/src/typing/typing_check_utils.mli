(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [type_file ctx fn ast] works as follows:
1. uses [ast] to obtain a list of all classes, funs, ...
2. uses [fn] for the error context
3. confusingly, looks up the def of the file using [Ast_provider.get_ast fn], not [ast]. *)
val type_file :
  Provider_context.t ->
  Relative_path.t ->
  Nast.program ->
  Tast.def list * Errors.t
