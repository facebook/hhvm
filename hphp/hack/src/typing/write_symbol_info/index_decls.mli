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
  Predicate.Fact_acc.t -> File_info.t -> Xrefs.t * Predicate.Fact_acc.t
