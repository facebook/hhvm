(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val process_xrefs_and_calls :
  Provider_context.t ->
  Predicate.Fact_acc.t ->
  File_info.t ->
  Predicate.Fact_acc.t * Xrefs.t
