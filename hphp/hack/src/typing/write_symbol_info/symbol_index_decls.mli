(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Generate facts for all declarations in AST, and
    a XRefs map for the module references *)
val process_decls :
  Provider_context.t ->
  Symbol_predicate.Fact_acc.t ->
  Symbol_file_info.t ->
  Symbol_xrefs.t * Symbol_predicate.Fact_acc.t
