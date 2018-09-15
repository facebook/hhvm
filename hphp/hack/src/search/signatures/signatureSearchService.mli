(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val add_function : TypecheckerOptions.t -> string -> unit
(** Given a function name, look up that function's signature on the typing heap
    and mutate the signature search index to include this function. *)

val build : TypecheckerOptions.t -> FileInfo.t Relative_path.Map.t -> unit
(** Given a Map of file info, retrieve each file's function name and update the
    index *)

val go :
  ParserOptions.t ->
  SignatureSearchParser.signature_query ->
  (Ast.pos, HackSearchService.search_result_type) SearchUtils.term list
(** Performs a query on a global index for matching functions.
    Returns an object for use in Nuclide and hh_search *)
